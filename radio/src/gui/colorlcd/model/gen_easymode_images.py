#!/usr/bin/env python3
"""Generate vehicle silhouette images for Easy Mode wizard as LVGL alpha-8bit C arrays."""

from PIL import Image, ImageDraw
import io, sys

W, H = 80, 64  # Image dimensions

def draw_airplane(draw):
    """Top-view airplane silhouette."""
    cx, cy = W//2, H//2
    # Fuselage
    fw, fh = 8, 44
    draw.rounded_rectangle([cx-fw//2, cy-fh//2, cx+fw//2, cy+fh//2], radius=4, fill=200)
    # Nose cone
    draw.polygon([(cx-fw//2, cy-fh//2), (cx, cy-fh//2-8), (cx+fw//2, cy-fh//2)], fill=220)
    # Main wings (swept)
    draw.polygon([(cx-4, cy-6), (cx-36, cy-10), (cx-34, cy+2), (cx-4, cy+4)], fill=255)
    draw.polygon([(cx+4, cy-6), (cx+36, cy-10), (cx+34, cy+2), (cx+4, cy+4)], fill=255)
    # Horizontal tail
    draw.polygon([(cx-3, cy+16), (cx-18, cy+14), (cx-16, cy+20), (cx-3, cy+20)], fill=230)
    draw.polygon([(cx+3, cy+16), (cx+18, cy+14), (cx+16, cy+20), (cx+3, cy+20)], fill=230)
    # Vertical tail (top view = thin line)
    draw.rectangle([cx-1, cy+12, cx+1, cy+22], fill=255)

def draw_helicopter(draw):
    """Top-view helicopter silhouette."""
    cx, cy = W//2, H//2 + 4
    # Fuselage (teardrop)
    draw.ellipse([cx-8, cy-12, cx+8, cy+8], fill=220)
    draw.polygon([(cx-4, cy+4), (cx, cy+6), (cx+4, cy+4)], fill=220)
    # Tail boom
    draw.rectangle([cx-2, cy+6, cx+2, cy+26], fill=180)
    # Tail fin
    draw.polygon([(cx-10, cy+22), (cx, cy+20), (cx+10, cy+22), (cx, cy+26)], fill=200)
    # Main rotor disc
    draw.ellipse([cx-28, cy-28, cx+28, cy+0], outline=200, width=2)
    # Rotor hub
    draw.ellipse([cx-3, cy-16, cx+3, cy-10], fill=255)
    # Rotor blades (2 lines)
    draw.line([(cx-26, cy-14), (cx+26, cy-14)], fill=255, width=3)
    draw.line([(cx, cy-26), (cx, cy-2)], fill=255, width=3)

def draw_glider(draw):
    """Top-view glider - long wingspan, slim fuselage."""
    cx, cy = W//2, H//2
    # Fuselage (very slim)
    fw = 5
    draw.rounded_rectangle([cx-fw//2, cy-20, cx+fw//2, cy+20], radius=3, fill=200)
    # Nose
    draw.polygon([(cx-fw//2, cy-20), (cx, cy-26), (cx+fw//2, cy-20)], fill=220)
    # Very long wings
    draw.polygon([(cx-3, cy-6), (cx-38, cy-8), (cx-37, cy-2), (cx-3, cy+2)], fill=255)
    draw.polygon([(cx+3, cy-6), (cx+38, cy-8), (cx+37, cy-2), (cx+3, cy+2)], fill=255)
    # T-tail horizontal
    draw.polygon([(cx-2, cy+14), (cx-12, cy+13), (cx-11, cy+17), (cx-2, cy+17)], fill=230)
    draw.polygon([(cx+2, cy+14), (cx+12, cy+13), (cx+11, cy+17), (cx+2, cy+17)], fill=230)
    # Vertical tail
    draw.rectangle([cx-1, cy+10, cx+1, cy+18], fill=255)

def draw_multirotor(draw):
    """Top-view quadcopter X-frame."""
    cx, cy = W//2, H//2
    armLen = 22
    motorR = 10
    # Arms (X pattern)
    draw.line([(cx-armLen, cy-armLen), (cx+armLen, cy+armLen)], fill=200, width=3)
    draw.line([(cx+armLen, cy-armLen), (cx-armLen, cy+armLen)], fill=200, width=3)
    # Center body
    draw.rounded_rectangle([cx-7, cy-7, cx+7, cy+7], radius=3, fill=240)
    # Motor circles
    for dx, dy in [(-armLen, -armLen), (armLen, -armLen), (armLen, armLen), (-armLen, armLen)]:
        mx, my = cx+dx, cy+dy
        draw.ellipse([mx-motorR, my-motorR, mx+motorR, my+motorR], outline=220, width=2)
        draw.ellipse([mx-3, my-3, mx+3, my+3], fill=255)
    # Direction arrow (front = top)
    draw.polygon([(cx, cy-12), (cx-4, cy-7), (cx+4, cy-7)], fill=255)

def draw_car(draw):
    """Top-view car silhouette."""
    cx, cy = W//2, H//2
    bw, bh = 24, 42
    # Body
    draw.rounded_rectangle([cx-bw//2, cy-bh//2, cx+bw//2, cy+bh//2], radius=6, fill=200)
    # Windshield area (lighter)
    draw.rounded_rectangle([cx-bw//2+3, cy-bh//2+8, cx+bw//2-3, cy-bh//2+16], radius=2, fill=140)
    # Rear window
    draw.rounded_rectangle([cx-bw//2+3, cy+bh//2-14, cx+bw//2-3, cy+bh//2-8], radius=2, fill=140)
    # Front wheels
    draw.rounded_rectangle([cx-bw//2-5, cy-bh//2+6, cx-bw//2, cy-bh//2+16], radius=2, fill=255)
    draw.rounded_rectangle([cx+bw//2, cy-bh//2+6, cx+bw//2+5, cy-bh//2+16], radius=2, fill=255)
    # Rear wheels
    draw.rounded_rectangle([cx-bw//2-5, cy+bh//2-16, cx-bw//2, cy+bh//2-6], radius=2, fill=255)
    draw.rounded_rectangle([cx+bw//2, cy+bh//2-16, cx+bw//2+5, cy+bh//2-6], radius=2, fill=255)

def draw_boat(draw):
    """Top-view boat/hull silhouette."""
    cx, cy = W//2, H//2
    # Hull (pointed bow, wider stern)
    hull = [(cx, cy-26), (cx-16, cy-4), (cx-14, cy+16), (cx-8, cy+22),
            (cx+8, cy+22), (cx+14, cy+16), (cx+16, cy-4)]
    draw.polygon(hull, fill=200)
    # Deck area
    draw.ellipse([cx-10, cy-10, cx+10, cy+8], fill=160)
    # Cabin
    draw.rounded_rectangle([cx-6, cy-4, cx+6, cy+6], radius=2, fill=240)
    # Bow detail
    draw.line([(cx, cy-24), (cx, cy-8)], fill=255, width=2)

def img_to_c_array(img, name):
    """Convert PIL Image to C array of alpha values."""
    data = list(img.getdata())
    lines = []
    lines.append(f"// {W}x{H} alpha-8bit image")
    lines.append(f"static const uint8_t {name}[{W*H}] = {{")
    for row in range(H):
        vals = data[row*W:(row+1)*W]
        line = "  " + ",".join(f"{v:3d}" for v in vals) + ","
        lines.append(line)
    lines.append("};")
    return "\n".join(lines)

def generate():
    vehicles = [
        ("easymode_img_airplane", draw_airplane),
        ("easymode_img_helicopter", draw_helicopter),
        ("easymode_img_glider", draw_glider),
        ("easymode_img_multirotor", draw_multirotor),
        ("easymode_img_car", draw_car),
        ("easymode_img_boat", draw_boat),
    ]

    header = []
    header.append("// Auto-generated vehicle silhouette images for Easy Mode wizard")
    header.append("// Format: LVGL LV_IMG_CF_ALPHA_8BIT (1 byte per pixel, 0=transparent, 255=opaque)")
    header.append(f"// Size: {W}x{H} pixels each")
    header.append("#pragma once")
    header.append(f"#define EASYMODE_IMG_W {W}")
    header.append(f"#define EASYMODE_IMG_H {H}")
    header.append("")

    for name, draw_fn in vehicles:
        img = Image.new("L", (W, H), 0)
        draw = ImageDraw.Draw(img)
        draw_fn(draw)
        header.append(img_to_c_array(img, name))
        header.append("")

    # Also generate a lookup table
    header.append("static const uint8_t* const easymode_images[] = {")
    header.append("  nullptr,  // EASYMODE_NONE")
    for name, _ in vehicles:
        header.append(f"  {name},")
    header.append("};")

    return "\n".join(header)

if __name__ == "__main__":
    print(generate())
