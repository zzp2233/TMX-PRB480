/*******************************************************************************
 * PRB480 1-Wire 认证芯片测试程序
 * 功能：演示 PRB480 的完整认证流程
 * 硬件：STM32F407ZG + PRB480，1-Wire 总线接 PG9
 * 通信：USART1 115200bps 调试输出
 *******************************************************************************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "ds18b20.h"

int main(void)
{
    /* ========== 变量定义 ========== */
    u8 rom[8];              /* PRB480 的 ROM ID (8字节唯一标识) */
    u8 page[32];            /* 从 PRB480 读取的页面数据 (32字节) */
    u8 mac[20];             /* MAC 认证码 (20字节 SHA1) */
    u8 readback[8];         /* 从暂存区读回的数据 */
    
    /* 第一密钥 - 用于 LoadFirstSecret 命令 (8字节) */
    u8 secret[8] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x00};
    
    /* 要写入暂存区的数据 (8字节) */
    u8 writeData[8] = {0x18, 0x29, 0x3F, 0x4E, 0x5D, 0x6C, 0x7B, 0x8A};
    
    /* CopyScratchpad 命令的 MAC 数据 (20字节) */
    u8 copyMac[20] = {0x48, 0x1A, 0x61, 0xD6, 0x22, 0xC9, 0x8A, 0x18,
                      0x19, 0xD8, 0xF4, 0x2C, 0x1A, 0xD0, 0x30, 0x0F,
                      0x39, 0x14, 0x67, 0xB2};
    
    u8 es;                  /* E/S: 结尾状态字 */
    u8 scratchOk = 0;       /* Scratchpad 写入并验证成功标志 */
    u8 i;                   /* 循环计数器 */
    u8 *useRom;             /* 指向 ROM ID 的指针，NULL 则使用 Skip ROM */

    /* ========== 系统初始化 ========== */
    SysTick_Init(168);                          /* 初始化系统滴答定时器 (168MHz) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); /* 配置中断优先级分组 */
    USART1_Init(115200);                        /* 初始化 UART1，波特率 115200 */
    LED_Init();                                 /* 初始化 LED 指示灯 */

    /* ========== Step 0: PRB480 初始化 ========== */
    /* 初始化 PRB480，配置 PG9 为开漏输出，复位并检测设备应答 */
    while(PRB480_Init())
    {
        printf("PRB480 not found, check PG9 connection!\r\n");
        delay_ms(500);
    }
    printf("PRB480 init OK!\r\n");

    /* ========== Step 1: 读取 ROM 地址 ========== */
    /* 读取 PRB480 的唯一 8 字节 ID，用于后续的 Match ROM 指令 */
    useRom = NULL;
    if(PRB480_ReadROM(rom) == 0)
    {
        useRom = rom;  /* 成功则使用 ROM ID 寻址 */
        printf("PRB480 ROM ID:");
        for(i = 0; i < 8; i++)
        {
            printf(" %02X", rom[i]);
        }
        printf("\r\n");
    }
    else
    {
        printf("PRB480 Read ROM failed, using Skip ROM fallback\r\n");
        /* 失败则使用 Skip ROM，广播方式寻址所有设备 */
    }

    /* ========== Step 2: 加载第一密钥 ========== */
    /* 流程：
     * 1. WriteScratchpad 0x0080 地址写入密钥数据
     * 2. ReadScratchpad 读回验证
     * 3. LoadFirstSecret 加载密钥到内部存储
     * 验证器返回 E/S 字节表示操作状态
     */
    if(PRB480_LoadFirstSecret(useRom, 0x0080, secret, &es) == 0)
    {
        printf("Load First Secret OK, E/S=0x%02X\r\n", es);
    }
    else
    {
        printf("Load First Secret failed\r\n");
    }

    /* ========== Step 3: 读取认证页面 ========== */
    /* 从 PRB480 读取受保护的数据页 (32字节) 和对应的 MAC 签名 (20字节)
     * PRB480 使用内部密钥对页面数据进行 SHA1 签名
     * 返回：
     * - page[32]: 页面数据
     * - mac[20]: SHA1 MAC 签名
     */
    if(PRB480_ReadAuthenticatedPage(NULL, 0x0060, page, mac) == 0)
    {
        printf("Authenticated Page 0x0060 data:\r\n");
        for(i = 0; i < 32; i++)
        {
            printf("%02X ", page[i]);
            if((i + 1) % 16 == 0) printf("\r\n");  /* 每 16 字节换行 */
        }
        printf("MAC:");
        for(i = 0; i < 20; i++)
        {
            printf(" %02X", mac[i]);
        }
        printf("\r\n");
    }
    else
    {
        printf("Read Authenticated Page failed\r\n");
    }

    /* ========== Step 4: 写入并验证暂存区 ========== */
    /* 手册“带验证的写操作”要求：
     * 1. Write Scratchpad 写入 8 字节数据
     * 2. Read Scratchpad 读回 TA1、TA2、E/S、数据和 CRC
     * 3. 确认目标地址、E/S 状态和读回数据正确后，再执行 Copy Scratchpad
     */
    if(PRB480_WriteAndVerifyScratchpad(useRom, 0x0000, writeData, &es) == 0)
    {
        scratchOk = 1;  /* 只有验证通过后，后续才允许 Copy Scratchpad */
        printf("Write and verify Scratchpad OK, E/S=0x%02X\r\n", es);
    }
    else
    {
        printf("Write and verify Scratchpad failed\r\n");
    }

    /* ========== Step 5: 复制暂存区到内存 ========== */
    /* 将暂存区数据和 MAC 签名复制到 PRB480 的内部非易失性内存
     * 需要提供正确的 MAC 值否则操作失败
     * 返回 ACK 字节 (0xAA 0xAA) 表示成功
     */
    if(scratchOk && PRB480_CopyScratchpad(useRom, 0x0000, es, copyMac) == 0)
    {
        printf("Copy Scratchpad OK\r\n");
    }
    else
    {
        printf(scratchOk ? "Copy Scratchpad failed\r\n" : "Copy Scratchpad skipped: scratchpad verify failed\r\n");
    }

    /* ========== Step 6: 读取内存 ========== */
    /* 读取 PRB480 内存中的数据进行验证 */
    if(PRB480_ReadMemory(NULL, 0x0000, readback, 8) == 0)
    {
        printf("Read Memory 0x0000 data:");
        for(i = 0; i < 8; i++)
        {
            printf(" %02X", readback[i]);
        }
        printf("\r\n");
    }
    else
    {
        printf("Read Memory failed\r\n");
    }

    /* ========== 完成，进入死循环闪灯 ========== */
    while(1)
    {
        LED1 = !LED1;  /* 切换 LED1 状态 */
        delay_ms(300); /* 延迟 300ms */
    }
}
