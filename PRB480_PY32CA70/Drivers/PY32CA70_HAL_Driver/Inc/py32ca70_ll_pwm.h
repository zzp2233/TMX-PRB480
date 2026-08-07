/**
  ******************************************************************************
  * @file    py32ca70_ll_pwm.h
  * @author  MCU Application Team
  * @brief   Header file of PWM LL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PY32CA70_LL_PWM_H
#define __PY32CA70_LL_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32ca7xx.h"

/** @addtogroup PY32CA70_LL_Driver
  * @{
  */

#if defined (PWM) || defined(PWM1)

/** @defgroup PWM_LL PWM
  * @{
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup PWM_LL_Private_Variables PWM Private Variables
  * @{
  */
static const uint8_t OFFSET_TAB_CMR[] =
{
  0x00U,         /* 0: PWMx_CH1  */
};

static const uint8_t SHIFT_TAB_OCxM[] =
{
  0U,            /* 0: OC1M */
};

static const uint8_t SHIFT_TAB_OCxPE[] =
{
  0U,            /* 0: OC1PE */
};

static const uint8_t SHIFT_TAB_CxP[] =
{
  0U,            /* 0: C1P */
};

/**
  * @}
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup PWM_LL_Private_Constants PWM Private Constants
  * @{
  */

/* Defines used for the bit position in the register and perform offsets */
#define PWM_POSITION_BRK_SOURCE            (POSITION_VAL(Source) & 0x1FUL)

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup PWM_LL_Private_Macros PWM Private Macros
  * @{
  */
/** @brief  Convert channel id into channel index.
  * @param  __CHANNEL__ This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1

  * @retval none
  */
#define PWM_GET_CHANNEL_INDEX( __CHANNEL__) \
  (((__CHANNEL__) == LL_PWM_CHANNEL_CH1) ? 0U :0U)
/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup PWM_LL_ES_INIT PWM Exported Init structure
  * @{
  */

/**
  * @brief  PWM Time Base configuration structure definition.
  */
typedef struct
{
  uint16_t Prescaler;         /*!< Specifies the prescaler value used to divide the PWM clock.
                                   This parameter can be a number between Min_Data=0x00 and Max_Data=0xFF.

                                   This feature can be modified afterwards using unitary function @ref LL_PWM_SetPrescaler().*/

  uint32_t CounterMode;       /*!< Specifies the counter mode.
                                   This parameter can be a value of @ref PWM_LL_EC_COUNTERMODE.

                                   This feature can be modified afterwards using unitary function @ref LL_PWM_SetCounterMode().*/

  uint32_t Autoreload;        /*!< Specifies the auto reload value to be loaded into the active
                                   Auto-Reload Register at the next update event.
                                   This parameter must be a number between Min_Data=0x0000 and Max_Data=0x3FF.

                                   This feature can be modified afterwards using unitary function @ref LL_PWM_SetAutoReload().*/
} LL_PWM_InitTypeDef;

/**
  * @brief  PWM Output Compare configuration structure definition.
  */
typedef struct
{
  uint32_t OCMode;        /*!< Specifies the output mode.
                               This parameter can be a value of @ref PWM_LL_EC_OCMODE.

                               This feature can be modified afterwards using unitary function @ref LL_PWM_OC_SetMode().*/

  uint32_t OCState;       /*!< Specifies the PWM Output Compare state.
                               This parameter can be a value of @ref PWM_LL_EC_OCSTATE.

                               This feature can be modified afterwards using unitary functions @ref LL_PWM_OC_EnableChannel() or @ref LL_PWM_OC_DisableChannel().*/


  uint32_t CompareValue;  /*!< Specifies the Compare value to be loaded into the Capture Compare Register.
                               This parameter can be a number between Min_Data=0x0000 and Max_Data=0x3FF.

                               This feature can be modified afterwards using unitary function LL_PWM_OC_SetCompareCHx (x=1).*/

  uint32_t OCPolarity;    /*!< Specifies the output polarity.
                               This parameter can be a value of @ref PWM_LL_EC_OCPOLARITY.

                               This feature can be modified afterwards using unitary function @ref LL_PWM_OC_SetPolarity().*/

} LL_PWM_OC_InitTypeDef;

/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/* Exported constants --------------------------------------------------------*/
/** @defgroup PWM_LL_Exported_Constants PWM Exported Constants
  * @{
  */

/** @defgroup PWM_LL_EC_GET_FLAG Get Flags Defines
  * @brief    Flags defines which can be used with LL_PWM_ReadReg function.
  * @{
  */
#define LL_PWM_SR_UIF                          PWM_SR_UIF           /*!< Update interrupt flag */
#define LL_PWM_SR_OC1IF                        PWM_SR_OC1IF         /*!< compare 1 interrupt flag */
#define LL_PWM_SR_LED1IF                       PWM_SR_LED1_ENDIF    /*!< LED1 data transmission completed interrupt flag */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_IT IT Defines
  * @brief    IT defines which can be used with LL_PWM_ReadReg and  LL_PWM_WriteReg functions.
  * @{
  */
#define LL_PWM_DIER_UIE                        PWM_DIER_UIE         /*!< Update interrupt enable */
#define LL_PWM_DIER_OC1IE                      PWM_DIER_OC1IE       /*!< compare 1 interrupt enable */
#define LL_PWM_DIER_LED1IE                     PWM_DIER_LED1_ENDIE  /*!< LED1 data transmission completed interrupt enable */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_UPDATESOURCE Update Source
  * @{
  */
#define LL_PWM_UPDATESOURCE_REGULAR            0x00000000U          /*!< Counter overflow/underflow, Setting the UG bit or Update generation through the slave mode controller generates an update request */
#define LL_PWM_UPDATESOURCE_COUNTER            PWM_CR1_URS          /*!< Only counter overflow/underflow generates an update request */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_COUNTERMODE Counter Mode
  * @{
  */
#define LL_PWM_COUNTERMODE_UP                  0x00000000U          /*!<Counter used as upcounter */
#define LL_PWM_COUNTERMODE_DOWN                PWM_CR1_DIR          /*!< Counter used as downcounter */
#define LL_PWM_COUNTERMODE_CENTER_DOWN         PWM_CR1_CMS_0        /*!< The counter counts up and down alternatively. Output compare interrupt flags of output channels  are set only when the counter is counting down. */
#define LL_PWM_COUNTERMODE_CENTER_UP           PWM_CR1_CMS_1        /*!<The counter counts up and down alternatively. Output compare interrupt flags of output channels  are set only when the counter is counting up */
#define LL_PWM_COUNTERMODE_CENTER_UP_DOWN      PWM_CR1_CMS          /*!< The counter counts up and down alternatively. Output compare interrupt flags of output channels  are set only when the counter is counting up or down. */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_COUNTERDIRECTION Counter Direction
  * @{
  */
#define LL_PWM_COUNTERDIRECTION_UP             0x00000000U          /*!< Pwm counter counts up */
#define LL_PWM_COUNTERDIRECTION_DOWN           PWM_CR1_DIR          /*!< Pwm counter counts down */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_LED_DATAWIDTH LED Data Length
  * @{
  */
#define LL_PWM_LED_DATALENGTH_8B            0x00000000U                /*!< Data length : 8bit */
#define LL_PWM_LED_DATALENGTH_16B           PWM_CR1_LEDWORD_0          /*!< Data length : 16bit */
#define LL_PWM_LED_DATALENGTH_24B           PWM_CR1_LEDWORD_1          /*!< Data length : 24bit */
/**
  * @}
  */

/** @defgroup PWM_LL_EC_CHANNEL Channel
  * @{
  */
#define LL_PWM_CHANNEL_CH1                     PWM_CER_C1E     /*!< Pwm input/output channel 1 */
/**
  * @}
  */

#if defined(USE_FULL_LL_DRIVER)
/** @defgroup PWM_LL_EC_OCSTATE Output Configuration State
  * @{
  */
#define LL_PWM_OCSTATE_DISABLE                 0x00000000U             /*!< OCx is not active */
#define LL_PWM_OCSTATE_ENABLE                  PWM_CER_C1E           /*!< OCx signal is output on the corresponding output pin */
/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

/** @defgroup PWM_LL_EC_OCMODE Output Configuration Mode
  * @{
  */
#define LL_PWM_OCMODE_PWM1                     (PWM_CMR_OC1M_1)                    /*!<In upcounting, channel y is active as long as PWMx_CNT<PWMx_CCRy else inactive.  In downcounting, channel y is inactive as long as PWMx_CNT>PWMx_CCRy else active.*/
#define LL_PWM_OCMODE_PWM2                     (PWM_CMR_OC1M_1 | PWM_CMR_OC1M_0)   /*!<In upcounting, channel y is inactive as long as PWMx_CNT<PWMx_CCRy else active.  In downcounting, channel y is active as long as PWMx_CNT>PWMx_CCRy else inactive*/
/**
  * @}
  */

/** @defgroup PWM_LL_EC_OCPOLARITY Output Configuration Polarity
  * @{
  */
#define LL_PWM_OCPOLARITY_HIGH                 0x00000000U                 /*!< OCxactive high*/
#define LL_PWM_OCPOLARITY_LOW                  PWM_CER_C1P                 /*!< OCxactive low*/
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/** @defgroup PWM_LL_Exported_Macros PWM Exported Macros
  * @{
  */

/** @defgroup PWM_LL_EM_WRITE_READ Common Write and read registers Macros
  * @{
  */
/**
  * @brief  Write a value in PWM register.
  * @param  __INSTANCE__ PWM Instance
  * @param  __REG__ Register to be written
  * @param  __VALUE__ Value to be written in the register
  * @retval None
  */
#define LL_PWM_WriteReg(__INSTANCE__, __REG__, __VALUE__) WRITE_REG((__INSTANCE__)->__REG__, (__VALUE__))

/**
  * @brief  Read a value in PWM register.
  * @param  __INSTANCE__ PWM Instance
  * @param  __REG__ Register to be read
  * @retval Register value
  */
#define LL_PWM_ReadReg(__INSTANCE__, __REG__) READ_REG((__INSTANCE__)->__REG__)
/**
  * @}
  */

/** @defgroup PWM_LL_EM_Exported_Macros Exported_Macros
  * @{
  */

/**
  * @brief  HELPER macro calculating the prescaler value to achieve the required counter clock frequency.
  * @note ex: @ref __LL_PWM_CALC_PSC (80000000, 1000000);
  * @param  __PWMCLK__ pwm input clock frequency (in Hz)
  * @param  __CNTCLK__ counter clock frequency (in Hz)
  * @retval Prescaler value  (between Min_Data=0 and Max_Data=65535)
  */
#define __LL_PWM_CALC_PSC(__PWMCLK__, __CNTCLK__)   \
  (((__PWMCLK__) >= (__CNTCLK__)) ? (uint32_t)(((__PWMCLK__)/(__CNTCLK__)) - 1U) : 0U)

/**
  * @brief  HELPER macro calculating the auto-reload value to achieve the required output signal frequency.
  * @note ex: @ref __LL_PWM_CALC_ARR (1000000, @ref LL_PWM_GetPrescaler (), 10000);
  * @param  __PWMCLK__ pwm input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __FREQ__ output signal frequency (in Hz)
  * @retval  Auto-reload value  (between Min_Data=0 and Max_Data=65535)
  */
#define __LL_PWM_CALC_ARR(__PWMCLK__, __PSC__, __FREQ__) \
  ((((__PWMCLK__)/((__PSC__) + 1U)) >= (__FREQ__)) ? (((__PWMCLK__)/((__FREQ__) * ((__PSC__) + 1U))) - 1U) : 0U)

/**
  * @brief  HELPER macro calculating the compare value required to achieve the required pwm output compare active/inactive delay.
  * @note ex: @ref __LL_PWM_CALC_DELAY (1000000, @ref LL_PWM_GetPrescaler (), 10);
  * @param  __PWMCLK__ pwm input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ pwm output compare active/inactive delay (in us)
  * @retval Compare value  (between Min_Data=0 and Max_Data=65535)
  */
#define __LL_PWM_CALC_DELAY(__PWMCLK__, __PSC__, __DELAY__)  \
  ((uint32_t)(((uint64_t)(__PWMCLK__) * (uint64_t)(__DELAY__)) \
              / ((uint64_t)1000000U * (uint64_t)((__PSC__) + 1U))))

/**
  * @brief  HELPER macro calculating the auto-reload value to achieve the required pulse duration (when the pwm operates in one pulse mode).
  * @note ex: @ref __LL_PWM_CALC_PULSE (1000000, @ref LL_PWM_GetPrescaler (), 10, 20);
  * @param  __PWMCLK__ pwm input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ pwm output compare active/inactive delay (in us)
  * @param  __PULSE__ pulse duration (in us)
  * @retval Auto-reload value  (between Min_Data=0 and Max_Data=65535)
  */
#define __LL_PWM_CALC_PULSE(__PWMCLK__, __PSC__, __DELAY__, __PULSE__)  \
  ((uint32_t)(__LL_PWM_CALC_DELAY((__PWMCLK__), (__PSC__), (__PULSE__)) \
              + __LL_PWM_CALC_DELAY((__PWMCLK__), (__PSC__), (__DELAY__))))


/**
  * @}
  */


/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup PWM_LL_Exported_Functions PWM Exported Functions
  * @{
  */

/** @defgroup PWM_LL_EF_Time_Base Time Base configuration
  * @{
  */
/**
  * @brief  Enable pwm counter.
  * @rmtoll CR1          CEN           LL_PWM_EnableCounter
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableCounter(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->CR1, PWM_CR1_CEN);
}

/**
  * @brief  Disable pwm counter.
  * @rmtoll CR1          CEN           LL_PWM_DisableCounter
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableCounter(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->CR1, PWM_CR1_CEN);
}

/**
  * @brief  Indicates whether the pwm counter is enabled.
  * @rmtoll CR1          CEN           LL_PWM_IsEnabledCounter
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledCounter(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->CR1, PWM_CR1_CEN) == (PWM_CR1_CEN)) ? 1UL : 0UL);
}

/**
  * @brief  Enable update event generation.
  * @rmtoll CR1          UDIS          LL_PWM_EnableUpdateEvent
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableUpdateEvent(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->CR1, PWM_CR1_UDIS);
}

/**
  * @brief  Disable update event generation.
  * @rmtoll CR1          UDIS          LL_PWM_DisableUpdateEvent
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableUpdateEvent(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->CR1, PWM_CR1_UDIS);
}

/**
  * @brief  Indicates whether update event generation is enabled.
  * @rmtoll CR1          UDIS          LL_PWM_IsEnabledUpdateEvent
  * @param  PWMx Pwm instance
  * @retval Inverted state of bit (0 or 1).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledUpdateEvent(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->CR1, PWM_CR1_UDIS) == (uint32_t)RESET) ? 1UL : 0UL);
}

/**
  * @brief  Set update event source
  * @note Update event source set to LL_PWM_UPDATESOURCE_REGULAR: any of the following events
  *       generate an update interrupt or DMA request if enabled:
  *        - Counter overflow/underflow
  *        - Setting the UG bit
  *        - Update generation through the slave mode controller
  * @note Update event source set to LL_PWM_UPDATESOURCE_COUNTER: only counter
  *       overflow/underflow generates an update interrupt or DMA request if enabled.
  * @note   Depending on devices and packages,DMA may not be available.
  *         Refer to device datasheet for DMA availability.
  * @rmtoll CR1          URS           LL_PWM_SetUpdateSource
  * @param  PWMx Pwm instance
  * @param  UpdateSource This parameter can be one of the following values:
  *         @arg @ref LL_PWM_UPDATESOURCE_REGULAR
  *         @arg @ref LL_PWM_UPDATESOURCE_COUNTER
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetUpdateSource(PWM_TypeDef *PWMx, uint32_t UpdateSource)
{
  MODIFY_REG(PWMx->CR1, PWM_CR1_URS, UpdateSource);
}

/**
  * @brief  Get actual event update source
  * @rmtoll CR1          URS           LL_PWM_GetUpdateSource
  * @param  PWMx Pwm instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_UPDATESOURCE_REGULAR
  *         @arg @ref LL_PWM_UPDATESOURCE_COUNTER
  */
__STATIC_INLINE uint32_t LL_PWM_GetUpdateSource(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_BIT(PWMx->CR1, PWM_CR1_URS));
}

/**
  * @brief  Set the pwm counter counting mode.
  * @note Macro IS_PWM_COUNTER_MODE_SELECT_INSTANCE(PWMx) can be used to
  *       check whether or not the counter mode selection feature is supported
  *       by a pwm instance.
  * @note Switching from Center Aligned counter mode to Edge counter mode (or reverse)
  *       requires a pwm reset to avoid unexpected direction
  *       due to DIR bit readonly in center aligned mode.
  * @rmtoll CR1          DIR           LL_PWM_SetCounterMode\n
  *         CR1          CMS           LL_PWM_SetCounterMode
  * @param  PWMx Pwm instance
  * @param  CounterMode This parameter can be one of the following values:
  *         @arg @ref LL_PWM_COUNTERMODE_UP
  *         @arg @ref LL_PWM_COUNTERMODE_DOWN
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_UP
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_DOWN
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_UP_DOWN
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetCounterMode(PWM_TypeDef *PWMx, uint32_t CounterMode)
{
  MODIFY_REG(PWMx->CR1, (PWM_CR1_DIR | PWM_CR1_CMS), CounterMode);
}

/**
  * @brief  Get actual counter mode.
  * @note Macro IS_PWM_COUNTER_MODE_SELECT_INSTANCE(PWMx) can be used to
  *       check whether or not the counter mode selection feature is supported
  *       by a pwm instance.
  * @rmtoll CR1          DIR           LL_PWM_GetCounterMode\n
  *         CR1          CMS           LL_PWM_GetCounterMode
  * @param  PWMx Pwm instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_COUNTERMODE_UP
  *         @arg @ref LL_PWM_COUNTERMODE_DOWN
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_UP
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_DOWN
  *         @arg @ref LL_PWM_COUNTERMODE_CENTER_UP_DOWN
  */
__STATIC_INLINE uint32_t LL_PWM_GetCounterMode(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_BIT(PWMx->CR1, PWM_CR1_DIR | PWM_CR1_CMS));
}

/**
  * @brief  Enable auto-reload (ARR) preload.
  * @rmtoll CR1          ARPE          LL_PWM_EnableARRPreload
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableARRPreload(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->CR1, PWM_CR1_ARPE);
}

/**
  * @brief  Disable auto-reload (ARR) preload.
  * @rmtoll CR1          ARPE          LL_PWM_DisableARRPreload
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableARRPreload(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->CR1, PWM_CR1_ARPE);
}

/**
  * @brief  Indicates whether auto-reload (ARR) preload is enabled.
  * @rmtoll CR1          ARPE          LL_PWM_IsEnabledARRPreload
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledARRPreload(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->CR1, PWM_CR1_ARPE) == (PWM_CR1_ARPE)) ? 1UL : 0UL);
}

/**
  * @brief  Enable LED.
  * @rmtoll CR1          LEDEN          LL_PWM_EnableLED
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableLED(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->CR1, PWM_CR1_LEDEN);
}

/**
  * @brief  Disable LED.
  * @rmtoll CR1          LEDEN          LL_PWM_DisableLED
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableLED(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->CR1, PWM_CR1_LEDEN);
}

/**
  * @brief  Indicates whether LED is enabled.
  * @rmtoll CR1          LEDEN          LL_PWM_IsEnabledLED
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledLED(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->CR1, PWM_CR1_LEDEN) == (PWM_CR1_LEDEN)) ? 1UL : 0UL);
}

/**
  * @brief  Start LED.
  * @rmtoll CR1          LEDCEN          LL_PWM_StartLED
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_TransmitLEDData(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->CR1, PWM_CR1_LEDCEN);
}

/**
  * @brief  Indicates whether the data transmission of the LED is complete..
  * @rmtoll CR1          LEDCEN          LL_PWM_IsEnabledLED
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsLEDDataTransmitCompleted(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->CR1, PWM_CR1_LEDCEN) == (PWM_CR1_LEDCEN)) ? 0UL : 1UL);
}

/**
  * @brief  Set LED data length.
  * @rmtoll CR1          LEDWORD           LL_PWM_SetLEDDataLength
  * @param  PWMx Pwm instance
  * @param  DataLength This parameter can be one of the following values:
  *         @arg @ref LL_PWM_LED_DATALENGTH_8B
  *         @arg @ref LL_PWM_LED_DATALENGTH_16B
  *         @arg @ref LL_PWM_LED_DATALENGTH_24B
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetLEDDataLength(PWM_TypeDef *PWMx, uint32_t DataLength)
{
  MODIFY_REG(PWMx->CR1, PWM_CR1_LEDWORD, DataLength);
}

/**
  * @brief  Get LED data length.
  * @rmtoll CR1          LEDWORD           LL_PWM_GetLEDDataLength
  * @param  PWMx Pwm instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_LED_DATALENGTH_8B
  *         @arg @ref LL_PWM_LED_DATALENGTH_16B
  *         @arg @ref LL_PWM_LED_DATALENGTH_24B
  */
__STATIC_INLINE uint32_t LL_PWM_GetLEDDataLength(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_BIT(PWMx->CR1, PWM_CR1_LEDWORD));
}

/**
  * @brief  Set the counter value.
  * @rmtoll CNT          CNT           LL_PWM_SetCounter
  * @param  PWMx Pwm instance
  * @param  Counter Counter value (between Min_Data=0 and Max_Data=0x3ff)
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetCounter(PWM_TypeDef *PWMx, uint32_t Counter)
{
  WRITE_REG(PWMx->CNT, Counter);
}

/**
  * @brief  Get the counter value.
  * @rmtoll CNT          CNT           LL_PWM_GetCounter
  * @param  PWMx Pwm instance
  * @retval Counter value (between Min_Data=0 and Max_Data=0x3ff )
  */
__STATIC_INLINE uint32_t LL_PWM_GetCounter(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_REG(PWMx->CNT));
}

/**
  * @brief  Get the current direction of the counter
  * @rmtoll CR1          DIR           LL_PWM_GetDirection
  * @param  PWMx Pwm instance
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_COUNTERDIRECTION_UP
  *         @arg @ref LL_PWM_COUNTERDIRECTION_DOWN
  */
__STATIC_INLINE uint32_t LL_PWM_GetDirection(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_BIT(PWMx->CR1, PWM_CR1_DIR));
}

/**
  * @brief  Set the prescaler value.
  * @note The counter clock frequency CK_CNT is equal to fCK_PSC / (PSC[7:0] + 1).
  * @note The prescaler can be changed on the fly as this control register is buffered. The new
  *       prescaler ratio is taken into account at the next update event.
  * @note Helper macro @ref __LL_PWM_CALC_PSC can be used to calculate the Prescaler parameter
  * @rmtoll PSC          PSC           LL_PWM_SetPrescaler
  * @param  PWMx Pwm instance
  * @param  Prescaler between Min_Data=0 and Max_Data=255
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetPrescaler(PWM_TypeDef *PWMx, uint32_t Prescaler)
{
  WRITE_REG(PWMx->PSC, Prescaler);
}

/**
  * @brief  Get the prescaler value.
  * @rmtoll PSC          PSC           LL_PWM_GetPrescaler
  * @param  PWMx Pwm instance
  * @retval  Prescaler value between Min_Data=0 and Max_Data=255
  */
__STATIC_INLINE uint32_t LL_PWM_GetPrescaler(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_REG(PWMx->PSC));
}

/**
  * @brief  Set the auto-reload value.
  * @note The counter is blocked while the auto-reload value is null.
  * @note Helper macro @ref __LL_PWM_CALC_ARR can be used to calculate the AutoReload parameter
  * @rmtoll ARR          ARR           LL_PWM_SetAutoReload
  * @param  PWMx Pwm instance
  * @param  AutoReload between Min_Data=0 and Max_Data=1023
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetAutoReload(PWM_TypeDef *PWMx, uint32_t AutoReload)
{
  WRITE_REG(PWMx->ARR, AutoReload);
}

/**
  * @brief  Get the auto-reload value.
  * @rmtoll ARR          ARR           LL_PWM_GetAutoReload
  * @param  PWMx Pwm instance
  * @retval Auto-reload value
  */
__STATIC_INLINE uint32_t LL_PWM_GetAutoReload(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_REG(PWMx->ARR));
}

/**
  * @}
  */

/** @defgroup PWM_LL_EF_Capture_Compare Capture Compare configuration
  * @{
  */

/**
  * @brief  Enable compare channels.
  * @rmtoll CER         C1E          LL_PWM_OC_EnableChannel
  * @param  PWMx Pwm instance
  * @param  Channels This parameter can be a combination of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_EnableChannel(PWM_TypeDef *PWMx, uint32_t Channels)
{
  SET_BIT(PWMx->CER, Channels);
}

/**
  * @brief  Disable compare channels.
  * @rmtoll CER         C1E          LL_PWM_OC_DisableChannel
  * @param  PWMx Pwm instance
  * @param  Channels This parameter can be a combination of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_DisableChannel(PWM_TypeDef *PWMx, uint32_t Channels)
{
  CLEAR_BIT(PWMx->CER, Channels);
}

/**
  * @brief  Indicate whether channel(s) is(are) enabled.
  * @rmtoll CER         C1E          LL_PWM_OC_IsEnabledChannel
  * @param  PWMx Pwm instance
  * @param  Channels This parameter can be a combination of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_OC_IsEnabledChannel(PWM_TypeDef *PWMx, uint32_t Channels)
{
  return ((READ_BIT(PWMx->CER, Channels) == (Channels)) ? 1UL : 0UL);
}

/**
  * @}
  */

/** @defgroup PWM_LL_EF_Output_Channel Output channel configuration
  * @{
  */
/**
  * @brief  Configure an output channel.
  * @rmtoll CER         C1P          LL_PWM_OC_ConfigOutput
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @param  Configuration This parameter must be a combination of all the following values:
  *         @arg @ref LL_PWM_OCPOLARITY_HIGH or @ref LL_PWM_OCPOLARITY_LOW
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_ConfigOutput(PWM_TypeDef *PWMx, uint32_t Channel, uint32_t Configuration)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  MODIFY_REG(PWMx->CER, (PWM_CER_C1P << SHIFT_TAB_CxP[iChannel]),
             (Configuration & PWM_CER_C1P) << SHIFT_TAB_CxP[iChannel]);
}

/**
  * @brief  Define the behavior of the output reference signal OCxREF from which
  *         OCx and OCxN (when relevant) are derived.
  * @rmtoll CMR        OC1M          LL_PWM_OC_SetMode
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @param  Mode This parameter can be one of the following values:
  *         @arg @ref LL_PWM_OCMODE_PWM1
  *         @arg @ref LL_PWM_OCMODE_PWM2
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_SetMode(PWM_TypeDef *PWMx, uint32_t Channel, uint32_t Mode)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  register __IO uint32_t *pReg = (__IO uint32_t *)((uint32_t)((uint32_t)(&PWMx->CMR) + OFFSET_TAB_CMR[iChannel]));
  MODIFY_REG(*pReg, ((PWM_CMR_OC1M) << SHIFT_TAB_OCxM[iChannel]),  Mode << SHIFT_TAB_OCxM[iChannel]);
}

/**
  * @brief  Get the output compare mode of an output channel.
  * @rmtoll CMR        OC1M          LL_PWM_OC_GetMode
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_OCMODE_PWM1
  *         @arg @ref LL_PWM_OCMODE_PWM2
  */
__STATIC_INLINE uint32_t LL_PWM_OC_GetMode(PWM_TypeDef *PWMx, uint32_t Channel)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  register const __IO uint32_t *pReg = (__IO uint32_t *)((uint32_t)((uint32_t)(&PWMx->CMR) + OFFSET_TAB_CMR[iChannel]));
  return (READ_BIT(*pReg, ((PWM_CMR_OC1M) << SHIFT_TAB_OCxM[iChannel])) >> SHIFT_TAB_OCxM[iChannel]);
}

/**
  * @brief  Set the polarity of an output channel.
  * @rmtoll CER         C1P          LL_PWM_OC_SetPolarity
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @param  Polarity This parameter can be one of the following values:
  *         @arg @ref LL_PWM_OCPOLARITY_HIGH
  *         @arg @ref LL_PWM_OCPOLARITY_LOW
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_SetPolarity(PWM_TypeDef *PWMx, uint32_t Channel, uint32_t Polarity)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  MODIFY_REG(PWMx->CER, (PWM_CER_C1P << SHIFT_TAB_CxP[iChannel]),  Polarity << SHIFT_TAB_CxP[iChannel]);
}

/**
  * @brief  Get the polarity of an output channel.
  * @rmtoll CER         C1P          LL_PWM_OC_GetPolarity
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval Returned value can be one of the following values:
  *         @arg @ref LL_PWM_OCPOLARITY_HIGH
  *         @arg @ref LL_PWM_OCPOLARITY_LOW
  */
__STATIC_INLINE uint32_t LL_PWM_OC_GetPolarity(PWM_TypeDef *PWMx, uint32_t Channel)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  return (READ_BIT(PWMx->CER, (PWM_CER_C1P << SHIFT_TAB_CxP[iChannel])) >> SHIFT_TAB_CxP[iChannel]);
}

/**
  * @brief  Enable compare register (PWMx_CCRx) preload for the output channel.
  * @rmtoll CMR        OC1PE          LL_PWM_OC_EnablePreload
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_EnablePreload(PWM_TypeDef *PWMx, uint32_t Channel)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  register __IO uint32_t *pReg = (__IO uint32_t *)((uint32_t)((uint32_t)(&PWMx->CMR) + OFFSET_TAB_CMR[iChannel]));
  SET_BIT(*pReg, (PWM_CMR_OC1PE << SHIFT_TAB_OCxPE[iChannel]));
}

/**
  * @brief  Disable compare register (PWMx_CCRx) preload for the output channel.
  * @rmtoll CMR        OC1PE          LL_PWM_OC_DisablePreload
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_DisablePreload(PWM_TypeDef *PWMx, uint32_t Channel)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  register __IO uint32_t *pReg = (__IO uint32_t *)((uint32_t)((uint32_t)(&PWMx->CMR) + OFFSET_TAB_CMR[iChannel]));
  CLEAR_BIT(*pReg, (PWM_CMR_OC1PE << SHIFT_TAB_OCxPE[iChannel]));
}

/**
  * @brief  Indicates whether compare register (PWMx_CCRx) preload is enabled for the output channel.
  * @rmtoll CMR        OC1PE          LL_PWM_OC_IsEnabledPreload
  * @param  PWMx Pwm instance
  * @param  Channel This parameter can be one of the following values:
  *         @arg @ref LL_PWM_CHANNEL_CH1
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_OC_IsEnabledPreload(PWM_TypeDef *PWMx, uint32_t Channel)
{
  register uint8_t iChannel = PWM_GET_CHANNEL_INDEX(Channel);
  register const __IO uint32_t *pReg = (__IO uint32_t *)((uint32_t)((uint32_t)(&PWMx->CMR) + OFFSET_TAB_CMR[iChannel]));
  register uint32_t bitfield = PWM_CMR_OC1PE << SHIFT_TAB_OCxPE[iChannel];
  return ((READ_BIT(*pReg, bitfield) == bitfield) ? 1UL : 0UL);
}

/**
  * @brief  Set compare value for output channel 1 (PWMx_CCR1).
  * @note Macro IS_PWM_OC1_INSTANCE(PWMx) can be used to check whether or not
  *       output channel 1 is supported by a pwm instance.
  * @rmtoll CCR1         CCR1          LL_PWM_OC_SetCompareCH1
  * @param  PWMx Pwm instance
  * @param  CompareValue between Min_Data=0 and Max_Data=1023
  * @retval None
  */
__STATIC_INLINE void LL_PWM_OC_SetCompareCH1(PWM_TypeDef *PWMx, uint32_t CompareValue)
{
  WRITE_REG(PWMx->CCR1, CompareValue);
}

/**
  * @brief  Get compare value (PWMx_CCR1) set for  output channel 1.
  * @note Macro IS_PWM_OC1_INSTANCE(PWMx) can be used to check whether or not
  *       output channel 1 is supported by a pwm instance.
  * @rmtoll CCR1         CCR1          LL_PWM_OC_GetCompareCH1
  * @param  PWMx Pwm instance
  * @retval CompareValue (between Min_Data=0 and Max_Data=1023)
  */
__STATIC_INLINE uint32_t LL_PWM_OC_GetCompareCH1(PWM_TypeDef *PWMx)
{
  return (uint32_t)(READ_REG(PWMx->CCR1));
}

/**
  * @}
  */

/** @defgroup PWM_LL_EF_FLAG_Management FLAG-Management
  * @{
  */
/**
  * @brief  Clear the update interrupt flag (UIF).
  * @rmtoll SR           UIF           LL_PWM_ClearFlag_UPDATE
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_ClearFlag_UPDATE(PWM_TypeDef *PWMx)
{
  WRITE_REG(PWMx->SR, ~(PWM_SR_UIF));
}

/**
  * @brief  Indicate whether update interrupt flag (UIF) is set (update interrupt is pending).
  * @rmtoll SR           UIF           LL_PWM_IsActiveFlag_UPDATE
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsActiveFlag_UPDATE(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->SR, PWM_SR_UIF) == (PWM_SR_UIF)) ? 1UL : 0UL);
}

/**
  * @brief  Clear the Compare 1 interrupt flag (OC1F).
  * @rmtoll SR           OC1IF         LL_PWM_ClearFlag_OC1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_ClearFlag_OC1(PWM_TypeDef *PWMx)
{
  WRITE_REG(PWMx->SR, ~(PWM_SR_OC1IF));
}

/**
  * @brief  Indicate whether Compare 1 interrupt flag (OC1F) is set (Compare 1 interrupt is pending).
  * @rmtoll SR           OC1IF         LL_PWM_IsActiveFlag_OC1
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsActiveFlag_OC1(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->SR, PWM_SR_OC1IF) == (PWM_SR_OC1IF)) ? 1UL : 0UL);
}

/**
  * @brief  Clear the LED1 data transmission completed interrupt flag.
  * @rmtoll SR           LED1_ENDIF         LL_PWM_ClearFlag_LED1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_ClearFlag_LED1(PWM_TypeDef *PWMx)
{
  WRITE_REG(PWMx->SR, ~(PWM_SR_LED1_ENDIF));
}

/**
  * @brief  Indicate whether LED1 data transmission completed interrupt flag is set.
  * @rmtoll SR           LED1_ENDIF         LL_PWM_IsActiveFlag_LED1
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsActiveFlag_LED1(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->SR, PWM_SR_LED1_ENDIF) == (PWM_SR_LED1_ENDIF)) ? 1UL : 0UL);
}
/**
  * @}
  */

/** @defgroup PWM_LL_EF_IT_Management IT-Management
  * @{
  */
/**
  * @brief  Enable update interrupt (UIE).
  * @rmtoll DIER         UIE           LL_PWM_EnableIT_UPDATE
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableIT_UPDATE(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->DIER, PWM_DIER_UIE);
}

/**
  * @brief  Disable update interrupt (UIE).
  * @rmtoll DIER         UIE           LL_PWM_DisableIT_UPDATE
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableIT_UPDATE(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->DIER, PWM_DIER_UIE);
}

/**
  * @brief  Indicates whether the update interrupt (UIE) is enabled.
  * @rmtoll DIER         UIE           LL_PWM_IsEnabledIT_UPDATE
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledIT_UPDATE(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->DIER, PWM_DIER_UIE) == (PWM_DIER_UIE)) ? 1UL : 0UL);
}

/**
  * @brief  Enable compare 1 interrupt (OC1IE).
  * @rmtoll DIER         OC1IE         LL_PWM_EnableIT_OC1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableIT_OC1(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->DIER, PWM_DIER_OC1IE);
}

/**
  * @brief  Disable compare 1  interrupt (OC1IE).
  * @rmtoll DIER         OC1IE         LL_PWM_DisableIT_OC1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableIT_OC1(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->DIER, PWM_DIER_OC1IE);
}

/**
  * @brief  Indicates whether the compare 1 interrupt (OC1IE) is enabled.
  * @rmtoll DIER         OC1IE         LL_PWM_IsEnabledIT_OC1
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledIT_OC1(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->DIER, PWM_DIER_OC1IE) == (PWM_DIER_OC1IE)) ? 1UL : 0UL);
}

/**
  * @brief  Enable LED 1 data transmission completed interrupt.
  * @rmtoll DIER         LED1_ENDIE         LL_PWM_EnableIT_LED1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_EnableIT_LED1(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->DIER, PWM_DIER_LED1_ENDIE);
}

/**
  * @brief  Disable LED 1 data transmission completed interrupt.
  * @rmtoll DIER         LED1_ENDIE         LL_PWM_DisableIT_LED1
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_DisableIT_LED1(PWM_TypeDef *PWMx)
{
  CLEAR_BIT(PWMx->DIER, PWM_DIER_LED1_ENDIE);
}

/**
  * @brief  Indicates whether the interrupt that indicates the completion of LED1 data transmission is enabled.
  * @rmtoll DIER         LED1_ENDIE         LL_PWM_IsEnabledIT_LED1
  * @param  PWMx Pwm instance
  * @retval State of bit (1 or 0).
  */
__STATIC_INLINE uint32_t LL_PWM_IsEnabledIT_LED1(PWM_TypeDef *PWMx)
{
  return ((READ_BIT(PWMx->DIER, PWM_DIER_LED1_ENDIE) == (PWM_DIER_LED1_ENDIE)) ? 1UL : 0UL);
}
/**
  * @}
  */
/** @defgroup PWM_LL_EF_EVENT_Management EVENT-Management
  * @{
  */
/**
  * @brief  Generate an update event.
  * @rmtoll EGR          UG            LL_PWM_GenerateEvent_UPDATE
  * @param  PWMx Pwm instance
  * @retval None
  */
__STATIC_INLINE void LL_PWM_GenerateEvent_UPDATE(PWM_TypeDef *PWMx)
{
  SET_BIT(PWMx->EGR, PWM_EGR_UG);
}

/**
  * @}
  */
  
/** @defgroup PWM_LL_EF_LED_Management LED-Management
  * @{
  */
/**
  * @brief  Set LED1 Data Register .
  * @rmtoll LED1_DATA         LED1_DATA        LL_PWM_SetLEDData
  * @param  PWMx Pwm instance
  * @param  Data Value between Min_Data=0x00 and Max_Data=0xFFFFFF
  * @retval None
  */
__STATIC_INLINE void LL_PWM_SetLEDData(PWM_TypeDef *PWMx, uint32_t Data)
{
  WRITE_REG(PWMx->LED1_DATA, Data);
}

/**
  * @brief  Get LED1 Data Register .
  * @rmtoll LED1_DATA         LED1_DATA        LL_PWM_GetLEDData
  * @param  PWMx Pwm instance
  * @retval Data (between Min_Data=0x00 and Max_Data=0xFFFFFF)
  */
__STATIC_INLINE uint32_t LL_PWM_GetLEDData(PWM_TypeDef *PWMx)
{
  return READ_REG(PWMx->LED1_DATA);
}
/**
  * @}
  */
#if defined(USE_FULL_LL_DRIVER)
/** @defgroup PWM_LL_EF_Init Initialisation and deinitialisation functions
  * @{
  */

ErrorStatus LL_PWM_DeInit(PWM_TypeDef *PWMx);
void LL_PWM_StructInit(LL_PWM_InitTypeDef *PWM_InitStruct);
ErrorStatus LL_PWM_Init(PWM_TypeDef *PWMx, LL_PWM_InitTypeDef *PWM_InitStruct);
void LL_PWM_OC_StructInit(LL_PWM_OC_InitTypeDef *PWM_OC_InitStruct);
ErrorStatus LL_PWM_OC_Init(PWM_TypeDef *PWMx, uint32_t Channel, LL_PWM_OC_InitTypeDef *PWM_OC_InitStruct);
/**
  * @}
  */
#endif /* USE_FULL_LL_DRIVER */

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

#ifdef __cplusplus
}
#endif

#endif /* __PY32CA70_LL_PWM_H */
/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
