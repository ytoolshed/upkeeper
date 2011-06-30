#include "controller.h"


static bool create_buddy_statedir(buddylist_t * buddy);
static bool remove_buddy_statedir(buddylist_t * buddy);
static bool spawn_buddy(buddylist_t * buddy);
static bool kill_buddy(buddylist_t * buddy);


/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
void *
buddy_list(void)
{
    UPKLIST_INIT(buddylist_t, blist);

    UPKLIST_APPEND(blist);
    strncpy(blist->thisp->buddy_name, "foobar", UPK_MAX_STRING_LEN - 1);
    return (void *) blist;
}


/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
void
create_buddy(buddylist_t * buddy)
{
    create_buddy_statedir(buddy);
    spawn_buddy(buddy);
}


/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
void
remove_buddy(buddylist_t * buddy)
{
    kill_buddy(buddy);
    remove_buddy_statedir(buddy);
}


/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
static bool
remove_buddy_statedir(buddylist_t * buddy)
{
    return false;
}


/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
static bool 
create_buddy_statedir(buddylist_t * buddy)
{
    char path[UPK_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT; 
    char linksrc[UPK_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT; 
    char * pathp = NULL;
    char * linksrcp = NULL;
    uint32_t n = 0;

    printf("creating dir %s\n",path);
    mkdir(path, 0700);

    strcat(path,"/");
    strcat(path,buddy->buddy_name);
    strcat(linksrc,"/");
    strcat(linksrc,buddy->buddy_name);

    printf("creating dir %s\n",path);
    mkdir(path, 0700);

    pathp=path + strlen(path);
    linksrcp=linksrc + strlen(linksrc);

    strcpy(pathp, "/actions");
    printf("creating dir %s\n",path);
    mkdir(path, 0700);

    strcpy(pathp, "/scripts");
    printf("creating dir %s\n",path);
    mkdir(path, 0700);

    strcpy(pathp, "/log");
    printf("creating dir %s\n",path);
    mkdir(path, 0700);

    strcpy(pathp, "/actions/start");
    strcpy(linksrcp, "/scripts/start");
    printf("creating link %s => %s\n",path, linksrc);
    symlink(linksrc,path);

    strcpy(pathp, "/actions/stop");
    strcpy(linksrcp, "/scripts/stop");
    printf("creating link %s => %s\n",path, linksrc);
    symlink(linksrc,path);

    strcpy(pathp, "/actions/reload");
    strcpy(linksrcp, "/scripts/reload");
    printf("creating link %s => %s\n",path, linksrc);
    symlink(linksrc,path);

    for(n = 0; n < 32; n++) {
        if(strlen(buddy->custom_actions[n]) > 0) {
            sprintf(pathp, "/actions/%02d", n);
            sprintf(linksrcp, "/scripts/%s", buddy->custom_actions[n]);
            printf("creating link %s => %s\n",path, linksrc);
            symlink(linksrc,path);
        }
    }

    strcpy(pathp, "/controller.sock");
    printf("creating link to socket %s => %s\n", path, upk_ctrl_config->controller_buddy_sock);
    return true;
}

/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
void
handle_buddies(void)
{
    //UPKLIST_METANODE(buddylist_t, blist) = buddy_list();

    /* UPKLIST_FOREACH(blist) {
        printf("buddy: %s\n", blist->thisp->buddy_name);
    } */

	return;
}

/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
static bool
spawn_buddy(buddylist_t * buddy)
{
    return false;
}

/* ******************************************************************************************************************
 * ****************************************************************************************************************** */
static bool
kill_buddy(buddylist_t * buddy)
{
    return false;
}
