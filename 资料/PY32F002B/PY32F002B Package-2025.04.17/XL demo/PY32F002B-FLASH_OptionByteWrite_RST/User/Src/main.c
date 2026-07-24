#include "main.h"
#include "User_LED.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"



//------------------------------------------------------------------------------
//修改下方宏定义选择PC0功能
//OB_SWD_PB6_GPIO_PC0 -->GPIO功能
//OB_SWD_PB6_NRST_PC0 -->复位功能
//------------------------------------------------------------------------------


#define MODE_PC0 OB_SWD_PB6_GPIO_PC0
//#define MODE_PC0 OB_SWD_PB6_NRST_PC0

static void APP_FlashOBProgram(void);

int main(void)
{
  HAL_Init();//HAL库初始化
  User_System_Clock_Init();//系统时钟配置

  /*******************************************/
	
	User_LED_Init();
  //User_USART1_Init();//串口初始化
 // printf("hello\r\n");//串口测试
  if(READ_BIT(FLASH->OPTR, OB_USER_SWD_NRST_MODE)!= MODE_PC0 )
  {
    /* OPTION Program */
    APP_FlashOBProgram();
  }
  else
  {
    LED_On();
  }
  


  
  while (1)
  {

	  
	  
  }
}

static void APP_FlashOBProgram(void)
{
  FLASH_OBProgramInitTypeDef OBInitCfg = {0};

  HAL_FLASH_Unlock();        /* Unlock Flash */
  HAL_FLASH_OB_Unlock();     /* Unlock Option */
  
  OBInitCfg.OptionType = OPTIONBYTE_USER;
  OBInitCfg.USERType = OB_USER_BOR_EN | OB_USER_BOR_LEV | OB_USER_IWDG_SW | OB_USER_IWDG_STOP | OB_USER_SWD_NRST_MODE;

  OBInitCfg.USERConfig = OB_BOR_DISABLE | OB_BOR_LEVEL_3p1_3p2 | OB_IWDG_SW | OB_IWDG_STOP_ACTIVE | MODE_PC0 ;

  /* Option Program */
  HAL_FLASH_OBProgram(&OBInitCfg);

  HAL_FLASH_Lock();      /* Lock Flash */
  HAL_FLASH_OB_Lock();   /* Lock Option */

  /* Option Launch */
  HAL_FLASH_OB_Launch();
}

