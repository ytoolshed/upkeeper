
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

    strncpy(pathbuf, upk_runtime_configuration.SvcRunPath, UPK_MAX_PATH_LEN);
    strcat(pathbuf,"/");

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
    if(! buddy) 
        return;
    kill_buddy(buddy);
    remove_buddy_statedir(buddy);
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
remove_buddy_statedir(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN] = ""; 
    char                    linksrc[UPK_MAX_PATH_LEN] = ""; 
    char                   *pathp = NULL;
    char                   *linksrcp = NULL;
    uint32_t                n = 0;
    DIR                    *buddy_log_dir;
    struct dirent          *ent;

    strcpy(path, upk_runtime_configuration.SvcRunPath);
    strcat(path, "/");
    strcat(path, buddy->Name);

    strcpy(linksrc, path);

    pathp = path + strlen(path);
    linksrcp = linksrc + strlen(linksrc);

    strcpy(pathp, "/controller.sock");
    upk_verbose("removing link to socket %s => %s\n", path, upk_runtime_configuration.controller_buddy_sock);
    unlink(path);

    if(buddy->CustomActions) {
        n = 0;
        UPKLIST_FOREACH(buddy->CustomActions) {
            sprintf(pathp, "/actions/%02d", n);
            sprintf(linksrcp, "/scripts/%s", buddy->CustomActions->thisp->name);

            upk_verbose("removing link %s => %s\n", path, linksrc);
            unlink(path);

            upk_verbose("removing custom action script %s\n", linksrc);
            unlink(linksrc);
            n++;
        }
    }

    strcpy(pathp, "/actions/reload");
    strcpy(linksrcp, "/scripts/reload");

    upk_verbose("removing link %s => %s\n", path, linksrc);
    unlink(path);
    upk_verbose("removing reload action script %s\n", linksrc);
    unlink(linksrc);

    strcpy(pathp, "/actions/start");
    strcpy(linksrcp, "/scripts/start");

    upk_verbose("removing link %s => %s\n", path, linksrc);
    unlink(path);
    upk_verbose("removing start action script %s\n", linksrc);
    unlink(linksrc);

    strcpy(pathp, "/actions/stop");
    strcpy(linksrcp, "/scripts/stop");

    upk_verbose("removing link %s => %s\n", path, linksrc);
    unlink(path);
    upk_verbose("removing stop action script %s\n", linksrc);
    unlink(linksrc);



    strcpy(pathp,"/log/");
    buddy_log_dir = opendir(path);

    if(buddy_log_dir) {
        while((ent = readdir(buddy_log_dir))) {
            if(*ent->d_name == '.')
                continue;
            strcat(path, ent->d_name);
            upk_verbose("removing log node %s\n", path);
        }
    }

    closedir(buddy_log_dir);

    strcpy(pathp, "/log/");
    upk_verbose("removing dir %s\n", path);
    rmdir(path);

    strcpy(pathp, "/scripts/");
    upk_verbose("removing dir %s\n", path);
    rmdir(path);

    strcpy(pathp, "/actions/");
    upk_verbose("removing dir %s\n", path);
    rmdir(path);

    *pathp = '\0';
    upk_verbose("removing dir %s\n", path);
    rmdir(path);

    /* FIXME: needs more sanity */
    strcpy(path, upk_runtime_configuration.SvcRunPath);
    strcat(path, "/");
    strcat(path, buddy->Name);

    upk_rm_rf(path);

    return true;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
create_buddy_statedir(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN] = ""; 
    char                    linksrc[UPK_MAX_PATH_LEN] = ""; 
    char                   *pathp = NULL;
    char                   *linksrcp = NULL;
    uint32_t                n = 0;
    FILE                   *output;

    strcpy(path, upk_runtime_configuration.SvcRunPath);
    upk_verbose("creating dir %s\n", path);
    mkdir(path, 0755);

    strcat(path, "/");
    strcat(path, buddy->Name);

    strcpy(linksrc,path);

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

    if(buddy->CustomActions) {
        n = 0;
        UPKLIST_FOREACH(buddy->CustomActions) {
            sprintf(pathp, "/actions/%02d", n);
            sprintf(linksrcp, "/scripts/%s", buddy->CustomActions->thisp->name);

            upk_verbose("Creating custom action script");
            output = fopen(linksrc, "w");
            fchmod(fileno(output), 0755);
            fprintf(output, "%s", buddy->CustomActions->thisp->script);
            fclose(output);

            upk_verbose("creating link %s => %s\n", path, linksrc);
            symlink(linksrc, path);
            n++;
        }
    }

    strcpy(pathp, "/controller.sock");
    upk_verbose("creating link to socket %s => %s\n", path, upk_runtime_configuration.controller_buddy_sock);
    symlink(upk_runtime_configuration.controller_socket, path);

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

    // int sockopts;
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
/*******************************************************************************************************************
 ********************************************************************************************************************/
upk_svc_desc_t *
lookup_buddy_from_path(char * buddy_path)
{
    char buddy_path_copy[UPK_MAX_PATH_LEN] = "", *buddy_name;

    strcpy(buddy_path_copy, buddy_path);
    buddy_name = basename(buddy_path_copy); /* old versions of basename modify their argument, so we work on a copy */

    UPKLIST_FOREACH(upk_runtime_configuration.svclist) {
        if( strcmp(upk_runtime_configuration.svclist->thisp->Name, buddy_name) == 0 )
            return upk_runtime_configuration.svclist->thisp;
    }
    return NULL;
}



/*******************************************************************************************************************
 ********************************************************************************************************************/
void
handle_buddies(void)
{
    char                    path[UPK_MAX_PATH_LEN] = "", *pathp;
    upk_svc_desc_t * buddy = NULL;
    DIR                    *buddydir;
    struct dirent          *ent;
    int                     fd;

    strcpy(path, upk_runtime_configuration.SvcRunPath);
    strcat(path, "/");

    pathp = path + strlen(path);

    buddydir = opendir(path);

    if(buddydir) {
        while((ent = readdir(buddydir))) {
            if(*ent->d_name == '.')
                continue;
            strcpy(pathp, ent->d_name);
            strcat(pathp, "/buddy.sock");

            fd = buddy_connect(path);
            if(fd >= 0)
                send_buddy_cmd(fd, UPK_CTRL_STATUS_REQ);
            else {
                buddy = lookup_buddy_from_path(ent->d_name);
                if(buddy) 
                    remove_buddy(buddy);
                else {
                    strcpy(pathp, ent->d_name);
                    upk_rm_rf(path); /* just nuke it */
                }
            }
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
    char buddy_verbosity[16] = "";
    char buddy_uuid[UPK_UUID_STRING_LEN + 1] = "";
    char buddy_path[UPK_MAX_PATH_LEN] = "";
    /* struct stat buddyst; */
    int8_t n = 0;
    struct timespec         timeout = {.tv_sec = 0,.tv_nsec = 100000000 };

    /* if( stat(upk_runtime_configuration.UpkBuddyPath, &buddyst) != 0 )
        return false;
    */

    if(upk_runtime_configuration.BuddyVerbosity > 0) {
        buddy_verbosity[0]='-';
        for(n = 1; n < upk_runtime_configuration.BuddyVerbosity && n < sizeof(buddy_verbosity) - 1; n++)
            buddy_verbosity[n] = 'v';
    }

    upk_uuid_to_string(buddy_uuid, &buddy->UUID);
    sprintf(buddy_path, "%s/%s", upk_runtime_configuration.SvcRunPath, buddy->Name);

    /* FIXME: stuff buddy pid somewhere, and catch buddy exits and log them */
    if((pid = fork()) == 0) {
        errno = 0;
        upk_verbose("%s '%s' '%s' '%s' '%s' '%s' '%s'\n", upk_runtime_configuration.UpkBuddyPath, buddy_verbosity, "--buddy_uuid",
              buddy_uuid, "--buddy_path", buddy_path, buddy->Name);
        execl(upk_runtime_configuration.UpkBuddyPath, upk_runtime_configuration.UpkBuddyPath, buddy_verbosity, "--buddy_uuid",
              buddy_uuid, "--buddy_path", buddy_path, buddy->Name, (char *) NULL);
        upk_fatal("could not execute `%s': %s\n", upk_runtime_configuration.UpkBuddyPath, strerror(errno));
        exit(1);
    }

    nanosleep(&timeout, NULL);

    return true;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static                  bool
send_buddy_cmd(int fd, buddy_cmnd_t cmd)
{
    fd_set                  sockfdset;
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
    if(fd >= 0)
        return send_buddy_cmd(fd, UPK_CTRL_SHUTDOWN);
    return false;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
start_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    if(fd >= 0)
        return send_buddy_cmd(fd, UPK_CTRL_ACTION_START);
    return false;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
stop_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    if(fd >= 0)
        return send_buddy_cmd(fd, UPK_CTRL_ACTION_STOP);
    return false;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
reload_buddy_svc(upk_svc_desc_t * buddy)
{
    char                    path[UPK_MAX_PATH_LEN];
    int                     fd;

    fd = buddy_connect(buddy_sock_path(buddy, path));
    if(fd >= 0)
        return send_buddy_cmd(fd, UPK_CTRL_ACTION_RELOAD);
    return false;
}
