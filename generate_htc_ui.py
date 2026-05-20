import os
from PIL import Image, ImageDraw, ImageFont

# Конфигурация карточек
CARD_W = 100
CARD_H = 110
HALF_W = CARD_W // 2  # 50
CORNER_RAD = 8
BG_COLOR_TOP1 = (245, 245, 245)
BG_COLOR_TOP2 = (220, 220, 220)
BG_COLOR_BOT1 = (200, 200, 200)
BG_COLOR_BOT2 = (235, 235, 235)
TEXT_COLOR = (40, 40, 40)
LINE_COLOR = (30, 30, 30)

def draw_half_card(digit, is_left):
    # Создаем изображение 2x размера для антиалиасинга
    img = Image.new('RGBA', (CARD_W * 2, CARD_H * 2), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    
    # Фон (одна общая карточка)
    # Top half
    for y in range(CARD_H):
        t = y / (CARD_H)
        # Градиенты
        if y < CARD_H/2:
            r = int(BG_COLOR_TOP1[0] * (1-t*2) + BG_COLOR_TOP2[0] * (t*2))
            g = int(BG_COLOR_TOP1[1] * (1-t*2) + BG_COLOR_TOP2[1] * (t*2))
            b = int(BG_COLOR_TOP1[2] * (1-t*2) + BG_COLOR_TOP2[2] * (t*2))
        else:
            t2 = (y - CARD_H/2) / (CARD_H/2)
            r = int(BG_COLOR_BOT1[0] * (1-t2) + BG_COLOR_BOT2[0] * t2)
            g = int(BG_COLOR_BOT1[1] * (1-t2) + BG_COLOR_BOT2[1] * t2)
            b = int(BG_COLOR_BOT1[2] * (1-t2) + BG_COLOR_BOT2[2] * t2)
        
        d.rectangle([0, y*2, CARD_W*2, y*2+2], fill=(r,g,b,255))

    # Скругляем углы (маска)
    mask = Image.new('L', (CARD_W * 2, CARD_H * 2), 0)
    dm = ImageDraw.Draw(mask)
    dm.rounded_rectangle([0, 0, CARD_W*2-1, CARD_H*2-1], radius=CORNER_RAD*2, fill=255)
    img.putalpha(mask)

    # Текст
    try:
        font = ImageFont.truetype("arialbd.ttf", 150)
    except:
        font = ImageFont.load_default() # Fallback

    text = str(digit)
    # Позиционирование текста по центру целой карты
    bbox = d.textbbox((0,0), text, font=font)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    
    # Сместим вверх на пару пикселей для визуального баланса
    tx = (CARD_W * 2 - tw) / 2 - bbox[0]
    ty = (CARD_H * 2 - th) / 2 - bbox[1] - 10
    
    d.text((tx, ty), text, font=font, fill=TEXT_COLOR)
    
    # Линия разделения
    gap_y = CARD_H
    d.rectangle([0, gap_y-2, CARD_W*2, gap_y+2], fill=LINE_COLOR)

    # Уменьшаем (сглаживание)
    img = img.resize((CARD_W, CARD_H), Image.LANCZOS)
    
    # Обрезаем нужную половину
    if is_left:
        return img.crop((0, 0, HALF_W, CARD_H))
    else:
        return img.crop((HALF_W, 0, CARD_W, CARD_H))

def to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def export_h_file():
    lefts = [draw_half_card(i, True) for i in range(10)]
    rights = [draw_half_card(i, False) for i in range(10)]
    
    with open('c:\\Users\\JZX\\Documents\\Arduino\\Land_Cruiser\\Htc_Clock_Data.h', 'w') as f:
        f.write("#ifndef HTC_CLOCK_DATA_H\n")
        f.write("#define HTC_CLOCK_DATA_H\n")
        f.write("#include <Arduino.h>\n")
        f.write(f"#define CLOCK_HALF_W {HALF_W}\n")
        f.write(f"#define CLOCK_H {CARD_H}\n\n")
        
        for k, arr in [("left", lefts), ("right", rights)]:
            for i, img in enumerate(arr):
                f.write(f"const uint16_t digit_{k}_{i}[] PROGMEM = {{\n    ")
                pixels = img.load()
                w, h = img.size
                cnt = 0
                for y in range(h):
                    for x in range(w):
                        p = pixels[x, y]
                        # Если прозрачность, примешиваем фоновый цвет
                        # Фон макета: kColorFrame = rgb565(43, 54, 72)
                        bg_r, bg_g, bg_b = 43, 54, 72
                        a = p[3] / 255.0
                        r = int(p[0] * a + bg_r * (1-a))
                        g = int(p[1] * a + bg_g * (1-a))
                        b = int(p[2] * a + bg_b * (1-a))
                        c565 = to_rgb565(r, g, b)
                        f.write(f"0x{c565:04X}, ")
                        cnt += 1
                        if cnt % 20 == 0:
                            f.write("\n    ")
                f.write("\n};\n\n")

        # Create arrays of pointers for easy access
        f.write("const uint16_t* const digits_left[10] = {")
        f.write(", ".join([f"digit_left_{i}" for i in range(10)]))
        f.write("};\n")
        f.write("const uint16_t* const digits_right[10] = {")
        f.write(", ".join([f"digit_right_{i}" for i in range(10)]))
        f.write("};\n")
        f.write("#endif\n")

if __name__ == '__main__':
    export_h_file()
    print("Htc_Clock_Data.h generated successfully!")
