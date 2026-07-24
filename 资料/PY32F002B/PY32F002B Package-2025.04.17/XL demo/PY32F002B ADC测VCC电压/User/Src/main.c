#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"
#include "ADC_VCC.h"

int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
	User_LED_Init();
  User_USART1_Init();//串口初始化
  printf("hello\r\n");//串口测试

	ADC_Config();


  
  while (1)
  {
    ADC_Out();
  }
}

