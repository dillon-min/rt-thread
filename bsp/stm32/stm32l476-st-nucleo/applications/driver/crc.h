#ifndef __CMD_CRC_H__
#define __CMD_CRC_H__

#include <rthw.h>
#include <rtthread.h>
void ByteToHexStr(uint32_t source, char* dest);
uint32_t crc32(uint8_t* buf, uint32_t len);
#endif
