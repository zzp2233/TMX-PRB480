/**
  ******************************************************************************
  * @file    py32ca70_ll_pwm.c
  * @author  MCU Application Team
  * @brief   PWM LL module driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by Puya under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#if defined(USE_FULL_LL_DRIVER)

/* Includes ------------------------------------------------------------------*/
#include "py32ca70_ll_pwm.h"
#include "py32ca70_ll_bus.h"
#include "py32ca70_ll_rcc.h"

#ifdef  USE_FULL_ASSERT
#include "ms32_assert.h"
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/** @addtogroup PY32CA70_LL_Driver
  * @{
  */

#if defined (PWM) || defined(PWM1)

/** @addtogroup PWM_LL
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @addtogroup PWM_LL_Private_Macros
  * @{
  */
#define IS_LL_PWM_COUNTERMODE(__VALUE__) (((__VALUE__) == LL_PWM_COUNTERMODE_UP) \
                                          || ((__VALUE__) == LL_PWM_COUNTERMODE_DOWN) \
                                          || ((__VALUE__) == LL_PWM_COUNTERMODE_CENTER_UP) \
                                          || ((__VALUE__) == LL_PWM_COUNTERMODE_CENTER_DOWN) \
                                          || ((__VALUE__) == LL_PWM_COUNTERMODE_CENTER_UP_DOWN))


#define IS_LL_PWM_OCMODE(__VALUE__) (((__VALUE__) == LL_PWM_OCMODE_PWM1) \
                                     || ((__VALUE__) == LL_PWM_OCMODE_PWM2))

#define IS_LL_PWM_OCSTATE(__VALUE__) (((__VALUE__) == LL_PWM_OCSTATE_DISABLE) \
                                      || ((__VALUE__) == LL_PWM_OCSTATE_ENABLE))

#define IS_LL_PWM_OCPOLARITY(__VALUE__) (((__VALUE__) == LL_PWM_OCPOLARITY_HIGH) \
                                         || ((__VALUE__) == LL_PWM_OCPOLARITY_LOW))
/**
  * @}
  */


/* Private function prototypes -----------------------------------------------*/
/** @defgroup PWM_LL_Private_Functions PWM Private Functions
  * @{
  */
static ErrorStatus OC1Config(PWM_TypeDef *PWMx, LL_PWM_OC_InitTypeDef *PWM_OCInitStruct);
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWM_LL_Exported_Functions
  * @{
  */

/** @addtogroup PWM_LL_EF_Init
  * @{
  */

/**
  * @brief  Set PWMx registers to their reset values.
  * @param  PWMx Pwm instance
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: PWMx registers are de-initialized
  *          - ERROR: invalid PWMx instance
  */
ErrorStatus LL_PWM_DeInit(PWM_TypeDef *PWMx)
{
  ErrorStatus result = SUCCESS;

  /* Check the parameters */
  assert_param(IS_PWM_INSTANCE(PWMx));

  if (PWMx == PWM1)
  {
    LL_APB1_GRP2_ForceReset(LL_APB1_GRP2_PERIPH_PWM1);
    LL_APB1_GRP2_ReleaseReset(LL_APB1_GRP2_PERIPH_PWM1);
  }
  else
  {
    result = ERROR;
  }

  return result;
}

/**
  * @brief  Set the fields of the time base unit configuration data structure
  *         to their default values.
  * @param  PWM_InitStruct pointer to a @ref LL_PWM_InitTypeDef structure (time base unit configuration data structure)
  * @retval None
  */
void LL_PWM_StructInit(LL_PWM_InitTypeDef *PWM_InitStruct)
{
  /* Set the default configuration */
  PWM_InitStruct->Prescaler         = (uint16_t)0x00;
  PWM_InitStruct->CounterMode       = LL_PWM_COUNTERMODE_UP;
  PWM_InitStruct->Autoreload        = 0x3FFU;
}

/**
  * @brief  Configure the PWMx time base unit.
  * @param  PWMx Pwm Instance
  * @param  PWM_InitStruct pointer to a @ref LL_PWM_InitTypeDef structure (PWMx time base unit configuration data structure)
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: PWMx registers are de-initialized
  *          - ERROR: not applicable
  */
ErrorStatus LL_PWM_Init(PWM_TypeDef *PWMx, LL_PWM_InitTypeDef *PWM_InitStruct)
{
  uint32_t tmpcr1;

  /* Check the parameters */
  assert_param(IS_PWM_INSTANCE(PWMx));
  assert_param(IS_LL_PWM_COUNTERMODE(PWM_InitStruct->CounterMode));

  tmpcr1 = LL_PWM_ReadReg(PWMx, CR1);
  
  if (IS_PWM_COUNTER_MODE_SELECT_INSTANCE(PWMx))
  {
    /* Select the Counter Mode */
    MODIFY_REG(tmpcr1, (PWM_CR1_DIR | PWM_CR1_CMS), PWM_InitStruct->CounterMode);
  }

  /* Write to PWMx CR1 */
  LL_PWM_WriteReg(PWMx, CR1, tmpcr1);

  LL_PWM_SetAutoReload(PWMx, PWM_InitStruct->Autoreload);

  LL_PWM_SetPrescaler(PWMx, PWM_InitStruct->Prescaler);

  /* Generate an update event to reload the Prescaler
     and the repetition counter value (if applicable) immediately */
  LL_PWM_GenerateEvent_UPDATE(PWMx);

  return SUCCESS;
}

/**
  * @brief  Set the fields of the PWMx output channel configuration data
  *         structure to their default values.
  * @param  PWM_OC_InitStruct pointer to a @ref LL_PWM_OC_InitTypeDef structure (the output channel configuration data structure)
  * @retval None
  */
void LL_PWM_OC_StructInit(LL_PWM_OC_InitTypeDef *PWM_OC_InitStruct)
{
  /* Set the default configuration */
  PWM_OC_InitStruct->OCMode       = LL_PWM_OCMODE_PWM1;
  PWM_OC_InitStruct->OCState      = LL_PWM_OCSTATE_DISABLE;
  PWM_OC_InitStruct->CompareValue = 0x00000000U;
  PWM_OC_InitStruct->OCPolarity   = LL_PWM_OCPOLARITY_HIGH;
}

/**
  * @brief  Configure the PWMx output channel.
  * @param  PWMx Pwm Instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @param  PWM_OC_InitStruct pointer to a @ref LL_PWM_OC_InitTypeDef structure (PWMx output channel configuration data structure)
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: PWMx output channel is initialized
  *          - ERROR: PWMx output channel is not initialized
  */
ErrorStatus LL_PWM_OC_Init(PWM_TypeDef *PWMx, uint32_t Channel, LL_PWM_OC_InitTypeDef *PWM_OC_InitStruct)
{
  ErrorStatus result = ERROR;

  switch (Channel)
  {
    case LL_PWM_CHANNEL_CH1:
      result = OC1Config(PWMx, PWM_OC_InitStruct);
      break;
    default:
      break;
  }

  return result;
}
/**
  * @}
  */

/**
  * @}
  */

/* Private functions --------------------------------------------------------*/
/** @addtogroup PWM_LL_Private_Functions PWM Private Functions
  *  @brief   Private functions
  * @{
  */
/**
  * @brief  Configure the PWMx output channel 1.
  * @param  PWMx Pwm Instance
  * @param  PWM_OCInitStruct pointer to the the PWMx output channel 1 configuration data structure
  * @retval An ErrorStatus enumeration value:
  *          - SUCCESS: PWMx registers are de-initialized
  *          - ERROR: not applicable
  */
static ErrorStatus OC1Config(PWM_TypeDef *PWMx, LL_PWM_OC_InitTypeDef *PWM_OCInitStruct)
{
  uint32_t tmpccmr1;
  uint32_t tmpccer;

  /* Check the parameters */
  assert_param(IS_PWM_OC1_INSTANCE(PWMx));
  assert_param(IS_LL_PWM_OCMODE(PWM_OCInitStruct->OCMode));
  assert_param(IS_LL_PWM_OCSTATE(PWM_OCInitStruct->OCState));
  assert_param(IS_LL_PWM_OCPOLARITY(PWM_OCInitStruct->OCPolarity));

  /* Disable the Channel 1: Reset the C1E Bit */
  CLEAR_BIT(PWMx->CER, PWM_CER_C1E);

  /* Get the PWMx CER register value */
  tmpccer = LL_PWM_ReadReg(PWMx, CER);

  /* Get the PWMx CMR register value */
  tmpccmr1 = LL_PWM_ReadReg(PWMx, CMR);

  /* Set the Output Compare Mode */
  MODIFY_REG(tmpccmr1, PWM_CMR_OC1M, PWM_OCInitStruct->OCMode);

  /* Set the Output Compare Polarity */
  MODIFY_REG(tmpccer, PWM_CER_C1P, PWM_OCInitStruct->OCPolarity);

  /* Set the Output State */
  MODIFY_REG(tmpccer, PWM_CER_C1E, PWM_OCInitStruct->OCState);

  /* Write to PWMx CMR */
  LL_PWM_WriteReg(PWMx, CMR, tmpccmr1);
  
  LL_PWM_OC_SetCompareCH1(PWMx, PWM_OCInitStruct->CompareValue);
  
  /* Write to PWMx CER */
  LL_PWM_WriteReg(PWMx, CER, tmpccer);

  return SUCCESS;
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* PWM */

/**
  * @}
  */

#endif /* USE_FULL_LL_DRIVER */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
