from PIL import Image
import sys

try:
    img = Image.open(r"C:\Users\JZX\.gemini\antigravity\brain\f17ffbb2-3f4d-49a3-992c-f867052befd2\media__1776622235163.png")
    img = img.convert("RGB")
    print(f"Image size: {img.size}")
    
    # Analyze center row roughly to find the clock cards
    w, h = img.size
    print(f"Middle pixels at y={h//2}:")
    for x in range(0, w, w//10):
        print(f"x={x}: {img.getpixel((x, h//2))}")
except Exception as e:
    print(e)
