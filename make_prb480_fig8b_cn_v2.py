from pathlib import Path
import math

from PIL import Image, ImageDraw, ImageFont


out = Path.cwd() / "PRB480_Figure8b_CN_v3.png"
W, H = 3300, 2300
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


F_TITLE = font(42)
F = font(24)
F_SMALL = font(20)
F_NOTE = font(18)
BLACK = (25, 25, 25)
BLUE = (25, 80, 150)
GREEN = (35, 120, 70)
RED = (170, 50, 45)
GRAY = (90, 90, 90)
LIGHT = (246, 249, 252)
NOTE = (255, 253, 232)
SHA = (240, 250, 245)
PROG = (253, 245, 238)


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


def center_text(box, text, fnt=F_SMALL, fill=BLACK, line_gap=5):
    x1, y1, x2, y2 = box
    lines = wrap_text(text, x2 - x1 - 22, fnt)
    hs = [
        d.textbbox((0, 0), line, font=fnt)[3]
        - d.textbbox((0, 0), line, font=fnt)[1]
        for line in lines
    ]
    total = sum(hs) + line_gap * (len(lines) - 1) if lines else 0
    y = y1 + (y2 - y1 - total) / 2
    for line, h in zip(lines, hs):
        tw = d.textbbox((0, 0), line, font=fnt)[2]
        d.text((x1 + (x2 - x1 - tw) / 2, y), line, font=fnt, fill=fill)
        y += h + line_gap


def rect(box, text, fnt=F_SMALL, fill=LIGHT, outline=BLACK, width=3):
    d.rounded_rectangle(box, radius=8, fill=fill, outline=outline, width=width)
    center_text(box, text, fnt)
    return box


def diamond(cx, cy, w, h, text, fnt=F_SMALL):
    pts = [(cx, cy - h / 2), (cx + w / 2, cy), (cx, cy + h / 2), (cx - w / 2, cy)]
    d.polygon(pts, fill="white", outline=BLACK)
    d.line(pts + [pts[0]], fill=BLACK, width=3)
    center_text((cx - w / 2 + 24, cy - h / 2 + 18, cx + w / 2 - 24, cy + h / 2 - 18), text, fnt)
    return (cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2)


def arrow(p1, p2, color=BLACK, width=3):
    d.line([p1, p2], fill=color, width=width)
    x1, y1 = p1
    x2, y2 = p2
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 14
    pts = [
        (x2, y2),
        (x2 + size * math.cos(ang + math.pi * 0.82), y2 + size * math.sin(ang + math.pi * 0.82)),
        (x2 + size * math.cos(ang - math.pi * 0.82), y2 + size * math.sin(ang - math.pi * 0.82)),
    ]
    d.polygon(pts, fill=color)


def polyline(points, color=BLACK, width=3, arrow_end=True):
    d.line(points, fill=color, width=width)
    if arrow_end and len(points) >= 2:
        arrow(points[-2], points[-1], color, width)


def label(x, y, text, color=BLUE):
    d.text((x, y), text, font=F_NOTE, fill=color)


center_text((0, 22, W, 95), "PRB480 图 8b 中文翻译：Load First Secret / Compute Next Secret", F_TITLE, BLUE)
center_text((0, 90, W, 135), "尽量保持原图框架：先判断 5Ah，再判断 33h；左侧为加载密钥，右侧为生成下一密钥", F, GRAY)

# Main top command chain.
rect((1030, 155, 1570, 225), "从图 8a 进入", F)
arrow((1300, 225), (1300, 275))
diamond(1300, 345, 410, 125, "5Ah？\n加载初始密钥", F_SMALL)
label(1090, 315, "是 Y", GREEN)
label(1515, 315, "否 N", RED)
polyline([(1505, 345), (1710, 345), (1710, 420)], BLACK)
diamond(1710, 495, 410, 125, "33h？\n生成下一密钥", F_SMALL)
label(1495, 465, "是 Y", GREEN)
label(1925, 465, "否 N", RED)
polyline([(1915, 495), (2170, 495)], BLACK)
rect((2170, 455, 2415, 535), "转到图 8c\n继续判断其他命令", F_SMALL, NOTE)

# LFS branch, left side, preserving downward decision style.
polyline([(1095, 345), (690, 345), (690, 430)], BLACK)
rect((420, 430, 960, 525), "注意：8 字节密钥必须先写入暂存器\n即先执行图 8a 的 Write Scratchpad", F_SMALL, NOTE)
arrow((690, 525), (690, 585))
rect((455, 585, 925, 675), "主机发送 TA1、TA2、E/S\n三字节来自 Read Scratchpad", F_SMALL)
arrow((690, 675), (690, 740))
diamond(690, 815, 410, 120, "认证字节匹配？\nTA1/TA2/E/S 是否一致", F_SMALL)
label(480, 785, "否 N", RED)
polyline([(485, 815), (245, 815), (245, 945)], RED)
rect((110, 945, 380, 1025), "失败\n主机读到全 1\nFFh 循环", F_NOTE, NOTE, RED)
label(705, 880, "是 Y", GREEN)
arrow((690, 875), (690, 945))
diamond(690, 1020, 370, 120, "目标地址是密钥区？\n通常为 0080h", F_SMALL)
label(705, 1085, "是 Y", GREEN)
label(480, 990, "否 N", RED)

# Normal secret path.
arrow((690, 1080), (690, 1150))
diamond(690, 1225, 345, 115, "是否写保护？\n密钥区保护", F_SMALL)
label(860, 1195, "是 Y", RED)
polyline([(862, 1225), (1030, 1225), (1030, 1345)], RED)
rect((900, 1345, 1160, 1425), "失败\n读到 FFh", F_NOTE, NOTE, RED)
label(705, 1285, "否 N", GREEN)
arrow((690, 1282), (690, 1360))

# Special EN_LFS path from address-not-secret branch.
polyline([(505, 1020), (350, 1020), (350, 1125)], BLACK)
diamond(350, 1200, 340, 115, "EN_LFS = 1？\n刷新写回模式", F_SMALL)
label(175, 1170, "否 N", RED)
polyline([(180, 1200), (80, 1200), (80, 1345)], RED)
rect((35, 1345, 250, 1425), "失败\n读到 FFh", F_NOTE, NOTE, RED)
label(365, 1260, "是 Y", GREEN)
polyline([(350, 1258), (350, 1500), (510, 1500)], BLACK)
diamond(690, 1500, 360, 115, "有效数据地址？\n00h 到 7Fh", F_SMALL)
label(510, 1468, "否 N", RED)
polyline([(510, 1500), (250, 1500)], RED)
rect((35, 1460, 250, 1540), "失败\n读到 FFh", F_NOTE, NOTE, RED)
label(705, 1560, "是 Y", GREEN)

# Merge to write protect/programming.
arrow((690, 1420), (690, 1450))
arrow((690, 1558), (690, 1605))
rect((455, 1605, 925, 1695), "等待 tPROG\n写入目标区域；总线保持高电平供电", F_SMALL, PROG)
arrow((690, 1695), (690, 1750))
rect((455, 1750, 925, 1835), "成功：AA = 1\nEN_LFS 清 0；暂存器填充 AAh", F_SMALL, SHA)

# CNS branch right side.
polyline([(1505, 495), (1295, 495), (1295, 585)], BLACK)
rect((1045, 585, 1545, 680), "注意：主机必须先把 8 字节\npartial secret 写入暂存器", F_SMALL, NOTE)
arrow((1295, 680), (1295, 740))
rect((1070, 740, 1520, 825), "主机发送 TA1、TA2\n指定参与计算的数据页", F_SMALL)
arrow((1295, 825), (1295, 890))
diamond(1295, 965, 360, 115, "有效数据地址？\n00h 到 7Fh", F_SMALL)
label(1115, 935, "否 N", RED)
polyline([(1115, 965), (960, 965), (960, 1095)], RED)
rect((835, 1095, 1085, 1175), "失败\n读到 FFh", F_NOTE, NOTE, RED)
label(1310, 1025, "是 Y", GREEN)
arrow((1295, 1022), (1295, 1090))
diamond(1295, 1165, 350, 115, "密钥是否写保护？", F_SMALL)
label(1470, 1135, "是 Y", RED)
polyline([(1470, 1165), (1630, 1165), (1630, 1290)], RED)
rect((1505, 1290, 1755, 1370), "失败\n读到 FFh", F_NOTE, NOTE, RED)
label(1310, 1225, "否 N", GREEN)
arrow((1295, 1222), (1295, 1300))
rect((1040, 1300, 1550, 1435), "等待 tCSHA\nPRB480 计算 MAC\n输入：当前密钥 + 页数据 + 暂存器 8 字节", F_SMALL, SHA)
arrow((1295, 1435), (1295, 1495))
rect((1040, 1495, 1550, 1610), "等待 tPROG\n将部分 MAC 写入 secret 寄存器\n作为新密钥", F_SMALL, PROG)
arrow((1295, 1610), (1295, 1668))
rect((1065, 1668, 1525, 1750), "成功：暂存器填充 AAh\n主机读到 AAh 循环", F_SMALL, SHA)

# Shared power/reset notes at bottom right, separate from core flow.
note_box = (1880, 850, 3180, 1420)
d.rounded_rectangle(note_box, radius=12, fill=(248, 250, 255), outline=BLUE, width=3)
notes = """少量注释：
1. 5Ah：加载初始密钥。它不直接接收密钥数据；密钥必须先由图 8a 写入暂存器。
2. 5Ah 发送 TA1、TA2、E/S，用来证明当前暂存器内容是刚才验证过的内容。
3. EN_LFS=1 是 Refresh Scratchpad 后的特殊刷新写回分支，普通加载密钥时通常为 0。
4. 33h：生成下一密钥。芯片用旧密钥、选中页数据、暂存器数据做 SHA-1，再把部分 MAC 写成新密钥。
5. tCSHA 和 tPROG 期间，主机应释放总线并保持高电平供电。
6. 图中多处 MASTER RESET 的含义：主机可复位中断当前命令，回到前面的命令选择流程。"""
y = note_box[1] + 25
for line in wrap_text(notes, note_box[2] - note_box[0] - 45, F_NOTE):
    d.text((note_box[0] + 25, y), line, font=F_NOTE, fill=BLACK)
    y += 29

d.rectangle((18, 18, W - 18, H - 18), outline=(180, 180, 180), width=2)
img.save(out)
print(out)
