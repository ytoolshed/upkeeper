#ifndef _UPK_JSON_FMT_H
#define _UPK_JSON_FMT_H

typedef struct _upk_json_data_output_options {
    char                   *pad;
    char                   *indent;
    char                   *sep;
    bool                    suppress_null_values;
} upk_json_data_output_opts_t;

#endif

