#ifndef UPKAPI_H
#define UPKAPI_H

#include "store/upk_db.h"

#define UPK_MAGIC "UPK_MAGIC"

typedef struct upk_api {
    char  *magic;
    struct upk_db upk_db;
} upk_api_t;

typedef struct upk_api_service {
    const char *package;
    const char *service;
    const char *cmdline;
    const char *state_desired;
    const char *state_actual;
    int         pid;
} upk_api_service_t;

int 
upk_api_init(
    struct upk_api *pupk_api
);

const char *
upk_api_service_desired_state_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
);

int 
upk_api_service_actual_state_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    upk_status_t    upk_status
);

const char *
upk_api_service_actual_state_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
);

int 
upk_api_service_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    char           *cmdline
);

int 
upk_api_service_visitor(
    struct upk_api *pupk_api,
    void           (*callback)(),
    void            *context
);

#endif
