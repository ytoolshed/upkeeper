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

static void             upk_ctrlconf_string_handler(char *key, char *string);
static void             upk_svcconf_bool_handler(char *key, _Bool val);
static void             upk_svcconf_string_handler(char *key, char *string);
static void             upk_ctrl_service_pop_handler(char *key, void *obj);
static inline void      upk_service_parse_handlers(void);
static void             upk_ctrlconf_object_handler(char *key, void *jobj);
static void             upk_ctrl_config_parse_handlers(void *buf);
static char            *upk_config_loadfile(const char *filename);
static void             upk_ctrl_config_pack(upk_controller_config_t * cfg, const char *json_string);

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
/* FIXME: allow specifying default via configure */

static void            *_upk_ctrl_service_pop_handler_data_buf;
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
    "        \"Package\": \"default\",\n" 
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
    "        \"StopScript\": \"#!/bin/sh\\nexec kill $1\\n\",\n" 
    "        \"ReloadScript\": \"#!/bin/sh\\nexec kill -HUP $1\\n\",\n" 
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
upk_controller_config_t upk_default_configuration;
upk_controller_config_t upk_runtime_configuration;

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrlconf_string_handler(char *key, char *string)
{

    upk_controller_config_t *data = upk_json_type_callbacks.data;

    if((strcasecmp(key, "StateDir") == 0))
        strncpy(data->StateDir, string, UPK_MAX_STRING_LEN - 1);
    else if(strcasecmp(key, "SvcConfigPath") == 0)
        strncpy(data->SvcConfigPath, string, UPK_MAX_STRING_LEN - 1);
    else if(strcasecmp(key, "SvcRunPath") == 0)
        strncpy(data->SvcRunPath, string, UPK_MAX_STRING_LEN - 1);
    else
        upk_fatal("invalid Configuration: %s", key);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_bool_handler(char *key, bool val)
{
    upk_svc_desc_t         *data = upk_json_type_callbacks.data;

    printf("setting %s to %s\n", key, ((val) ? "true" : "false"));
    if((strcasecmp(key, "RandomizeRateLimit") == 0))
        data->RandomizeRateLimit = val;
    else if(strcasecmp(key, "UnconfigureOnFileRemoval") == 0)
        data->UnconfigureOnFileRemoval = val;
    else if(strcasecmp(key, "PreferBuddyStateForStopped") == 0)
        data->PreferBuddyStateForStopped = val;
    else if(strcasecmp(key, "PreferBuddyStateForRunning") == 0)
        data->PreferBuddyStateForRunning = val;
    else if(strcasecmp(key, "UnconfigureOnFileRemoval") == 0)
        data->UnconfigureOnFileRemoval = val;
    else
        upk_fatal("invalid Configuration: %s", key);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_int_handler(char *key, int val)
{
    upk_svc_desc_t         *data = upk_json_type_callbacks.data;

    printf("setting %s to %d\n", key, val);
    if((strcasecmp(key, "StartPriority") == 0))
        data->StartPriority = val;
    else if(strcasecmp(key, "BuddyShutdownTimeout") == 0)
        data->BuddyShutdownTimeout = val;
    else if(strcasecmp(key, "KillTimeout") == 0)
        data->KillTimeout = val;
    else if(strcasecmp(key, "UserMaxRestarts") == 0)
        data->UserMaxRestarts = val;
    else if(strcasecmp(key, "UserRestartWindow") == 0)
        data->UserRestartWindow = val;
    else if(strcasecmp(key, "UserRateLimit") == 0)
        data->UserRateLimit = val;
    else if(strcasecmp(key, "SetUID") == 0)
        data->SetUID = val;
    else if(strcasecmp(key, "SetGID") == 0)
        data->SetGID = val;
    else if(strcasecmp(key, "RingbufferSize") == 0)
        data->RingbufferSize = val;
    else if(strcasecmp(key, "ReconnectRetries") == 0)
        data->ReconnectRetries = val;
    else
        upk_fatal("invalid Configuration: %s", key);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_string_handler(char *key, char *string)
{

    upk_svc_desc_t         *data = upk_json_type_callbacks.data;

    printf("Setting %s to %s\n", key, string);

    if((strcasecmp(key, "Package") == 0)) {
        strncpy(data->Package, string, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "Name") == 0) {
        strncpy(data->Name, string, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "Provides") == 0) {
        strncpy(data->Provides, string, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "UUID") == 0) {
        upk_string_to_uuid(string, &data->UUID);
    } else if(strcasecmp(key, "ShortDescription") == 0) {
        strncpy(data->ShortDescription, string, UPK_MAX_STRING_LEN - 1);
    } else if(strcasecmp(key, "LongDescription") == 0) {
        data->LongDescription = calloc(1, strnlen(string, 16384) + 1);
        strcpy(data->LongDescription, string);
    } else if(strcasecmp(key, "ExecStart") == 0) {
        strncpy(data->ExecStart, string, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "StartScript") == 0) {
        data->StartScript = calloc(1, strlen(string) + 1);
        strcpy(data->StartScript, string);
    } else if(strcasecmp(key, "StopScript") == 0) {
        data->StopScript = calloc(1, strlen(string) + 1);
        strcpy(data->StopScript, string);
    } else if(strcasecmp(key, "ReloadScript") == 0) {
        data->ReloadScript = calloc(1, strlen(string) + 1);
        strcpy(data->ReloadScript, string);
    } else if(strcasecmp(key, "PipeStdoutScript") == 0) {
        data->PipeStdoutScript = calloc(1, strlen(string) + 1);
        strcpy(data->PipeStdoutScript, string);
    } else if(strcasecmp(key, "PipeStderrScript") == 0) {
        data->PipeStderrScript = calloc(1, strlen(string) + 1);
        strcpy(data->PipeStderrScript, string);
    } else if(strcasecmp(key, "RedirectStdout") == 0) {
        strncpy(data->RedirectStdout, string, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "RedirectStderr") == 0) {
        strncpy(data->RedirectStderr, string, UPK_MAX_PATH_LEN - 1);
    } else if(strcasecmp(key, "InitialState") == 0) {
        if((strcasecmp(string, "running") == 0) || (strcasecmp(string, "up") == 0) || (strcasecmp(string, "start") == 0)
           || (strcasecmp(string, "run") == 0)) {
            data->InitialState = UPK_STATE_RUNNING;
        } else {
            data->InitialState = UPK_STATE_SHUTDOWN;
        }
    } else {
        upk_fatal("invalid Configuration: %s", key);
    }
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_svcconf_double_handler(char *key, double val)
{
    upk_fatal("invalid Configuration: %s", key);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrl_service_pop_handler(char *key, void *obj)
{
    printf("returning to a simpler time, when key was %s\n", key);
    upk_ctrl_config_parse_handlers(_upk_ctrl_service_pop_handler_data_buf);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static inline void
upk_service_parse_handlers(void)
{
    _upk_ctrl_service_pop_handler_data_buf = upk_json_type_callbacks.data;
    upk_json_type_callbacks.data =
        &((upk_controller_config_t *) _upk_ctrl_service_pop_handler_data_buf)->ServiceDefaults;

    upk_json_type_callbacks.upk_json_null = NULL;
    upk_json_type_callbacks.upk_json_bool = upk_svcconf_bool_handler;
    upk_json_type_callbacks.upk_json_double = upk_svcconf_double_handler;
    upk_json_type_callbacks.upk_json_int = upk_svcconf_int_handler;
    upk_json_type_callbacks.upk_json_string = upk_svcconf_string_handler;
    upk_json_type_callbacks.upk_json_array = upk_json_default_array_handler;
    upk_json_type_callbacks.upk_json_object = upk_json_default_object_handler;
    upk_json_type_callbacks.upk_json_pop_obj = upk_ctrl_service_pop_handler;
    upk_json_type_callbacks.upk_json_pop_array = NULL;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrlconf_object_handler(char *key, void *jobj)
{
    if((strcasecmp(key, "ServiceDefaults") == 0)) {
        strncpy(((upk_controller_config_t *) upk_json_type_callbacks.data)->ServiceDefaults.Name, key,
                UPK_MAX_STRING_LEN - 1);
        upk_service_parse_handlers();
    } else if((strcasecmp(key, "Services") == 0)) {
        upk_service_parse_handlers();
    }
    return;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_ctrl_config_parse_handlers(void *buf)
{
    upk_json_type_callbacks.data = buf;
    upk_json_type_callbacks.upk_json_null = NULL;
    upk_json_type_callbacks.upk_json_bool = NULL;
    upk_json_type_callbacks.upk_json_double = NULL;
    upk_json_type_callbacks.upk_json_int = NULL;
    upk_json_type_callbacks.upk_json_string = upk_ctrlconf_string_handler;
    upk_json_type_callbacks.upk_json_array = NULL;
    upk_json_type_callbacks.upk_json_object = upk_ctrlconf_object_handler;
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
static void
upk_ctrl_config_pack(upk_controller_config_t * cfg, const char *json_string)
{
    upk_ctrl_config_parse_handlers(cfg);
    upk_json_parse_string(json_string);
    return;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_load_config(void)
{
    char                   *json;

    upk_ctrl_config_pack(&upk_default_configuration, upk_default_configuration_vec);

    if(strlen(upk_ctrl_configuration_file) > 0) {
        json = upk_config_loadfile(upk_ctrl_configuration_file);
        if(json)
            upk_ctrl_config_pack(&upk_runtime_configuration, json);
    }
    return;
}

/**
  @}
  */
