#include "ADC.h"



//ADC参数配置
void ADC_Init(void)
{
    /*时钟配置*/
    RCC->APBRSTR2 |= (1<<20);//复位ADC时钟
    RCC->APBRSTR2 &= ~(1<<20);
    RCC->APBENR2 |= (1<<20);//使能ADC时钟
    /* ADC校准 */
    ADC1->CR |= (1UL<<31UL);//启动ADC校准
    while( (ADC1->CR&(1UL<<31UL))==1);//等待ADC校准完成

    /* ADC配置 */
    
    ADC1->CFGR1 |= (1<<12);//数据覆盖

    /* ADC时钟配置 */
    ADC1->CFGR2 |= (1<<28);//2分频

    /* ADC采样时间配置 */
    ADC1->SMPR |= (5<<0);
}

uint16_t ADC_Value[2];


void ADC_EN(void)
{
    HAL_Delay(300);//ADC再次启动时间需要大于8个ADC时钟

    /*ADC通道配置*/
    //配置通道需要在ADC启动之前
    ADC1->CHSELR |= (1<<0);//使能通道0
	ADC1->CHSELR |= (1<<4);//使能通道1
    /*启动ADC*/
    ADC1->CR |= (1<<0);//使能ADC ADEN位
    ADC1->CR |= (1<<2);//启动ADC ADSTART位

    for(uint8_t i=0 ;i<2 ;i++)
    {   
        while( (ADC1->ISR&(1<<2)&&(1<<3))==0);//等待转换结束
        ADC_Value[i] = ADC1->DR;
    }
    //到这里ADC会停止 可以变换通道
    printf("ADC0=%f\r\n",((3.3/4096)*ADC_Value[0]));
    printf("ADC1=%f\r\n",((3.3/4096)*ADC_Value[1]));
    
}

