#include "CRC16.h"

uint16_t Cal_Crc16_bit( uint16_t crc, uint8_t dat )
{
  uint16_t temp  = 0;
  temp   = (crc & 1) ^ (dat & 1);	
  crc  = (crc & 0xfffd) + ((crc & 0x0002) ^ (temp << 1));
  crc  = (crc & 0xbfff) + ((crc & 0x4000) ^ (temp << 14));
  crc  =  crc >> 1;
  crc  = (crc & 0x7fff) + (temp << 15);
  return crc;
}

/********************************************************************
*功能：crc16计算
*输入参数：dat,数据;num,字节数
*输出参数：crc
********************************************************************/
uint16_t Cal_Crc16( uint8_t* dat, uint8_t num ) 
{
  uint8_t i     = 0;
  uint8_t j     = 0;
  uint16_t crc16 = 0;
  
  for ( i = 0; i < num; i++ ) 
  {
    for ( j = 0; j < 8; j++ ) 
      crc16 = Cal_Crc16_bit( crc16, dat[i]>>j );
  }
  return ~crc16;
}


