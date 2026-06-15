from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


base = Path.cwd() / "PRB480_Figure8b_original_page.png"
out = Path.cwd() / "PRB480_Figure8b_CN_overlay_v2.png"
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


F = font(18)
FS = font(16)
FT = font(14)
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


def center_text(box, text, fnt=FS, fill=BLACK, line_gap=2):
    x1, y1, x2, y2 = box
    lines = wrap_text(text, x2 - x1 - 6, fnt)
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
    center_text((cx - w / 2 + 16, cy - h / 2 + 10, cx + w / 2 - 16, cy + h / 2 - 10), text, fnt)


def note(box, text):
    d.rectangle(box, fill="white")
    x1, y1, x2, y2 = box
    y = y1
    for line in wrap_text(text, x2 - x1, FT):
        d.text((x1, y), line, font=FT, fill=BLUE)
        y += 17


def label(box, text, color=BLUE):
    d.rectangle(box, fill="white")
    center_text(box, text, FT, color)


# Top command line.
label((180, 552, 300, 590), "来自图8a", BLUE)
diamond(405, 586, 215, 92, "5Ah\n加载初始密钥？", FS)
label((515, 568, 535, 590), "否", RED)
label((410, 638, 430, 662), "是", GREEN)
diamond(1295, 586, 230, 92, "33h\n生成下一密钥？", FS)
label((1410, 568, 1432, 590), "否", RED)
label((1292, 638, 1314, 662), "是", GREEN)
label((1515, 552, 1655, 590), "到图8c", BLUE)

# Load First Secret branch.
rect((286, 690, 516, 742), "主机发送 TA1、TA2\n和 E/S 字节", FS)
note((560, 665, 860, 755), "注：8字节密钥必须先写入暂存器。")
diamond(405, 824, 220, 98, "认证字节\n匹配？", FS)
label((512, 805, 532, 828), "是", GREEN)
label((405, 874, 425, 896), "否", RED)
diamond(718, 899, 230, 96, "地址是\n密钥区？", FS)
label((810, 884, 830, 906), "是", GREEN)
label((583, 885, 605, 907), "否", RED)
diamond(560, 990, 230, 94, "地址 < 7Fh？", FS)
label((545, 965, 565, 988), "否", RED)
label((557, 1034, 577, 1056), "是", GREEN)
diamond(560, 1127, 260, 96, "EN_LFS 标志\n= 1？", FS)
label((545, 1082, 565, 1105), "否", RED)
label((672, 1095, 692, 1118), "是", GREEN)
diamond(858, 1220, 225, 96, "是否\n写保护？", FS)
label((730, 1190, 750, 1212), "是", GREEN)
label((850, 1274, 870, 1298), "否", RED)
rect((824, 1320, 895, 1366), "AA = 1", FS)
rect((740, 1388, 974, 1464), "主机等待 PRB480\n把暂存器数据\n复制到存储器", FT)
label((615, 1428, 725, 1455), "持续：tPROG", BLUE)
rect((785, 1514, 918, 1560), "PRB480\n发送“0”", FT)
diamond(838, 1645, 205, 92, "主机发送\n复位？", FT)
label((735, 1612, 755, 1635), "是", GREEN)
label((850, 1692, 870, 1715), "否", RED)
rect((785, 1740, 918, 1784), "PRB480\n发送“1”", FT)
diamond(838, 1870, 205, 92, "主机发送\n复位？", FT)
label((855, 1840, 875, 1862), "否", RED)
label((850, 1910, 870, 1932), "是", GREEN)

# Left failure path.
rect((335, 1600, 456, 1660), "主机\n接收“1”", FT)
diamond(390, 1870, 215, 92, "主机发送\n复位？", FT)
label((480, 1840, 500, 1862), "否", RED)
label((395, 1920, 415, 1942), "是", GREEN)
label((185, 1950, 285, 1975), "到图8a", BLUE)

# Compute Next Secret branch.
rect((1178, 690, 1408, 742), "主机发送\nTA1、TA2", FS)
note((1415, 660, 1648, 765), "注：主机必须先把8字节部分密钥写入暂存器。")
rect((1180, 775, 1408, 830), "PRB480 清除\nEN_LFS = 0", FS)
diamond(1295, 900, 230, 96, "有效数据\n地址？", FS)
label((1168, 888, 1188, 910), "否", RED)
label((1400, 885, 1420, 907), "是", GREEN)
diamond(1430, 990, 230, 96, "是否\n写保护？", FS)
label((1302, 982, 1322, 1005), "是", GREEN)
label((1445, 1045, 1465, 1068), "否", RED)
rect((1308, 1088, 1576, 1220), "主机等待 PRB480\n计算认证码\n当前密钥 + 页数据\n+ 暂存器部分密钥", FT)
label((1205, 1140, 1320, 1165), "持续：tCSHA", BLUE)
rect((1336, 1270, 1558, 1342), "主机等待 PRB480\n把部分 MAC 复制到\n密钥寄存器", FT)
label((1195, 1300, 1325, 1325), "持续：tPROG", BLUE)
rect((1323, 1395, 1562, 1448), "PRB480 用 AAh\n填充暂存器", FT)
rect((1370, 1518, 1492, 1558), "PRB480\n发送“0”", FT)
diamond(1435, 1645, 210, 92, "主机发送\n复位？", FT)
label((1525, 1618, 1545, 1640), "是", GREEN)
label((1430, 1692, 1450, 1715), "否", RED)
rect((1370, 1740, 1492, 1784), "PRB480\n发送“1”", FT)
diamond(1435, 1870, 210, 92, "主机发送\n复位？", FT)
label((1332, 1844, 1352, 1865), "否", RED)
label((1432, 1910, 1452, 1932), "是", GREEN)

# Right failure path.
rect((1080, 1600, 1198, 1660), "主机\n接收“1”", FT)
diamond(1138, 1870, 215, 92, "主机发送\n复位？", FT)
label((1228, 1840, 1248, 1862), "否", RED)
label((1145, 1920, 1165, 1942), "是", GREEN)

# Bottom common labels.
label((835, 1958, 1105, 1992), "1-Wire 空闲保持高电平供电", BLUE)
label((1510, 1948, 1655, 1976), "来自图8c", BLUE)

img.save(out)
print(out)
