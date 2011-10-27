
/****************************************************************************
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

#ifndef _UPK_BUDDY_H
#define _UPK_BUDDY_H
/* 

   @mainpage Buddy

   "buddy" is the servicer-restarter component of upkeeper. It has 3 primary design goals:

   1) Reliability - a buddy should never abort or otherwise fail after having been properly initialized. It
   is being relied upon to restart other services when they fail, and so what it does, and how it does it
   must be kept as simple, and as safe as possible. For instance, there is no runtime heap allocation, and
   runtime stack allocation is kept to a minimum, so even with restrictive ulimits, a buddy should not fail
   after initialization.

   2) Audit - buddy implements a fixed ringbuffer, which it uses to queue events it has monitored, and report 
   those back to the controller daemon if/when the controller is available. To that end, buddy will send the
   entire-contents of its ringbuffer to a controller when it connects to buddy. However, two circumstances
   exist where the buddy may need to actively connect the controller, rather than passively waiting for one
   to come by; they are a) when the ringbuffer has exceeded 3/4 fullness and b) when a buddy is terminated in 
   a mannor other than a controller shutting it down.

   3) Flexibility - While buddy must remain somewhat limited in what it can do itself, it has been designed
   specifically to remain flexible in how it is used, and amenable to the use of adjuncts to extend and
   enhance its own functionality. In essence, buddy will avoid any behavior that might prevent you from
   doing whatever you need to do.


   Buddy operates like this:

   * buddy starts

   * buddy initializes its ringbuffer, some string buffers, and a creates a domain socket to listen for
   controllers.

   * buddy registers a signal handler for CHLD which sets a flag to restart its managed process.

   * buddy goes into an eventloop where it sits and waits for a controller to tell it what to do.

   * (xyzzy: nothing happens)

   * sometime later, a controller connects to the buddy, and issues a command, eg, start, or runonce, etc.

   * buddy performs the action associated with whatever command it received, and reports back to the
   controller everything in its ringbuffer, the last item being the state of the buddy after completing the
   last action (but not necessarily the result of that last action; the controller may not obtain a result
   until some future poll)

   * the 'start' action is the only action that will be monitored. The script associated with the start
   action is what buddy will monitor; hence, that script should usually 'exec /path/to/yourservice' after
   performing any initialization you may require.

   * buddy supports up to 32 user-defined actions, which can be named anything you like, and can do anything
   you want; They receive the pid (and subsequently, the pgrp and sid) of the managed process as their first
   (and currently, only) argument. [note that buddy calls these user-defined actions by number, and not by
   name, as it simplifies the protocol, and keeps buddy from having to perform (or potentially failing to
   perform) bounds-checking for string operations]

   If a the monitored child dies during its lifetime, buddy will restart it. If that child is respawning too
   quickly, it will be delayed, to help ensure the controller has time to collect information on the
   failures, and to help prevent the respawn to negatively impact the system. */



#include "buddyinc.h"
#include "ctrl_protocol.h"
#include <upkeeper/upk_error.h>
#include <upkeeper/upk_uuid.h>
#include <upkeeper/upk_types.h>

#define BUDDY_MAX_STRING_LEN UPK_MAX_STRING_LEN
#define BUDDY_MAX_PATH_LEN UPK_MAX_PATH_LEN

extern upk_uuid_t       buddy_uuid;
extern char            *buddy_service_name;
extern char             buddy_root_path[BUDDY_MAX_PATH_LEN];
extern int32_t          verbosity;
extern uid_t            buddy_setuid;
extern gid_t            buddy_setgid;
extern size_t           ringbuffer_size;
extern long             reconnect_retries;
extern char           **proc_envp;
extern void             buddy_init(diag_output_callback_t logger);
extern int32_t          buddy_event_loop(void);
extern void             buddy_cleanup(void);


/* FIXME: this needs to be specified via ./configure (at _least_ prefix aware) */
#define DEFAULT_BUDDY_ROOT "/var/run/upkeeper/buddy"

#endif
