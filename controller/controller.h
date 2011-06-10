#ifndef _UPK_CONTROLLER_H
#define _UPK_CONTROLLER_H

#include <upkeeper.h>

#define UPK_CTRL_MAX_CLIENTS 10240

typedef struct _client_fdlist client_fdlist_t;
struct _client_fdlist {
    int32_t                 fd;
    client_fdlist_t        *next;
};

typedef enum {
    UPK_CLIENT_DISCONNECT = 1,
} upk_clientstatus_t;


extern int              socket_setup(const char *sock_path);
extern upk_clientstatus_t handle_client(int32_t client_fd);
extern void             handle_signals(void);
extern void             handle_buddies(void);
int32_t ctrl_sock_setup(upk_controller_config_t * config);


#endif
