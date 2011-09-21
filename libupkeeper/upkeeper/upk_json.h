/* ***************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ************************************************************************** */

#ifndef _UPK_JSON_H
#define _UPK_JSON_H

#include "upk_include.h"
#include <json/json.h>
#include <stdio.h>

typedef struct _upk_json_string {
    char *c_str;
    char *esc_str;
} upk_json_string_t;

typedef struct _upk_json_type {
    uint8_t                 type;
    union {
        bool                    bl;
        double                  dbl;
        int                     i;
        upk_json_string_t      str;
        struct json_object     *obj;
    } val;
} upk_json_val_t;

typedef struct _upk_json_stack_node upk_json_stack_node_t;
typedef                 UPKLIST_METANODE(upk_json_stack_node_t, upk_json_stack_meta_p), upk_json_stack_meta_t;
typedef void            (*upk_json_handler_t) (upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v);


typedef struct _upk_json_stack_handlers {
    upk_json_handler_t      json_null;
    upk_json_handler_t      json_bool;
    upk_json_handler_t      json_double;
    upk_json_handler_t      json_int;
    upk_json_handler_t      json_string;
    upk_json_handler_t      json_array;
    upk_json_handler_t      json_object;
    upk_json_handler_t      after_json_obj_pop;
    upk_json_handler_t      after_json_array_pop;
} upk_json_handlers_t;


struct _upk_json_stack_node {
    void                   *data;
    upk_json_handlers_t     handlers;
    upk_json_stack_node_t  *next;
};

/* upkeeper/upk_json.c */
extern void             upk_json_stack_pop(upk_json_stack_meta_t * meta);
extern void             upk_json_stack_push(upk_json_stack_meta_t * meta, upk_json_stack_node_t * node);
extern void             upk_json_parse_node(upk_json_stack_meta_t * meta, char *key, struct json_object *jobj);
extern struct json_object * upk_json_parse_string(const char *string);
extern char            *upk_json_obj_to_string(struct json_object *obj, upk_json_data_output_opts_t opts);
extern void             upk_json_obj_to_stream(struct json_object *obj, FILE *stream, upk_json_data_output_opts_t opts);

#endif
