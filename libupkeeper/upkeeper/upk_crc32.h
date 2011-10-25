
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

#ifndef _UPK_CRC32_H
#define _UPK_CRC32_H

#include "upk_std_include.h"

extern void             upk_create_crc32_table(uint32_t crc_table[256]);
extern uint32_t         upk_crc32(unsigned char *block, uint32_t len);
extern bool             upk_verify_crc32(unsigned char *block, uint32_t crc32, uint32_t len);

#endif
