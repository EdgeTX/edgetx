
class Display:

    def __init__(self, w, h, phys_w, phys_h, depth, color, oled, bl_color):
        self.w = w
        self.h = h
        self.phys_w = phys_w
        self.phys_h = phys_h
        self.depth = depth
        self.color = color
        self.oled = oled
        self.backlight_color = bl_color

def parse_lcd(hw_defs):

    fw = f'LCD_W'
    fh = f'LCD_H'
    fphys_w = f'LCD_PHYS_W'
    fphys_h = f'LCD_PHYS_H'
    fdepth = f'LCD_DEPTH'
    foled = f'OLED_SCREEN'
    fbl_color = f'HAS_BACKLIGHT_COLOR'

    w = hw_defs[fw]
    h = hw_defs[fh]
    if fphys_w in hw_defs:
      phys_w = hw_defs[fphys_w]
    else:
      phys_w = w
    if fphys_h in hw_defs:
      phys_h = hw_defs[fphys_h]
    else:
      phys_h = h
    depth = hw_defs[fdepth]

    if phys_w == fw:
      phys_w = w
    if phys_w == fh:
      phys_w = h
    if phys_h == fh:
      phys_h = h
    if phys_h == fw:
      phys_h = w

    if foled in hw_defs:
      oled = 1
    else:
      oled = 0

    if depth < 16:
      color = 0
    else:
      color = 1

    if fbl_color in hw_defs:
      bl_color = 1
    else:
      bl_color = 0

    return Display(w, h, phys_w, phys_h, depth, color, oled, bl_color)
