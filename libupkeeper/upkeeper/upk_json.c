#include "upk_json.h"

void                    upk_json_default_null_handler(char *key);
void                    upk_json_default_bool_handler(char *key, _Bool val);
void                    upk_json_default_double_handler(char *key, double val);
void                    upk_json_default_int_handler(char *key, int val);
void                    upk_json_default_string_handler(char *key, char *val);
void                    upk_json_default_array_handler(char *key, void *jobj);
void                    upk_json_default_object_handler(char *key, void *jobj);
static inline void      upk_json_value(char *key, struct json_object *jobj);
static inline void      upk_json_parse_array(char *key, struct json_object *jarray);

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_null_handler(char *key)
{
    printf("\"%s\":", key);
    printf("Null\n");
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_bool_handler(char *key, bool val)
{
    printf("\"%s\":", key);
    printf("%s\n", (val) ? "true" : "false");
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_double_handler(char *key, double val)
{
    printf("\"%s\":", key);
    printf("%lf\n", val);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_int_handler(char *key, int val)
{
    printf("\"%s\":", key);
    printf("%d\n", val);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_string_handler(char *key, char *val)
{
    printf("\"%s\":", key);
    printf("%s\n", val);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_array_handler(char *key, void *jobj)
{
    return;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_json_default_object_handler(char *key, void *jobj)
{
    return;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
upk_json_type_handlers_t upk_json_type_callbacks = {
    .data = NULL,
    .upk_json_null = upk_json_default_null_handler,
    .upk_json_bool = upk_json_default_bool_handler,
    .upk_json_double = upk_json_default_double_handler,
    .upk_json_int = upk_json_default_int_handler,
    .upk_json_string = upk_json_default_string_handler,
    .upk_json_array = upk_json_default_array_handler,
    .upk_json_object = upk_json_default_object_handler,
    .upk_json_pop_obj = NULL,
    .upk_json_pop_array = NULL,
};

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_value(char *key, struct json_object *jobj)
{
    enum json_type          type = 0;

    type = (jobj) ? json_object_get_type(jobj) : json_type_null;

    switch (type) {
    case json_type_null:
        if(upk_json_type_callbacks.upk_json_null)
            upk_json_type_callbacks.upk_json_null(key);
        break;
    case json_type_boolean:
        if(upk_json_type_callbacks.upk_json_bool)
            upk_json_type_callbacks.upk_json_bool(key, json_object_get_boolean(jobj));
        break;
    case json_type_double:
        if(upk_json_type_callbacks.upk_json_double)
            upk_json_type_callbacks.upk_json_double(key, json_object_get_double(jobj));
        break;
    case json_type_int:
        if(upk_json_type_callbacks.upk_json_int)
            upk_json_type_callbacks.upk_json_int(key, json_object_get_int(jobj));
        break;
    case json_type_string:
        if(upk_json_type_callbacks.upk_json_string)
            upk_json_type_callbacks.upk_json_string(key, json_object_get_string(jobj));
        break;
    case json_type_object:
        if(upk_json_type_callbacks.upk_json_object)
            upk_json_type_callbacks.upk_json_object(key, jobj);
        break;
    case json_type_array:
        if(upk_json_type_callbacks.upk_json_array)
            upk_json_type_callbacks.upk_json_array(key, jobj);
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
void
upk_json_parse_node(struct json_object *jobj)
{
    enum json_type          type = 0;
    upk_json_obj_iter_t     it;
    struct json_object     *jptr;

    json_object_object_foreachC(jobj, it) {
        type = (it.val) ? json_object_get_type(it.val) : json_type_null;

        switch (type) {
        case json_type_object:
            jptr = json_object_object_get(jobj, it.key);
            upk_json_value(it.key, jptr);
            upk_json_parse_node(jptr);
            if(upk_json_type_callbacks.upk_json_pop_obj)
                upk_json_type_callbacks.upk_json_pop_obj(it.key, jptr);
            break;
        case json_type_array:
            jptr = json_object_object_get(jobj, it.key);
            upk_json_value(it.key, jptr);
            upk_json_parse_array(it.key, jptr);
            if(upk_json_type_callbacks.upk_json_pop_obj)
                upk_json_type_callbacks.upk_json_pop_array(it.key, jptr);
            break;
        default:
            upk_json_value(it.key, it.val);
            break;
        }
        if(!it.entry->next)
            break;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
upk_json_parse_array(char *key, struct json_object *jarray)
{
    int                     array_len, i;
    enum json_type          type = 0;
    struct json_object     *jvalue = NULL;
    char                    label[UPK_MAX_STRING_LEN] = "";

    array_len = json_object_array_length(jarray);

    for(i = 0; i < array_len; ++i) {
        jvalue = json_object_array_get_idx(jarray, i);
        type = json_object_get_type(jvalue);
        snprintf(label, UPK_MAX_STRING_LEN - 1, "%s[%d]", key, i);
        switch (type) {
        case json_type_object:
            upk_json_value(label, jvalue);
            upk_json_parse_node(jvalue);
            if(upk_json_type_callbacks.upk_json_pop_obj)
                upk_json_type_callbacks.upk_json_pop_obj(key, jvalue);
            break;
        case json_type_array:
            upk_json_value(label, jvalue);
            upk_json_parse_array(label, jvalue);
            if(upk_json_type_callbacks.upk_json_pop_array)
                upk_json_type_callbacks.upk_json_pop_array(key, jvalue);
            break;
        default:
            upk_json_value(label, jvalue);
            break;
        }
    }
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
bool
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

    upk_json_parse_node(jobj);
    json_object_put(jobj);

    IF_UPK_ERROR {
        jobj = NULL;
    }

    json_tokener_free(tok);

    return (jobj) ? true : false;
}
