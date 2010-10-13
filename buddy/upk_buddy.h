#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

int upk_buddy_start(
  sqlite3 *pdb, 
  char    *package,
  char    *service,
  char    *argv[], 
  char    *env[]
);

int
upk_buddy_stop(
  sqlite3 *pdb, 
  int      buddy_pid
);

int
upk_buddy_call_home(
  sqlite3  pdb, 
  int      buddy_pid,
);
