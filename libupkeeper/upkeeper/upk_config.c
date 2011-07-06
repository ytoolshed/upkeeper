#include "upk_include.h"
#include "upk_json.h"

/**
  @file
  @brief configuration defaults, and implementation
  @addtogroup config_impl
  @{
  */

/* ******************************************************************************************************************
   ****************************************************************************************************************** */

#define _stringify(A) #A
#define stringify(A) _stringify(A)

/* upkeeper/upk_config.c */
static void             upk_ctrlconf_string_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                    upk_json_val_t v);
static void             upk_ctrlconf_object_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                    upk_json_val_t v);
static void             upk_ctrlconf_pack(upk_controller_config_t * cfg, const char *json_string);
static void             upk_svcconf_bool_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v);
static void             upk_svcconf_int_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v);
static void             upk_svcconf_string_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                   upk_json_val_t v);
static void             upk_conf_error_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v);
static void             upk_ctrl_service_pop_handler(upk_json_stack_meta_t * meta, void *data, char *key,
                                                     upk_json_val_t v);
static char            *upk_config_loadfile(const char *filename);
void                    upk_ctrl_load_config(void);
char                   *upk_json_serialize_svc_config(upk_svc_desc_t * svc);

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
/* FIXME: allow specifying default via configure */
const char              upk_ctrl_configuration_file[UPK_MAX_PATH_LEN] = stringify(CONF_SYSCONFDIR) "/upkeeper.conf";

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
/* *INDENT-OFF* */
const char upk_default_configuration_vec[] = 
    "{\n" 
	"    // StateDir\n" 
	"    // Path to variable state-dir for controller and buddies\n" 
    "    \"StateDir\": \"" stringify(CONF_LOCALSTATEDIR) "/upkeeper\",\n" 
    "    \n" 
    "    // SvcConfigPath\n" 
    "    // Path to location of service configuration files\n" 
    "    \"SvcConfigPath\": \"" stringify(CONF_SYSCONFDIR) "/upkeeper.d\",\n" 
    "    \n" 
    "    // SvcRunPath\n" 
    "    // Path to location to setup and run buddies\n" 
    "    \"SvcRunPath\": \"" stringify(CONF_LOCALSTATEDIR) "/upkeeper/buddies\",\n" 
    "    \n" 
    "    // ServiceDefaults:\n" 
    "    \"ServiceDefaults\": {\n" 
    "        \"Name\": \"default\",\n"
    "        \"Package\": \"\",\n"
    "        \"Provides\": Null,\n" 
    "        \"UUID\": Null,\n" 
    "        \"ShortDescription\": Null,\n" 
    "        \"LongDescription\": Null,\n" 
    "        \"Prerequisites\": Null,\n" 
    "        \"StartPriority\": 0,\n" 
    "        \"BuddyShutdownTimeout\": -1,\n" 
    "        \"KillTimeout\": 60,\n" 
    "        \"UserMaxRestarts\": Null,\n" 
    "        \"UserRestartWindow\": Null,\n" 
    "        \"UserRateLimit\": Null,\n" 
    "        \"RandomizeRateLimit\": false,\n" 
    "        \"SetUID\": 0,\n" 
    "        \"SetGID\": 0,\n" 
    "        \"RingbufferSize\": 64,\n" 
    "        \"ReconnectRetries\": 10,\n" 
    "        \"ExecStart\": Null,\n" 
    "        \"StartScript\": \"#!/bin/sh\\nexec %(EXEC_START)\\n\",\n" 
    "        \"ExecStop\": \"kill\",\n" 
    "        \"StopScript\": \"#!/bin/sh\\nexec %(EXEC_STOP) $1\\n\",\n" 
    "        \"ExecReload\": \"kill -HUP\",\n" 
    "        \"ReloadScript\": \"#!/bin/sh\\nexec %(EXEC_RELOAD) $1\\n\",\n" 
    "        \"CustomActions\": Null,\n" 
    "        \"PipeStdoutScript\": Null,\n" 
    "        \"PipeStderrScript\": Null,\n" 
    "        \"RedirectStdout\": Null,\n" 
    "        \"RedirectStderr\": Null,\n" 
    "        \"InitialState\": \"stopped\",\n" 
    "        \"UnconfigureOnFileRemoval\": false,\n" 
    "        \"PreferBuddyStateForStopped\": false,\n" 
    "        \"PreferBuddyStateForRunning\": true,\n"
    "    },\n"
    "}\n"
    ;
/* *INDENT-ON* */

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
upk_controller_config_t
    upk_default_configuration;
upk_controller_config_t
    upk_runtime_configuration;

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrlconf_string_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_controller_config_t *d = data;

    if((strcasecmp(key, "StateDir") == 0))
        strncpy(d->StateDir, v.val.str, UPK_MAX_STRING_LEN - 1);
    else if(strcasecmp(key, "SvcConfigPath") == 0)
        strncpy(d->SvcConfigPath, v.val.str, UPK_MAX_STRING_LEN - 1);
    else if(strcasecmp(key, "SvcRunPath") == 0)
        strncpy(d->SvcRunPath, v.val.str, UPK_MAX_STRING_LEN - 1);
    else
        upk_fatal("invalid Configuration: %s", key);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrlconf_object_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_controller_config_t *d = data;

    upk_json_stack_node_t   service_node = {
        .data = &d->ServiceDefaults,
        .handlers.json_null = NULL,                        /* probably not an error, skip it for now */
        .handlers.json_bool = upk_svcconf_bool_handler,
        .handlers.json_double = upk_conf_error_handler,
        .handlers.json_int = upk_svcconf_int_handler,
        .handlers.json_string = upk_svcconf_string_handler,
        .handlers.json_array = NULL,                       /* XXX: unimplemented, and needs to be!! */
        .handlers.json_object = NULL,                      /* XXX: unimplemented, and needs to be!! */
        .handlers.after_json_obj_pop = NULL,
        .handlers.after_json_array_pop = NULL,
    };

    if((strcasecmp(key, "ServiceDefaults") == 0)) {
        strncpy(d->ServiceDefaults.Name, key, UPK_MAX_STRING_LEN - 1);
        upk_json_stack_push(meta, &service_node);
    } else if((strcasecmp(key, "Services") == 0)) {
        upk_json_stack_push(meta, &service_node);
    }
    return;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrlconf_pack(upk_controller_config_t * cfg, const char *json_string)
{
    struct json_object     *obj = NULL;
    upk_json_stack_node_t   ctrl_node = {
        .data = cfg,
        .handlers.json_string = upk_ctrlconf_string_handler,
        .handlers.json_object = upk_ctrlconf_object_handler,
        .handlers.after_json_obj_pop = upk_ctrl_service_pop_handler,
    };

    upk_json_stack_meta_t  *meta = calloc(1, sizeof(*meta));
    upk_json_stack_push(meta, &ctrl_node);

    obj = upk_json_parse_string(json_string);

    if(obj)
        upk_json_parse_node(meta, NULL, obj);

    UPKLIST_FREE(meta);

    return;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_bool_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_svc_desc_t         *d = data;

    upk_debug1("upk_svcconf_bool_handler: setting %s to %s\n", key, ((v.val.bl) ? "true" : "false"));
    if((strcasecmp(key, "RandomizeRateLimit") == 0))
        d->RandomizeRateLimit = v.val.bl;
    else if(strcasecmp(key, "UnconfigureOnFileRemoval") == 0)
        d->UnconfigureOnFileRemoval = v.val.bl;
    else if(strcasecmp(key, "PreferBuddyStateForStopped") == 0)
        d->PreferBuddyStateForStopped = v.val.bl;
    else if(strcasecmp(key, "PreferBuddyStateForRunning") == 0)
        d->PreferBuddyStateForRunning = v.val.bl;
    else if(strcasecmp(key, "UnconfigureOnFileRemoval") == 0)
        d->UnconfigureOnFileRemoval = v.val.bl;
    else
        upk_alert("invalid Configuration: %s", key);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_int_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_svc_desc_t         *d = data;

    printf("setting %s to %d\n", key, v.val.i);
    if((strcasecmp(key, "StartPriority") == 0))
        d->StartPriority = v.val.i;
    else if(strcasecmp(key, "BuddyShutdownTimeout") == 0)
        d->BuddyShutdownTimeout = v.val.i;
    else if(strcasecmp(key, "KillTimeout") == 0)
        d->KillTimeout = v.val.i;
    else if(strcasecmp(key, "UserMaxRestarts") == 0)
        d->UserMaxRestarts = v.val.i;
    else if(strcasecmp(key, "UserRestartWindow") == 0)
        d->UserRestartWindow = v.val.i;
    else if(strcasecmp(key, "UserRateLimit") == 0)
        d->UserRateLimit = v.val.i;
    else if(strcasecmp(key, "SetUID") == 0)
        d->SetUID = v.val.i;
    else if(strcasecmp(key, "SetGID") == 0)
        d->SetGID = v.val.i;
    else if(strcasecmp(key, "RingbufferSize") == 0)
        d->RingbufferSize = v.val.i;
    else if(strcasecmp(key, "ReconnectRetries") == 0)
        d->ReconnectRetries = v.val.i;
    else
        upk_alert("invalid Configuration: %s", key);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_string_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{

    upk_svc_desc_t         *d = data;

    printf("Setting %s to %s\n", key, v.val.str);

    if((strcasecmp(key, "Package") == 0)) {
        strncpy(d->Package, v.val.str, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "Name") == 0) {
        strncpy(d->Name, v.val.str, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "Provides") == 0) {
        strncpy(d->Provides, v.val.str, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "UUID") == 0) {
        upk_string_to_uuid(v.val.str, &d->UUID);
    } else if(strcasecmp(key, "ShortDescription") == 0) {
        strncpy(d->ShortDescription, v.val.str, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "LongDescription") == 0) {
        d->LongDescription = calloc(1, strnlen(v.val.str, 16384) + 1);
        strcpy(d->LongDescription, v.val.str);
    } else if(strcasecmp(key, "ExecStart") == 0) {
        strncpy(d->ExecStart, v.val.str, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "StartScript") == 0) {
        d->StartScript = calloc(1, strlen(v.val.str) + 1);
        strcpy(d->StartScript, v.val.str);
    } else if(strcasecmp(key, "ExecStop") == 0) {
        strncpy(d->ExecStop, v.val.str, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "StopScript") == 0) {
        d->StopScript = calloc(1, strlen(v.val.str) + 1);
        strcpy(d->StopScript, v.val.str);
    } else if(strcasecmp(key, "ExecReload") == 0) {
        strncpy(d->ExecReload, v.val.str, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "ReloadScript") == 0) {
        d->ReloadScript = calloc(1, strlen(v.val.str) + 1);
        strcpy(d->ReloadScript, v.val.str);
    } else if(strcasecmp(key, "PipeStdoutScript") == 0) {
        d->PipeStdoutScript = calloc(1, strlen(v.val.str) + 1);
        strcpy(d->PipeStdoutScript, v.val.str);
    } else if(strcasecmp(key, "PipeStderrScript") == 0) {
        d->PipeStderrScript = calloc(1, strlen(v.val.str) + 1);
        strcpy(d->PipeStderrScript, v.val.str);
    } else if(strcasecmp(key, "RedirectStdout") == 0) {
        strncpy(d->RedirectStdout, v.val.str, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "RedirectStderr") == 0) {
        strncpy(d->RedirectStderr, v.val.str, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "InitialState") == 0) {
        if((strcasecmp(v.val.str, "running") == 0) || (strcasecmp(v.val.str, "up") == 0)
           || (strcasecmp(v.val.str, "start") == 0)
           || (strcasecmp(v.val.str, "run") == 0)) {
            d->InitialState = UPK_STATE_RUNNING;
        } else {
            d->InitialState = UPK_STATE_SHUTDOWN;
        }
    } else {
        upk_alert("invalid Configuration: %s", key);
    }
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_conf_error_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    upk_alert("invalid Configuration: %s", key);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrl_service_pop_handler(upk_json_stack_meta_t * meta, void *data, char *key, upk_json_val_t v)
{
    printf("returning to a simpler time, when key was %s\n", key);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static char            *
upk_config_loadfile(const char *filename)
{
    return NULL;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_ctrl_load_config(void)
{
    char                   *json;

    upk_ctrlconf_pack(&upk_default_configuration, upk_default_configuration_vec);

    if(strlen(upk_ctrl_configuration_file) > 0) {
        json = upk_config_loadfile(upk_ctrl_configuration_file);
        if(json)
            upk_ctrlconf_pack(&upk_runtime_configuration, json);
    }
    return;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
char                   *
upk_json_serialize_svc_config(upk_svc_desc_t * svc)
{
    struct json_object     *obj, *p;
    char                    uuid_buf[37] = "";
    upk_json_data_output_opts_t opts = {.pad = " ",.indent = "    ",.sep = "\n" };

    obj = json_object_new_object();

    json_object_object_add(obj, "Package", json_object_new_string(svc->Package));
    json_object_object_add(obj, "Name", json_object_new_string(svc->Name));
    json_object_object_add(obj, "Provides", json_object_new_string(svc->Provides));
    upk_string_to_uuid(uuid_buf, &svc->UUID);
    json_object_object_add(obj, "UUID", json_object_new_string(uuid_buf));
    json_object_object_add(obj, "ShortDescription", json_object_new_string(svc->ShortDescription));
    if(svc->LongDescription)
        json_object_object_add(obj, "LongDescription", json_object_new_string(svc->LongDescription));

    p = json_object_new_array();
    if(svc->Prerequisites) {
        UPKLIST_FOREACH(svc->Prerequisites) {
            json_object_array_add(p, json_object_new_string(svc->Prerequisites->thisp->svc_id));
        }
    }
    json_object_object_add(obj, "Prerequisites", p);

    json_object_object_add(obj, "StartPriority", json_object_new_int(svc->StartPriority));
    json_object_object_add(obj, "BuddyShutdownTimeout", json_object_new_int(svc->BuddyShutdownTimeout));
    json_object_object_add(obj, "KillTimeout", json_object_new_int(svc->KillTimeout));
    json_object_object_add(obj, "MaxConsecutiveFailures", json_object_new_int(svc->MaxConsecutiveFailures));
    json_object_object_add(obj, "UserMaxRestarts", json_object_new_int(svc->UserMaxRestarts));
    json_object_object_add(obj, "UserRestartWindow", json_object_new_int(svc->UserRestartWindow));
    json_object_object_add(obj, "UserRateLimit", json_object_new_int(svc->UserRateLimit));
    json_object_object_add(obj, "RandomizeRateLimit", json_object_new_boolean(svc->RandomizeRateLimit));
    json_object_object_add(obj, "SetUID", json_object_new_int(svc->SetUID));
    json_object_object_add(obj, "SetGID", json_object_new_int(svc->SetGID));
    json_object_object_add(obj, "RingbufferSize", json_object_new_int(svc->RingbufferSize));
    json_object_object_add(obj, "ReconnectRetries", json_object_new_int(svc->ReconnectRetries));
    json_object_object_add(obj, "SetGID", json_object_new_int(svc->SetGID));
    json_object_object_add(obj, "UnconfigureOnFileRemoval", json_object_new_boolean(svc->UnconfigureOnFileRemoval));
    json_object_object_add(obj, "PreferBuddyStateForStopped", json_object_new_boolean(svc->PreferBuddyStateForStopped));
    json_object_object_add(obj, "PreferBuddyStateForRunning", json_object_new_boolean(svc->PreferBuddyStateForRunning));
    json_object_object_add(obj, "ExecStart", json_object_new_string(svc->ExecStart));
    json_object_object_add(obj, "StartScript", json_object_new_string((svc->StartScript) ? svc->StartScript : ""));
    json_object_object_add(obj, "ExecStop", json_object_new_string(svc->ExecStop));
    json_object_object_add(obj, "StopScript", json_object_new_string((svc->StopScript) ? svc->StopScript : ""));
    json_object_object_add(obj, "ExecReload", json_object_new_string(svc->ExecReload));
    json_object_object_add(obj, "ReloadScript", json_object_new_string((svc->ReloadScript) ? svc->ReloadScript : ""));

    p = json_object_new_object();
    if(svc->custom_action_scripts) {
        UPKLIST_FOREACH(svc->custom_action_scripts) {
            json_object_object_add(p, "Name", json_object_new_string(svc->custom_action_scripts->thisp->name));
            json_object_object_add(p, "Script",
                                   json_object_new_string((svc->custom_action_scripts->thisp->script) ? svc->
                                                          custom_action_scripts->thisp->script : ""));
        }
    }
    json_object_object_add(p, "CustomActions", p);

    json_object_object_add(obj, "PipeStdoutScript",
                           json_object_new_string((svc->PipeStdoutScript) ? svc->PipeStdoutScript : ""));
    json_object_object_add(obj, "PipeStderrScript",
                           json_object_new_string((svc->PipeStderrScript) ? svc->PipeStderrScript : ""));
    json_object_object_add(obj, "RedirectStdout", json_object_new_string(svc->RedirectStdout));
    json_object_object_add(obj, "RedirectStderr", json_object_new_string(svc->RedirectStderr));

    return upk_json_obj_to_string(obj, opts);
}






/**
  @}
  */
