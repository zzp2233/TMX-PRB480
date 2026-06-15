from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


base = Path.cwd() / "PRB480_Figure8b_original_page.png"
out = Path.cwd() / "PRB480_Figure8b_CN_overlay_v3.png"
img = Image.open(base).convert("RGB")
d = ImageDraw.Draw(img)

font_paths = [
    r"C:\Windows\Fonts\msyh.ttc",
    r"C:\Windows\Fonts\simhei.ttf",
    r"C:\Windows\Fonts\simsun.ttc",
]
font_path = next((fp for fp in font_paths if Path(fp).exists()), None)


def font(size):
    return ImageFont.truetype(font_path, size) if font_path else ImageFont.load_default()


F = font(17)
FS = font(15)
FT = font(13)
BLACK = (0, 0, 0)
BLUE = (18, 80, 150)
GREEN = (28, 120, 70)
RED = (170, 45, 45)


def wrap_text(text, max_w, fnt):
    lines = []
    for para in text.split("\n"):
        cur = ""
        for ch in para:
            test = cur + ch
            if d.textbbox((0, 0), test, font=fnt)[2] <= max_w:
                cur = test
            else:
                if cur:
                    lines.append(cur)
                cur = ch
        if cur:
            lines.append(cur)
    return lines


def center_text(box, text, fnt=FS, fill=BLACK, line_gap=1):
    x1, y1, x2, y2 = box
    lines = wrap_text(text, x2 - x1 - 8, fnt)
    heights = [
        d.textbbox((0, 0), line, font=fnt)[3] - d.textbbox((0, 0), line, font=fnt)[1]
        for line in lines
    ]
    total = sum(heights) + line_gap * (len(lines) - 1) if lines else 0
    y = y1 + (y2 - y1 - total) / 2
    for line, h in zip(lines, heights):
        tw = d.textbbox((0, 0), line, font=fnt)[2]
        d.text((x1 + (x2 - x1 - tw) / 2, y), line, font=fnt, fill=fill)
        y += h + line_gap


def rect(box, text, fnt=FS):
    d.rectangle(box, fill="white", outline=BLACK, width=2)
    center_text(box, text, fnt)


def diamond(cx, cy, w, h, text, fnt=FS):
    pts = [(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)]
    d.polygon(pts, fill="white", outline=BLACK)
    d.line(pts + [pts[0]], fill=BLACK, width=2)
    center_text((cx - w / 2 + 18, cy - h / 2 + 10, cx + w / 2 - 18, cy + h / 2 - 10), text, fnt)


def note(box, text, fnt=FT):
    d.rectangle(box, fill="white")
    x1, y1, x2, y2 = box
    y = y1
    for line in wrap_text(text, x2 - x1, fnt):
        d.text((x1, y), line, font=fnt, fill=BLUE)
        y += fnt.size + 2


def label(box, text, color=BLUE):
    d.rectangle(box, fill="white")
    center_text(box, text, FT, color)


# Top command line.
label((180, 552, 300, 590), "来自图8a", BLUE)
diamond(405, 586, 250, 104, "5Ah？\n加载初始密钥\n把暂存器数据写入密钥区", FT)
label((515, 568, 535, 590), "否", RED)
label((410, 638, 430, 662), "是", GREEN)
diamond(1295, 586, 260, 104, "33h？\n生成下一密钥\n用 SHA-1 更新密钥", FT)
label((1410, 568, 1432, 590), "否", RED)
label((1292, 638, 1314, 662), "是", GREEN)
label((1515, 552, 1655, 590), "到图8c", BLUE)

# Load First Secret branch.
rect((260, 678, 540, 750), "主机发送 TA1、TA2、E/S\n这三字节来自 Read Scratchpad\n用于证明暂存器内容有效", FT)
note((560, 665, 880, 755), "注：5Ah 不直接发送密钥；8字节密钥必须先用 Write Scratchpad 写入暂存器，地址通常为 0080h。")
diamond(405, 824, 260, 110, "认证字节匹配？\n检查 TA1/TA2/E/S\n不是 SHA-1 MAC", FT)
label((512, 805, 532, 828), "是", GREEN)
label((405, 874, 425, 896), "否", RED)
diamond(718, 899, 250, 105, "地址是密钥区？\n正常加载密钥时\n目标应为 0080h", FT)
label((810, 884, 830, 906), "是", GREEN)
label((583, 885, 605, 907), "否", RED)
diamond(560, 990, 285, 108, "地址是否小于 7Fh？\n特殊刷新写回时\n目标必须在用户数据区", FT)
note(
    (690, 940, 1010, 1035),
    "说明：这是 EN_LFS=1 时的特殊分支。\n地址 < 7Fh 表示目标落在 00h-7Fh 用户数据区，\nLoad First Secret 此时不是写密钥，而是把暂存器数据刷新写回数据区。",
    FT,
)
label((545, 965, 565, 988), "否", RED)
label((557, 1034, 577, 1056), "是", GREEN)
diamond(560, 1127, 280, 108, "EN_LFS 标志 = 1？\n由 Refresh Scratchpad 置位\n普通加载密钥一般为 0", FT)
label((545, 1082, 565, 1105), "否", RED)
label((672, 1095, 692, 1118), "是", GREEN)
diamond(858, 1220, 250, 105, "是否写保护？\n密钥区或目标数据区\n被保护则失败", FT)
label((730, 1190, 750, 1212), "是", GREEN)
label((850, 1274, 870, 1298), "否", RED)
rect((812, 1310, 908, 1372), "AA = 1\n授权/写入成功标志", FT)
rect((720, 1380, 995, 1475), "等待 tPROG\nPRB480 把暂存器数据\n复制到目标存储器\n期间总线保持高电平", FT)
label((615, 1428, 725, 1455), "持续：tPROG", BLUE)
rect((775, 1508, 930, 1566), "PRB480 发送“0”\n进入后续状态循环", FT)
diamond(838, 1645, 230, 100, "主机发送复位？\n是：结束当前流程\n否：继续读状态", FT)
label((735, 1612, 755, 1635), "是", GREEN)
label((850, 1692, 870, 1715), "否", RED)
rect((775, 1732, 930, 1792), "PRB480 发送“1”\n后续读为 FFh", FT)
diamond(838, 1870, 230, 100, "主机发送复位？\n复位后回到图8a", FT)
label((855, 1840, 875, 1862), "否", RED)
label((850, 1910, 870, 1932), "是", GREEN)

# Left failure path.
rect((320, 1588, 470, 1670), "主机接收“1”\n失败/无效时\n通常读到 FFh", FT)
diamond(390, 1870, 235, 100, "主机发送复位？\n是：回图8a\n否：继续循环", FT)
label((480, 1840, 500, 1862), "否", RED)
label((395, 1920, 415, 1942), "是", GREEN)
label((185, 1950, 285, 1975), "到图8a", BLUE)

# Compute Next Secret branch.
rect((1160, 678, 1425, 750), "主机发送 TA1、TA2\n指定参与 SHA-1 计算的数据页\n不是密钥地址", FT)
note((1415, 660, 1660, 765), "注：33h 前必须先把8字节 partial secret 写入暂存器。它参与生成下一密钥。")
rect((1165, 765, 1420, 840), "PRB480 清除 EN_LFS = 0\n进入普通生成下一密钥流程", FT)
diamond(1295, 900, 260, 105, "有效数据地址？\n必须指向用户数据页\n00h 到 7Fh", FT)
label((1168, 888, 1188, 910), "否", RED)
label((1400, 885, 1420, 907), "是", GREEN)
diamond(1430, 990, 250, 105, "是否写保护？\n密钥被保护时\n不能生成新密钥", FT)
label((1302, 982, 1322, 1005), "是", GREEN)
label((1445, 1045, 1465, 1068), "否", RED)
rect((1290, 1075, 1595, 1230), "等待 tCSHA\nPRB480 内部计算认证码\n输入：当前密钥 + 页数据\n+ 暂存器 8 字节 partial secret", FT)
label((1205, 1140, 1320, 1165), "持续：tCSHA", BLUE)
rect((1325, 1260, 1570, 1352), "等待 tPROG\n把部分 MAC 复制到\nsecret 密钥寄存器\n作为新密钥", FT)
label((1195, 1300, 1325, 1325), "持续：tPROG", BLUE)
rect((1310, 1385, 1575, 1458), "PRB480 用 AAh 填充暂存器\n表示生成流程完成", FT)
rect((1360, 1508, 1504, 1568), "PRB480 发送“0”\n进入状态循环", FT)
diamond(1435, 1645, 235, 100, "主机发送复位？\n是：结束流程\n否：继续读状态", FT)
label((1525, 1618, 1545, 1640), "是", GREEN)
label((1430, 1692, 1450, 1715), "否", RED)
rect((1360, 1732, 1504, 1794), "PRB480 发送“1”\n后续读为 FFh", FT)
diamond(1435, 1870, 235, 100, "主机发送复位？\n复位后回图8a", FT)
label((1332, 1844, 1352, 1865), "否", RED)
label((1432, 1910, 1452, 1932), "是", GREEN)

# Right failure path.
rect((1065, 1588, 1215, 1670), "主机接收“1”\n地址/保护失败时\n通常读到 FFh", FT)
diamond(1138, 1870, 235, 100, "主机发送复位？\n是：回图8a\n否：继续循环", FT)
label((1228, 1840, 1248, 1862), "否", RED)
label((1145, 1920, 1165, 1942), "是", GREEN)

# Bottom common labels.
label((830, 1958, 1120, 1992), "1-Wire 空闲保持高电平供电", BLUE)
label((1510, 1948, 1655, 1976), "来自图8c", BLUE)

img.save(out)
print(out)
