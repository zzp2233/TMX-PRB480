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

    /////////////////////////////////////////////////
    ADC1->CFGR1 |= (1<<13);  //连续转换模式
    /////////////////////////////////////////////////
    ADC1->CFGR1 |= (1<<2);  //扫描序列-->向下扫描    连续模式 必须开这个 从11-0扫描
    ////////////////////////////////////////////////

    ADC1->CFGR1 |= (1<<12);//数据覆盖

    /* ADC时钟配置 */
    ADC1->CFGR2 |= (1<<28);//2分频

    /* ADC采样时间配置 */
    ADC1->SMPR |= (5<<0);

    /*ADC通道配置*/
    //配置通道需要在ADC启动之前
    ADC1->CHSELR |= (1<<0);//使能通道0
    ADC1->CHSELR |= (1<<4);//使能通道4
    /*启动ADC*/
    ADC1->CR |= (1<<0);//使能ADC ADEN位
    ADC1->CR |= (1<<2);//启动ADC ADSTART位
}

uint16_t ADC_Value[2];


void ADC_EN(void)
{
    HAL_Delay(300);

    for(uint8_t i=0 ;i<2 ;i++)
    {
        while( ((ADC1->ISR&(1<<2))&(ADC1->ISR&(1<<3)))==0);//等待转换结束
        ADC_Value[i] = ADC1->DR;
    }
    printf("ADC4=%f\r\n",((3.3/4096)*ADC_Value[0]));  
    printf("ADC0=%f\r\n",((3.3/4096)*ADC_Value[1]));  

    //与单次模式的区别就是 ADC转换完成 不会停止 需要关闭ADC才能换通道
}

