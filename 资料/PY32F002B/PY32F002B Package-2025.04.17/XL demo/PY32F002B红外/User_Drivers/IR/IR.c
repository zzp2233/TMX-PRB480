#include "IR.h"


void delay_us(uint32_t Delay)
{
	uint32_t cnt = Delay * 5;   // 32Mhz ,其他频率其他倍数
	uint32_t i = 0;
	for(i = 0; i < cnt; i++)__NOP();
}


int8_t ir_flag;
uint8_t ir_buff[4];


void IR_Init(void){

	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();                          /* Enable GPIOA clock */

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;            /* Push-pull output */
	GPIO_InitStruct.Pull = GPIO_PULLUP;                    /* Enable pull-up */
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;          /* GPIO speed */  
	/* GPIO initialization */
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 	
	
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	
}

int8_t IR_Read(uint8_t *rdata){
	uint32_t cnt;
	uint8_t i,j;
	uint8_t rx;
	if(IR_INPUT == 0){
		//检测低电平的有效性 9ms
		cnt = 0;
		while(!IR_INPUT)
		{
			cnt++;
			delay_us(10);
			if(cnt > 1000){ //不合法，超时退出
				return -1;
			}
		}
		cnt =0;
		//检测高电平的有效性 4.5ms
		while(IR_INPUT)
		{
			cnt++;
			delay_us(10);
			if(cnt > 550){ //不合法，超时退出
				return -2;
			}
		}
		
		
		//前面引导码正确，下面开始接收！！！！！！！！！！！！！！！！！！！
		//接收四个字节数据

		for(i=0; i<4; i++){
			for(j=0; j<8; j++){
				cnt =0;
				while(!IR_INPUT)//检测560us低电平
				{
					cnt++;
					delay_us(10);
					if(cnt > 62){ //不合法，超时退出
						return -3;
					}
				}	
				delay_us(600);//延时600us后 判断是高还是低 高电平为1 低电平为0
				if(IR_INPUT)
				{
					rx |= 1<<j;
					cnt = 0;
					while(IR_INPUT){
						cnt++;
						delay_us(10);
						if(cnt > 100){
							return -4;
						}
						
					}
				}
			}
			ir_buff[i] = rx;
			rx = 0;	
		}
		delay_us(600);
//		if(ir_buff[0] + ir_buff[1] == 255)//验证数据的准确性
//			if(ir_buff[2] + ir_buff[3] == 255)
//				return 1;
		
		
	}
	return 1;
//	return -5;
}


void EXTI0_1_IRQHandler(void){
	ir_flag = IR_Read(ir_buff);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	if(ir_flag < 0)
		return;
	uint8_t k =4;
	while(k--){
		printf("%d",ir_buff[4-k]);
	}
}



