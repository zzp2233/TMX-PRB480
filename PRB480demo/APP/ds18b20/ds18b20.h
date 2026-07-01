#ifndef _DS18B20_H
#define _DS18B20_H

#include "system.h"

/* ========== PRB480 小板接口引脚定义 ========== */
/* 默认连接：PC1=IO3/ADC采样，PG10=IO1响应PMOS控制，PG11=IO2功率PMOS控制 */
#define PRB480_DQ_IN         PCin(1)    /* IO3/PC1：采集 PRB480 返回信号 */
#define PRB480_RESP_PMOS     PGout(10)  /* IO1/PG10：响应 PMOS 控制 */
#define PRB480_POWER_PMOS    PGout(11)  /* IO2/PG11：额外供电 PMOS 控制 */


void PRB480_ResponsePMOS_On(void);        /* 打开响应 PMOS，PC1 可采样 IO3 */
void PRB480_ResponsePMOS_Off(void);       /* 关闭响应 PMOS，PC1 采样 IO3 为高阻态 */
void PRB480_PowerPMOS_On(void);           /* 打开功率 PMOS，PC1 采样 IO3 为高阻态 */
void PRB480_PowerPMOS_Off(void);          /* 关闭功率 PMOS，PC1 采样 IO3 为高阻态 */


void TMX_Delay_us(u32 us);
void TMX_Delay_ms(u32 ms);

/* ========== 1-Wire 低层通信函数 ========== */
void PRB480_IO_IN(void);        /* 配置 PC1/IO3 为输入模式 */
void PRB480_IO_OUT(void);       /* 兼容旧接口，PC1 不再作为输出驱动 */
void PRB480_BoardInterfaceConfig(void); /* 固定小板接口：PC1=IO3，PG10=IO1，PG11=IO2 */
void PRB480_SetPinRoles(u16 dqPin, u16 respPin, u16 powerPin); /* 配置 IO3/IO1/IO2 引脚 */
void PRB480_SetReadSampleDelay(u8 delayUs); /* 设置读 bit 时进入 tREAD0 的采样延时 */
void PRB480_SetAdcThreshold(u16 threshold); /* 设置 PC1 ADC 的 0/1 阈值 */
u16 PRB480_ReadAdcValue(void);  /* 调试用：读取一次 PC1/ADC123_IN11 原始 ADC 值 */
u16 PRB480_GetLastReadAdc(void);/* 调试用：返回最近一次 ReadBit 的 ADC 原始值 */
void PRB480_DebugAdcLevels(void); /* 调试用：打印 PG10/PG11 状态下的 PC1 ADC 电平 */
u8 PRB480_Init(void);           /* PRB480 初始化：配置引脚，执行 tRSTL/tSTD 复位时序 */
u8 PRB480_Reset(void);          /* 复位命令：总线低电平 tRSTL，释放后等待 tSTD */
u8 PRB480_ReadBit(void);        /* 按读时序读取 1 bit：Q5/Q6 形成读窗口，在 tREAD0 内采样 */
void PRB480_DebugReadSlotLoop(u16 count); /* 调试用：不发命令，只循环产生 ReadBit 同款读时隙，count=0 表示一直循环 */
void PRB480_WriteBit(u8 bit);   /* 按写时序写 1 bit：1=一个低脉冲，0=两个低脉冲 */
void PRB480_WriteByte(u8 dat);  /* 写一个字节 (LSB 优先，8个比特) */
u8 PRB480_ReadByte(void);       /* 读一个字节 (LSB 优先，8个比特) */

/* ========== 校验和计算函数 ========== */
u16 PRB480_CalcCRC16(u8 *data, u8 len);  /* 计算 CRC16 校验值 (用于暂存区验证) */
void PRB480_CalcSHA1(unsigned char *data, unsigned int len, unsigned char mac[20]);
                                /* 计算 SHA1 摘要 (用于 MAC 认证) */

/* ========== PRB480 设备寻址命令 ========== */
u8 PRB480_ReadROM(u8 rom[8]);   /* 0x33 - 读取设备的 8 字节唯一 ROM ID */
void PRB480_SkipROM(void);      /* 0xCC - 跳过 ROM，广播方式寻址所有设备 */
void PRB480_MatchROM(u8 rom[8]);/* 0x55 - 匹配 ROM，选择特定设备 (需要 8 字节 ROM ID) */

/* ========== PRB480 数据操作命令 ========== */
u8 PRB480_ReadMemory(u8 *rom, u16 addr, u8 *buf, u8 len);
    /* 0xF0 - 读取内存
     * @rom: ROM ID 指针 (NULL 则 Skip ROM)
     * @addr: 起始地址
     * @buf: 读入缓冲区
     * @len: 读取长度
     * @返回: 0=成功, 1=失败
     */

u8 PRB480_WriteScratchpad(u8 *rom, u16 addr, u8 *dat, u8 len, u16 *crc);
    /* 0x0F - 写入暂存区 (临时缓冲区，8字节)
     * @rom: ROM ID 指针
     * @addr: 起始地址 (通常 0x0000)
     * @dat: 要写入的数据
     * @len: 固定 8 字节
     * @crc: 输出，PRB480 计算的 CRC16
     * @返回: 0=CRC匹配, 1=CRC错误或通信失败
     */

u8 PRB480_ReadScratchpad(u8 *rom, u8 *ta1, u8 *ta2, u8 *es, u8 *buf, u8 len, u16 *crc);
    /* 0xAA - 读取暂存区并验证 CRC16
     * @rom: ROM ID 指针
     * @ta1, @ta2: 输出，暂存区的地址字节
     * @es: 输出，结尾状态字 (E/S)
     * @buf: 输出，暂存区的数据
     * @len: 固定 8 字节
     * @crc: 输出，PRB480 计算的 CRC16
     * @返回: 0=CRC匹配, 1=CRC错误
     */

u8 PRB480_WriteAndVerifyScratchpad(u8 *rom, u16 addr, u8 *dat, u8 *es);
    /* 0x0F + 0xAA - 写入并读回验证暂存区
     * @rom: ROM ID 指针
     * @addr: 目标地址，必须 8 字节对齐
     * @dat: 要写入并验证的 8 字节数据
     * @es: 输出，验证通过后的 E/S 字节
     * @返回: 0=写入并验证成功, 1=失败
     */

u8 PRB480_LoadFirstSecret(u8 *rom, u16 addr, u8 *secret, u8 *es);
    /* 0x5A - 加载第一密钥
     * 流程: WriteScratchpad -> ReadScratchpad -> LoadFirstSecret
     * @rom: ROM ID 指针
     * @addr: 密钥存放地址
     * @secret: 8字节的密钥数据
     * @es: 输出，结尾状态字
     * @返回: 0=成功, 1=失败
     */

u8 PRB480_CopyScratchpad(u8 *rom, u16 addr, u8 es, u8 mac[20]);
    /* 0x55 - 复制暂存区到内存（需要 MAC 签名验证）
     * @rom: ROM ID 指针
     * @addr: 目标地址
     * @es: 结尾状态字 (从 ReadScratchpad 获得)
     * @mac: 20字节的 MAC 签名 (必须正确才能成功)
     * @返回: 0=成功, 1=MAC错误或失败
     */

u8 PRB480_CopyScratchpadVerified(u8 *rom, u16 addr, u8 *writeData, u8 *secret, u8 *pageData, u8 mac[20], u8 *copyStatus);
    /* 0x55 - 标准 Copy Scratchpad 授权写入流程
     * @rom: ROM ID 指针，必须用于 MAC 输入
     * @addr: 目标地址，数据页 0x0000~0x007F 或配置页 0x0088~0x009F，必须 8 字节对齐
     * @writeData: 待写入的 8 字节数据
     * @secret: 当前 8 字节 secret
     * @pageData: 输出，数据页为页首 32 字节，配置页为 0x0080 起 32 字节
     * @mac: 输出，主机侧计算出的 20 字节 Copy MAC
     * @copyStatus: 输出，Copy Scratchpad 原始状态字节 AAh/00h/FFh
     * @返回: 0=成功, 1=失败
     */

u8 PRB480_ReadAuthenticatedPage(u8 *rom, u16 addr, u8 page[32], u8 mac[20]);
    /* 0xA5 - 读取受保护页面及其 MAC 签名
     * @rom: ROM ID 指针
     * @addr: 页面地址
     * @page: 输出，32字节的页面数据
     * @mac: 输出，20字节的 SHA1-MAC 签名
     * @返回: 0=成功, 1=失败
     */


/*******************************************************************************
* 名    称         : PRB480_AuthenticatedPagePacket
* 说    明         : Read Authenticated Page 命令返回的数据包结构
*                    包含页面数据、challenge、设备返回的 MAC、主机计算的 MAC、
*                    两段 CRC16 以及末尾交替响应字节
*******************************************************************************/
typedef struct
{
    u8 page[32];           /* 认证读命令返回的 32 字节页面数据 */
    u8 challenge[5];       /* CODEX: 图 8d 使用 scratchpad 中的 5 字节 challenge */
    u8 device_mac[20];     /* 器件返回的 20 字节 MAC */
    u8 host_mac[20];       /* 主机用同样输入重算得到的 20 字节 MAC */
    u16 page_crc16;        /* 页面数据段返回的 CRC16 */
    u16 mac_crc16;         /* MAC 数据段返回的 CRC16 */
    u8 trailer[2];         /* 末尾交替响应字节，通常为 0xAA/0x55 */
} PRB480_AuthenticatedPagePacket;

/*******************************************************************************
* 名    称         : PRB480_WriteAuthorizedBlock
* 功    能         : 按 Copy Scratchpad[55h] 授权写入流程写 1 个 8 字节块
* 输入参数         : rom    - 目标器件 ROM ID，Copy MAC 必须使用 ROM 前 7 字节
*                    secret - 当前 8 字节 secret
*                    addr   - 目标地址，数据页 0x0000~0x007F 或配置页 0x0088~0x009F
*                    data   - 待写入的 8 字节数据
* 输出参数         : es     - 返回 Read Scratchpad 验证通过后的 E/S
*                    mac    - 返回主机侧实时计算出的 20 字节写授权 MAC
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_WriteAuthorizedBlock(u8 *rom, u8 *secret, u16 addr, u8 *data, u8 *es, u8 mac[20]);

/*******************************************************************************
* 名    称         : PRB480_ReadAuthenticatedPageEx
* 功    能         : 按“认证读页”流程读取页面并在主机侧校验 MAC
* 输入参数         : rom       - 目标器件 ROM ID，NULL 表示 Skip ROM
*                    secret    - 当前 8 字节 secret
*                    addr      - 目标页首地址，必须 32 字节对齐
*                    challenge - 5 字节 challenge，来自 scratchpad contents
* 输出参数         : packet    - 完整返回包
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_ReadAuthenticatedPageEx(u8 *rom, u8 *secret, u16 addr, u8 challenge[5], PRB480_AuthenticatedPagePacket *packet);

u8 PRB480_LoadPartialSecretScratchpad(u8 *rom, u16 addr, u8 partial[8], u8 *es);       /* 写入 Compute Next Secret 使用的 8 字节 partial secret */
u8 PRB480_ComputeNextSecret(u8 *rom, u16 addr);                                       /* 0x33 - Compute Next Secret，生成下一阶段密钥 */
u8 PRB480_VerifyScratchpadFilledAA(u8 *rom);                                          /* 图 8b：验证 scratchpad 被 0xAA 填充 */

#endif

