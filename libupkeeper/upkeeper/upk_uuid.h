
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

#ifndef _UPK_UUID_H
#define _UPK_UUID_H

/** @file 
 *  @brief definition of upk uuid implementation.
 *
 *  Functions for version 4 UUIDs (random)
 */

#include "upk_std_include.h"

/**
  @addtogroup uuid
  @{
 */


/** @brief structure for holding 16 byte UUID.
 *
 *  Despite only using v4 uuids, field labels chosen to reflect convention
 *  but All bits save those for version and id are random; and do not contain
 *  the information the label might imply
 */
typedef struct _upk_uuid {
    uint32_t                time_low;                      /*!< low resolution bits, usually seconds since epoch
                                                              (random in this impl) */
    uint16_t                time_mid;                      /*!< mid resolution bits (random in this impl) */
    uint16_t                time_high_and_version;         /*!< high-resolution bits, and the version id of the uuid
                                                              (time_high is random in this impl; version is 0x40) */
    uint8_t                 clk_seq_high;                  /*!< clock sequence high bits (0x80 - 0xB0 in this impl) */
    uint8_t                 clk_seq_low;                   /*!< clock sequence low bits (random in this impl) */
    uint8_t                 node[6];                       /*!< unique, usually random number (random in this impl) */
} upk_uuid_t;

#define UPK_UUID_LEN sizeof(upk_uuid_t)                    /*! 16 */
#define UPK_UUID_STRING_LEN 36

/** 
 * @addtogroup uuid_functions 
 * @{ 
 */

/** *****************************************************************************************************************
   @brief seed random number pool used by upk_gen_uuid_bytes.

   Only necessary if /dev/urandom and /dev/random are unavailable; for instance, check upk_uuid_open_random to see if
   you get a valid fd; if not, call this (unless you have already seeded random elsewhere, in whatever manner you
   prefer
   ****************************************************************************************************************** */
extern void             upk_uuid_seed_random(void);

/** ******************************************************************************************************************
   @brief open random device, prefering /dev/urandom, but also trying /dev/random if urandom is unavailable.

   @return fd of opened device @return < 0 on error (check errno)
   ****************************************************************************************************************** */
extern int              upk_uuid_open_random(void);

/** ******************************************************************************************************************
   @brief collect and/or generate 16 bytes of random data.

   pack the structure you passed; also sets version correctly on structure to conform with spec

   @param[out] buf uuid structure to populate
   ****************************************************************************************************************** */
extern void             upk_gen_uuid_bytes(upk_uuid_t * buf);

/** ****************************************************************************************************************** 
   @brief convert uuid to string and place in buf.

   @param[out] buf string buffer (must be at least (UPK_UUID_STRING_LEN + 1) bytes long)

   @param[in] uuid uuid structure to convert
   ****************************************************************************************************************** */
extern void             upk_uuid_to_string(char *buf, const upk_uuid_t * uuid);

/** ******************************************************************************************************************
   @brief test if a given string is a valid uuid string.

   verify string is 36 chars long; and consists of a correctly hiphen-separated sequence of hexidecimal digits

   @param[in] string    string to check
   ****************************************************************************************************************** */
extern bool             is_valid_upk_uuid_string(const char *string);

/** ******************************************************************************************************************
   @brief convert uuid string into uuid structure.

   @param[in] string uuid string to convert

   @param[out] uuid pointer to buffer to populate
   ****************************************************************************************************************** */
extern void             upk_string_to_uuid(upk_uuid_t * uuid, const char *string);

/**
 * @}
 * @}
 */


#endif
