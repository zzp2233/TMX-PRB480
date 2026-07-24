#include "User_System_Clock_Config.h"


//系统时钟配置
void User_System_Clock_Init(void)
{
    /*OSC配置*/
    /*002B上电以后默认使用HSI作为系统时钟 24Mhz*/

    /*系统时钟配置*/
    //系统时钟来源HSI 24Mhz
    //AHB总线不分频 24Mhz
    RCC->CFGR |= (4<<12);//APB总线2分频 12Mhz
}

//打印系统时钟频率
void Get_System_Clock(void)
{
	printf("System_Clock:%dMHz\r\n",HAL_RCC_GetSysClockFreq()/1000000);  //打印系统时钟
	printf("AHB_Clock:%dMHz\r\n",HAL_RCC_GetHCLKFreq()/1000000);         //打印AHB时钟
	printf("APB_Clock:%dMHz\r\n",HAL_RCC_GetPCLK1Freq()/1000000);        //打印APB时钟
}



