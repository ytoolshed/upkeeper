#ifndef UPKAPI_H
#define UPKAPI_H

#include "store/upk_db.h"

typedef struct upk_api {
    struct upk_db upk_db;
} upk_api_t;

int 
upk_api_init(
    struct upk_api *pupk_api
);

int 
upk_api_service_desired_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
);

int 
upk_api_service_actual_status_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    upk_status_t    upk_status
);

int 
upk_api_service_actual_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
);

#endif
