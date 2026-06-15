from pathlib import Path
import math

from PIL import Image, ImageDraw, ImageFont


out = Path.cwd() / "PRB480_Figure8a_CN_v2.png"
W, H = 1900, 2500
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
F_TINY = font(21)
BLACK = (25, 25, 25)
BLUE = (25, 80, 150)
GREEN = (35, 120, 70)
RED = (170, 50, 45)
GRAY = (90, 90, 90)
LIGHT = (245, 248, 252)
NOTE = (255, 252, 230)


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


def center_text(box, text, fnt, fill=BLACK, line_gap=6):
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


def rect(box, text, fnt=F_SMALL, fill=LIGHT, outline=BLACK, width=3):
    d.rounded_rectangle(box, radius=10, fill=fill, outline=outline, width=width)
    center_text(box, text, fnt)


def diamond(cx, cy, w, h, text, fnt=F_SMALL):
    pts = [(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)]
    d.polygon(pts, fill="white", outline=BLACK)
    d.line(pts + [pts[0]], fill=BLACK, width=3)
    center_text((cx - w / 2 + 20, cy - h / 2 + 18, cx + w / 2 - 20, cy + h / 2 - 18), text, fnt)


def arrow(p1, p2, color=BLACK, width=3):
    d.line([p1, p2], fill=color, width=width)
    x1, y1 = p1
    x2, y2 = p2
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 16
    pts = [
        (x2, y2),
        (x2 + size * math.cos(ang + math.pi * 0.82), y2 + size * math.sin(ang + math.pi * 0.82)),
        (x2 + size * math.cos(ang - math.pi * 0.82), y2 + size * math.sin(ang - math.pi * 0.82)),
    ]
    d.polygon(pts, fill=color)


def label(x, y, text, color=BLUE):
    d.text((x, y), text, font=F_TINY, fill=color)


center_text((0, 30, W, 115), "PRB480 图 8a 中文流程图：写暂存器 / 读暂存器", F_TITLE, BLUE)
center_text((0, 110, W, 165), "在完成 ROM 功能命令之后，进入存储器 / SHA-1 功能命令阶段", F_SMALL, GRAY)

rect((650, 190, 1250, 270), "从 ROM 功能流程图进入\n已完成复位和 ROM 选择")
rect((650, 330, 1250, 420), "主机发送存储器功能命令\n主机 = STM32")
arrow((950, 270), (950, 330))

diamond(950, 535, 420, 140, "命令是 0Fh？\n写暂存器")
arrow((950, 420), (950, 465))
label(1180, 510, "否 N")
label(765, 510, "是 Y")

# Write Scratchpad branch.
arrow((740, 535), (500, 535))
arrow((500, 535), (500, 650))
rect((260, 650, 740, 750), "主机发送目标地址\nTA1=地址低字节，TA2=地址高字节")
arrow((500, 750), (500, 815))
diamond(500, 900, 360, 130, "地址 < A0h？\n是否为有效地址")
label(310, 865, "否 N", RED)
arrow((320, 900), (180, 900))
arrow((180, 900), (180, 1055))
rect((40, 1055, 320, 1155), "地址无效\n继续读到全 1\n即 FFh 循环", F_TINY, NOTE, RED)
label(655, 865, "是 Y", GREEN)
arrow((500, 965), (500, 1030))
rect((260, 1030, 740, 1135), "PRB480 清除 EN_LFS\n表示普通写暂存器流程")
arrow((500, 1135), (500, 1190))
rect((220, 1190, 780, 1330), "初始化暂存器状态\n字节计数器=0；清 PF、AA\n地址低 3 位清 0；E/S 低 3 位置 1")
arrow((500, 1330), (500, 1400))
rect((260, 1400, 740, 1500), "主机发送 1 个数据字节\n写入暂存器", F_TINY)
arrow((500, 1500), (500, 1560))
rect((280, 1560, 720, 1650), "PRB480 字节计数器 +1", F_TINY)
arrow((500, 1650), (500, 1715))
diamond(500, 1810, 350, 130, "字节计数器 = 7？\n是否已满 8 字节", F_TINY)
label(690, 1775, "否 N：继续收下一个字节")
d.line([(675, 1810), (860, 1810), (860, 1450), (740, 1450)], fill=BLUE, width=3)
arrow((750, 1450), (740, 1450), BLUE)
label(515, 1888, "是 Y", GREEN)
arrow((500, 1875), (500, 1940))
diamond(500, 2025, 330, 120, "是否有不完整字节？\n比如中途停止", F_TINY)
label(680, 1990, "是 Y", RED)
arrow((665, 2025), (780, 2025))
arrow((780, 2025), (780, 2140))
rect((660, 2140, 900, 2220), "PF = 1\n暂存器数据无效", F_TINY, NOTE, RED)
label(500, 2090, "否 N", GREEN)
arrow((500, 2085), (500, 2165))
rect((230, 2165, 770, 2285), "PRB480 返回 CRC16\n计算内容：0Fh + TA1 + TA2\n+ 主机发送的 8 字节数据", F_TINY)
arrow((500, 2285), (500, 2350))
rect((280, 2350, 720, 2430), "之后继续读：全 1\n即 FFh、FFh、FFh...", F_TINY, NOTE)

# Read Scratchpad branch.
arrow((1160, 535), (1400, 535))
arrow((1400, 535), (1400, 650))
diamond(1400, 735, 420, 140, "命令是 AAh？\n读暂存器")
label(1620, 700, "否 N", RED)
arrow((1610, 735), (1780, 735))
arrow((1780, 735), (1780, 870))
rect((1645, 870, 1880, 965), "转到图 8b\n继续判断其他命令", F_TINY, NOTE)
label(1210, 700, "是 Y", GREEN)
arrow((1190, 735), (1120, 735))
arrow((1120, 735), (1120, 850))
rect((900, 850, 1340, 965), "主机读取 TA1、TA2 和 E/S\nE/S 是结束地址和状态字节", F_TINY)
arrow((1120, 965), (1120, 1035))
rect((900, 1035, 1340, 1125), "PRB480 设置暂存器\n字节计数器 = 0", F_TINY)
arrow((1120, 1125), (1120, 1195))
rect((900, 1195, 1340, 1295), "主机读取 1 个暂存器数据字节", F_TINY)
arrow((1120, 1295), (1120, 1360))
rect((920, 1360, 1320, 1450), "PRB480 字节计数器 +1", F_TINY)
arrow((1120, 1450), (1120, 1515))
diamond(1120, 1610, 350, 130, "字节计数器 = 7？\n是否已读完 8 字节", F_TINY)
label(1300, 1575, "否 N：继续返回下一个字节")
d.line([(1295, 1610), (1500, 1610), (1500, 1245), (1340, 1245)], fill=BLUE, width=3)
arrow((1350, 1245), (1340, 1245), BLUE)
label(1135, 1688, "是 Y", GREEN)
arrow((1120, 1675), (1120, 1760))
rect((850, 1760, 1390, 1900), "主机读取 CRC16\n计算内容：AAh + TA1 + TA2 + E/S\n+ PRB480 返回的 8 字节暂存器数据", F_TINY)
arrow((1120, 1900), (1120, 1980))
rect((900, 1980, 1340, 2065), "之后继续读：全 1\n即 FFh、FFh、FFh...", F_TINY, NOTE)

note_box = (1180, 2140, 1860, 2440)
d.rounded_rectangle(note_box, radius=12, fill=(248, 250, 255), outline=BLUE, width=3)
notes = """少量注释：
1. 图 8a 只处理 0Fh 写暂存器和 AAh 读暂存器。
2. 写暂存器只是写入 8 字节暂存器，不等于写入 FRAM。
3. 读暂存器用于验证 TA1、TA2、E/S、数据和 CRC16。
4. 地址低 3 位会被强制清零，所以写入地址应 8 字节对齐。
5. PF=1 表示暂存器数据无效，不能继续复制或加载密钥。"""
y = note_box[1] + 25
for line in wrap_text(notes, note_box[2] - note_box[0] - 40, F_TINY):
    d.text((note_box[0] + 25, y), line, font=F_TINY, fill=BLACK)
    y += 31

d.rectangle((20, 20, W - 20, H - 20), outline=(180, 180, 180), width=2)
img.save(out)
print(out)
