
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

/* ***************************************************************************
   ************************************************************************* */
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

/* ***************************************************************************
   ************************************************************************* */
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

/* ***************************************************************************
   ************************************************************************* */
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

/* ***************************************************************************
   ************************************************************************* */
struct timeval
upk_double_to_timeval(long double r)
{
    struct timeval          val = { ((int) r), ((int) ((r - (long double) ((int) r)) * 1000000)) };
    return val;
}
