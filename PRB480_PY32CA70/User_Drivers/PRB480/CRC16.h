#ifndef _CRC16_H
#define _CRC16_H

#include "PRB480.h"

u16 Cal_Crc16(u8* dat, u8 num);
u16 Cal_Crc16_bit(u16 crc, u8 dat);

#endif

