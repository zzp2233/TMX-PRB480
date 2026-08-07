#include "main.h"
#include "py32ca70xx_ll_Start_Kit.h"
#include "PRB480.h"

static void APP_SystemClockConfig(void)
{
    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1U) {}
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS) {}
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_Init1msTick(8000000U);
    LL_SetSystemCoreClock(8000000U);
}

int main(void)
{
    u8 rom[8];
    u8 readback[8];
    u8 secret[8] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x00};
    u8 writeData[8] = {0x18, 0x29, 0x3F, 0x4E, 0x5D, 0x6C, 0x7B, 0x8A};
    u8 challenge[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 copyMac[20] = {0};
    u8 pageData[32] = {0};
    u8 es;
    u8 copyOk;
    u8 copyStatus = 0xFF;
    u8 i;
    u8 *useRom;
    PRB480_AuthenticatedPagePacket authPacket;
    u8 config[24];
    u8 chipData[160];
    u16 addr;
    u8 referenceBlock0[8] = {
        0x5A, 0xC2, 0xB1, 0x80,
        0x96, 0x46, 0x10, 0x77
    };

    u8 framWrite[8] = {
        0xAF, 0xF0, 0x10, 0x04,
        0xA0, 0x31, 0x03, 0xC7
    };

    u8 framRead[8] = {0};
    u8 framResult;
    u8 round;
    u8 status;

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    APP_SystemClockConfig();
    BSP_UART_Config();

    printf("\r\nPRB480 PY32CA70 demo start\r\n");

    PRB480_BoardInterfaceConfig();
    PRB480_SetAdcThreshold(990);
// PRB480_DebugQ2Q3PulseTest();

    if (PRB480_Init())
    {
        printf("PRB480 init/reset failed\r\n");
    }
    else
    {
        printf("PRB480 reset OK\r\n");
    }

    useRom = rom;
    //PRB480_Test_step1_step2(useRom, 0x0000, chipData, 160);

    if (PRB480_ReadROM(rom) == 0)
    {
        printf("PRB480 ROM ID:");
        for (i = 0; i < 8; i++)
        {
            printf(" %02X", rom[i]);
        }
        printf("\r\n");
    }
    else
    {
        // printf("PRB480 Read ROM failed or CRC8 invalid\r\n");
        // while (1)
        // {
        //     HAL_Delay(120);
        // }
    }

    printf("\r\n========== Step 2  Read_memory ==========\r\n");
    printf("Secret:");
    for (i = 0; i < 8; i++)
    {
        printf(" %02X", secret[i]);
    }
    printf("\r\n");

    if (PRB480_ReadMemory(useRom, 0x0088, config, 24) == 0)
    {
        printf("Config 0x0088-0x009F:");
        for (i = 0; i < 24; i++)
        {
            printf(" %02X", config[i]);
        }
        printf("\r\n");
    }
    else
    {
        printf("Read config failed\r\n");
    }

    if (PRB480_ReadMemory(useRom, 0x0000, chipData, 160) == 0)
    {
        printf("Read Memory 0x0000-0x009F:\r\n");
        for (addr = 0; addr < 160; addr++)
        {
            if ((addr & 0x07) == 0)
            {
                printf("0x%04X:", addr);
            }

            printf(" %02X", chipData[addr]);

            if ((addr & 0x07) == 0x07)
            {
                printf("\r\n");
            }
        }
    }
    else
    {
        printf("Read Memory 0x0000-0x009F failed\r\n");
    }

    printf("\r\n========== Step 3  load_first_secret ==========\r\n");
    if (PRB480_LoadFirstSecret(useRom, 0x0080, secret) == 0)
    {
        printf("Load First Secret OK\r\n");
    }
    else
    {
        printf("Load First Secret failed\r\n");
    }

    printf("\r\n========== Step 4  read_authenticated_page ==========\r\n");
    if (PRB480_ReadAuthenticatedPageEx(useRom, secret, 0x0060, challenge, &authPacket) == 0)
    {
        printf("Read Authenticated Page OK\r\n");

        printf("Challenge:");
        for (i = 0; i < 5; i++)
        {
            printf(" %02X", authPacket.challenge[i]);
        }
        printf("\r\n");

        printf("Device MAC:");
        for (i = 0; i < 20; i++)
        {
            printf(" %02X", authPacket.device_mac[i]);
        }
        printf("\r\n");

        printf("Host   MAC:");
        for (i = 0; i < 20; i++)
        {
            printf(" %02X", authPacket.host_mac[i]);
        }
        printf("\r\n");

        printf("Page CRC16=0x%04X, MAC CRC16=0x%04X\r\n",
               authPacket.page_crc16,
               authPacket.mac_crc16);
    }
    else
    {
        printf("Read Authenticated Page failed\r\n");
    }
    // while (1)
    // {
    //     HAL_Delay(300);
    // }
    // PRB480_ResponsePMOS_Off();
    // PRB480_PowerPMOS_Off();

    // while (1)
    // {
    //     HAL_Delay(300);
    // }


    // printf("\r\n========== Step 4 ==========\r\n");
    printf("WriteData:");
    for (i = 0; i < 8; i++)
    {
        printf(" %02X", writeData[i]);
    }
    printf("\r\n");

    printf("\r\n========== Step 5  copy_scratchpad ==========\r\n");
    copyOk = PRB480_CopyScratchpadVerified(useRom, 0x0000, writeData, secret, pageData, copyMac, &copyStatus);

    printf("PageData:");
    for (i = 0; i < 32; i++)
    {
        printf(" %02X", pageData[i]);
    }
    printf("\r\n");

    printf("Host Copy MAC:");
    for (i = 0; i < 20; i++)
    {
        printf(" %02X", copyMac[i]);
    }
    printf("\r\n");

    printf("Copy Scratchpad return status=0x%02X\r\n", copyStatus);
    if (copyOk == 0)
    {
        printf("Authenticated Copy Scratchpad OK\r\n");
    }
    else
    {
        printf("Authenticated Copy Scratchpad failed\r\n");
    }

    if (PRB480_ReadMemory(useRom, 0x0000, readback, 8) == 0)
    {
        printf("Read Memory 0x0000 data:");
        for (i = 0; i < 8; i++)
        {
            printf(" %02X", readback[i]);
        }
        printf("\r\n");
        printf("Write verify %s\r\n", (copyOk == 0) ? "OK" : "FAILED");
    }
    else
    {
        printf("Read Memory failed\r\n");
    }

    printf("\r\n");
    printf("Rewrite back to the original data\r\n");
    //重新写回原始数据
    PRB480_CopyScratchpadVerified(useRom, 0x0000, referenceBlock0, secret, pageData, copyMac, &copyStatus);
 

   
    printf("\r\n========== Step 6  FRAM_Verify ==========\r\n");

    printf("Write:");
    for (i = 0; i < 8; i++)
    {
        printf(" %02X", framWrite[i]);
    }
    printf("\r\n");

    framResult = PRB480_FRAM_Verify(framWrite, framRead);

    printf("Read :");
    for (i = 0; i < 8; i++)
    {
        printf(" %02X", framRead[i]);
    }
    printf("\r\n");

    if (framResult == 0)
    {
        printf("FRAM VERIFY OK\r\n");
    }
    else
    {
        printf("FRAM VERIFY FAIL\r\n");
    }

    printf("===== FRAM VERIFY END =====\r\n");

 
    PRB480_ResponsePMOS_Off();
    PRB480_PowerPMOS_Off();

    while (1)
    {
        LL_mDelay(300);
    }
}
