from pathlib import Path
import math

from PIL import Image, ImageDraw, ImageFont


out = Path.cwd() / "PRB480_Figure8b_CN.png"
W, H = 2100, 2600
img = Image.new("RGB", (W, H), "white")
d = ImageDraw.Draw(img)

font_paths = [
    r"C:\Windows\Fonts\msyh.ttc",
    r"C:\Windows\Fonts\simhei.ttf",
    r"C:\Windows\Fonts\simsun.ttc",
]
font_path = next((fp for fp in font_paths if Path(fp).exists()), None)


def font(size):
    return ImageFont.truetype(font_path, size) if font_path else ImageFont.load_default()


F_TITLE = font(44)
F_SMALL = font(24)
F_TINY = font(20)
F_NOTE = font(19)
BLACK = (25, 25, 25)
BLUE = (25, 80, 150)
GREEN = (35, 120, 70)
RED = (170, 50, 45)
GRAY = (90, 90, 90)
LIGHT = (245, 248, 252)
NOTE = (255, 252, 230)
SHA = (240, 250, 245)
PROG = (252, 245, 240)


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


def center_text(box, text, fnt, fill=BLACK, line_gap=5):
    x1, y1, x2, y2 = box
    lines = wrap_text(text, x2 - x1 - 24, fnt)
    heights = [
        d.textbbox((0, 0), line, font=fnt)[3]
        - d.textbbox((0, 0), line, font=fnt)[1]
        for line in lines
    ]
    total = sum(heights) + line_gap * (len(lines) - 1) if lines else 0
    y = y1 + (y2 - y1 - total) / 2
    for line, h in zip(lines, heights):
        tw = d.textbbox((0, 0), line, font=fnt)[2]
        d.text((x1 + (x2 - x1 - tw) / 2, y), line, font=fnt, fill=fill)
        y += h + line_gap


def rect(box, text, fnt=F_TINY, fill=LIGHT, outline=BLACK, width=3):
    d.rounded_rectangle(box, radius=10, fill=fill, outline=outline, width=width)
    center_text(box, text, fnt)


def diamond(cx, cy, w, h, text, fnt=F_TINY):
    pts = [(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)]
    d.polygon(pts, fill="white", outline=BLACK)
    d.line(pts + [pts[0]], fill=BLACK, width=3)
    center_text((cx - w / 2 + 18, cy - h / 2 + 15, cx + w / 2 - 18, cy + h / 2 - 15), text, fnt)


def arrow(p1, p2, color=BLACK, width=3):
    d.line([p1, p2], fill=color, width=width)
    x1, y1 = p1
    x2, y2 = p2
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 15
    pts = [
        (x2, y2),
        (x2 + size * math.cos(ang + math.pi * 0.82), y2 + size * math.sin(ang + math.pi * 0.82)),
        (x2 + size * math.cos(ang - math.pi * 0.82), y2 + size * math.sin(ang - math.pi * 0.82)),
    ]
    d.polygon(pts, fill=color)


def label(x, y, text, color=BLUE):
    d.text((x, y), text, font=F_NOTE, fill=color)


center_text((0, 25, W, 105), "PRB480 图 8b 中文流程图：加载初始密钥 / 生成下一密钥", F_TITLE, BLUE)
center_text((0, 105, W, 160), "对应命令：Load First Secret [5Ah] 与 Compute Next Secret [33h]", F_SMALL, GRAY)

# Entry and command decisions.
rect((750, 185, 1350, 265), "从图 8a 进入\n继续判断存储器 / SHA-1 功能命令", F_TINY)
arrow((1050, 265), (1050, 325))
diamond(1050, 420, 440, 140, "命令是 5Ah？\n加载初始密钥", F_TINY)
label(825, 385, "是 Y", GREEN)
label(1280, 385, "否 N", RED)

# LFS branch left.
arrow((830, 420), (560, 420))
arrow((560, 420), (560, 520))
rect((255, 520, 865, 635), "注意：8 字节密钥必须先写入暂存器\n先执行 Write Scratchpad，目标地址通常为 0080h", F_TINY, NOTE)
arrow((560, 635), (560, 710))
rect((290, 710, 830, 820), "主机发送 TA1、TA2 和 E/S\n这 3 字节来自 Read Scratchpad 读回值", F_TINY)
arrow((560, 820), (560, 900))
diamond(560, 985, 390, 125, "认证字节匹配？\nTA1/TA2/E/S 是否正确", F_NOTE)
label(350, 950, "否 N", RED)
arrow((365, 985), (160, 985))
arrow((160, 985), (160, 1255))
rect((35, 1255, 285, 1345), "失败\n继续读到全 1\n即 FFh 循环", F_NOTE, NOTE, RED)
label(575, 1050, "是 Y", GREEN)
arrow((560, 1048), (560, 1130))
diamond(560, 1215, 360, 125, "地址是密钥区？\n通常为 0080h", F_NOTE)
label(375, 1180, "否 N", RED)
arrow((380, 1215), (320, 1215))
arrow((320, 1215), (320, 1430))
diamond(560, 1430, 380, 130, "EN_LFS = 1？\n是否为刷新写回模式", F_NOTE)
label(360, 1395, "否 N", RED)
arrow((370, 1430), (160, 1430))
arrow((160, 1430), (160, 1555))
rect((35, 1555, 285, 1645), "失败\n继续读到 FFh", F_NOTE, NOTE, RED)
label(575, 1278, "是 Y", GREEN)
arrow((560, 1278), (560, 1570))
label(760, 1395, "是 Y：特殊分支", BLUE)
arrow((750, 1430), (880, 1430))
arrow((880, 1430), (880, 1570))
diamond(880, 1660, 360, 125, "地址在数据区？\n00h 到 7Fh", F_NOTE)
label(1065, 1625, "否 N", RED)
arrow((1060, 1660), (1220, 1660))
arrow((1220, 1660), (1220, 1770))
rect((1095, 1770, 1345, 1860), "失败\n继续读到 FFh", F_NOTE, NOTE, RED)
label(725, 1625, "是 Y", GREEN)
arrow((700, 1660), (560, 1660))
arrow((560, 1660), (560, 1740))
diamond(560, 1830, 360, 130, "是否写保护？\n密钥区或目标数据区", F_NOTE)
label(745, 1795, "是 Y", RED)
arrow((740, 1830), (940, 1830))
arrow((940, 1830), (940, 1950))
rect((815, 1950, 1065, 2040), "失败\n继续读到 FFh", F_NOTE, NOTE, RED)
label(575, 1895, "否 N", GREEN)
arrow((560, 1895), (560, 1980))
rect((320, 1980, 800, 2085), "等待 tPROG\n芯片把暂存器数据写入目标区域\n期间总线保持高电平供电", F_NOTE, PROG)
arrow((560, 2085), (560, 2160))
rect((310, 2160, 810, 2280), "操作成功\nAA = 1；EN_LFS 清 0\n暂存器填充 AAh", F_NOTE, SHA)
arrow((560, 2280), (560, 2355))
rect((350, 2355, 770, 2440), "主机继续读到 AAh 循环\n表示加载 / 刷新成功", F_NOTE, NOTE)

# CNS branch right.
arrow((1270, 420), (1525, 420))
arrow((1525, 420), (1525, 515))
diamond(1525, 610, 430, 140, "命令是 33h？\n生成下一密钥", F_TINY)
label(1300, 575, "是 Y", GREEN)
label(1745, 575, "否 N", RED)
arrow((1740, 610), (1940, 610))
arrow((1940, 610), (1940, 740))
rect((1810, 740, 2070, 835), "转到图 8c\n继续判断其他命令", F_NOTE, NOTE)

arrow((1310, 610), (1200, 610))
arrow((1200, 610), (1200, 715))
rect((970, 715, 1430, 835), "注意：主机必须先把 8 字节\npartial secret 写入暂存器", F_TINY, NOTE)
arrow((1200, 835), (1200, 920))
rect((980, 920, 1420, 1025), "主机发送 TA1、TA2\n用于指定参与计算的数据页", F_TINY)
arrow((1200, 1025), (1200, 1110))
diamond(1200, 1200, 370, 130, "有效数据地址？\n00h 到 7Fh", F_NOTE)
label(1010, 1165, "否 N", RED)
arrow((1015, 1200), (940, 1200))
arrow((940, 1200), (940, 1320))
rect((815, 1320, 1065, 1410), "失败\n继续读到 FFh", F_NOTE, NOTE, RED)
label(1215, 1265, "是 Y", GREEN)
arrow((1200, 1265), (1200, 1350))
diamond(1200, 1440, 360, 130, "密钥是否写保护？", F_NOTE)
label(1385, 1405, "是 Y", RED)
arrow((1380, 1440), (1540, 1440))
arrow((1540, 1440), (1540, 1560))
rect((1415, 1560, 1665, 1650), "失败\n继续读到 FFh", F_NOTE, NOTE, RED)
label(1215, 1505, "否 N", GREEN)
arrow((1200, 1505), (1200, 1590))
rect((950, 1590, 1450, 1730), "等待 tCSHA\nPRB480 计算 MAC\n输入：当前密钥 + 选中页数据\n+ 暂存器中的 8 字节 partial secret", F_NOTE, SHA)
arrow((1200, 1730), (1200, 1815))
rect((950, 1815, 1450, 1945), "等待 tPROG\nPRB480 将部分 MAC\n复制到 secret 寄存器作为新密钥", F_NOTE, PROG)
arrow((1200, 1945), (1200, 2030))
rect((970, 2030, 1430, 2140), "生成成功\n暂存器填充 AAh", F_NOTE, SHA)
arrow((1200, 2140), (1200, 2215))
rect((990, 2215, 1410, 2300), "主机继续读到 AAh 循环\n表示新密钥生成成功", F_NOTE, NOTE)

# Reset notes, matching repeated reset checks in original flowchart.
reset_box = (1500, 2070, 2035, 2440)
d.rounded_rectangle(reset_box, radius=12, fill=(248, 250, 255), outline=BLUE, width=3)
notes = """少量注释：
1. 图 8b 处理 5Ah 和 33h 两条密钥相关命令。
2. 5Ah 不直接接收 8 字节密钥；密钥必须先在图 8a 写入暂存器。
3. 5Ah 发送的是 TA1、TA2、E/S 三个认证字节。
4. 33h 用旧密钥、页数据和暂存器数据，通过 SHA-1 生成新密钥。
5. tPROG / tCSHA 期间，总线要保持高电平供电。
6. 流程中任意 MASTER RESET 可中断当前命令并回到前面的选择流程。"""
y = reset_box[1] + 25
for line in wrap_text(notes, reset_box[2] - reset_box[0] - 40, F_NOTE):
    d.text((reset_box[0] + 25, y), line, font=F_NOTE, fill=BLACK)
    y += 30

d.rectangle((20, 20, W - 20, H - 20), outline=(180, 180, 180), width=2)
img.save(out)
print(out)
