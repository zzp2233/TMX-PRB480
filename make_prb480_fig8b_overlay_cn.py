from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


base = Path.cwd() / "PRB480_Figure8b_original_page.png"
out = Path.cwd() / "PRB480_Figure8b_CN_overlay.png"
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


F = font(22)
FS = font(19)
FT = font(16)
FL = font(18)
BLUE = (20, 78, 150)
BLACK = (0, 0, 0)
RED = (170, 45, 45)
GREEN = (25, 125, 70)


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


def cover(box):
    d.rectangle(box, fill="white")


def text_center(box, text, fnt=FS, fill=BLACK, cover_bg=True, line_gap=3):
    if cover_bg:
        cover(box)
    x1, y1, x2, y2 = box
    lines = wrap_text(text, x2 - x1 - 4, fnt)
    hs = [
        d.textbbox((0, 0), line, font=fnt)[3] - d.textbbox((0, 0), line, font=fnt)[1]
        for line in lines
    ]
    total = sum(hs) + line_gap * (len(lines) - 1) if lines else 0
    y = y1 + (y2 - y1 - total) / 2
    for line, h in zip(lines, hs):
        tw = d.textbbox((0, 0), line, font=fnt)[2]
        d.text((x1 + (x2 - x1 - tw) / 2, y), line, font=fnt, fill=fill)
        y += h + line_gap


def text_left(box, text, fnt=FS, fill=BLACK, cover_bg=True, line_gap=3):
    if cover_bg:
        cover(box)
    x1, y1, x2, y2 = box
    y = y1
    for line in wrap_text(text, x2 - x1, fnt):
        d.text((x1, y), line, font=fnt, fill=fill)
        y += fnt.size + line_gap


def yn(box, s):
    color = GREEN if s == "是" else RED
    text_center(box, s, FT, color)


# Top connectors and command decisions.
text_center((185, 560, 275, 585), "来自图8a", FT, BLUE)
text_center((1537, 560, 1635, 585), "到图8c", FT, BLUE)
text_center((340, 560, 465, 618), "5Ah\n加载初始密钥？", FS)
text_center((1250, 560, 1370, 620), "33h\n生成下一密钥？", FS)
yn((515, 568, 535, 590), "否")
yn((412, 638, 432, 662), "是")
yn((1410, 568, 1432, 590), "否")
yn((1292, 638, 1314, 662), "是")

# Left Load First Secret branch.
text_center((292, 688, 510, 740), "主机发送 TA1、TA2 和 E/S\n字节", FS)
text_left((570, 675, 820, 745), "注：8字节密钥必须先写入暂存器。", FS, BLUE)
text_center((330, 795, 500, 860), "认证字节\n匹配？", FS)
yn((512, 805, 532, 828), "是")
yn((405, 873, 425, 895), "否")
text_center((620, 870, 770, 925), "地址是\n密钥区？", FS)
yn((810, 883, 830, 905), "是")
yn((585, 885, 605, 907), "否")
text_center((485, 965, 660, 1015), "地址 < 7Fh？", FS)
yn((545, 965, 565, 988), "否")
yn((557, 1034, 577, 1056), "是")
text_center((455, 1080, 685, 1140), "EN_LFS 标志\n= 1？", FS)
yn((545, 1082, 565, 1105), "否")
yn((672, 1095, 692, 1118), "是")
text_center((780, 1170, 945, 1235), "是否\n写保护？", FS)
yn((730, 1190, 750, 1212), "是")
yn((850, 1275, 870, 1298), "否")
text_center((810, 1320, 885, 1360), "AA = 1", FS)
text_center((735, 1390, 965, 1465), "主机等待 PRB480\n把暂存器数据\n复制到存储器", FT)
text_center((615, 1428, 710, 1455), "持续：tPROG", FT, BLUE)
text_center((780, 1520, 900, 1560), "PRB480\n发送“0”", FT)
text_center((745, 1620, 930, 1670), "主机发送\n复位？", FT)
yn((735, 1612, 755, 1635), "是")
yn((850, 1692, 870, 1715), "否")
text_center((785, 1740, 905, 1780), "PRB480\n发送“1”", FT)
text_center((745, 1845, 930, 1895), "主机发送\n复位？", FT)
yn((855, 1840, 875, 1862), "否")
yn((850, 1910, 870, 1932), "是")

# Left failure loop.
text_center((335, 1600, 455, 1660), "主机\n接收“1”", FT)
text_center((300, 1840, 475, 1895), "主机发送\n复位？", FT)
yn((480, 1840, 500, 1862), "否")
yn((395, 1920, 415, 1942), "是")
text_center((185, 1950, 280, 1975), "到图8a", FT, BLUE)

# Right Compute Next Secret branch.
text_center((1175, 688, 1405, 740), "主机发送\nTA1、TA2", FS)
text_left((1425, 670, 1630, 745), "注：主机必须先把8字节部分密钥写入暂存器。", FS, BLUE)
text_center((1190, 770, 1405, 830), "PRB480 清除\nEN_LFS = 0", FS)
text_center((1230, 865, 1390, 930), "有效数据\n地址？", FS)
yn((1168, 888, 1188, 910), "否")
yn((1400, 885, 1420, 907), "是")
text_center((1345, 965, 1515, 1025), "是否\n写保护？", FS)
yn((1302, 982, 1322, 1005), "是")
yn((1445, 1045, 1465, 1068), "否")
text_center((1320, 1088, 1570, 1218), "主机等待 PRB480\n计算当前密钥、页数据、\n暂存器8字节部分密钥的\n认证码", FT)
text_center((1205, 1140, 1320, 1165), "持续：tCSHA", FT, BLUE)
text_center((1338, 1270, 1555, 1340), "主机等待 PRB480\n把部分 MAC 复制到\n密钥寄存器", FT)
text_center((1195, 1300, 1325, 1325), "持续：tPROG", FT, BLUE)
text_center((1325, 1395, 1560, 1445), "PRB480 用 AAh\n填充暂存器", FT)
text_center((1370, 1518, 1490, 1555), "PRB480\n发送“0”", FT)
text_center((1338, 1620, 1535, 1670), "主机发送\n复位？", FT)
yn((1525, 1618, 1545, 1640), "是")
yn((1430, 1692, 1450, 1715), "否")
text_center((1370, 1740, 1490, 1780), "PRB480\n发送“1”", FT)
text_center((1338, 1845, 1535, 1895), "主机发送\n复位？", FT)
yn((1332, 1844, 1352, 1865), "否")
yn((1432, 1910, 1452, 1932), "是")

# Right failure loop.
text_center((1080, 1600, 1195, 1660), "主机\n接收“1”", FT)
text_center((1045, 1840, 1225, 1895), "主机发送\n复位？", FT)
yn((1228, 1840, 1248, 1862), "否")
yn((1145, 1920, 1165, 1942), "是")

# Bottom/common notes.
text_center((846, 1960, 1060, 1990), "1-Wire 空闲保持高电平供电", FT, BLUE)
text_center((1518, 1950, 1640, 1975), "来自图8c", FT, BLUE)

# Tiny additional notes placed outside the flowchart frame, not changing arrows/framework.
text_left(
    (210, 2078, 1610, 2145),
    "注释：5Ah 用于把暂存器中的 8 字节写入密钥区；33h 用旧密钥、页数据和暂存器数据计算新密钥。tCSHA/tPROG 期间总线需保持高电平供电。",
    FT,
    BLUE,
    cover_bg=False,
)

img.save(out)
print(out)
