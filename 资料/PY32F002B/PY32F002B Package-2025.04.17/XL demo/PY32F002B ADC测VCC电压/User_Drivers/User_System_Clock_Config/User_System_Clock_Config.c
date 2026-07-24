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


