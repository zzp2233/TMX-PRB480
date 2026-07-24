#ifndef __USER_USART1_H__
#define __USER_USART1_H__
//--------------------------------
#include "main.h"
//--------------------------------
void User_USART1_Init(void);
void User_USART1_Test(void);
//----------------------------------
#define USART1_Rec_SIZE 16 //接收缓冲区的大小
//-------------------------------------------------------------------------------------
extern uint8_t G_UART1_RecBuffer[USART1_Rec_SIZE];//接收数据缓冲区
extern uint8_t G_UART1_RecFlag;//空闲中断标志 也可以理解为接收完成标志







#endif

