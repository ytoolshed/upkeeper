#ifndef _UPK_CONTROLLER_H
#define _UPK_CONTROLLER_H

#include <upkeeper.h>
#include <stdio.h>
#include <ctrl_protocol.h>

#define UPK_CTRL_MAX_CLIENTS 10240

typedef struct _client_fdlist client_fdlist_t;
struct _client_fdlist {
    int32_t                 fd;
    char                    readbuf[UPK_MAX_PACKET_SIZE];
    char                    writebuf[UPK_MAX_PACKET_SIZE];
    client_fdlist_t        *next;
};

typedef enum {
    UPK_CLIENT_DISCONNECT = 1,
} upk_clientstatus_t;

typedef struct _buddylist buddylist_t;
struct _buddylist {
    char                    buddy_name[UPK_MAX_STRING_LEN];
    char                    custom_actions[32][UPK_MAX_STRING_LEN];
    buddylist_t            *next;
};

extern upk_controller_config_t *upk_ctrl_config;


extern int              socket_setup(const char *sock_path);
extern upk_clientstatus_t handle_client(int32_t client_fd);
extern void             handle_signals(void);
extern void             handle_buddies(void);

/* XXX: refactoring */
extern int32_t          ctrl_sock_setup(void);
extern void             event_loop(int32_t listen_sock);

extern void * buddy_list(void);
extern void create_buddy(buddylist_t * buddy);



#endif
