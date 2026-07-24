#include "User_UART1.h"
/****************************
 * USART1_TX---->PA3
 * USART1_RX---->PA4
 * 002B只有一个串口USART1
****************************/
uint8_t G_UART1_RecBuffer[UART1_Rec_SIZE];//接收数据缓冲区
uint8_t UART1_Rec_Count=0;//接收计数
uint8_t G_UART1_RecFlag=0;//空闲中断标志 也可以理解为接收完成标志

UART_HandleTypeDef USART1_Handle;//串口句柄

/*串口MSP回调函数*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)//如果基地址是USART1
    {
        __HAL_RCC_GPIOA_CLK_ENABLE(); //使能GPIOA时钟
        __HAL_RCC_USART1_CLK_ENABLE();//使能串口时钟

        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Pin       = GPIO_PIN_3|GPIO_PIN_4; //PA3 PA4
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;      //复用模式
        GPIO_InitStruct.Pull      = GPIO_PULLUP;           //上拉
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH; //超高速
        GPIO_InitStruct.Alternate = GPIO_AF1_USART1;        //AF1_USART1
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        /* 串口中断使能 */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}
/*串口参数配置*/
void User_USART1_Init(void)
{

    USART1_Handle.Instance          = USART1;
    USART1_Handle.Init.BaudRate     = 115200;
    USART1_Handle.Init.WordLength   = UART_WORDLENGTH_8B;
    USART1_Handle.Init.StopBits     = UART_STOPBITS_1;
    USART1_Handle.Init.Parity       = UART_PARITY_NONE;
    USART1_Handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    USART1_Handle.Init.Mode         = UART_MODE_TX_RX;
    HAL_UART_Init(&USART1_Handle);


    USART1->CR1 |= (1<<2);//接收使能
    USART1->CR1 |= (1<<3);//发送使能
    USART1->CR1 |= (1<<4);//空闲中断使能
    USART1->CR1 |= (1<<5);//接收中断使能

    USART1->DR = 0;
}

//串口发送数据函数（1字节）
void USART1_Send_Byte(uint8_t dat)
{
    while( (USART1->SR&(1<<6)) ==0);//等待上一个数据发送完成
    USART1->DR = dat;//将数据写入DR寄存器
}

//重定向printf
int fputc(int ch, FILE *f)
{
  USART1_Send_Byte(ch);
  return ch;
}

void USART1_IRQHandler(void)
{
    if( (USART1->SR & (1<<5)) != 0)//产生接收中断 
    {
        G_UART1_RecBuffer[UART1_Rec_Count]= (uint8_t)USART1->DR;//将数据寄存器的值转移
        UART1_Rec_Count++;
        /*清除中断标志位*/
		USART1->SR &= ~(1<<5);
    }
    if( (USART1->SR & (1<<4)) != 0)//产生空闲中断  
    {
        G_UART1_RecFlag=1;//接收完成标志置1
        /*清除中断标志位*/
        USART1->SR;
        USART1->DR;
    }

}


/*********************串口测试程序***********************/
void User_USART1_Test(void)
{
    if(G_UART1_RecFlag)//产生空闲中断
    {
        uint16_t Len = strlen((char *)G_UART1_RecBuffer);//计算出接收到的数据长度
        for(uint8_t i=0; i<Len ; i++)
        {
            printf("%c" , G_UART1_RecBuffer[i]);//将接收到的数据打印出来
        }
        printf("\r\n");
        memset(G_UART1_RecBuffer , 0 , Len);//清空接收缓冲区
        G_UART1_RecFlag=0;//接收完成标志归0
        UART1_Rec_Count=0;//接收计数归0
    }
}






