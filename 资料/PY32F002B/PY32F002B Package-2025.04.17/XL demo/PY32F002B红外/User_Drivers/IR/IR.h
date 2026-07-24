#ifndef IR__H
#define IR__H

#include "main.h"


#define IR_INPUT   HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)

extern int8_t ir_flag;
extern uint8_t ir_buff[4];


void IR_Init(void);
void delay_us(uint32_t Delay);
int8_t IR_Read(uint8_t *rdata);








#endif





