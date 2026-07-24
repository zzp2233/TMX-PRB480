#include "User_IWDG.h"

void IWDG_Init(void)
{
	RCC->CSR |= (1<<0);//使能LSI
	
	IWDG ->KR = 0X5555;  /* 使能对IWDG->PR和IWDG->RLR的写 */
	IWDG ->PR |= 101 << 0 ;       /* 设置分频系数 */
	
	//IWDG ->SR &= ~(1<<1);
	IWDG ->RLR = 1536;  /* 从加载寄存器 IWDG->RLR */
	IWDG ->KR = 0XAAAA;  /* 喂狗 */
	IWDG ->KR = 0XCCCC;  /* 使能看门狗 */
}


void IWDG_Feed(void)
{
    IWDG->KR = 0XAAAA;  /* 喂狗 */
}
