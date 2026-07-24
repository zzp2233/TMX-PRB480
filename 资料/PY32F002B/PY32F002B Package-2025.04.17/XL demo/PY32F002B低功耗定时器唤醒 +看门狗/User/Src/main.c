#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"
#include "User_LPTIM.h"
#include "User_STOP_Mode.h"
#include "User_IWDG.h"

//低功耗定时器10s唤醒一次，唤醒3s后继续进入低功耗，也可以外部中断PA4唤醒


void Exit8_Init(void);
int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
//   User_LED_Init();
//   User_USART1_Init();//串口初始化
//   printf("hello\r\n");//串口测试


	Exit8_Init();
	HAL_Delay(3000);
	User_LPTIM_Init();
	HAL_Delay(5);
//	LPTIM_Start();
//	STOP_EN();
    IWDG_Init();


  while (1)
  {
	  
	  HAL_Delay(3000);
//	  STOP_EN();
	  //LED_Test();
// 	  printf("STOP\r\n");
	  LPTIM_Start();//启动低功耗定时器
	  
      STOP_EN();//进入低功耗模式
	  IWDG_Feed();
  }
}




//外部中断配置
void Exit8_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();                          /* Enable GPIOA clock */

  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;            /* Push-pull output */
  GPIO_InitStruct.Pull = GPIO_PULLUP;                    /* Enable pull-up */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;          /* GPIO speed */  
  

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 


    NVIC_SetPriority(EXTI4_15_IRQn,1);
    /* 使能中断 */
    NVIC_EnableIRQ(EXTI4_15_IRQn);	
}

//中断服务函数
void EXTI4_15_IRQHandler(void)
{
    //LPTIM1->CR &= ~(1<<0);//关闭LPTIM
    User_System_Clock_Init();//重新配置时钟
    //printf("MCU Run\r\n");
    EXTI->PR |= (1<<4);//清除中断标记位

}


