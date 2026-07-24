#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"
#include "IR.h"
uint32_t System_Clock;
uint32_t AHB_Clock;
uint32_t APB_Clock;
int main(void)
{
  HAL_Init();//HAL库初始化
	
  User_System_Clock_Init();//系统时钟配置
	IR_Init();
  User_USART1_Init();//串口初始化
  Get_System_Clock();
	System_Clock = HAL_RCC_GetSysClockFreq();
	AHB_Clock = HAL_RCC_GetHCLKFreq();
	APB_Clock = HAL_RCC_GetPCLK1Freq();
  /*******************************************/
	
//	User_LED_Init();

 // printf("hello\r\n");//串口测试
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);

	
	
	


  
  while (1)
  {
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
//		delay_us(600);
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
//		delay_us(600);
  }
}

