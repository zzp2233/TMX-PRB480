#ifndef _DS18B20_H
#define _DS18B20_H

#include "system.h"

/* ========== PRB480 1-Wire 总线引脚定义 ========== */
/* PG9 用作开漏输出引脚，连接 PRB480 的 DQ 线 */
#define PRB480_DQ_OUT PGout(9)  /* 写入 1-Wire 总线 (输出驱动) */
#define PRB480_DQ_IN  PGin(9)   /* 读取 1-Wire 总线 (输入采样) */

/* ========== 1-Wire 低层通信函数 ========== */
void PRB480_IO_IN(void);        /* 配置 PG9 为输入模式（浮动，上拉释放） */
void PRB480_IO_OUT(void);       /* 配置 PG9 为输出模式（开漏驱动） */
u8 PRB480_Init(void);           /* PRB480 初始化：配置引脚，执行复位和应答检测 */
u8 PRB480_Reset(void);          /* 复位命令：主机拉低总线 750us，从机应答 */
u8 PRB480_ReadBit(void);        /* 从总线读一个比特 (时序: 2us低 + 12us采样 + 50us恢复) */
void PRB480_WriteBit(u8 bit);   /* 向总线写一个比特 (0: 60us低, 1: 2us低+60us高) */
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

u8 PRB480_ReadAuthenticatedPage(u8 *rom, u16 addr, u8 page[32], u8 mac[20]);
    /* 0xA5 - 读取受保护页面及其 MAC 签名
     * @rom: ROM ID 指针
     * @addr: 页面地址
     * @page: 输出，32字节的页面数据
     * @mac: 输出，20字节的 SHA1-MAC 签名
     * @返回: 0=成功, 1=失败
     */

#endif

