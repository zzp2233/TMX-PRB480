#ifndef _CRC16_H
#define _CRC16_H

#include "system.h"

u16 Cal_Crc16(u8* dat, u8 num);
u16 Cal_Crc16_bit(u16 crc, u8 dat);

#endif
