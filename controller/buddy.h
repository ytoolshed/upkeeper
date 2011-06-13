#ifndef _UPK_BUDDY_H
#define _UPK_BUDDY_H

#include "buddyinc.h"
#include "ctrl_protocol.h"

#define BUDDY_MAX_STRING_LEN 2048
#define BUDDY_MAX_PATH_LEN 8192
#define BUDDY_MAX_CONNECTIONS 16

extern size_t           ringbuffer_size;
extern int              buddy_shutdown;
extern char            *buddy_service_name;
extern int32_t          verbose;
extern uid_t            buddy_uid;
extern gid_t            buddy_gid;

extern void             buddy_init(void);
extern int32_t          buddy_event_loop(void);
extern void             buddy_cleanup(void);

/* FIXME: this needs to be specified via ./configure (at _least_ prefix aware) */
#define DEFAULT_BUDDY_ROOT "/var/run/upkeeper/buddy"

#endif
