#include "ADC_VCC.h"


ADC_HandleTypeDef   ADC_Handle;
uint32_t            adc_value;
float               T_VCC;

void ADC_Config(void)
{
  __HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
  __HAL_RCC_ADC_CLK_ENABLE();

  /* ADC校准 */
  ADC_Handle.Instance = ADC1;
  /* ADC校准 */
  HAL_ADCEx_Calibration_Start(&ADC_Handle);                   

  ADC_Handle.Instance = ADC1;
  ADC_Handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;              /* 设置ADC时钟 */
  ADC_Handle.Init.Resolution = ADC_RESOLUTION_12B;                        /* 转换分辨率12bit */
  ADC_Handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;                        /* 数据右对齐 */
  ADC_Handle.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;              /* 设置ADC转换方向, 向上 */
  ADC_Handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                     /* ADC_EOC_SINGLE_CONV:单次采样 ; ADC_EOC_SEQ_CONV:序列采样 */
  ADC_Handle.Init.LowPowerAutoWait = ENABLE;                              /* 等待转换模式开启 */
  ADC_Handle.Init.ContinuousConvMode = DISABLE;                           /* 单次转换模式 */
  ADC_Handle.Init.DiscontinuousConvMode = DISABLE;                        /* 不使能非连续模式 */
  ADC_Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                  /* 软件触发 */
  ADC_Handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;   /* 触发边沿无 */
//  ADC_Handle.Init.DMAContinuousRequests = DISABLE;                        /* DMA不使能 */
  ADC_Handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                     /* 当过载发生时，覆盖上一个值 */
  ADC_Handle.Init.SamplingTimeCommon = ADC_SAMPLETIME_71CYCLES_5;         /* 设置采样周期41.5个ADC时钟 */
  /* ADC初始化 */
  HAL_ADC_Init(&ADC_Handle);

   /* 配置ADC通道 */
  ADC_ChannelConfTypeDef ADC_Channel;
  ADC_Channel.Channel = ADC_CHANNEL_VREFINT;                                 /* 设置采样通道 */
  ADC_Channel.Rank = ADC_RANK_CHANNEL_NUMBER;                                /* 设置加入规则组通道 */
  HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_Channel);                 
}

void ADC_Out(void)
{
    /* 启动ADC */
    HAL_ADC_Start(&ADC_Handle);

    /* 等待ADC转换完成 */	
    HAL_ADC_PollForConversion(&ADC_Handle, 10000000); 

    /* 获取ADC值 */
    adc_value = HAL_ADC_GetValue(&ADC_Handle); 

    /* 计算VCC电压 */	
    T_VCC = (4095 * 1.2) / adc_value;               
    printf("VCC:%f V\r\n", T_VCC);
    HAL_Delay(1000);
}

