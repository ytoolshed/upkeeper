#ifndef _UPK_CRC32_H
#define _UPK_CRC32_H

static void upk_create_crc32_table(uint32_t crc_table[256]);
uint32_t upk_crc32(unsigned char *block, uint32_t len);
bool upk_verify_crc32(unsigned char *block, uint32_t crc32, uint32_t len);

#endif
