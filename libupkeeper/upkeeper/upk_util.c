
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

#include "upk_include.h"
#include <stdio.h>

/** ***************************************************************************
 **************************************************************************** */
bool
upk_numeric_string(const char *string, long *num)
{
    char                   *p = NULL;
    uint8_t                 base = 10;

    if(strncmp(string, "0", 1) == 0)
        base = 8;
    if(strncmp(string, "0x", 2) == 0)
        base = 16;

    errno = 0;
    *num = strtoul(string, &p, base);
    if(p == string)
        return false;

    if((errno == ERANGE && (*num == LONG_MAX || *num == LONG_MIN)) || (*num <= 0))
        return false;

    return true;
}

/** ***************************************************************************
 **************************************************************************** */
bool
upk_boolean_string(const char *string, bool * val)
{
    long                    buf;

    if(upk_numeric_string(string, &buf)) {
        if(buf == 0) {
            *val = false;
            return true;
        } else if(buf == 1) {
            *val = true;
            return true;
        }
    } else if(strncasecmp(string, "true", strlen("true")) == 0) {
        *val = true;
        return true;
    } else if(strncasecmp(string, "false", strlen("false")) == 0) {
        *val = false;
        return true;
    } else if(strncasecmp(string, "yes", strlen("yes")) == 0) {
        *val = true;
        return true;
    } else if(strncasecmp(string, "no", strlen("no")) == 0) {
        *val = false;
        return true;
    }
    return false;
}

/** ***************************************************************************
 **************************************************************************** */
void
upk_replace_string(char **haystack, const char *needle, const char *repl)
{
    char                   *cp, *seek;
    size_t                  len = strlen(*haystack);
    size_t                  nlen = strlen(needle);
    size_t                  rlen = strlen(repl);
    size_t                  newsize = (len + rlen - nlen) + 1;
    size_t                  offset;

    if((cp = strstr(*haystack, needle))) {
        offset = cp - *haystack;
        if(newsize > len) {
            *haystack = realloc(*haystack, newsize);
            cp = *haystack + offset;
            for(seek = *haystack + len; seek != cp + nlen - 1; seek--)
                *(seek + rlen - nlen) = *seek;
        } else if(newsize < len) {
            for(seek = cp + rlen; (seek + nlen - rlen) != (*haystack) + len; seek++)
                *seek = *(seek + nlen - rlen);
            *seek = '\0';
            *haystack = realloc(*haystack, newsize);
            cp = *haystack + offset;
        }

        memcpy(cp, repl, rlen);
    }
}

/** ***************************************************************************
 **************************************************************************** */
struct timeval
upk_double_to_timeval(long double r)
{
    struct timeval          val = { ((int) r), ((int) ((r - (long double) ((int) r)) * 1000000)) };
    return val;
}


/** ***************************************************************************
 **************************************************************************** */
int
upk_rm_rf(char *start_path)
{
    DIR                    *dir;
    char                    fpath[UPK_MAX_PATH_LEN] = "";
    char                   *fpathp = NULL;
    struct dirent          *ent;
    struct stat             s;
    long                    count = 0;

    if(start_path && strlen(start_path) > 0) {
        strcpy(fpath, start_path);
        strcat(fpath, "/");
        fpathp = fpath + strlen(fpath);
        dir = opendir(fpath);
        if(dir) {
            while((ent = readdir(dir))) {
                if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;
                strcpy(fpathp, ent->d_name);
                stat(fpath, &s);
                if(S_ISDIR(s.st_mode)) {
                    count += upk_rm_rf(fpath);
                } else {
                    errno = 0;
                    if(unlink(fpath) == 0)
                        count++;
                    else
                        upk_warn("cannot remove file: %s: %s\n", fpath, strerror(errno));
                }
            }
            closedir(dir);
            errno = 0;
            if(rmdir(start_path) == 0)
                count++;
            else
                upk_warn("cannot remove directory: %s: %s\n", start_path, strerror(errno));
        }
    }
    return count;
}

/** ***************************************************************************
  @brief create a directory and any path components leading up to it.

  @param[in] path                   directory to create.
  
  @return the number of directories successfully created. 
 **************************************************************************** */
int
upk_mkdir_p(const char *path)
{
    upk_stringlist_meta_t  *pathlist = calloc(1, sizeof(*pathlist));
    char                    buf[UPK_MAX_STRING_LEN] = "", *pdir = NULL;
    int                     count = 0;
    struct stat             st;

    strncpy(buf, path, UPK_MAX_STRING_LEN - 1);                                          /* bad dirname's modify their argument */
    UPKLIST_PREPEND(pathlist);
    strncpy(pathlist->thisp->string, path, UPK_MAX_STRING_LEN - 1);

    while((pdir = dirname(buf)) && (strncmp("/", pdir, 2) != 0) && (*pdir != '\0')) {
        UPKLIST_PREPEND(pathlist);                                                       /* can use dlists as llists */
        strncpy(pathlist->thisp->string, pdir, UPK_MAX_STRING_LEN - 1);
    }

    UPKLIST_FOREACH(pathlist) {
        errno = 0;
        if(stat(pathlist->thisp->string, &st) == 0)
            continue;                                                                    /* some versions of mkdir will erroneously change
                                                                                            the permissions of existing directories; this
                                                                                            skips anything that exists */
        errno = 0;
        if(mkdir(pathlist->thisp->string, 0777) != 0) {
            if(errno != EEXIST) {
                upk_warn("cannot create directory: %s: %s\n", pathlist->thisp->string, strerror(errno));
            }
        } else {
            count++;
        }
    }

    UPKLIST_FREE(pathlist);

    return count;
}
