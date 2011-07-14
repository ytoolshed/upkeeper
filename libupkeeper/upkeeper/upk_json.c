#include "upk_json.h"
#include <stdarg.h>


typedef enum {
    data_type_stream,
    data_type_string,
} upk_json_output_type_t;

typedef union _upk_stream_string {
    FILE                   *file;
    struct {
        char                   *p;
        size_t                  len;
    } str;
} upk_stream_string_t;


typedef struct _upk_json_output_data {
    upk_json_output_type_t  type;
    upk_stream_string_t     d;
    upk_json_data_output_opts_t opts;
} upk_json_output_data_t;


static inline void      upk_json_print_indent(upk_json_stack_meta_t * meta, upk_json_output_data_t * data);
static void             upk_json_output_null_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                     upk_json_val_t v);
static void             upk_json_output_bool_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                     upk_json_val_t v);
static void             upk_json_output_double_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                       upk_json_val_t v);
static void             upk_json_output_int_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                    upk_json_val_t v);
static void             upk_json_output_string_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                       upk_json_val_t v);
static void             upk_json_output_array_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                      upk_json_val_t v);
static void             upk_json_output_object_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                       upk_json_val_t v);
static void             upk_json_output_after_json_obj_pop_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                                   upk_json_val_t v);
static void             upk_json_output_after_json_array_pop_handler(upk_json_stack_meta_t * meta, void *data,
                                                                     char *key, upk_json_val_t v);

static inline void      upk_json_value(upk_json_stack_meta_t * meta, char *key, struct json_object *jobj);
static inline void      upk_json_parse_array(upk_json_stack_meta_t * meta, char *key, struct json_object *jarray);
static inline void      _upk_json_parse_node(upk_json_stack_meta_t * meta, char *key, struct json_object *obj);


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_append_string(upk_json_output_data_t * data, char *string)
{
    if(!data->d.str.p && data->d.str.len == 0) {
        data->d.str.len++;
        data->d.str.p = calloc(1, 1);
    }
    data->d.str.len = data->d.str.len + strnlen(string, UPK_MAX_STRING_LEN);
    data->d.str.p = realloc(data->d.str.p, data->d.str.len);
    strcat(data->d.str.p, string);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_output_json_string(upk_json_output_data_t * data, char *fmt, ...)
{
    va_list                 ap;
    char                    buf[UPK_MAX_STRING_LEN] = "";

    switch (data->type) {
    case data_type_stream:
        va_start(ap, fmt);
        vfprintf(data->d.file, fmt, ap);
        va_end(ap);
        break;
    case data_type_string:
        va_start(ap, fmt);
        vsnprintf(buf, UPK_MAX_STRING_LEN - 1, fmt, ap);
        va_end(ap);
        upk_json_append_string(data, buf);
        break;
    }
}



/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_print_indent(upk_json_stack_meta_t * meta, upk_json_output_data_t * data)
{
    uint32_t                n = 0;

    for(n = 1; n < meta->count; n++)
        upk_output_json_string(data, data->opts.indent);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_null_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;
    if(d->opts.suppress_null_values)
        return;

    upk_json_print_indent(meta, data);
    if(key)
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.pad);

    upk_output_json_string(data, "Null,%s", d->opts.sep);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_bool_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key)
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.pad);

    upk_output_json_string(data, "%s,%s", (v.val.bl) ? "true" : "false", d->opts.sep);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_double_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key)
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.pad);

    upk_output_json_string(data, "%lf,%s", v.val.dbl, d->opts.sep);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_int_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key)
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.pad);

    upk_output_json_string(data, "%d,%s", v.val.i, d->opts.sep);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_string_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key)
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.pad);

    upk_output_json_string(data, "%s,%s", (v.val.str.esc_str) ? v.val.str.esc_str : "", d->opts.sep);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_array_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key && strnlen(key, UPK_MAX_STRING_LEN) > 0) {
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.sep);
        upk_json_print_indent(meta, data);
    }

    upk_output_json_string(data, "[%s", d->opts.sep);
    upk_json_stack_push(meta, meta->head);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_object_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    if(key && strnlen(key, UPK_MAX_STRING_LEN) > 0) {
        upk_output_json_string(data, "\"%s\"%s:%s", key, d->opts.pad, d->opts.sep);
        upk_json_print_indent(meta, data);
    }

    upk_output_json_string(data, "{%s", d->opts.sep);
    upk_json_stack_push(meta, meta->head);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_after_json_obj_pop_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    upk_output_json_string(data, "},%s", d->opts.sep);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_output_after_json_array_pop_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_json_output_data_t *d = data;

    upk_json_print_indent(meta, data);
    upk_output_json_string(data, "],%s", d->opts.sep);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_stack_pop(upk_json_stack_meta_t * meta)
{
    UPKLIST_FOREACH(meta) {
        UPKLIST_UNLINK(meta);
        break;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_stack_push(upk_json_stack_meta_t * meta, upk_json_stack_node_t * node)
{
    UPKLIST_PREPEND(meta);
    memcpy(meta->thisp, node, sizeof(*meta->thisp) - sizeof(meta->thisp->next));
}


/* *******************************************************************************************************************
   json-c escapes forward slashes, which makes the already rather ugly configuration file look even uglier. the json
   spec does _NOT_ require escaping forward slashes; so this undoes that; the alternative is to escape everything
   myself, duplicating what json_object_to_json_string does for strings via json_escape_string, and I'd rather not...
   ******************************************************************************************************************* */
static inline char     *
upk_json_unescape_forwardslash(char *str)
{
    char                   *p = str;
    size_t                  skip = 0;

    while(*(p + skip) != '\0') {
        if(*(p + skip) == '\\' && *(p + skip + 1) == '/')
            skip++;
        *p = *(p + skip);
        p++;
    }
    *p = '\0';

    return str;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_value(upk_json_stack_meta_t * meta, char *key, struct json_object *jobj)
{
    enum json_type          type = 0;
    upk_json_val_t          v = { 0 };

    type = (jobj) ? json_object_get_type(jobj) : json_type_null;
    v.type = (uint8_t) type;

    switch (type) {
    case json_type_null:
        if(meta->head->handlers.json_null)
            meta->head->handlers.json_null(meta, meta->head->data, key, v);
        break;
    case json_type_boolean:
        if(meta->head->handlers.json_bool) {
            v.val.bl = json_object_get_boolean(jobj);
            meta->head->handlers.json_bool(meta, meta->head->data, key, v);
        }
        break;
    case json_type_double:
        if(meta->head->handlers.json_double) {
            v.val.dbl = json_object_get_double(jobj);
            meta->head->handlers.json_double(meta, meta->head->data, key, v);
        }
        break;
    case json_type_int:
        if(meta->head->handlers.json_int) {
            v.val.i = json_object_get_int(jobj);
            meta->head->handlers.json_int(meta, meta->head->data, key, v);
        }
        break;
    case json_type_string:
        if(meta->head->handlers.json_string) {
            v.val.str.c_str = json_object_get_string(jobj);
            v.val.str.esc_str = upk_json_unescape_forwardslash(json_object_to_json_string(jobj));
            meta->head->handlers.json_string(meta, meta->head->data, key, v);
        }
        break;
    case json_type_object:
        if(meta->head->handlers.json_object) {
            v.val.obj = jobj;
            meta->head->handlers.json_object(meta, meta->head->data, key, v);
        }
        break;
    case json_type_array:
        if(meta->head->handlers.json_array) {
            v.val.obj = jobj;
            meta->head->handlers.json_array(meta, meta->head->data, key, v);
        }
        break;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
typedef struct upk_json_obj_iter {
    struct lh_entry        *entry;
    char                   *key;
    struct json_object     *val;
} upk_json_obj_iter_t;


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_parse_obj(upk_json_stack_meta_t * meta, char *key, struct json_object *jobj)
{
    upk_json_obj_iter_t     it;

    json_object_object_foreachC(jobj, it) {
        _upk_json_parse_node(meta, it.key, it.val);
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_parse_array(upk_json_stack_meta_t * meta, char *key, struct json_object *jarray)
{
    int                     array_len, i;
    struct json_object     *jvalue = NULL;

    array_len = json_object_array_length(jarray);

    for(i = 0; i < array_len; ++i) {
        jvalue = json_object_array_get_idx(jarray, i);
        _upk_json_parse_node(meta, NULL, jvalue);
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
_upk_json_parse_node(upk_json_stack_meta_t * meta, char *key, struct json_object *obj)
{
    upk_json_val_t          v = { 0 };
    enum json_type          type = 0;

    type = (obj) ? json_object_get_type(obj) : json_type_null;
    v.type = type;

    upk_json_value(meta, key, obj);
    switch (type) {
    case json_type_object:
        upk_json_parse_obj(meta, key, obj);
        upk_json_stack_pop(meta);
        if(meta->head->handlers.after_json_obj_pop) {
            v.val.obj = obj;
            meta->head->handlers.after_json_obj_pop(meta, meta->head->data, key, v);
        }
        break;
    case json_type_array:
        upk_json_parse_array(meta, key, obj);
        upk_json_stack_pop(meta);
        if(meta->head->handlers.after_json_array_pop) {
            v.val.obj = obj;
            meta->head->handlers.after_json_array_pop(meta, meta->head->data, key, v);
        }
        break;
    default:
        break;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_parse_node(upk_json_stack_meta_t * meta, char *key, struct json_object *obj)
{
    _upk_json_parse_node(meta, key, obj);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */

struct json_object     *
upk_json_parse_string(const char *string)
{
    struct json_tokener    *tok;
    struct json_object     *jobj;
    const char             *cp = string;
    char                    near[32] = "", *p = NULL;
    uint32_t                nline = 1, ncol = 0;

    UPK_ERR_INIT;

    tok = json_tokener_new();
    jobj = json_tokener_parse_ex(tok, (char *) string, -1);

    if(tok->err != json_tokener_success) {
        for(cp = string; cp < (string + tok->char_offset); cp++, ncol++) {
            if(*cp == '\n') {
                ncol = 0;
                nline++;
            }
        }
        strncpy(near, (string + tok->char_offset), 31);
        if((p = strchr(near, '\n')))
            *p = '\0';

    }

    UPK_FUNC_ASSERT_MSG((tok->err == json_tokener_success), UPK_JSON_PARSE_ERROR,
                        "%s: near: ``%s''; line: %d; column: %d; byte offset %d\n", json_tokener_errors[tok->err], near,
                        nline, ncol - 1, tok->char_offset);

    IF_UPK_ERROR {
        if(jobj)
            json_object_put(jobj);
        jobj = NULL;
    }
    json_tokener_free(tok);

    return jobj;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_json_obj_serializer(struct json_object *obj, upk_json_output_data_t * data)
{
    upk_json_stack_node_t   initial_node = {
        .data = data,
        .handlers.json_null = upk_json_output_null_handler,
        .handlers.json_bool = upk_json_output_bool_handler,
        .handlers.json_double = upk_json_output_double_handler,
        .handlers.json_int = upk_json_output_int_handler,
        .handlers.json_string = upk_json_output_string_handler,
        .handlers.json_array = upk_json_output_array_handler,
        .handlers.json_object = upk_json_output_object_handler,
        .handlers.after_json_obj_pop = upk_json_output_after_json_obj_pop_handler,
        .handlers.after_json_array_pop = upk_json_output_after_json_array_pop_handler,
    };
    upk_json_stack_meta_t  *meta = NULL;

    meta = calloc(1, sizeof(*meta));

    upk_json_stack_push(meta, &initial_node);
    upk_json_parse_node(meta, NULL, obj);


    UPKLIST_FREE(meta);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
char                   *
upk_json_obj_to_string(struct json_object *obj, upk_json_data_output_opts_t opts)
{
    char                   *p;
    upk_json_output_data_t  data = {
        .type = data_type_string,
        .d.str = {0},
        .opts = opts,
    };

    upk_json_obj_serializer(obj, &data);
    p = data.d.str.p;

    return p;
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_obj_to_stream(struct json_object *obj, FILE * stream, upk_json_data_output_opts_t opts)
{
    upk_json_output_data_t  data = {
        .type = data_type_stream,
        .d.file = stream,
        .opts = opts,
    };

    upk_json_obj_serializer(obj, &data);
}
