
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

#ifndef _CTRL_PROTOCOL_H
#define _CTRL_PROTOCOL_H

/**
  @file
  @brief types used to communicate with buddy.
  */

/**
  @addtogroup controller
  @{
  */

#include "buddyinc.h"

/**
  @brief controller side of buddy protocol.

  This is what controller sends to buddy
  */
typedef enum _buddy_cmnd {
    UPK_CTRL_NONE = 0,                                     /*!< unset */
    UPK_CTRL_ACK = 1,                                      /*!< controller received the last msg buddy sent
                                                              it */
    UPK_CTRL_SHUTDOWN,                                     /*!< buddy shutdown */
    UPK_CTRL_STATUS_REQ,                                   /*!< report status to controller */
    UPK_CTRL_ACTION_START,                                 /* run start script and monitor */
    UPK_CTRL_ACTION_STOP,                                  /*!< run stop script, which should kill the
                                                              monitored process */
    UPK_CTRL_ACTION_RELOAD,                                /*!< run reload script, which could, for example, 
                                                              send SIGHUP */
    UPK_CTRL_ACTION_RUNONCE,                               /*!< run a monitored process in a restarter...
                                                              only once... ??? */
    UPK_CTRL_CUSTOM_ACTION_00,                             /*!< run user-defined action 00 */
    UPK_CTRL_CUSTOM_ACTION_01,                             /*!< run user-defined action 01 */
    UPK_CTRL_CUSTOM_ACTION_02,                             /*!< run user-defined action 02 */
    UPK_CTRL_CUSTOM_ACTION_03,                             /*!< run user-defined action 03 */
    UPK_CTRL_CUSTOM_ACTION_04,                             /*!< run user-defined action 04 */
    UPK_CTRL_CUSTOM_ACTION_05,                             /*!< run user-defined action 05 */
    UPK_CTRL_CUSTOM_ACTION_06,                             /*!< run user-defined action 06 */
    UPK_CTRL_CUSTOM_ACTION_07,                             /*!< run user-defined action 07 */
    UPK_CTRL_CUSTOM_ACTION_08,                             /*!< run user-defined action 08 */
    UPK_CTRL_CUSTOM_ACTION_09,                             /*!< run user-defined action 09 */
    UPK_CTRL_CUSTOM_ACTION_10,                             /*!< run user-defined action 10 */
    UPK_CTRL_CUSTOM_ACTION_11,                             /*!< run user-defined action 11 */
    UPK_CTRL_CUSTOM_ACTION_12,                             /*!< run user-defined action 12 */
    UPK_CTRL_CUSTOM_ACTION_13,                             /*!< run user-defined action 13 */
    UPK_CTRL_CUSTOM_ACTION_14,                             /*!< run user-defined action 14 */
    UPK_CTRL_CUSTOM_ACTION_15,                             /*!< run user-defined action 15 */
    UPK_CTRL_CUSTOM_ACTION_16,                             /*!< run user-defined action 16 */
    UPK_CTRL_CUSTOM_ACTION_17,                             /*!< run user-defined action 17 */
    UPK_CTRL_CUSTOM_ACTION_18,                             /*!< run user-defined action 18 */
    UPK_CTRL_CUSTOM_ACTION_19,                             /*!< run user-defined action 19 */
    UPK_CTRL_CUSTOM_ACTION_20,                             /*!< run user-defined action 20 */
    UPK_CTRL_CUSTOM_ACTION_21,                             /*!< run user-defined action 21 */
    UPK_CTRL_CUSTOM_ACTION_22,                             /*!< run user-defined action 22 */
    UPK_CTRL_CUSTOM_ACTION_23,                             /*!< run user-defined action 23 */
    UPK_CTRL_CUSTOM_ACTION_24,                             /*!< run user-defined action 24 */
    UPK_CTRL_CUSTOM_ACTION_25,                             /*!< run user-defined action 25 */
    UPK_CTRL_CUSTOM_ACTION_26,                             /*!< run user-defined action 26 */
    UPK_CTRL_CUSTOM_ACTION_27,                             /*!< run user-defined action 27 */
    UPK_CTRL_CUSTOM_ACTION_28,                             /*!< run user-defined action 28 */
    UPK_CTRL_CUSTOM_ACTION_29,                             /*!< run user-defined action 29 */
    UPK_CTRL_CUSTOM_ACTION_30,                             /*!< run user-defined action 30 */
    UPK_CTRL_CUSTOM_ACTION_31,                             /*!< run user-defined action 31 */
    UPK_CTRL_SIGNAL_01,                                    /*!< send managed process signal 01 */
    UPK_CTRL_SIGNAL_02,                                    /*!< send managed process signal 02 */
    UPK_CTRL_SIGNAL_03,                                    /*!< send managed process signal 03 */
    UPK_CTRL_SIGNAL_04,                                    /*!< send managed process signal 04 */
    UPK_CTRL_SIGNAL_05,                                    /*!< send managed process signal 05 */
    UPK_CTRL_SIGNAL_06,                                    /*!< send managed process signal 06 */
    UPK_CTRL_SIGNAL_07,                                    /*!< send managed process signal 07 */
    UPK_CTRL_SIGNAL_08,                                    /*!< send managed process signal 08 */
    UPK_CTRL_SIGNAL_09,                                    /*!< send managed process signal 09 */
    UPK_CTRL_SIGNAL_10,                                    /*!< send managed process signal 10 */
    UPK_CTRL_SIGNAL_11,                                    /*!< send managed process signal 11 */
    UPK_CTRL_SIGNAL_12,                                    /*!< send managed process signal 12 */
    UPK_CTRL_SIGNAL_13,                                    /*!< send managed process signal 13 */
    UPK_CTRL_SIGNAL_14,                                    /*!< send managed process signal 14 */
    UPK_CTRL_SIGNAL_15,                                    /*!< send managed process signal 15 */
    UPK_CTRL_SIGNAL_16,                                    /*!< send managed process signal 16 */
    UPK_CTRL_SIGNAL_17,                                    /*!< send managed process signal 17 */
    UPK_CTRL_SIGNAL_18,                                    /*!< send managed process signal 18 */
    UPK_CTRL_SIGNAL_19,                                    /*!< send managed process signal 19 */
    UPK_CTRL_SIGNAL_20,                                    /*!< send managed process signal 20 */
    UPK_CTRL_SIGNAL_21,                                    /*!< send managed process signal 21 */
    UPK_CTRL_SIGNAL_22,                                    /*!< send managed process signal 22 */
    UPK_CTRL_SIGNAL_23,                                    /*!< send managed process signal 23 */
    UPK_CTRL_SIGNAL_24,                                    /*!< send managed process signal 24 */
    UPK_CTRL_SIGNAL_25,                                    /*!< send managed process signal 25 */
    UPK_CTRL_SIGNAL_26,                                    /*!< send managed process signal 26 */
    UPK_CTRL_SIGNAL_27,                                    /*!< send managed process signal 27 */
    UPK_CTRL_SIGNAL_28,                                    /*!< send managed process signal 28 */
    UPK_CTRL_SIGNAL_29,                                    /*!< send managed process signal 29 */
    UPK_CTRL_SIGNAL_30,                                    /*!< send managed process signal 30 */
    UPK_CTRL_SIGNAL_31,                                    /*!< send managed process signal 31 */
    UPK_CTRL_SIGNAL_32,                                    /*!< send managed process signal 32 */
} buddy_cmnd_t;

/**
  @brief states for buddy
  */
typedef enum {
    BUDDY_UNKNOWN = 0,
    BUDDY_STOPPED = 1,                                     /*!< the current state of the managed process is
                                                              stopped */
    BUDDY_RUNNING,                                         /*!< the current state of the managed process is
                                                              running */
    BUDDY_RANONCE,                                         /*!< the current state of the managed process was 
                                                              that it ran once */
} buddy_runstate_t;

/**
  @brief information buddy returns.
  */
typedef struct _buddy_info buddy_info_t;

/**
  @brief information buddy returns.

  This is what buddy sends to controller.
  */
struct _buddy_info {
    bool                    populated;                     /*!< this member is populated; used in ringbuffer 
                                                            */
    pid_t                   service_pid;                   /*!< pid of the managed process */
    pid_t                   wait_pid;                      /*!< pid of the process just waited on, if
                                                              applicable */
    buddy_cmnd_t            command;                       /*!< last command handled by buddy */
    buddy_runstate_t        desired_state;                 /*!< desired state of the buddy */
    siginfo_t               siginfo;                       /*!< siginfo struct if available */
    int                     wait_status;                   /*!< wait status, if available */
    time_t                  timestamp;                     /*!< timestamp */
    size_t                  remaining;                     /*!< number of remaining messages to be sent from 
                                                              buddy */
    upk_uuid_t              uuid;                          /*!< uuid of service managed by buddy */
    size_t                  slot_n;                        /*!< used for diagnostics */
    buddy_info_t           *next;                          /*!< pointer to next member in ringbuffer */
};

/* FIXME: switch this over to use the runtime_configuration item */
#define DEFAULT_BUDDY_ROOT "/var/run/upkeeper/buddy"

/**
  @}
  */

#endif
