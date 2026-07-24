#include "User_LPTIM.h"


/*****************************
 * 低功耗定时器初始化
******************************/
void User_LPTIM_Init(void)
{
    /* 低功耗定时器时钟配置 */
    RCC->CSR |= (1<<0);//使能LSI
    while( (RCC->CSR&(1<<1))==0);//等待HSI稳定
    RCC->CCIPR |= (1<<18);//低功耗定时器时钟选择LSI
    
    /* 低功耗定时器配置 */
    RCC->APBENR1 |= (1UL<<31UL);//使能低功耗定时器时钟


    LPTIM1->CFGR |= (7<<9);//128分频

    /* 低功耗定时器中断配置 */
    NVIC_SetPriority(LPTIM1_IRQn,2);//配置中断优先级
    NVIC_EnableIRQ(LPTIM1_IRQn);//使能中断
}

//启动低功耗定时器
void LPTIM_Start(void)
{
  LPTIM1->CFGR |= (1<<22);//寄存器在当前 LPTIM 周期结束时更新

  LPTIM1->IER |= (1<<1);//允许中断

  LPTIM1->CR |= (1<<0);//LPTIM使能

  LPTIM1->ARR = 2500;//自动重装载值 10S

  LPTIM1->CR |= (1<<1);//单次计数模式
}

//低功耗定时器中断函数
void LPTIM1_IRQHandler(void)
{
  LPTIM1->ICR |= (1<<1);//清除中断标记位
  User_System_Clock_Init();//重新配置系统时钟 其实002B没这个必要
  printf("MCU Run\r\n");
  /*重新配置XL2400P*/
//  RF_GPIO_Init();
//  RF_SPI_Init();
  
//  RF_Init();
//  RF_Tx_Mode();
  //RF_Reset();
  
}


















