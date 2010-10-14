#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
typedef struct 
/* 
   Create a buddies to start and watch a processe.

   * TODO:
   Create n buddies to start and watch m processes.
   n is implementation defined.
   m is the length of length of struct command

   todo: less args
   todo: list return type / multiple arg
*/
int upk_buddy_start(
                    sqlite3 *pdb, 
                    const char    *package,
                    const char    *service,
                    const char    *command,
                    const char    *env[],
);

/* 
   stop the application that buddy was watching.
   TODO: see buddy_start return type.

 */

int upk_buddy_stop(
  sqlite3 *pdb, 
  int      buddy_pid
);


/*
  TODO: move to public header to be called by application.
  can we reliably verify unix uid across a socket?
  think more about this
*/

int
upk_buddy_call_home(
  sqlite3  pdb, 
  int      buddy_pid
);
