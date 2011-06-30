#ifndef _UPK_JSON_H
#define _UPK_JSON_H

#include <json/json.h>
#include <stdio.h>
#include "upk_include.h"

typedef struct _upk_json_type_handlers {
    void *data;
    void (*upk_json_null)(char *key);
    void (*upk_json_bool)(char *key, bool val);
    void (*upk_json_double)(char *key, double val);
    void (*upk_json_int)(char *key, int val);
    void (*upk_json_string)(char *key, char *val);
    void (*upk_json_array)(char *key, void *jobj);
    void (*upk_json_object)(char *key, void *jobj);
    void (*upk_json_pop_obj)(char *key, void *jobj);
    void (*upk_json_pop_array)(char *key, void *jobj);
} upk_json_type_handlers_t;

extern upk_json_type_handlers_t upk_json_type_callbacks;

/* upkeeper/upk_json.c */
extern void upk_json_default_null_handler(char *key);
extern void upk_json_default_bool_handler(char *key, bool val);
extern void upk_json_default_double_handler(char *key, double val);
extern void upk_json_default_int_handler(char *key, int val);
extern void upk_json_default_string_handler(char *key, char *val);
extern void upk_json_default_array_handler(char *key, void *jobj);
extern void upk_json_default_object_handler(char *key, void *jobj);
extern upk_json_type_handlers_t upk_json_type_callbacks;
extern void upk_json_parse_node(struct json_object *jobj);
extern bool upk_json_parse_string(const char *string);

#endif
