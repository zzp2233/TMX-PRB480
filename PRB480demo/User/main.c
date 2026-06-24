/*******************************************************************************
 * PRB480 1-Wire 认证芯片测试程序
 * 功能：演示 PRB480 的完整认证流程
 * 硬件：PC1=IO3采样，PG10=IO1响应PMOS控制，PG11=IO2功率PMOS控制
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
    u8 rom[8];                         /* 保存从器件读出的 8 字节 ROM ID */
    u8 readback[8];                    /* 保存最终从 EEPROM 回读的 8 字节数据 */

    u8 secret[8] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x00};
    /* 当前用于认证流程的 first secret */

    u8 writeData[8] = {0x18, 0x29, 0x3F, 0x4E, 0x5D, 0x6C, 0x7B, 0x8A};
    /* 准备写入地址 0x0000 的 8 字节用户数据 */

    u8 challenge[3] = {0xAB, 0xCD, 0xEF};
    /* Read Authenticated Page 使用的 3 字节 challenge */

    u8 copyMac[20] = {0};
    /* 主机侧实时计算得到的 Copy Scratchpad 认证 MAC */

    u8 pageData[32] = {0};
    /* Copy Scratchpad MAC 使用的目标页原始 32 字节数据 */

    u8 es;
    /* Read Scratchpad 验证通过后的 E/S 状态字 */

    u8 copyOk;
    /* 保存 Copy Scratchpad 授权写入流程返回结果 */

    u8 copyStatus = 0xFF;
    /* 保存 Copy Scratchpad 返回的 AAh/00h/FFh 原始状态 */

    u8 i;
    /* 通用循环变量 */

    u8 *useRom;
    /* 指向当前要操作器件的 ROM ID */

    PRB480_AuthenticatedPagePacket authPacket;
    /* 保存认证读页返回的完整结果包 */

    /* ========== 系统初始化 ========== */
    SysTick_Init(168);                          /* 初始化系统滴答定时器 (168MHz) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); /* 配置中断优先级分组 */
    USART1_Init(115200);                        /* 初始化 UART1，波特率 115200 */
    LED_Init();                                 /* 初始化 LED 指示灯 */

    /* ========== 固定当前单总线角色 ========== */
    PRB480_BoardInterfaceConfig();

    /* ========== Step 0: PRB480 初始化 ========== */
    /* 初始化 PRB480：PC1 采样 IO3，PG10 控制响应 PMOS，PG11 控制额外供电 PMOS */
    // while(PRB480_Init())
    // {
    //     printf("PRB480 not found, check PC1(IO3)/PG10(IO1)/PG11(IO2) connection!\r\n");
    //     delay_ms(500);
    // }
    // printf("PRB480 init OK!\r\n");
    PRB480_Init();
    //PRB480_DebugReadSlotLoop(100);  /* 调试用：循环产生 100 次 ReadBit 同款读时隙 */
    //PRB480_DebugAdcLevels();                  /* 调试 PC1 ADC 是否随 PG10/PG11 控制变化 */
    /* ========== Step 1: 读取并校验 ROM 地址 ========== */
    /* 这里不再退回 Skip ROM。
     * 因为后续认证算法要用到 ROM ID，本例要求 ROM 必须读对且 CRC8 正确。
     */
    useRom = rom;                                  /* 后续所有操作默认都用 Match ROM 访问当前器件 */

    if(PRB480_ReadROM(rom) == 0)                   /* 读取 8 字节 ROM 并做 CRC8 校验 */
    {
        printf("PRB480 ROM ID:");                  /* 打印 ROM 头 */
        for(i = 0; i < 8; i++)                     /* 逐字节打印 8 字节 ROM */
        {
            printf(" %02X", rom[i]);               /* 打印第 i 个字节 */
        }
        printf("\r\n");                            /* 换行结束 */
    }
    else
    {
        printf("PRB480 Read ROM failed or CRC8 invalid\r\n"); /* ROM 读取失败或 CRC8 错误 */
        while(1)
        {
            LED1 = !LED1;                          /* 快速闪灯表示错误 */
            delay_ms(120);                         /* 错误状态下短周期闪烁 */
        }
    }


    printf("Secret:");                              /* 打印当前 first secret 标题 */
    for(i = 0; i < 8; i++)                          /* 逐字节打印 8 字节 secret */
    {
        printf(" %02X", secret[i]);                 /* 打印第 i 个 secret 字节 */
    }
    printf("\r\n");                                 /* secret 打印结束换行 */

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

    /* ========== Step 3: 认证读页面 ========== */
    /* 完整流程：
     * 1. 先把 challenge 写入 scratchpad 指定字节
     * 2. 执行 Read Authenticated Page (0xA5)
     * 3. 读取 page / CRC16 / device MAC / CRC16 / trailer
     * 4. 主机侧用相同输入重算 MAC，与器件返回的 MAC 比较
     */
    if(PRB480_ReadAuthenticatedPageEx(useRom, secret, 0x0060, challenge, &authPacket) == 0)
    {
        printf("Read Authenticated Page OK\r\n");  /* 认证读成功 */

        printf("Challenge:");                      /* 打印 challenge 标题 */
        for(i = 0; i < 3; i++)                     /* 逐字节打印 3 字节 challenge */
            printf(" %02X", authPacket.challenge[i]); /* 打印第 i 个 challenge 字节 */
        printf("\r\n");                            /* challenge 打印结束换行 */

        printf("Device MAC:");                     /* 打印器件返回 MAC 标题 */
        for(i = 0; i < 20; i++)                    /* 遍历 20 字节 MAC */
            printf(" %02X", authPacket.device_mac[i]); /* 打印器件返回的第 i 个 MAC 字节 */
        printf("\r\n");                            /* device MAC 打印结束换行 */

        printf("Host   MAC:");                     /* 打印主机重算 MAC 标题 */
        for(i = 0; i < 20; i++)                    /* 遍历 20 字节 MAC */
            printf(" %02X", authPacket.host_mac[i]); /* 打印主机计算的第 i 个 MAC 字节 */
        printf("\r\n");                            /* host MAC 打印结束换行 */

        printf("Page CRC16=0x%04X, MAC CRC16=0x%04X\r\n",
               authPacket.page_crc16,              /* 打印页面段 CRC16 */
               authPacket.mac_crc16);              /* 打印 MAC 段 CRC16 */
    }
    else
    {
        printf("Read Authenticated Page failed\r\n"); /* 认证读流程失败 */
    }

    /* ========== Step 4: 准备 8 字节 writeData ========== */
    printf("WriteData:");                          /* 打印待写入数据标题 */
    for(i = 0; i < 8; i++)                         /* 逐字节打印 8 字节写入数据 */
    {
        printf(" %02X", writeData[i]);             /* 打印第 i 个写入数据字节 */
    }
    printf("\r\n");                                /* writeData 打印结束换行 */

    /* ========== Step 5: 按图 8c 执行 Copy Scratchpad[55h] ========== */
    /* 完整流程：
     * 1. 读取目标页原始 pageData
     * 2. Write Scratchpad 写入 8 字节 writeData
     * 3. Read Scratchpad 验证真实 TA1 / TA2 / E/S / PF / AA / 数据 / CRC
     * 4. 主机侧根据 secret + pageData[0..27] + scratchpad + ROM[0..6] + MP 生成 Copy MAC
     * 5. 发送 0x55 + TA1 + TA2 + E/S
     * 6. 等待 tCSHA 后发送 20 字节 MAC
     * 7. 等待 tPROG 后读取 AAh / 00h / FFh 状态
     * 8. 成功后 Read Memory 读回验证
     */
    copyOk = PRB480_CopyScratchpadVerified(useRom, 0x0000, writeData, secret, pageData, copyMac, &copyStatus); /* 执行标准 Copy Scratchpad 授权写 */

    printf("PageData:");                           /* 打印目标页原始数据标题 */
    for(i = 0; i < 32; i++)                        /* 逐字节打印目标页 32 字节数据 */
    {
        printf(" %02X", pageData[i]);              /* 打印第 i 个页面字节 */
    }
    printf("\r\n");                                /* pageData 打印结束换行 */

    printf("Host Copy MAC:");                      /* 打印主机 Copy MAC 标题 */
    for(i = 0; i < 20; i++)                        /* 逐字节打印 20 字节 Copy MAC */
    {
        printf(" %02X", copyMac[i]);               /* 打印第 i 个 Copy MAC 字节 */
    }
    printf("\r\n");                                /* Copy MAC 打印结束换行 */

    printf("Copy Scratchpad return status=0x%02X\r\n", copyStatus); /* 打印 Copy Scratchpad 返回状态 */

    if(copyOk == 0)                                 /* 判断 Copy Scratchpad 是否成功 */
    {
        es = 0x87;                                  /* 成功时记录 AA=1、E=111b 的 E/S 状态 */
        printf("Authenticated Copy Scratchpad OK, E/S=0x%02X\r\n", es); /* 打印授权写入成功 */
    }
    else
    {
        printf("Authenticated Copy Scratchpad failed\r\n"); /* 认证写入失败 */
    }

    /* ========== Step 6: Read Memory 读回目标地址验证写入结果 ========== */
    /* 读取 PRB480 内存中的数据进行验证 */
    if(PRB480_ReadMemory(useRom, 0x0000, readback, 8) == 0)   /* 用 Match ROM 回读写入后的 8 字节数据 */
    {
        printf("Read Memory 0x0000 data:");
        for(i = 0; i < 8; i++)
        {
            printf(" %02X", readback[i]);
        }
        printf("\r\n");
        printf("Write verify %s\r\n", (copyOk == 0) ? "OK" : "FAILED"); /* 打印最终写入验证结论 */
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
