#include "main.h"
#include "User_UART1.h"
#include "User_System_Clock_Config.h"
#include "PRB480.h"

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

    HAL_Init();
    User_System_Clock_Init();
    User_USART1_Init();

    printf("\r\nPRB480 PY32F002B demo start\r\n");

    PRB480_BoardInterfaceConfig();
    PRB480_SetAdcThreshold(600);

    if (PRB480_Init())
    {
        printf("PRB480 init/reset failed\r\n");
    }
    else
    {
        printf("PRB480 reset OK\r\n");
    }

    useRom = rom;

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
        printf("PRB480 Read ROM failed or CRC8 invalid\r\n");
        while (1)
        {
            HAL_Delay(120);
        }
    }



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

    printf("\r\n========== Step 2 ==========\r\n");
    if (PRB480_LoadFirstSecret(useRom, 0x0080, secret, &es) == 0)
    {
        printf("Load First Secret OK, E/S=0x%02X\r\n", es);
    }
    else
    {
        printf("Load First Secret failed\r\n");
    }

    printf("\r\n========== Step 3 ==========\r\n");
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
    while (1)
    {
        HAL_Delay(300);
    }



    printf("\r\n========== Step 4 ==========\r\n");
    printf("WriteData:");
    for (i = 0; i < 8; i++)
    {
        printf(" %02X", writeData[i]);
    }
    printf("\r\n");

    printf("\r\n========== Step 5 ==========\r\n");
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

    printf("\r\n========== Step 6 ==========\r\n");
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

    PRB480_ResponsePMOS_Off();
    PRB480_PowerPMOS_Off();

    while (1)
    {
        HAL_Delay(300);
    }
}
