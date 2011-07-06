#ifndef _UPK_CONFIG_H
#define _UPK_CONFIG_H

/**
  @file
  @brief configuration implementation for controller and services.

  Structures and prototypes handling and defining service and controller's configuration.
  */

/**
  @addtogroup config_impl
  @{
  */

#include "upk_types.h"
#include "upk_include.h"
#include "upk_uuid.h"

/**
  @addtogroup services
  @{
  */

/**
  @brief service configuration.
  */
typedef struct _upk_svc_desc upk_svc_desc_t;

/**
  @brief service configuration.
  */
struct _upk_svc_desc {
    char                    Package[UPK_MAX_STRING_LEN];   /*!< an optional prefix to be preppended to the service
                                                              name */

    char                    Name[UPK_MAX_STRING_LEN];      /*!< service name. if pkg is used, the concatination of
                                                              <pkg>::<name> must be unique; otherwise this name must be 
                                                              unique */

    /* FIXME: this should be a list */
    char                    Provides[UPK_MAX_STRING_LEN];  /*!< a string describing the function of this service. this 
                                                              can be used in prerequisite constraints upon other
                                                              services. i.e. a prerequisite might be "entropy-service", 
                                                              and any service providing "entropy-service" would then
                                                              satisfy that prerequisite [vaguely remeniscent of the
                                                              debian "alternatives" system] */

    upk_uuid_t              UUID;                          /*!< a uuid for a service; dynamically generated at service
                                                              creation in most cases */

    char                    ShortDescription[UPK_MAX_STRING_LEN];   /*! a brief description of the service */

    char                   *LongDescription;               /*!< an arbitrary length description of the service */

    upk_svclisthead_t      *Prerequisites;                 /*!< A list of prerequisite services that must be started
                                                              prior to this service; either by name, pkg-prefix, or by
                                                              what they provide */

    int32_t                 StartPriority;                 /*!< similar to prerequisites, provide a fixed numeric
                                                              start priority to indicate service startup order */

    int32_t                 BuddyShutdownTimeout;          /*!< when a service is stopped; how many seconds to wait
                                                              before shutting the buddy down to conserve resources. a
                                                              value < 0 means to leave the buddy running indefinately */

    int32_t                 KillTimeout;                   /*!< how long to wait for a service to stop before issuing
                                                              TERM and, if necessary, KILL signals to bring it down. A
                                                              negative value means to wait indefiniately for a stopped
                                                              process to terminate */

    int32_t                 MaxConsecutiveFailures;        /*!< Maximum number of times a process may fail in-a-row
                                                              before its state is changed to down; a negative value
                                                              indicates to restart forever (and is the default) */

    int32_t                 UserMaxRestarts;               /*!< user-defined max number of restarts within restart
                                                              window */

    int32_t                 UserRestartWindow;             /*!< user-defined restart window, in seconds */

    int32_t                 UserRateLimit;                 /*!< duration, in seconds, to wait between respawn attempts 
                                                            */

    bool                    RandomizeRateLimit;            /*!< a flag to enable/disable adding a randomized "jitter"
                                                              to the user_ratelimit */

    uid_t                   SetUID;                        /*!< if controller and/or buddy is run euid root; which uid
                                                              to run the service as */

    gid_t                   SetGID;                        /*!< if controller and/or buddy is run euid root; which gid
                                                              to run the service as */

    size_t                  RingbufferSize;                /*!< size of the ringbuffer to maintain in the buddy */

    int32_t                 ReconnectRetries;              /*!< number of times to retry connections to the controler
                                                              when emergent actions occur in the buddy; (-1 for
                                                              indefinate) */

    char                    ExecStart[UPK_MAX_PATH_LEN];   /*!< command to exec for start. Default: 'kill', see "StartScript" */

    char                   *StartScript;                   /*!< script to run to start the monitored process; replaces
                                                              the default of 'exec %(ExecStart)' */

    char                    ExecStop[UPK_MAX_PATH_LEN];    /*!< executable to exec for stop. see "StopScript" */

    char                   *StopScript;                    /*!< replace the default stop script of 'exec %(EXEC_STOP) $1';
                                                              argv[1] == pid of monitored process */

    char                    ExecReload[UPK_MAX_PATH_LEN];    /*!< command to exec for reload. Default: 'kill -HUP', see "ReloadScript" */

    char                   *ReloadScript;                  /*!< replace the default reload script of 'exec kill -HUP
                                                              $1'; argv[1] == pid of monitored process */

    upk_cust_actscr_listhead_t *custom_action_scripts;     /*!< linked list of custom actions */

    char                   *PipeStdoutScript;              /*!< optional script to pipe stdout to. for instance: 'exec 
                                                              logger -p local0.notice' */

    char                   *PipeStderrScript;              /*!< optional script to pipe stderr to. for instance: 'exec 
                                                              logger -p local0.warn' */

    char                    RedirectStdout[UPK_MAX_PATH_LEN];   /*!< optional place to direct stdout. Note that if you 
                                                                   pipe stdout elsewhere, this might never be written
                                                                   to, unless the thing you pipe to prints to stdout
                                                                   itself */

    char                    RedirectStderr[UPK_MAX_PATH_LEN];   /*!< optional place to direct stderr. Note that if you 
                                                                   pipe stderr elsewhere, this might never be written
                                                                   to, unless the thing you pipe to prints to stderr
                                                                   itself */

    upk_state_t             InitialState;                  /*!< state the service should be set to initially; this is
                                                              used only when a service is first configured */

    bool                    UnconfigureOnFileRemoval;      /*!< May be used by a package to instruct the controler to
                                                              remove a configured service if the file defining that
                                                              service ever disappears. possibly useful in packaging to
                                                              cleanup the controller on package removal. The default
                                                              behavior is to ignore file removal, and require explicit
                                                              manual removal of configured services */

    bool                    PreferBuddyStateForStopped;    /*!< if the controller starts/restartindent: Standard
                                                              input:189: Error:Stmt nesting error. s, and buddy has a
                                                              service state set to "stopped", but controller's
                                                              data-store believes the service should be running, prefer 
                                                              buddy's world view, and update the data-store to reflect
                                                              the stopped state (the default is to trust the
                                                              data-store; which would cause the service to be started */

    bool                    PreferBuddyStateForRunning;    /*!< if the controller starts/restarts, and buddy has a
                                                              service state set to "running", but controller's
                                                              data-store believes the service should be stopped, prefer 
                                                              buddy's world view, and update the data-store to reflect
                                                              the running state (the default is to trust the
                                                              data-store, which would cause the service to be stopped */

    upk_svc_desc_t         *next;                          /*!< for use in lists */
};

/**
  @}
  @addtogroup controller
  @{
  */

/**
  @brief controller configuration.
  */
typedef struct _upk_controller_config {
    char                    StateDir[UPK_MAX_PATH_LEN];    /*!< path to variable state-dir for controller and buddies */

    char                    SvcConfigPath[UPK_MAX_PATH_LEN];    /*!< path to service configuration files */

    char                    SvcRunPath[UPK_MAX_PATH_LEN];  /*!< path where buddy's will be created and run, usually
                                                              ${statedir}/buddies */

    char                    controller_socket[UPK_MAX_PATH_LEN];    /*!< path to the controller socket, used
                                                                       internally */

    char                    controller_buddy_sock[UPK_MAX_PATH_LEN];    /*!< path to buddy socket; used internally */

    upk_svc_desc_t          ServiceDefaults;               /*!< default service configuration parameters; used
                                                              whenever an individual service omits something */

    upk_svc_desc_t         *svclist;                       /*!< head-pointer to list of service descriptions */
} upk_controller_config_t;

/**
  @}
  @addtogroup functions
  @{
  */

/* upkeeper/upk_config.c */

/* upk_config.c */
extern const char upk_ctrl_configuration_file[UPK_MAX_PATH_LEN];
extern const char upk_default_configuration_vec[];
extern upk_controller_config_t upk_default_configuration;
extern upk_controller_config_t upk_runtime_configuration;
extern void upk_ctrl_load_config(void);
extern char *upk_json_serialize_svc_config(upk_svc_desc_t *svc);

/** 
  @}
  */

#endif
