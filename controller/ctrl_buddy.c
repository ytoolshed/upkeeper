#include <upkeeper.h>
#include "controller.h"


static bool             create_buddy_statedir(upk_svc_desc_t * buddy);
static bool             remove_buddy_statedir(upk_svc_desc_t * buddy);
bool                    spawn_buddy(upk_svc_desc_t * buddy);
bool                    kill_buddy(upk_svc_desc_t * buddy);
static bool             send_buddy_cmd(int fd, buddy_cmnd_t cmd);

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static inline char     *
buddy_sock_path(upk_svc_desc_t * buddy, char *pathbuf)
{
    char                   *pathp;

    strncpy(pathbuf, DEFAULT_BUDDY_ROOT "/", UPK_MAX_PATH_LEN);
    pathp = pathbuf + strlen(pathbuf);

    strcpy(pathp, buddy->Name);
    strcat(pathp, "/buddy.sock");

    return pathbuf;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
create_buddy(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    static struct stat      st;

    stat(buddy_sock_path(buddy, path), &st);
    if(S_ISSOCK(st.st_mode))
        return;

    create_buddy_statedir(buddy);
    spawn_buddy(buddy);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
remove_buddy(upk_svc_desc_t * buddy)
{
    kill_buddy(buddy);
    remove_buddy_statedir(buddy);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
remove_buddy_statedir(upk_svc_desc_t * buddy)
{
    return false;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
create_buddy_statedir(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT;
    char                    linksrc[UPK_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT;
    char                   *pathp = NULL;
    char                   *linksrcp = NULL;
    uint32_t                n = 0;
    FILE                   *output;

    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    strcat(path, "/");
    strcat(path, buddy->Name);
    strcat(linksrc, "/");
    strcat(linksrc, buddy->Name);

    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    pathp = path + strlen(path);
    linksrcp = linksrc + strlen(linksrc);

    strcpy(pathp, "/actions");
    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    strcpy(pathp, "/scripts");
    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    strcpy(pathp, "/log");
    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    strcpy(pathp, "/actions/start");
    strcpy(linksrcp, "/scripts/start");
    output = fopen(linksrc, "w");
    fchmod(fileno(output), 0755);
    fprintf(output, "%s", buddy->StartScript);
    fclose(output);

    upk_verbose("creating link %s => %s\n", path, linksrc);
    symlink(linksrc, path);

    strcpy(pathp, "/actions/stop");
    strcpy(linksrcp, "/scripts/stop");

    output = fopen(linksrc, "w");
    fchmod(fileno(output), 0755);
    fprintf(output, "%s", buddy->StopScript);
    fclose(output);

    upk_verbose("creating link %s => %s\n", path, linksrc);
    symlink(linksrc, path);

    strcpy(pathp, "/actions/reload");
    strcpy(linksrcp, "/scripts/reload");

    output = fopen(linksrc, "w");
    fchmod(fileno(output), 0755);
    fprintf(output, "%s", buddy->ReloadScript);
    fclose(output);

    upk_verbose("creating link %s => %s\n", path, linksrc);
    symlink(linksrc, path);

    if(buddy->custom_action_scripts) {
        n = 0;
        UPKLIST_FOREACH(buddy->custom_action_scripts) {
            sprintf(pathp, "/actions/%02d", n);
            sprintf(linksrcp, "/scripts/%s", buddy->custom_action_scripts->thisp->name);

            upk_verbose("Creating custom action script");
            output = fopen(linksrc, "w");
            fchmod(fileno(output), 0755);
            fprintf(output, "%s", buddy->custom_action_scripts->thisp->script);
            fclose(output);

            upk_verbose("creating link %s => %s\n", path, linksrc);
            symlink(linksrc, path);
            n++;
        }
    }

    strcpy(pathp, "/controller.sock");
    upk_verbose("creating link to socket %s => %s\n", path, upk_runtime_configuration.controller_buddy_sock);
    return true;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
int
buddy_connect(const char *sockpath)
{
    struct sockaddr_un      sa = { 0 };
    int                     sa_len = sizeof(sa), fd = -2;


    strncpy(sa.sun_path, sockpath, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);

    errno = 0;
    if(connect(fd, (struct sockaddr *) &sa, sa_len) != 0) {
        upk_alert("Unable to connect to socket %s: %s\n", sockpath, strerror(errno));
        fd = -2;
    }
    return fd;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
buddy_info_t           *
buddy_readstate(int fd)
{
    buddy_info_t            buf = { 0 };
    buddy_cmnd_t            cmd = UPK_CTRL_ACK;
    fd_set                  sockfdset;
    char                    uuid_buf[37] = "";
    //int sockopts;
    struct timeval          timeout = {
        .tv_sec = 0,
        .tv_usec = 10000,
    };


    FD_ZERO(&sockfdset);
    FD_SET(fd, &sockfdset);
    if(select(fd + 1, NULL, &sockfdset, NULL, &timeout) && FD_ISSET(fd, &sockfdset)) {
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        while(select(fd + 1, &sockfdset, NULL, NULL, &timeout) && FD_ISSET(fd, &sockfdset)
              && (read(fd, &buf, sizeof(buf)) > 0)) {

            upk_debug1("Recording buddy state_info:\n");
            upk_debug1("\tservice_pid: %ld\n", buf.service_pid);
            upk_debug1("\twait_pid: %ld\n", buf.wait_pid);
            upk_debug1("\twait_status: %ld\n", buf.wait_status);
            upk_debug1("\tcommand: %d\n", buf.command);
            upk_debug1("\tdesired_state: %d\n", buf.desired_state);
            upk_debug1("\ttimestamp: %ld\n", buf.timestamp);
            upk_uuid_to_string(uuid_buf, &buf.uuid);
            upk_debug1("\tbuddy UUID: %s\n", uuid_buf);
            upk_debug1("\tremaining: %ld\n", buf.remaining);
            if(buf.wait_status) {
                if(WIFEXITED(buf.wait_status))
                    upk_debug0("pid %d exited with: %d\n", buf.wait_pid, WEXITSTATUS(buf.wait_status));
                if(WIFSIGNALED(buf.wait_status))
                    upk_debug0("pid %d was signaled with %d\n", buf.wait_pid, WTERMSIG(buf.wait_status));
            }


            if(select(fd + 1, NULL, &sockfdset, NULL, &timeout) && FD_ISSET(fd, &sockfdset))
                write(fd, &cmd, sizeof(cmd));
            
            if(buf.remaining == 0)
                break;
        }
    }
    return NULL;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
handle_buddies(void)
{
    char                    path[UPK_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT "/", *pathp;
    DIR                    *buddydir;
    struct dirent          *ent;
    int                     fd;


    pathp = path + strlen(path);

    buddydir = opendir(path);

    if(buddydir) {
        while((ent = readdir(buddydir))) {
            if(*ent->d_name == '.')
                continue;
            strcpy(pathp, ent->d_name);
            strcat(pathp, "/buddy.sock");

            fd = buddy_connect(path);
            send_buddy_cmd(fd, UPK_CTRL_STATUS_REQ);
        }
        closedir(buddydir);
    }

    return;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
spawn_buddy(upk_svc_desc_t * buddy)
{
    pid_t                   pid;
    struct timespec         timeout = {.tv_sec = 0,.tv_nsec = 100000000 };

    if((pid = fork()) == 0) {
        errno = 0;
        execl("/home/cthulhu/git/ytoolshed/upkeeper/controller/upk_buddy",
              "/home/cthulhu/git/ytoolshed/upkeeper/controller/upk_buddy", "-vvv", "--buddy_uuid",
              "12345678-1234-5678-1234-0123456789AB", buddy->Name, (char *) NULL);
        upk_fatal("Something terrible happened: %s\n", strerror(errno));
    }
    errno = 0;
    nanosleep(&timeout, NULL);

    return true;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
send_buddy_cmd(int fd, buddy_cmnd_t cmd)
{
    fd_set sockfdset;
    struct timeval          timeout = {
        .tv_sec = 0,
        .tv_usec = 10000,
    };


    FD_ZERO(&sockfdset);
    FD_SET(fd, &sockfdset);

    if(fd >= 0) {
        if(select(fd + 1, NULL, &sockfdset, NULL, &timeout) && FD_ISSET(fd, &sockfdset)) {
            write(fd, &cmd, sizeof(cmd));
            buddy_readstate(fd);
        }
        shutdown(fd, SHUT_RDWR);
        close(fd);
        return true;
    }
    return false;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
kill_buddy(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    return send_buddy_cmd(fd, UPK_CTRL_SHUTDOWN);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
start_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    return send_buddy_cmd(fd, UPK_CTRL_ACTION_START);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
stop_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    return send_buddy_cmd(fd, UPK_CTRL_ACTION_STOP);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
reload_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    return send_buddy_cmd(fd, UPK_CTRL_ACTION_RELOAD);
}
