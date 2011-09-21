/* ***************************************************************************
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

#include "upk_uuid.h"
#include <stdio.h>

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_uuid_seed_random(void)
{
    struct timeval          tv = { 0 };

    gettimeofday(&tv, 0);
    srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
int
upk_uuid_open_random(void)
{
    int                     fd = -2;

    if((fd = open("/dev/urandom", O_RDONLY)) == -1)
        fd = open("/dev/random", O_RDONLY);

    return fd;
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_gen_uuid_bytes(upk_uuid_t * buf)
{
    int32_t                 n = 0, fd = -2, nbuf;
    void                   *p = buf;

    if((fd = upk_uuid_open_random()) >= 0) {
        do {
            n = read(fd, buf, UPK_UUID_LEN);
        } while(n < UPK_UUID_LEN && (errno == EAGAIN || errno == EINTR));
        close(fd);
    }

    if(n != UPK_UUID_LEN) {
        for(p = buf; ((size_t) p - (size_t) buf) < UPK_UUID_LEN; p += sizeof(nbuf)) {
            nbuf = (int32_t) rand();
            memcpy(p, &nbuf, sizeof(nbuf));
        }
    }

    buf->time_high_and_version = (buf->time_high_and_version & 0x0FFF) | 0x4000;    /* version 4, random */
    buf->clk_seq_high = (buf->clk_seq_high & 0xBF) | 0x80; /* id, 8,9,A, or B */
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_uuid_to_string(char *buf, const upk_uuid_t * uuid)
{
    snprintf(buf, UPK_UUID_STRING_LEN + 1, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", uuid->time_low,
             uuid->time_mid, uuid->time_high_and_version, uuid->clk_seq_high, uuid->clk_seq_low, uuid->node[0],
             uuid->node[1], uuid->node[2], uuid->node[3], uuid->node[4], uuid->node[5]);
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
bool
is_valid_upk_uuid_string(const char *string)
{
    int32_t                 i = 0;
    const char             *p = string;
   
    for(i = 0, p = string; i < 36; i++, p++) {
        if((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
            if(*p != '-')
                return false;
        }
        else if(!c_isxdigit(*p))
            return false;
    }
    if(*p != '\0') 
        return false;

    return true;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
void
upk_string_to_uuid(const char *string, upk_uuid_t * uuid)
{
    int32_t                 n = 0;
    char                    buf[3] = "";
    const char             *p;

    if(!is_valid_upk_uuid_string(string))
        return;

    uuid->time_low = strtoul(string, NULL, 16);
    uuid->time_mid = strtoul(string + 9, NULL, 16);
    uuid->time_high_and_version = strtoul(string + 14, NULL, 16);

    buf[0] = *(string + 19);
    buf[1] = *(string + 20);
    uuid->clk_seq_high = strtoul(buf, NULL, 16);

    buf[0] = *(string + 21);
    buf[1] = *(string + 22);
    uuid->clk_seq_low = strtoul(buf, NULL, 16);

    p = string + 24;
    for(n = 0; n < 6; n++) {
        buf[0] = *p++;
        buf[1] = *p++;
        uuid->node[n] = strtoul(buf, NULL, 16);
    }
}
