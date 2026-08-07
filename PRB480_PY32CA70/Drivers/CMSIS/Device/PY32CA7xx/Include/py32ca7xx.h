/**
  ******************************************************************************
  * @file    py32ca7xx.h
  * @brief   CMSIS Py32ca7xx Device Peripheral Access Layer Header File.
  * @version v1.0.0
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) MS Semiconductor Co.
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup py32ca7xx
  * @{
  */
#ifndef __PY32CA7XX_H
#define __PY32CA7XX_H

/**
  * @brief MS32C0 Family
  */
#if !defined (MS32C0)
#define MS32C0
#endif /* MS32C0 */

/**  Uncomment the line below according to the target MS32 device used in your
  *  application.
  */

#if !defined (MS32C001x2)
/* #define MS32C001x2  */  /*!< MS32C001x2  Devices (MS32C001x2  microcontrollers where the Flash memory is 8   Kbytes) */ 
#endif
#if !defined (PY32CA70x4)
/* #define PY32CA70x4  */  /*!< PY32CA70x4  Devices (PY32CA70x4  microcontrollers where the Flash memory is 8   Kbytes) */ 
#endif
#if !defined (MS32C011x3)
/* #define MS32C011x3  */  /*!< MS32C011x3  Devices (MS32C011x3  microcontrollers where the Flash memory is 12   Kbytes) */ 
#endif
#if !defined (MS32C012x3)
/* #define MS32C012x3  */  /*!< MS32C012x3  Devices (MS32C012x3  microcontrollers where the Flash memory is 12   Kbytes) */ 
#endif
/**  Tip: To avoid modifying this file each time you need to switch between these
  *       devices, you can define the device in your toolchain compiler preprocessor.
  */

#if (defined(MS32C001x2))
#define MS32C001PRE
#endif
#if (defined(PY32CA70x4))
#define PY32CA70PRE
#endif
#if (defined(MS32C011x3))
#define MS32C011PRE
#endif
#if (defined(MS32C012x3))
#define MS32C012PRE
#endif

/**
  * @brief CMSIS Device version number V0.5.1
  */
#define __MS32C0_DEVICE_VERSION_MAIN   (0x00) /*!< [31:24] main version */
#define __MS32C0_DEVICE_VERSION_SUB1   (0x05) /*!< [23:16] sub1 version */
#define __MS32C0_DEVICE_VERSION_SUB2   (0x01) /*!< [15:8]  sub2 version */
#define __MS32C0_DEVICE_VERSION_RC     (0x00) /*!< [7:0]  release candidate */
#define __MS32C0_DEVICE_VERSION        ((__MS32C0_DEVICE_VERSION_MAIN  << 24)\
                                        |(__MS32C0_DEVICE_VERSION_SUB1 << 16)\
                                        |(__MS32C0_DEVICE_VERSION_SUB2 << 8 )\
                                        |(__MS32C0_DEVICE_VERSION_RC))

/**
  * @brief Device_Included
  */
#if defined(MS32C001x2)
#include "ms32c001x2.h"
#elif defined(PY32CA70x4)
#include "py32ca70x4.h"
#elif defined(MS32C011x3)
#include "ms32c011x3.h"
#elif defined(MS32C012x3)
#include "ms32c012x3.h"
#else
#error "Please select first the target Py32ca7xx device used in your application (in py32ca7xx.h file)"
#endif /* Device_Included */

/**
  * @brief Exported_types
  */
typedef enum
{
  RESET = 0,
  SET = !RESET
} FlagStatus, ITStatus;

typedef enum
{
  DISABLE = 0,
  ENABLE = !DISABLE
} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum 
{
  SUCCESS = 0U,
  ERROR = !SUCCESS
} ErrorStatus;

/**
  * @brief Exported_macros
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

/* Use of interrupt control for register exclusive access */
/* Atomic 32-bit register access macro to set one or several bits */
#define ATOMIC_SET_BIT(REG, BIT)                             \
  do {                                                       \
    uint32_t primask;                                        \
    primask = __get_PRIMASK();                               \
    __set_PRIMASK(1);                                        \
    SET_BIT((REG), (BIT));                                   \
    __set_PRIMASK(primask);                                  \
  } while(0)

/* Atomic 32-bit register access macro to clear one or several bits */
#define ATOMIC_CLEAR_BIT(REG, BIT)                           \
  do {                                                       \
    uint32_t primask;                                        \
    primask = __get_PRIMASK();                               \
    __set_PRIMASK(1);                                        \
    CLEAR_BIT((REG), (BIT));                                 \
    __set_PRIMASK(primask);                                  \
  } while(0)

/* Atomic 32-bit register access macro to clear and set one or several bits */
#define ATOMIC_MODIFY_REG(REG, CLEARMSK, SETMASK)            \
  do {                                                       \
    uint32_t primask;                                        \
    primask = __get_PRIMASK();                               \
    __set_PRIMASK(1);                                        \
    MODIFY_REG((REG), (CLEARMSK), (SETMASK));                \
    __set_PRIMASK(primask);                                  \
  } while(0)

/* Atomic 16-bit register access macro to set one or several bits */
#define ATOMIC_SETH_BIT(REG, BIT) ATOMIC_SET_BIT(REG, BIT)                                   \

/* Atomic 16-bit register access macro to clear one or several bits */
#define ATOMIC_CLEARH_BIT(REG, BIT) ATOMIC_CLEAR_BIT(REG, BIT)                               \

/* Atomic 16-bit register access macro to clear and set one or several bits */
#define ATOMIC_MODIFYH_REG(REG, CLEARMSK, SETMASK) ATOMIC_MODIFY_REG(REG, CLEARMSK, SETMASK) \

#define HW32_REG(ADDRESS)     ( * ((volatile unsigned           int * )(ADDRESS)))

#define HW16_REG(ADDRESS)     ( * ((volatile unsigned short     int * )(ADDRESS)))

#define HW8_REG(ADDRESS)      ( * ((volatile unsigned          char * )(ADDRESS)))

#define M32(ADDRESS)          HW32_REG(ADDRESS)

#define M16(ADDRESS)          HW16_REG(ADDRESS)

#define M8(ADDRESS)           HW8_REG(ADDRESS)

/**
  * @}
  */

/**
  * @brief Comment the line below if you will not use the peripherals drivers.
  *        In this case, these drivers will not be included and the application code will
  *        be based on direct access to peripherals registers
  */
#if !defined  (USE_HAL_DRIVER)
/*#define USE_HAL_DRIVER */
#endif /* USE_HAL_DRIVER */

#if defined (USE_HAL_DRIVER)
#if defined(MS32C001PRE)
#include "ms32c00x_hal.h"
#else
#include "py32ca7xx_hal.h"
#endif /* Device_Included */
#endif /* USE_HAL_DRIVER */

#endif /* __Py32ca7xx_H */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/************************ (C) COPYRIGHT MS *****END OF FILE******************/

