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

static u8 PRB480_CheckAlternatingResponse(u8 first, u8 second);                       /* 检查成功后的交替响应 */
static u8 PRB480_CalcCRC8(u8 *data, u8 len);                                          /* 计算 ROM 用 CRC8 */
static u16 PRB480_PageStart(u16 addr);                                                /* 取得 32 字节页首地址 */
static u8 PRB480_VerifyReadAuthPageCRC(u8 *prefix, u8 prefix_len, u8 *payload, u8 payload_len, u16 bus_crc); /* 校验认证读 CRC16 */
static void PRB480_RunMacCompression(const u8 message[64], u32 state[5]);            /* 执行 PRB480 单块 SHA-1 压缩 */
static void PRB480_StateToBusMAC(const u32 state[5], u8 mac[20]);                    /* 把内部状态转换成总线输出 MAC */
static void PRB480_GenerateCopyScratchpadMAC(u8 *secret, u8 *rom, u16 addr, u8 *page, u8 *scratchpad, u8 mac[20]); /* 生成写授权 MAC */
static void PRB480_GenerateReadAuthPageMAC(u8 *secret, u8 *rom, u16 addr, u8 *page, u8 challenge[3], u8 mac[20]);  /* 生成认证读校验 MAC */
static u8 PRB480_LoadChallengeScratchpad(u8 *rom, u16 addr, u8 challenge[3], u8 *es); /* challenge 写入 scratchpad */
static u8 PRB480_ReadAuthenticatedPageRaw(u8 *rom, u16 addr, u8 challenge[3], PRB480_AuthenticatedPagePacket *packet); /* 原始认证读流程 */

static u8 PRB480_CheckPresence(void);        /* 检测 1-Wire presence 应答 */
static u8 PRB480_CommandStart(u8 *rom);      /* 发送 Reset + ROM 寻址命令 */
static u8 PRB480_CheckWriteAddress(u16 addr);/* 检查写地址是否 8 字节对齐 */
static u8 PRB480_CheckScratchpadES(u8 es);   /* 检查 E/S 状态是否合法 */
static u8 PRB480_VerifyScratchpad(u8 *rom, u16 addr, u8 *expected, u8 *ta1, u8 *ta2, u8 *es); /* 读回并验证 scratchpad */


/*******************************************************************************
* 名    称         : PRB480_CheckWriteAddress
* 功    能         : 检查 Write Scratchpad 流程允许使用的目标地址是否合法
* 说    明         : 对应图 8a 中 “Address < A0h?” 的判断
*                    1. 地址必须小于等于 0x009F
*                    2. 地址必须 8 字节对齐，因为地址低 3 位会被器件强制清零
* 输入参数         : addr - 目标地址
* 返 回 值         : 0 合法，1 非法
*******************************************************************************/
static u8 PRB480_CheckWriteAddress(u16 addr)
{
    /* 先检查地址是否越过图 8a 允许的 scratchpad 命令地址窗口 */
    if (addr > 0x009F) return 1;                  /* 大于 0x009F 直接判非法 */

    /* 再检查地址是否为 8 字节对齐 */
    if (addr & 0x0007) return 1;                  /* 低 3 位非 0 说明未按 8 字节块对齐 */

    /* 两项都通过则地址合法 */
    return 0;                                     /* 返回 0 表示合法 */
}                                            


/*******************************************************************************
* 名    称         : PRB480_CheckAlternatingResponse
* 功    能         : 检查器件是否返回成功时的交替位型
* 说    明         : PRB480 / DS28E01 系列在某些命令成功后，不一定返回两个 0xAA，
*                    而是返回交替位型，对应按字节读通常表现为 0xAA / 0x55 或
*                    0x55 / 0xAA
* 输入参数         : first  - 第 1 个读回字节
*                    second - 第 2 个读回字节
* 返 回 值         : 0 成功模式，1 非成功模式
*******************************************************************************/
static u8 PRB480_CheckAlternatingResponse(u8 first, u8 second)
{
    /* 判断是否是 0xAA 后跟 0x55 */
    if ((first == 0xAA && second == 0x55) ||      /* 第一种成功模式 */
        (first == 0x55 && second == 0xAA))        /* 第二种成功模式 */
    {
        return 0;                                 /* 返回 0 表示交替响应正确 */
    }

    /* 其他组合一律按失败处理 */
    return 1;                                     /* 返回 1 表示不是成功响应 */
}

/*******************************************************************************
* 名    称         : PRB480_CalcCRC8
* 功    能         : 计算 1-Wire ROM 使用的 CRC8
* 说    明         : 用于校验 Read ROM 返回的 8 字节注册码是否正确，
*                    第 8 字节本身就是前 7 字节的 CRC8
* 输入参数         : data - 数据缓冲区
*                    len  - 数据长度
* 返 回 值         : CRC8 结果；对完整合法 8 字节 ROM 来说应为 0
*******************************************************************************/
static u8 PRB480_CalcCRC8(u8 *data, u8 len)
{
    u8 crc = 0;                                   /* CRC8 初值为 0 */
    u8 i;                                         /* 外层字节循环变量 */
    u8 j;                                         /* 内层位循环变量 */
    u8 current;                                   /* 当前参与计算的字节 */
    u8 mix;                                       /* 当前最低位混合结果 */

    /* 逐字节处理输入数据 */
    for (i = 0; i < len; i++)                     /* 遍历 data[0] ~ data[len-1] */
    {
        current = data[i];                        /* 取出当前字节 */

        /* 每个字节逐位参与 CRC8 计算 */
        for (j = 0; j < 8; j++)                   /* 1 个字节共 8 位 */
        {
            mix = (crc ^ current) & 0x01;         /* 取 CRC 和当前数据最低位异或结果 */
            crc >>= 1;                            /* CRC 右移 1 位，为反馈多项式做准备 */

            if (mix)                              /* 如果最低位异或结果为 1 */
            {
                crc ^= 0x8C;                      /* 异或 1-Wire CRC8 反馈项 */
            }

            current >>= 1;                        /* 当前数据右移，准备处理下一位 */
        }
    }

    /* 返回 CRC8 结果 */
    return crc;                                   /* 完整合法 ROM 应得到 0 */
}

/*******************************************************************************
* 名    称         : PRB480_PageStart
* 功    能         : 将任意地址折算到所在 32 字节页的首地址
* 输入参数         : addr - 任意地址
* 返 回 值         : 页首地址，如 0x0067 -> 0x0060
*******************************************************************************/
static u16 PRB480_PageStart(u16 addr)
{
    return (u16)(addr & 0xFFE0);                  /* 清零低 5 位，保留页首地址 */
}

/*******************************************************************************
* 名    称         : PRB480_VerifyReadAuthPageCRC
* 功    能         : 校验 Read Authenticated Page 命令中的 CRC16
* 说    明         : 该函数用于校验两类 CRC：
*                    1. 页面数据段 CRC16
*                    2. MAC 数据段 CRC16
* 输入参数         : prefix      - CRC 前缀数据
*                    prefix_len  - 前缀长度
*                    payload     - 主体数据
*                    payload_len - 主体长度
*                    bus_crc     - 器件返回的 CRC16
* 返 回 值         : 0 校验正确，1 校验失败
*******************************************************************************/
static u8 PRB480_VerifyReadAuthPageCRC(u8 *prefix, u8 prefix_len, u8 *payload, u8 payload_len, u16 bus_crc)
{
    u8 verify[40];                                /* 临时拼装需要参与 CRC 的完整数据 */
    u8 i;                                         /* 循环变量 */
    u8 offset = 0;                                /* 当前写入 verify 的偏移 */
    u16 local_crc;                                /* 主机本地计算得到的 CRC16 */

    /* 先拷贝前缀数据 */
    for (i = 0; i < prefix_len; i++)              /* 遍历 prefix */
    {
        verify[offset++] = prefix[i];             /* 依次写入临时缓冲区 */
    }

    /* 再拷贝主体数据 */
    for (i = 0; i < payload_len; i++)             /* 遍历 payload */
    {
        verify[offset++] = payload[i];            /* 继续顺序写入临时缓冲区 */
    }

    /* 对拼装出的完整数据计算本地 CRC16 */
    local_crc = PRB480_CalcCRC16(verify, offset); /* 计算主机侧 CRC16 */

    /* 将本地 CRC16 与总线返回值比较 */
    return (local_crc == bus_crc) ? 0 : 1;        /* 相同返回 0，否则返回 1 */
}

/*******************************************************************************
* 名    称         : PRB480_RunMacCompression
* 功    能         : 按 PRB480/DS28E01 规则计算单个 64 字节块的 SHA-1 压缩结果
* 说    明         : 这里不是普通 SHA1(message) 的最终摘要输出，
*                    而是器件认证算法使用的单块压缩后 A/B/C/D/E 原始状态值
* 输入参数         : message - 64 字节输入块
* 输出参数         : state   - 5 个 32 位状态字
* 返 回 值         : 无
*******************************************************************************/
static void PRB480_RunMacCompression(const u8 message[64], u32 state[5])
{
    u32 w[80];                                    /* SHA-1 消息调度数组 */
    u32 a;                                        /* SHA-1 工作寄存器 A */
    u32 b;                                        /* SHA-1 工作寄存器 B */
    u32 c;                                        /* SHA-1 工作寄存器 C */
    u32 d;                                        /* SHA-1 工作寄存器 D */
    u32 e;                                        /* SHA-1 工作寄存器 E */
    u32 t;                                        /* 临时变量 */
    u8 i;                                         /* 循环变量 */

    /* 先把 64 字节输入块拆成 16 个 32 位大端字 */
    for (i = 0; i < 16; i++)                      /* 遍历 16 个原始字 */
    {
        w[i] = ((u32)message[i * 4] << 24) |      /* message[4*i+0] 作为最高字节 */
               ((u32)message[i * 4 + 1] << 16) |  /* message[4*i+1] 作为次高字节 */
               ((u32)message[i * 4 + 2] << 8) |   /* message[4*i+2] 作为次低字节 */
               ((u32)message[i * 4 + 3]);         /* message[4*i+3] 作为最低字节 */
    }

    /* 扩展成 80 个 32 位字 */
    for (i = 16; i < 80; i++)                     /* 从第 16 个字扩展到第 79 个字 */
    {
        t = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]; /* 按 SHA-1 规则异或组合 */
        w[i] = (t << 1) | (t >> 31);              /* 左循环移 1 位 */
    }

    /* 初始化 5 个工作寄存器 */
    a = 0x67452301UL;                             /* A 初值 */
    b = 0xEFCDAB89UL;                             /* B 初值 */
    c = 0x98BADCFEUL;                             /* C 初值 */
    d = 0x10325476UL;                             /* D 初值 */
    e = 0xC3D2E1F0UL;                             /* E 初值 */

    /* 执行 80 轮压缩运算 */
    for (i = 0; i < 80; i++)                      /* i 对应 SHA-1 的第 i 轮 */
    {
        if (i < 20)                               /* 前 20 轮使用 f1 和常数 K1 */
            t = ((b & c) | ((~b) & d)) + 0x5A827999UL; /* 选择函数 */
        else if (i < 40)                          /* 第 20~39 轮使用异或函数 */
            t = (b ^ c ^ d) + 0x6ED9EBA1UL;       /* 异或函数 */
        else if (i < 60)                          /* 第 40~59 轮使用多数函数 */
            t = ((b & c) | (b & d) | (c & d)) + 0x8F1BBCDCUL; /* 多数函数 */
        else                                      /* 第 60~79 轮再次使用异或函数 */
            t = (b ^ c ^ d) + 0xCA62C1D6UL;       /* 异或函数 */

        t += ((a << 5) | (a >> 27)) + e + w[i];   /* 累加左旋后的 A、E 和本轮消息字 */
        e = d;                                    /* 工作寄存器向后移位 */
        d = c;                                    /* 工作寄存器向后移位 */
        c = (b << 30) | (b >> 2);                 /* B 左旋 30 位写入 C */
        b = a;                                    /* A 写入 B */
        a = t;                                    /* 计算结果写回 A */
    }

    /* 输出最终压缩状态 */
    state[0] = a;                                 /* 保存 A */
    state[1] = b;                                 /* 保存 B */
    state[2] = c;                                 /* 保存 C */
    state[3] = d;                                 /* 保存 D */
    state[4] = e;                                 /* 保存 E */
}

/*******************************************************************************
* 名    称         : PRB480_StateToBusMAC
* 功    能         : 将 A/B/C/D/E 状态字转换成器件总线发送顺序的 20 字节 MAC
* 说    明         : 总线顺序不是 A->E，而是 E、D、C、B、A，
*                    且每个 32 位字按低字节在前发送
* 输入参数         : state - 5 个 32 位状态字
* 输出参数         : mac   - 20 字节 MAC
* 返 回 值         : 无
*******************************************************************************/
static void PRB480_StateToBusMAC(const u32 state[5], u8 mac[20])
{
    u32 word;                                     /* 当前正在拆分输出的 32 位状态字 */
    u8 i;                                         /* 单个 32 位字内的字节循环变量 */
    u8 out = 0;                                   /* MAC 输出缓冲区偏移 */
    u8 idx;                                       /* 状态字索引 */

    /* 按 E、D、C、B 的顺序输出 */
    for (idx = 4; idx > 0; idx--)                /* 先处理 state[4] 到 state[1] */
    {
        word = state[idx];                        /* 取当前状态字 */

        for (i = 0; i < 4; i++)                  /* 每个状态字拆成 4 个字节 */
        {
            mac[out++] = (u8)(word & 0xFF);      /* 先输出最低字节 */
            word >>= 8;                           /* 再右移准备下一字节 */
        }
    }

    /* 最后输出 A，也就是 state[0] */
    word = state[0];                              /* 取 A 状态字 */
    for (i = 0; i < 4; i++)                       /* 同样按低字节在前输出 */
    {
        mac[out++] = (u8)(word & 0xFF);          /* 输出当前最低字节 */
        word >>= 8;                               /* 右移准备下一个字节 */
    }
}

/*******************************************************************************
* 名    称         : PRB480_GenerateCopyScratchpadMAC
* 功    能         : 生成 Copy Scratchpad 写授权 MAC
* 说    明         : 输入块由以下部分组成：
*                    1. 当前 secret 前 4 字节
*                    2. 目标页前 28 字节旧数据
*                    3. scratchpad 中待写入的 8 字节新数据
*                    4. 页面选择字节
*                    5. ROM 前 7 字节
*                    6. 当前 secret 后 4 字节
*                    7. 固定尾巴
* 输入参数         : secret     - 当前 8 字节 secret
*                    rom        - 8 字节 ROM ID
*                    addr       - 目标地址
*                    page       - 目标页 32 字节数据
*                    scratchpad - 8 字节待写入数据
* 输出参数         : mac        - 20 字节 MAC
* 返 回 值         : 无
*******************************************************************************/
static void PRB480_GenerateCopyScratchpadMAC(u8 *secret, u8 *rom, u16 addr, u8 *page, u8 *scratchpad, u8 mac[20])
{
    u8 msg[64] = {0};                             /* 认证算法使用的 64 字节输入块 */
    u32 state[5];                                 /* 压缩后的 A/B/C/D/E 状态字 */
    u8 i;                                         /* 循环变量 */

    /* 填入 secret 前 4 字节 */
    for (i = 0; i < 4; i++)                       /* i = 0~3 */
        msg[i] = secret[i];                       /* msg[0..3] <- secret[0..3] */

    /* 填入目标页前 28 字节旧数据 */
    for (i = 0; i < 28; i++)                      /* i = 0~27 */
        msg[4 + i] = page[i];                     /* msg[4..31] <- page[0..27] */

    /* 填入 8 字节待写入新数据 */
    for (i = 0; i < 8; i++)                       /* i = 0~7 */
        msg[32 + i] = scratchpad[i];              /* msg[32..39] <- scratchpad[0..7] */

    /* 填入页面选择字节，Copy Scratchpad 的高 4 位为 0 */
    msg[40] = (u8)((PRB480_PageStart(addr) >> 5) & 0x0F); /* 低 4 位为页号 */

    /* 填入 ROM 前 7 字节，不含 ROM CRC8 */
    for (i = 0; i < 7; i++)                       /* i = 0~6 */
        msg[41 + i] = rom[i];                     /* msg[41..47] <- rom[0..6] */

    /* 填入 secret 后 4 字节 */
    for (i = 0; i < 4; i++)                       /* i = 0~3 */
        msg[48 + i] = secret[4 + i];              /* msg[48..51] <- secret[4..7] */

    /* 填入固定尾巴 */
    msg[52] = 0xFF;                               /* 固定字节 1 */
    msg[53] = 0xFF;                               /* 固定字节 2 */
    msg[54] = 0xFF;                               /* 固定字节 3 */
    msg[55] = 0x80;                               /* SHA-1 padding 起始字节 */
    msg[56] = 0x00;                               /* 固定 0 */
    msg[57] = 0x00;                               /* 固定 0 */
    msg[58] = 0x00;                               /* 固定 0 */
    msg[59] = 0x00;                               /* 固定 0 */
    msg[60] = 0x00;                               /* 固定 0 */
    msg[61] = 0x00;                               /* 固定 0 */
    msg[62] = 0x01;                               /* 长度高字节 */
    msg[63] = 0xB8;                               /* 长度低字节，表示 440 bit */

    /* 执行单块压缩运算 */
    PRB480_RunMacCompression(msg, state);         /* 得到 A/B/C/D/E */

    /* 按器件总线顺序输出 20 字节 MAC */
    PRB480_StateToBusMAC(state, mac);             /* 生成最终 MAC */
}

/*******************************************************************************
* 名    称         : PRB480_GenerateReadAuthPageMAC
* 功    能         : 生成 Read Authenticated Page 的主机侧校验 MAC
* 说    明         : 输入块由以下部分组成：
*                    1. 当前 secret 前 4 字节
*                    2. 32 字节页数据
*                    3. 4 个 0xFF
*                    4. 页面选择字节（高 4 位固定 0x4）
*                    5. ROM 前 7 字节
*                    6. 当前 secret 后 4 字节
*                    7. 3 字节 challenge
*                    8. 固定尾巴
* 输入参数         : secret    - 当前 8 字节 secret
*                    rom       - 8 字节 ROM ID
*                    addr      - 目标页首地址
*                    page      - 32 字节页数据
*                    challenge - 3 字节 challenge
* 输出参数         : mac       - 20 字节 MAC
* 返 回 值         : 无
*******************************************************************************/
static void PRB480_GenerateReadAuthPageMAC(u8 *secret, u8 *rom, u16 addr, u8 *page, u8 challenge[3], u8 mac[20])
{
    u8 msg[64] = {0};                             /* 认证读使用的 64 字节输入块 */
    u32 state[5];                                 /* 压缩后的内部状态 */
    u8 i;                                         /* 循环变量 */

    /* 填入 secret 前 4 字节 */
    for (i = 0; i < 4; i++)                       /* i = 0~3 */
        msg[i] = secret[i];                       /* msg[0..3] <- secret[0..3] */

    /* 填入 32 字节页数据 */
    for (i = 0; i < 32; i++)                      /* i = 0~31 */
        msg[4 + i] = page[i];                     /* msg[4..35] <- page[0..31] */

    /* 填入 4 个 0xFF */
    msg[36] = 0xFF;                               /* 固定 FF 1 */
    msg[37] = 0xFF;                               /* 固定 FF 2 */
    msg[38] = 0xFF;                               /* 固定 FF 3 */
    msg[39] = 0xFF;                               /* 固定 FF 4 */

    /* 填入页面选择字节，认证读命令的高 4 位固定为 0x4 */
    msg[40] = (u8)(0x40 | ((PRB480_PageStart(addr) >> 5) & 0x0F)); /* 0x40 + 页号 */

    /* 填入 ROM 前 7 字节 */
    for (i = 0; i < 7; i++)                       /* i = 0~6 */
        msg[41 + i] = rom[i];                     /* msg[41..47] <- rom[0..6] */

    /* 填入 secret 后 4 字节 */
    for (i = 0; i < 4; i++)                       /* i = 0~3 */
        msg[48 + i] = secret[4 + i];              /* msg[48..51] <- secret[4..7] */

    /* 填入 3 字节 challenge */
    msg[52] = challenge[0];                       /* challenge 第 1 字节 */
    msg[53] = challenge[1];                       /* challenge 第 2 字节 */
    msg[54] = challenge[2];                       /* challenge 第 3 字节 */

    /* 填入 SHA-1 padding 和长度尾巴 */
    msg[55] = 0x80;                               /* SHA-1 padding 起始 */
    msg[56] = 0x00;                               /* 固定 0 */
    msg[57] = 0x00;                               /* 固定 0 */
    msg[58] = 0x00;                               /* 固定 0 */
    msg[59] = 0x00;                               /* 固定 0 */
    msg[60] = 0x00;                               /* 固定 0 */
    msg[61] = 0x00;                               /* 固定 0 */
    msg[62] = 0x01;                               /* 长度高字节 */
    msg[63] = 0xB8;                               /* 长度低字节 */

    /* 执行单块压缩运算 */
    PRB480_RunMacCompression(msg, state);         /* 计算内部状态 */

    /* 按器件总线格式导出 MAC */
    PRB480_StateToBusMAC(state, mac);             /* 输出 20 字节 MAC */
}

/*******************************************************************************
* 名    称         : PRB480_LoadChallengeScratchpad
* 功    能         : 将 3 字节 challenge 按器件要求装入 scratchpad
* 说    明         : Read Authenticated Page 使用 scratchpad 的第 4、5、6 号字节
*                    作为 challenge，因此这里仍走 Figure 8a 的
*                    Write Scratchpad + Read Scratchpad 验证流程
* 输入参数         : rom       - 目标器件 ROM ID
*                    addr      - 目标页地址
*                    challenge - 3 字节 challenge
* 输出参数         : es        - 验证通过后的 E/S
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
static u8 PRB480_LoadChallengeScratchpad(u8 *rom, u16 addr, u8 challenge[3], u8 *es)
{
    u8 frame[8] = {0};                            /* scratchpad 8 字节临时数据 */

    frame[4] = challenge[0];                      /* 第 5 个字节放 challenge[0] */
    frame[5] = challenge[1];                      /* 第 6 个字节放 challenge[1] */
    frame[6] = challenge[2];                      /* 第 7 个字节放 challenge[2] */

    return PRB480_WriteAndVerifyScratchpad(rom, PRB480_PageStart(addr), frame, es); /* 走图 8a 验证写流程 */
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
/*******************************************************************************
* 名    称         : PRB480_ReadROM
* 功    能         : 读取器件 64 位 ROM ID 并校验 CRC8
* 说    明         : 对应 ROM Function Command 中的 Read ROM(0x33)
*                    读回的第 8 字节为前 7 字节的 CRC8
* 输入参数         : rom - 8 字节缓冲区
* 输出参数         : rom - 返回完整 ROM ID
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_ReadROM(u8 rom[8])
{
    u8 i;                                         /* ROM 字节读取循环变量 */

    if (rom == 0) return 1;                       /* 缓冲区为空则直接失败 */
    if (PRB480_Reset()) return 1;                 /* 复位失败则返回 1 */

    PRB480_WriteByte(0x33);                       /* 发送 Read ROM 命令 */

    for (i = 0; i < 8; i++)                       /* 连续读取 8 字节 ROM ID */
    {
        rom[i] = PRB480_ReadByte();               /* 读回第 i 个 ROM 字节 */
    }

    return (PRB480_CalcCRC8(rom, 8) == 0) ? 0 : 1; /* 用 CRC8 校验整个 8 字节 ROM */
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
/*******************************************************************************
* 名    称         : PRB480_LoadFirstSecret
* 功    能         : 执行 Load First Secret(0x5A) 流程
* 说    明         : 对应图 8b 左侧流程：
*                    1. 先把 8 字节 secret 写入 scratchpad
*                    2. 再用 Read Scratchpad 读回 TA1/TA2/E/S 做授权验证
*                    3. 最后发送 0x5A + TA1 + TA2 + E/S
* 输入参数         : rom    - 目标器件 ROM ID，NULL 则 Skip ROM
*                    addr   - 必须为 0x0080
*                    secret - 8 字节初始 secret
* 输出参数         : es     - 返回验证通过的 E/S
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_LoadFirstSecret(u8 *rom, u16 addr, u8 *secret, u8 *es)
{
    u16 crc;                                      /* 保存 Write Scratchpad 返回的 CRC16 */
    u8 ta1;                                       /* 保存 Read Scratchpad 返回的 TA1 */
    u8 ta2;                                       /* 保存 Read Scratchpad 返回的 TA2 */
    u8 localEs;                                   /* 保存 Read Scratchpad 返回的 E/S */
    u8 ack1;                                      /* 保存成功后的第 1 个响应字节 */
    u8 ack2;                                      /* 保存成功后的第 2 个响应字节 */

    /* -------- 执行流程说明 --------
     * 1. 检查参数和地址
     * 2. 把 secret 写入 scratchpad
     * 3. 读回 scratchpad，验证 TA1 / TA2 / E/S / 数据
     * 4. 重新发起总线事务
     * 5. 发送 0x5A + TA1 + TA2 + E/S
     * 6. 等待内部编程完成
     * 7. 读取交替响应，判断是否成功
     */

    if (secret == 0) return 1;                    /* secret 指针为空，直接失败 */
    if (addr != 0x0080) return 1;                 /* Load First Secret 只允许写入 secret 地址 */
    if (PRB480_CheckWriteAddress(addr)) return 1; /* 地址不合法则失败 */

    if (PRB480_WriteScratchpad(rom, addr, secret, 8, &crc)) return 1; /* 第 1 步：写 scratchpad */
    if (PRB480_VerifyScratchpad(rom, addr, secret, &ta1, &ta2, &localEs)) return 1; /* 第 2 步：读回验证 */

    if (es) *es = localEs;                        /* 如果调用者需要，就把验证得到的 E/S 返回出去 */
    if (PRB480_CommandStart(rom)) return 1;       /* 重新开始一帧新的命令事务 */

    PRB480_WriteByte(0x5A);                       /* 发送 Load First Secret 命令 */
    PRB480_WriteByte(ta1);                        /* 发送授权地址低字节 TA1 */
    PRB480_WriteByte(ta2);                        /* 发送授权地址高字节 TA2 */
    PRB480_WriteByte(localEs);                    /* 发送授权状态字 E/S */

    delay_ms(10);                                 /* 等待内部 secret 编程完成 */

    ack1 = PRB480_ReadByte();                     /* 读取第 1 个响应字节 */
    ack2 = PRB480_ReadByte();                     /* 读取第 2 个响应字节 */

    return PRB480_CheckAlternatingResponse(ack1, ack2); /* 用交替模式判断是否成功 */
}                                     

/*******************************************************************************
* 名    称         : PRB480_CopyScratchpad
* 功    能         : 执行 Copy Scratchpad(0x55) 认证写入命令
* 说    明         : 对应图 8b 中普通数据写入路径
*                    主机必须在发送 TA1/TA2/E/S 后，再发送正确的 20 字节 MAC
* 输入参数         : rom  - 目标器件 ROM ID，NULL 则 Skip ROM
*                    addr - 目标地址，必须在 0x0000~0x007F 且 8 字节对齐
*                    es   - Read Scratchpad 读回的 E/S
*                    mac  - 主机实时计算的 20 字节认证 MAC
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_CopyScratchpad(u8 *rom, u16 addr, u8 es, u8 mac[20])
{
    u8 i;                                         /* MAC 发送循环变量 */
    u8 ack1;                                      /* 器件返回的第 1 个响应字节 */
    u8 ack2;                                      /* 器件返回的第 2 个响应字节 */

    /* -------- 执行流程说明 --------
     * 1. 检查参数、地址、E/S
     * 2. 发起新事务
     * 3. 发送 0x55 + TA1 + TA2 + E/S
     * 4. 等待器件先计算内部 MAC
     * 5. 主机发送自己的 20 字节 MAC
     * 6. 等待内部编程完成
     * 7. 读取交替响应判断是否成功
     */

    if (mac == 0) return 1;                       /* MAC 缓冲区为空则失败 */
    if (addr > 0x007F) return 1;                  /* 只能写用户数据区 */
    if (PRB480_CheckWriteAddress(addr)) return 1; /* 地址必须满足 8 字节对齐 */
    if (PRB480_CheckScratchpadES(es)) return 1;   /* E/S 不符合完整写入条件则失败 */
    if (PRB480_CommandStart(rom)) return 1;       /* 重新开始命令事务 */

    PRB480_WriteByte(0x55);                       /* 发送 Copy Scratchpad 命令 */
    PRB480_WriteByte((u8)(addr & 0xFF));          /* 发送目标地址低字节 TA1 */
    PRB480_WriteByte((u8)(addr >> 8));            /* 发送目标地址高字节 TA2 */
    PRB480_WriteByte(es);                         /* 发送上一步验证得到的 E/S */

    delay_ms(2);                                  /* 等待器件先完成内部 MAC 计算 */

    for (i = 0; i < 20; i++)                      /* 逐字节发送主机侧 20 字节 MAC */
    {
        PRB480_WriteByte(mac[i]);                 /* 发送第 i 个 MAC 字节 */
    }

    delay_ms(10);                                 /* 等待 FRAM / EEPROM 编程完成 */

    ack1 = PRB480_ReadByte();                     /* 读取第 1 个响应字节 */
    ack2 = PRB480_ReadByte();                     /* 读取第 2 个响应字节 */

    return PRB480_CheckAlternatingResponse(ack1, ack2); /* 用交替模式判断是否成功 */
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


/*******************************************************************************
* 名    称         : PRB480_WriteAuthorizedBlock
* 功    能         : 执行完整的“认证写 8 字节块”流程
* 说    明         : 对应 Figure 8a + Figure 8b 的组合流程：
*                    1. 先读目标页旧数据
*                    2. Write Scratchpad
*                    3. Read Scratchpad 验证
*                    4. 主机根据 secret / ROM / 页数据 / 新数据计算 MAC
*                    5. Copy Scratchpad
* 输入参数         : rom    - 目标器件 ROM ID，NULL 则 Skip ROM
*                    secret - 当前 8 字节 secret
*                    addr   - 目标地址
*                    data   - 待写入 8 字节数据
* 输出参数         : es     - 验证通过后的 E/S
*                    mac    - 主机计算出的 20 字节 MAC
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_WriteAuthorizedBlock(u8 *rom, u8 *secret, u16 addr, u8 *data, u8 *es, u8 mac[20])
{
    u8 page[32];                                  /* 保存目标页当前旧数据 */
    u8 localEs;                                   /* 保存验证通过后的 E/S */

    /* -------- 执行流程说明 --------
     * 1. 检查输入参数和地址
     * 2. 读取目标页旧数据，因为写授权 MAC 需要它
     * 3. 走 Figure 8a：Write Scratchpad + Read Scratchpad 验证
     * 4. 生成主机侧 Copy Scratchpad 认证 MAC
     * 5. 发送 Copy Scratchpad 命令
     * 6. 如成功则返回 E/S 和 MAC
     */

    if (secret == 0 || data == 0 || mac == 0) return 1; /* 任一必要指针为空都失败 */
    if (addr > 0x007F) return 1;                        /* 认证写只允许写用户数据区 */
    if (PRB480_CheckWriteAddress(addr)) return 1;       /* 地址必须合法且 8 字节对齐 */

    if (PRB480_ReadMemory(rom, PRB480_PageStart(addr), page, 32)) return 1;   /* 先读整页旧数据 */
    if (PRB480_WriteAndVerifyScratchpad(rom, addr, data, &localEs)) return 1; /* 再按图 8a 写并验证 scratchpad */

    PRB480_GenerateCopyScratchpadMAC(secret, rom, addr, page, data, mac);     /* 生成写授权 MAC */

    if (PRB480_CopyScratchpad(rom, addr, localEs, mac)) return 1;             /* 发送 Copy Scratchpad 完成认证写 */

    if (es) *es = localEs;                        /* 如调用者需要，则返回 E/S */
    return 0;                                     /* 整个认证写流程成功 */
}

/*******************************************************************************
* 名    称         : PRB480_ReadAuthenticatedPageRaw
* 功    能         : 按器件真实总线流程读取认证页原始返回包
* 说    明         : 该函数只负责：
*                    1. 装载 challenge 到 scratchpad
*                    2. 发送 A5h 命令
*                    3. 接收 page / CRC / device MAC / CRC / trailer
*                    4. 完成两段 CRC 校验和末尾交替响应检查
*                    不负责主机侧 MAC 重算
* 输入参数         : rom       - 目标器件 ROM ID
*                    addr      - 页首地址，必须 32 字节对齐
*                    challenge - 3 字节 challenge
* 输出参数         : packet    - 原始返回包
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
static u8 PRB480_ReadAuthenticatedPageRaw(u8 *rom, u16 addr, u8 challenge[3], PRB480_AuthenticatedPagePacket *packet)
{
    u8 req[3];                                    /* 保存 A5h + TA1 + TA2，用于第一段 CRC 校验 */
    u8 page_crc_buf[33];                          /* 保存 32 字节页数据 + 分隔字节 0xFF */
    u8 i;                                         /* 循环变量 */
    u8 separator;                                 /* 页面数据和 CRC 之间的固定分隔字节 */
    u8 scratchpadEs;                              /* challenge 写入 scratchpad 后返回的 E/S */

    /* -------- 执行流程说明 --------
     * 1. 检查参数与页地址
     * 2. 把 3 字节 challenge 先写进 scratchpad
     * 3. 发起新的事务，发送 0xA5 + TA1 + TA2
     * 4. 接收 32 字节页数据
     * 5. 接收 0xFF 分隔字节和第 1 段 CRC16
     * 6. 接收 20 字节 device MAC 和第 2 段 CRC16
     * 7. 接收末尾交替响应字节
     * 8. 逐段校验 CRC 和 trailer
     */

    if (challenge == 0 || packet == 0) return 1;  /* challenge 或 packet 为空则失败 */
    if (addr > 0x007F) return 1;                  /* 认证读页只能读用户数据页 */
    if (addr & 0x001F) return 1;                  /* 地址必须 32 字节页对齐 */

    memset(packet, 0, sizeof(PRB480_AuthenticatedPagePacket)); /* 清空整个返回包结构体 */
    memcpy(packet->challenge, challenge, 3);                   /* 先把 challenge 记录进返回包，便于调试打印 */

    if (PRB480_LoadChallengeScratchpad(rom, addr, challenge, &scratchpadEs)) return 1; /* 先把 challenge 写入 scratchpad */
    if (PRB480_CommandStart(rom)) return 1;       /* 再开始新的认证读命令事务 */

    PRB480_WriteByte(0xA5);                       /* 发送 Read Authenticated Page 命令 */
    PRB480_WriteByte((u8)(addr & 0xFF));          /* 发送页首地址低字节 TA1 */
    PRB480_WriteByte((u8)(addr >> 8));            /* 发送页首地址高字节 TA2 */

    for (i = 0; i < 32; i++)                      /* 连续读取 32 字节页数据 */
    {
        packet->page[i] = PRB480_ReadByte();      /* 保存第 i 个页面字节 */
    }

    separator = PRB480_ReadByte();                /* 读取页数据后面的固定 0xFF 分隔字节 */
    if (separator != 0xFF) return 1;              /* 如果不是 0xFF，则总线数据异常 */

    packet->page_crc16 = PRB480_ReadByte();       /* 读取页数据段 CRC16 低字节 */
    packet->page_crc16 |= ((u16)PRB480_ReadByte() << 8); /* 读取页数据段 CRC16 高字节 */

    req[0] = 0xA5;                                /* req[0] 保存命令字节 */
    req[1] = (u8)(addr & 0xFF);                   /* req[1] 保存 TA1 */
    req[2] = (u8)(addr >> 8);                     /* req[2] 保存 TA2 */

    memcpy(page_crc_buf, packet->page, 32);       /* 先复制 32 字节页数据 */
    page_crc_buf[32] = 0xFF;                      /* 再补上分隔字节 0xFF 参与 CRC 校验 */

    if (PRB480_VerifyReadAuthPageCRC(req, 3, page_crc_buf, 33, packet->page_crc16)) return 1; /* 校验第 1 段 CRC16 */

    delay_ms(2);                                  /* 等待器件完成内部 MAC 计算 */

    for (i = 0; i < 20; i++)                      /* 连续读取 20 字节 device MAC */
    {
        packet->device_mac[i] = PRB480_ReadByte();/* 保存第 i 个 MAC 字节 */
    }

    packet->mac_crc16 = PRB480_ReadByte();        /* 读取 MAC 段 CRC16 低字节 */
    packet->mac_crc16 |= ((u16)PRB480_ReadByte() << 8); /* 读取 MAC 段 CRC16 高字节 */

    if (PRB480_VerifyReadAuthPageCRC(packet->device_mac, 20, 0, 0, packet->mac_crc16)) return 1; /* 校验第 2 段 CRC16 */

    packet->trailer[0] = PRB480_ReadByte();       /* 读取末尾第 1 个交替响应字节 */
    packet->trailer[1] = PRB480_ReadByte();       /* 读取末尾第 2 个交替响应字节 */

    if (PRB480_CheckAlternatingResponse(packet->trailer[0], packet->trailer[1])) return 1; /* 检查 trailer 是否成功 */

    (void)scratchpadEs;                           /* 当前只保留 challenge 写入已成功这一事实，不再单独使用其值 */
    return 0;                                     /* 原始认证读总线流程成功 */
}

/*******************************************************************************
* 名    称         : PRB480_ReadAuthenticatedPageEx
* 功    能         : 执行完整的“认证读页”流程
* 说    明         : 在 Raw 流程基础上，主机再用同样的 secret / ROM / page /
*                    challenge 重新计算 MAC，并与器件返回值比较
* 输入参数         : rom       - 目标器件 ROM ID
*                    secret    - 当前 8 字节 secret
*                    addr      - 页首地址
*                    challenge - 3 字节 challenge
* 输出参数         : packet    - 完整返回包
* 返 回 值         : 0 成功，1 失败
*******************************************************************************/
u8 PRB480_ReadAuthenticatedPageEx(u8 *rom, u8 *secret, u16 addr, u8 challenge[3], PRB480_AuthenticatedPagePacket *packet)
{
    /* -------- 执行流程说明 --------
     * 1. 先执行原始总线认证读流程
     * 2. 再由主机用同样输入重算 20 字节 MAC
     * 3. 比较 host_mac 与 device_mac 是否完全一致
     */

    if (secret == 0 || challenge == 0 || packet == 0) return 1;                                  /* 参数为空直接失败 */
    if (PRB480_ReadAuthenticatedPageRaw(rom, addr, challenge, packet)) return 1;                 /* 先走原始认证读流程 */   

    PRB480_GenerateReadAuthPageMAC(secret, rom, addr, packet->page, challenge, packet->host_mac); /* 主机侧重算 MAC */

    return (memcmp(packet->host_mac, packet->device_mac, 20) == 0) ? 0 : 1;                       /* 对比主机和器件 MAC */
}