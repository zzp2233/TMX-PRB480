#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"
#include "User_LPTIM.h"
#include "User_STOP_Mode.h"

void Exit8_Init(void);
int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
//	User_LED_Init();
  //User_USART1_Init();//串口初始化
 // printf("hello\r\n");//串口测试


	Exit8_Init();
	HAL_Delay(3000);
	STOP_EN();



  while (1)
  {
	HAL_Delay(3000);
	STOP_EN();
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
  
  /* GPIO initialization */
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
//	RCC->IOPENR |= (1<<0); //使能GPIOA时钟
//    /* PA8 */
//    GPIOA->MODER &= ~(3<<16);//输入模式
//    GPIOA->PUPDR |= (1<<16);//上拉输入

//    EXTI->IMR |= (1<<8); //中断
//    EXTI->FTSR |= (1<<8);//下降沿触发

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



