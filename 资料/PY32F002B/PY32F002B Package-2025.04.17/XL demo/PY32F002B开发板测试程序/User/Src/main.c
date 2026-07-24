#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"


int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
	User_LED_Init();
  //User_USART1_Init();//串口初始化
 // printf("hello\r\n");//串口测试



  
  while (1)
  {
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
		HAL_Delay(300);
  }
}

