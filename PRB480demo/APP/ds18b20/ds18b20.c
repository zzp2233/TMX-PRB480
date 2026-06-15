/*******************************************************************************
 * PRB480 1-Wire 认证芯片驱动程序
 * 功能：实现 1-Wire 总线通信和 PRB480 命令协议
 * 硬件：PG9 (开漏输出，上拉到 3.3V)
 *******************************************************************************/

#include "ds18b20.h"
#include "SysTick.h"
#include "CRC16.h"
#include "SHA1.h"
#include <string.h>

#define PRB480_SCRATCHPAD_SIZE      8       /* Scratchpad 固定为 8 字节 */

#define PRB480_ES_E_MASK            0x07    /* E/S bit[2:0]：Ending Offset 掩码 */
#define PRB480_ES_E_FULL            0x07    /* 写满 8 字节后 Ending Offset 应为 111b */
#define PRB480_ES_PF                0x20    /* E/S bit5：PF，Partial Byte Flag */
#define PRB480_ES_AA                0x80    /* E/S bit7：AA，Authorization Accepted */

static u8 PRB480_CheckPresence(void);        /* 检测 1-Wire presence 应答 */
static u8 PRB480_CommandStart(u8 *rom);      /* 发送 Reset + ROM 寻址命令 */
static u8 PRB480_CheckWriteAddress(u16 addr);/* 检查写地址是否 8 字节对齐 */
static u8 PRB480_CheckScratchpadES(u8 es);   /* 检查 E/S 状态是否合法 */
static u8 PRB480_VerifyScratchpad(u8 *rom, u16 addr, u8 *expected, u8 *ta1, u8 *ta2, u8 *es); /* 读回并验证 scratchpad */


static u8 PRB480_CheckWriteAddress(u16 addr) /* 检查写入地址是否符合手册要求 */
{                                              
    return (addr & 0x0007) ? 1 : 0;           /* 地址低 3 位必须为 0，否则不是 8 字节边界 */
}                                             

static u8 PRB480_CheckScratchpadES(u8 es)     /* 检查 Read Scratchpad 读回的 E/S 字节 */
{                                              
    if ((es & PRB480_ES_E_MASK) != PRB480_ES_E_FULL) return 1; /* E[2:0] 应为 111b，表示写满 8 字节 */
    if (es & PRB480_ES_PF) return 1;                           /* PF=1 表示部分字节/无效写入，失败 */
    if (es & PRB480_ES_AA) return 1;                           /* AA=1 表示已复制授权，刚写完 scratchpad 时不应为 1 */
    return 0;                                                  /* E/S 状态正常 */
}                                            

static u8 PRB480_VerifyScratchpad(u8 *rom, u16 addr, u8 *expected, u8 *ta1, u8 *ta2, u8 *es) /* 验证 scratchpad */
{                                              
    u8 readback[PRB480_SCRATCHPAD_SIZE];      /* 保存 Read Scratchpad 读回的 8 字节数据 */
    u16 crc;                                  /* 保存芯片返回的 CRC16 */

    if (PRB480_ReadScratchpad(rom, ta1, ta2, es, readback, PRB480_SCRATCHPAD_SIZE, &crc)) return 1; /* 读回 scratchpad 并校验 CRC */

    if (*ta1 != (u8)(addr & 0xFF)) return 1;  /* 检查 TA1 是否等于目标地址低字节 */
    if (*ta2 != (u8)(addr >> 8)) return 1;    /* 检查 TA2 是否等于目标地址高字节 */
    if (PRB480_CheckScratchpadES(*es)) return 1; /* 检查 E/S 状态位是否正常 */
    if (memcmp(readback, expected, PRB480_SCRATCHPAD_SIZE) != 0) return 1; /* 比较读回数据和期望数据 */

    return 0;                                 /* scratchpad 验证通过 */
}                                            




/* ========== PG9 引脚配置函数，不使用内部上拉是因为PG9已经外接了上拉电阻 ========== */

/**
 * @func PRB480_IO_IN
 * @brief 配置 PG9 为输入模式 (浮动，依靠外部上拉)
 * 1-Wire 总线是开漏/开集的，当不驱动时应释放总线，让上拉电阻拉高
 */
void PRB480_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;     /* 输入浮动模式 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/**
 * @func PRB480_IO_OUT
 * @brief 配置 PG9 为输出模式 (开漏输出)
 * 用于主机驱动总线拉低 (写 0)
 */
void PRB480_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    /* 输出模式 */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;   // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 不启用内部上下拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/**
 * @func PRB480_Init
 * @brief PRB480 初始化
 * 1. 使能 GPIOG 时钟
 * 2. 配置 PG9 为开漏输出
 * 3. 执行复位命令并检测设备应答
 * @return 0=初始化成功, 1=设备无应答
 */
u8 PRB480_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    PRB480_IO_OUT();
    PRB480_DQ_OUT = 1;  /* 初始状态：总线拉高 */
    delay_us(10);
    return PRB480_Reset();
}

/**
 * @func PRB480_Reset
 * @brief 1-Wire 复位时序
 * 时序：
 *   - 拉低 750us (主机初始化脉冲)
 *   - 释放 15us
 *   - 检测设备应答脉冲
 * @return 0=检测到应答, 1=无应答或超时
 */
u8 PRB480_Reset(void)
{
    PRB480_IO_OUT();
    PRB480_DQ_OUT = 0;  /* 主机拉低总线 750us */
    delay_us(750);
    PRB480_DQ_OUT = 1;  /* 释放总线，让上拉拉高 */
    delay_us(15);
    return PRB480_CheckPresence();  /* 检测设备应答 */
}

/**
 * @func PRB480_CheckPresence
 * @brief 检测 1-Wire 从设备的应答脉冲
 * 应答时序：设备在主机释放后的 15-240us 内拉低总线 60-240us
 * @return 0=检测到应答, 1=无应答
 */
static u8 PRB480_CheckPresence(void)
{
    u8 retry = 0;

    PRB480_IO_IN();
    /* 等待总线释放 (最多 200us) */
    while (PRB480_DQ_IN && retry < 200)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 200) return 1;  /* 总线未被拉低，无应答 */

    /* 等待总线恢复高电平 (最多 240us) */
    retry = 0;
    while (!PRB480_DQ_IN && retry < 240)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 240) return 1;  /* 总线未释放，超时 */

    return 0;  /* 正常应答 */
}

/* ========== 1-Wire 比特级通信函数 ========== */

/**
 * @func PRB480_WriteBit
 * @brief 向 1-Wire 总线写一个比特
 * 写 0 时序：拉低 60us
 * 写 1 时序：拉低 2us，释放 60us (由上拉拉高)
 */
void PRB480_WriteBit(u8 bit)
{
    PRB480_IO_OUT();
    PRB480_DQ_OUT = 0;  /* 拉低总线 2us */
    delay_us(2);
    if (bit)
    {
        /* 写 1：快速释放 */
        PRB480_DQ_OUT = 1;  /* 释放总线，让上拉拉高 */
        delay_us(60);       /* 等待恢复 */
    }
    else
    {
        /* 写 0：继续拉低 60us */
        delay_us(60);       /* 总线保持拉低 60us */
        PRB480_DQ_OUT = 1;  /* 释放总线 */
    }
}

/**
 * @func PRB480_ReadBit
 * @brief 从 1-Wire 总线读一个比特
 * 时序：
 *   - 拉低 2us 初始化
 *   - 释放总线
 *   - 等待 12us 后采样 (在此时间内，写 1 的从设备会拉低)
 *   - 继续等待 50us 恢复
 * @return 比特值 (0 或 1)
 */
u8 PRB480_ReadBit(void)
{
    u8 data;

    PRB480_IO_OUT();
    PRB480_DQ_OUT = 0;  /* 主机拉低 2us 作为读请求 */
    delay_us(2);
    PRB480_DQ_OUT = 1;  /* 释放总线 */
    PRB480_IO_IN();     /* 切换为输入模式采样 */
    delay_us(12);       /* 等待 12us 让从设备做出响应 */

    data = PRB480_DQ_IN ? 1 : 0;  /* 采样总线电平 */
    delay_us(50);       /* 等待恢复到完整的时间槽 */

    return data;
}

/* ========== 1-Wire 字节级通信函数 ========== */

/**
 * @func PRB480_WriteByte
 * @brief 向 1-Wire 总线写一个字节 (LSB 优先)
 * @param dat 要写入的字节数据
 */
void PRB480_WriteByte(u8 dat)
{
    u8 j;
    for (j = 0; j < 8; j++)
    {
        PRB480_WriteBit(dat & 0x01);  /* 写最低位 */
        dat >>= 1;                     /* 右移准备下一位 */
    }
}

/**
 * @func PRB480_ReadByte
 * @brief 从 1-Wire 总线读一个字节 (LSB 优先)
 * @return 读取的字节数据
 */
u8 PRB480_ReadByte(void)
{
    u8 i, j;
    u8 dat = 0;

    for (i = 0; i < 8; i++)
    {
        j = PRB480_ReadBit();           /* 读一个比特 */
        dat = (j << 7) | (dat >> 1);    /* 放入最高位，整体右移 */
    }

    return dat;
}

/* ========== 校验和计算函数 ========== */

/**
 * @func PRB480_CalcCRC16
 * @brief 计算 PRB480/1-Wire CRC16 反码
 * 用于验证暂存区写入、读回等 1-Wire 数据完整性
 */
u16 PRB480_CalcCRC16(u8 *data, u8 len)
{
    return Cal_Crc16(data, len);  /* 调用外部 CRC16 库函数 */
}

/**
 * @func PRB480_CalcSHA1
 * @brief 计算数据的 SHA1 消息摘要
 * 用于 MAC 签名验证，验证数据的真实性和完整性
 */
void PRB480_CalcSHA1(unsigned char *data, unsigned int len, unsigned char mac[20])
{
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, data, len);
    SHA1Final(mac, &ctx);  /* 输出 20 字节的 SHA1 摘要 */
}

/* ========== PRB480 设备寻址命令 ========== */

/**
 * @func PRB480_ReadROM
 * @brief 读取 PRB480 的 ROM 地址 (唯一 ID)
 * 命令序列：
 *   1. 发送复位 + 应答检测
 *   2. 发送 0x33 (Read ROM 命令)
 *   3. 读取 8 字节的 ROM ID
 * @param rom[8] 输出缓冲区，存放 8 字节的 ROM ID
 * @return 0=成功, 1=失败
 */
u8 PRB480_ReadROM(u8 rom[8])
{
    u8 i;

    if (PRB480_Reset()) return 1;  /* 复位失败 */

    PRB480_WriteByte(0x33);  /* 发送 Read ROM 命令 */
    for (i = 0; i < 8; i++)
    {
        rom[i] = PRB480_ReadByte();  /* 读取 8 字节 ROM ID */
    }
    return 0;
}

/**
 * @func PRB480_SkipROM
 * @brief 跳过 ROM 寻址
 * 命令：0xCC
 * 用途：在总线上只有一个设备，或者想同时寻址所有设备时使用
 */
void PRB480_SkipROM(void)
{
    PRB480_WriteByte(0xCC);  /* Skip ROM 命令 */
}

/**
 * @func PRB480_MatchROM
 * @brief 匹配特定的 ROM 地址
 * 命令序列：
 *   1. 发送 0x55 (Match ROM 命令)
 *   2. 发送 8 字节的 ROM ID
 * 用途：在多设备总线上选择特定的设备
 */
void PRB480_MatchROM(u8 rom[8])
{
    u8 i;

    PRB480_WriteByte(0x55);  /* Match ROM 命令 */
    for (i = 0; i < 8; i++)
    {
        PRB480_WriteByte(rom[i]);  /* 发送设备的 ROM ID */
    }
}

/**
 * @func PRB480_CommandStart
 * @brief 发送通用的前导序列 (复位 + 寻址)
 * 所有 PRB480 数据操作命令的通用前导
 * @param rom ROM ID 指针，NULL 则使用 Skip ROM，否则使用 Match ROM
 * @return 0=成功, 1=复位失败
 */
static u8 PRB480_CommandStart(u8 *rom)
{
    if (PRB480_Reset()) return 1;  /* 复位并检测应答 */
    if (rom)
    {
        PRB480_MatchROM(rom);  /* 使用 ROM ID 寻址特定设备 */
    }
    else
    {
        PRB480_SkipROM();  /* 广播方式，寻址所有设备 */
    }
    return 0;
}

/* ========== PRB480 数据操作命令 ========== */

/**
 * @func PRB480_ReadMemory
 * @brief 0xF0 - 读取 PRB480 内存数据
 * 命令序列：
 *   1. 复位 + 寻址
 *   2. 发送 0xF0 命令
 *   3. 发送起始地址 (低字节, 高字节)
 *   4. 读取指定长度的数据
 * @param rom ROM ID 指针 (NULL 则 Skip ROM)
 * @param addr 起始地址 (16位)
 * @param buf 输出缓冲区
 * @param len 读取长度
 * @return 0=成功, 1=失败
 */
u8 PRB480_ReadMemory(u8 *rom, u16 addr, u8 *buf, u8 len)
{
    u8 i;

    if (PRB480_CommandStart(rom)) return 1;

    PRB480_WriteByte(0xF0);  /* Read Memory 命令 */
    PRB480_WriteByte((u8)(addr & 0xFF));      /* 地址低字节 */
    PRB480_WriteByte((u8)(addr >> 8));        /* 地址高字节 */

    for (i = 0; i < len; i++)
    {
        buf[i] = PRB480_ReadByte();  /* 逐字节读取 */
    }
    return 0;
}

/**
 * @func PRB480_WriteScratchpad
 * @brief 0x0F - 写入暂存区
 * 暂存区是 PRB480 的临时数据缓冲区，最大 8 字节
 * 命令序列：
 *   1. 复位 + 寻址
 *   2. 发送 0x0F 命令
 *   3. 发送起始地址 (低字节, 高字节)
 *   4. 发送数据
 *   5. 读取 CRC16 (PRB480 计算的校验值)
 * CRC16 用于验证数据完整性
 * @param rom ROM ID 指针
 * @param addr 起始地址 (通常 0x0000)
 * @param dat 要写入的数据
 * @param len 写入长度 (最大 8)
 * @param crc 输出，PRB480 返回的 CRC16 值
 * @return 0=CRC16 匹配, 1=CRC 错误或失败
 */
u8 PRB480_WriteScratchpad(u8 *rom, u16 addr, u8 *dat, u8 len, u16 *crc) /* 写 8 字节到 scratchpad */
{                                                                        
    u8 tx[3 + PRB480_SCRATCHPAD_SIZE];                                  /* 保存命令码、地址、数据，用于本地 CRC */
    u8 i;                                                               /* 循环变量 */
    u8 crcLo;                                                           /* 芯片返回 CRC 低字节 */
    u8 crcHi;                                                           /* 芯片返回 CRC 高字节 */
    u16 busCrc;                                                         /* 总线上读到的 CRC16 */
    u16 localCrc;                                                       /* 本地计算的 CRC16 */

    if (dat == 0) return 1;                                             /* 数据指针不能为空 */
    if (len != PRB480_SCRATCHPAD_SIZE) return 1;                        /* 手册要求 scratchpad 写入固定 8 字节 */
    if (PRB480_CheckWriteAddress(addr)) return 1;                       /* 写地址必须 8 字节对齐 */
    if (PRB480_CommandStart(rom)) return 1;                             /* 发送 Reset + ROM 寻址，失败则返回 */

    PRB480_WriteByte(0x0F);                                             /* 发送 Write Scratchpad 命令码 0x0F */
    PRB480_WriteByte((u8)(addr & 0xFF));                                /* 发送 TA1，目标地址低字节 */
    PRB480_WriteByte((u8)(addr >> 8));                                  /* 发送 TA2，目标地址高字节 */

    tx[0] = 0x0F;                                                       /* CRC 输入第 1 字节：命令码 */
    tx[1] = (u8)(addr & 0xFF);                                          /* CRC 输入第 2 字节：TA1 */
    tx[2] = (u8)(addr >> 8);                                            /* CRC 输入第 3 字节：TA2 */

    for (i = 0; i < PRB480_SCRATCHPAD_SIZE; i++)                        /* 循环发送 8 字节数据 */
    {                                                                   /* 循环开始 */
        PRB480_WriteByte(dat[i]);                                       /* 发送第 i 个数据字节 */
        tx[3 + i] = dat[i];                                             /* 保存第 i 个数据字节用于 CRC */
    }                                                                   /* 循环结束 */

    crcLo = PRB480_ReadByte();                                          /* 读取芯片返回的 CRC 低字节 */
    crcHi = PRB480_ReadByte();                                          /* 读取芯片返回的 CRC 高字节 */
    busCrc = crcLo | ((u16)crcHi << 8);                                 /* 合成总线返回的 CRC16 */

    localCrc = PRB480_CalcCRC16(tx, 3 + PRB480_SCRATCHPAD_SIZE);         /* 本地计算 CRC16：0x0F + TA1 + TA2 + 8字节数据 */

    if (crc) *crc = busCrc;                                             /* 如果调用者需要，就输出芯片返回 CRC */

    return (busCrc == localCrc) ? 0 : 1;                                /* CRC 一致返回成功，否则失败 */
}                                                                        

/**
 * @func PRB480_ReadScratchpad
 * @brief 0xAA - 读取暂存区并验证 CRC16
 * 命令序列：
 *   1. 复位 + 寻址
 *   2. 发送 0xAA 命令
 *   3. 读取 TA1, TA2, E/S 三个状态字节
 *   4. 读取暂存区数据
 *   5. 读取 CRC16 验证
 * @param rom ROM ID 指针
 * @param ta1 输出，地址字节 1
 * @param ta2 输出，地址字节 2
 * @param es 输出，结尾状态字 (E/S)
 * @param buf 输出，暂存区数据
 * @param len 读取长度
 * @param crc 输出，PRB480 的 CRC16
 * @return 0=CRC 匹配成功, 1=CRC 错误
 */
u8 PRB480_ReadScratchpad(u8 *rom, u8 *ta1, u8 *ta2, u8 *es, u8 *buf, u8 len, u16 *crc) /* 读取 scratchpad */
{                                                                                       
    u8 verify[4 + PRB480_SCRATCHPAD_SIZE];                                             /* 保存 CRC 校验输入数据 */
    u8 i;                                                                              /* 循环变量 */
    u8 crcLo;                                                                          /* 芯片返回 CRC 低字节 */
    u8 crcHi;                                                                          /* 芯片返回 CRC 高字节 */
    u16 busCrc;                                                                        /* 总线上读到的 CRC16 */
    u16 localCrc;                                                                      /* 本地计算的 CRC16 */

    if (ta1 == 0 || ta2 == 0 || es == 0 || buf == 0) return 1;                         /* 输出指针不能为空 */
    if (len != PRB480_SCRATCHPAD_SIZE) return 1;                                       /* 手册要求读取完整 8 字节 scratchpad */
    if (PRB480_CommandStart(rom)) return 1;                                            /* 发送 Reset + ROM 寻址，失败则返回 */

    PRB480_WriteByte(0xAA);                                                            /* 发送 Read Scratchpad 命令码 0xAA */

    *ta1 = PRB480_ReadByte();                                                          /* 读取 TA1，目标地址低字节 */
    *ta2 = PRB480_ReadByte();                                                          /* 读取 TA2，目标地址高字节 */
    *es  = PRB480_ReadByte();                                                          /* 读取 E/S，传输状态字节 */

    for (i = 0; i < PRB480_SCRATCHPAD_SIZE; i++)                                       /* 循环读取 8 字节 scratchpad 数据 */
    {                                                                                  /* 循环开始 */
        buf[i] = PRB480_ReadByte();                                                    /* 读取第 i 个数据字节 */
    }                                                                                  /* 循环结束 */

    crcLo = PRB480_ReadByte();                                                         /* 读取 CRC 低字节 */
    crcHi = PRB480_ReadByte();                                                         /* 读取 CRC 高字节 */
    busCrc = crcLo | ((u16)crcHi << 8);                                                /* 合成总线返回的 CRC16 */

    verify[0] = 0xAA;                                                                  /* CRC 输入第 1 字节：Read Scratchpad 命令码 */
    verify[1] = *ta1;                                                                  /* CRC 输入第 2 字节：TA1 */
    verify[2] = *ta2;                                                                  /* CRC 输入第 3 字节：TA2 */
    verify[3] = *es;                                                                   /* CRC 输入第 4 字节：E/S */

    for (i = 0; i < PRB480_SCRATCHPAD_SIZE; i++)                                       /* 循环保存 8 字节数据到 CRC 输入数组 */
    {                                                                                  /* 循环开始 */
        verify[4 + i] = buf[i];                                                        /* CRC 输入后续字节：scratchpad 数据 */
    }                                                                                  /* 循环结束 */

    localCrc = PRB480_CalcCRC16(verify, 4 + PRB480_SCRATCHPAD_SIZE);                   /* 本地计算 CRC16：0xAA + TA1 + TA2 + E/S + 8字节数据 */

    if (crc) *crc = busCrc;                                                            /* 如果调用者需要，就输出芯片返回 CRC */

    return (busCrc == localCrc) ? 0 : 1;                                               /* CRC 一致返回成功，否则失败 */
}                                                                                       

/**
 * @func PRB480_WriteAndVerifyScratchpad
 * @brief 写入 8 字节 scratchpad，并按手册“带验证的写操作”读回确认
 * 流程：
 *   1. WriteScratchpad 写入 8 字节数据
 *   2. ReadScratchpad 读回 TA1、TA2、E/S、8 字节数据和 CRC
 *   3. 比较 TA1/TA2、E/S 状态和读回数据
 * @param rom ROM ID 指针
 * @param addr 目标地址，必须 8 字节对齐
 * @param dat 期望写入的 8 字节数据
 * @param es 输出，验证通过后的 E/S 字节
 * @return 0=写入并验证成功, 1=失败
 */
u8 PRB480_WriteAndVerifyScratchpad(u8 *rom, u16 addr, u8 *dat, u8 *es)
{
    u16 crc;            /* 保存 Write Scratchpad 返回的 CRC16 */
    u8 ta1;             /* 保存 Read Scratchpad 读回的 TA1 */
    u8 ta2;             /* 保存 Read Scratchpad 读回的 TA2 */
    u8 localEs;         /* 保存 Read Scratchpad 读回的 E/S */

    if (dat == 0) return 1;                                                        /* 写入数据指针不能为空 */
    if (PRB480_WriteScratchpad(rom, addr, dat, PRB480_SCRATCHPAD_SIZE, &crc)) return 1; /* 先写满 8 字节 scratchpad */
    if (PRB480_VerifyScratchpad(rom, addr, dat, &ta1, &ta2, &localEs)) return 1;   /* 再读回并验证地址、状态、数据和 CRC */

    if (es) *es = localEs;                                                        /* 如果调用者需要，就返回验证后的 E/S */

    return 0;                                                                     /* scratchpad 写入并验证成功 */
}
/**
 * @func PRB480_LoadFirstSecret
 * @brief 0x5A - 加载第一密钥到内部存储
 * 这是一个高层命令，包含三个步骤：
 *   1. WriteScratchpad - 将密钥写入暂存区
 *   2. ReadScratchpad - 从暂存区读回验证
 *   3. LoadFirstSecret - 将暂存区的数据加载到内部存储
 * @param rom ROM ID 指针
 * @param addr 密钥存放地址
 * @param secret 8 字节密钥数据
 * @param es 输出，结尾状态字
 * @return 0=成功, 1=失败
 */
u8 PRB480_LoadFirstSecret(u8 *rom, u16 addr, u8 *secret, u8 *es) /* 加载 first secret */
{                                                                 
    u16 crc;                                                     /* 保存 Write Scratchpad 返回的 CRC16 */
    u8 ta1;                                                      /* 保存 Read Scratchpad 读回的 TA1 */
    u8 ta2;                                                      /* 保存 Read Scratchpad 读回的 TA2 */
    u8 localEs;                                                  /* 保存 Read Scratchpad 读回的 E/S */
    u8 ack1;                                                     /* 保存第 1 个 ACK 字节 */
    u8 ack2;                                                     /* 保存第 2 个 ACK 字节 */

    if (secret == 0) return 1;                                   /* secret 指针不能为空 */
    if (PRB480_CheckWriteAddress(addr)) return 1;                /* secret 写入地址必须 8 字节对齐 */

    if (PRB480_WriteScratchpad(rom, addr, secret, PRB480_SCRATCHPAD_SIZE, &crc)) return 1; /* 先写 8 字节 secret 到 scratchpad */

    if (PRB480_VerifyScratchpad(rom, addr, secret, &ta1, &ta2, &localEs)) return 1;        /* 读回并验证 TA1/TA2/E/S/数据/CRC */

    if (es) *es = localEs;                                      /* 如果调用者需要，就输出验证后的 E/S */

    if (PRB480_CommandStart(rom)) return 1;                     /* 重新开始 1-Wire 事务并寻址芯片 */

    PRB480_WriteByte(0x5A);                                     /* 发送 Load First Secret 命令码 0x5A */
    PRB480_WriteByte(ta1);                                      /* 发送已验证的 TA1 */
    PRB480_WriteByte(ta2);                                      /* 发送已验证的 TA2 */
    PRB480_WriteByte(localEs);                                  /* 发送已验证的 E/S */

    delay_ms(20);                                               /* 等待内部写入 secret 完成 */

    ack1 = PRB480_ReadByte();                                   /* 读取第 1 个 ACK 字节 */
    ack2 = PRB480_ReadByte();                                   /* 读取第 2 个 ACK 字节 */

    return (ack1 == 0xAA && ack2 == 0xAA) ? 0 : 1;              /* 两个 ACK 都是 0xAA 表示成功 */
}                                                                

/**
 * @func PRB480_CopyScratchpad
 * @brief 0x55 - 复制暂存区到内存（需要 MAC 签名）
 * PRB480 会验证提供的 MAC 签名，只有签名正确才会执行复制操作
 * @param rom ROM ID 指针
 * @param addr 目标地址
 * @param es 结尾状态字 (从 ReadScratchpad 获得)
 * @param mac 20 字节的 MAC 签名（必须正确）
 * @return 0=成功, 1=MAC 错误或失败
 */
u8 PRB480_CopyScratchpad(u8 *rom, u16 addr, u8 es, u8 mac[20]) /* 复制 scratchpad 到 EEPROM */
{                                                               
    u8 i;                                                      /* 循环变量 */
    u8 ack1;                                                   /* 保存第 1 个 ACK 字节 */
    u8 ack2;                                                   /* 保存第 2 个 ACK 字节 */

    if (mac == 0) return 1;                                    /* MAC 指针不能为空 */
    if (PRB480_CheckWriteAddress(addr)) return 1;              /* 目标地址必须 8 字节对齐 */
    if (PRB480_CheckScratchpadES(es)) return 1;                /* E/S 必须是刚才 Read Scratchpad 验证通过的值 */
    if (PRB480_CommandStart(rom)) return 1;                    /* 发送 Reset + ROM 寻址，失败则返回 */

    PRB480_WriteByte(0x55);                                    /* 发送 Copy Scratchpad 命令码 0x55 */
    PRB480_WriteByte((u8)(addr & 0xFF));                       /* 发送 TA1，目标地址低字节 */
    PRB480_WriteByte((u8)(addr >> 8));                         /* 发送 TA2，目标地址高字节 */
    PRB480_WriteByte(es);                                      /* 发送 E/S，必须来自 Read Scratchpad */

    delay_ms(10);                                              /* 等待芯片准备/计算授权流程 */

    for (i = 0; i < 20; i++)                                   /* 循环发送 20 字节 MAC */
    {                                                          /* 循环开始 */
        PRB480_WriteByte(mac[i]);                              /* 发送第 i 个 MAC 字节 */
    }                                                          /* 循环结束 */

    delay_ms(20);                                              /* 等待 EEPROM 编程完成 */

    ack1 = PRB480_ReadByte();                                  /* 读取第 1 个 ACK 字节 */
    ack2 = PRB480_ReadByte();                                  /* 读取第 2 个 ACK 字节 */

    return (ack1 == 0xAA && ack2 == 0xAA) ? 0 : 1;             /* 两个 ACK 都是 0xAA 表示复制成功 */
}                                                               

/**
 * @func PRB480_ReadAuthenticatedPage
 * @brief 0xA5 - 读取受保护的页面及其 MAC 签名
 * 命令序列：
 *   1. 复位 + 寻址
 *   2. 发送 0xA5 命令和地址
 *   3. 读取 32 字节的页面数据
 *   4. 读取 0xFF 分隔符
 *   5. 读取 CRC16 (可选忽略)
 *   6. 延迟 20ms (PRB480 计算 MAC)
 *   7. 读取 20 字节的 SHA1-MAC 签名
 *   8. 读取第二个 CRC16
 *   9. 读取状态字节
 * @param rom ROM ID 指针
 * @param addr 页面地址
 * @param page 输出，32 字节的页面数据
 * @param mac 输出，20 字节的 SHA1-MAC 签名
 * @return 0=成功, 1=失败
 */
u8 PRB480_ReadAuthenticatedPage(u8 *rom, u16 addr, u8 page[32], u8 mac[20])
{
    u8 i;
    u8 separator;
    u8 crcLo;
    u8 crcHi;
    u8 crc2Lo;
    u8 crc2Hi;
    u8 status1;
    u8 status2;

    if (PRB480_CommandStart(rom)) return 1;

    PRB480_WriteByte(0xA5);  /* Read Authenticated Page 命令 */
    PRB480_WriteByte((u8)(addr & 0xFF));      /* 地址低字节 */
    PRB480_WriteByte((u8)(addr >> 8));        /* 地址高字节 */

    /* 读取 32 字节的页面数据 */
    for (i = 0; i < 32; i++)
    {
        page[i] = PRB480_ReadByte();
    }

    separator = PRB480_ReadByte();  /* 读取 0xFF 分隔符 */
    if (separator != 0xFF) return 1;  /* 分隔符检查 */

    /* 读取第一个 CRC16 (可选) */
    crcLo = PRB480_ReadByte();
    crcHi = PRB480_ReadByte();
    (void)crcLo;  /* 防止编译警告 */
    (void)crcHi;

    /* PRB480 计算 MAC 需要 20ms */
    delay_ms(20);
    
    /* 读取 20 字节的 MAC 签名 */
    for (i = 0; i < 20; i++)
    {
        mac[i] = PRB480_ReadByte();
    }

    /* 读取第二个 CRC16 (可选) */
    crc2Lo = PRB480_ReadByte();
    crc2Hi = PRB480_ReadByte();
    (void)crc2Lo;
    (void)crc2Hi;

    /* 读取状态字节 */
    status1 = PRB480_ReadByte();
    status2 = PRB480_ReadByte();
    return (status1 == 0x00 && status2 == 0x01) ? 0 : 1;  /* 状态检查 */
}
