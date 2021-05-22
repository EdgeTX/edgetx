#include "opentx.h"
#include "../../common/arm/stm32/bootloader/boot.h"
#include "../../common/arm/stm32/bootloader/bin_files.h"

const uint8_t __bmp_plug_usb_rle[] {
#include "bmp_plug_usb.lbm"
};
RLEBitmap BMP_PLUG_USB(BMP_ARGB4444, __bmp_plug_usb_rle);

const uint8_t __bmp_usb_plugged_rle[] {
#include "bmp_usb_plugged.lbm"
};
RLEBitmap BMP_USB_PLUGGED(BMP_ARGB4444, __bmp_usb_plugged_rle);

const uint8_t __bmp_background_rle[] {
#include "bmp_background.lbm"
};
RLEBitmap BMP_BACKGROUND(BMP_ARGB4444, __bmp_background_rle);

const uint8_t LBM_FLASH[] = {
#include "icon_flash.lbm"
};

const uint8_t LBM_EXIT[] = {
#include "icon_exit.lbm"
};

const uint8_t LBM_SD[] = {
#include "icon_sd.lbm"
};

const uint8_t LBM_FILE[] = {
#include "icon_file.lbm"
};

const uint8_t LBM_ERROR[] = {
#include "icon_error.lbm"
};

const uint8_t LBM_OK[] = {
#include "icon_ok.lbm"
};

#define BL_GREEN      COLOR2FLAGS(RGB(73, 219, 62))
#define BL_RED        COLOR2FLAGS(RED)
#define BL_BACKGROUND COLOR2FLAGS(BLACK)
#define BL_FOREGROUND COLOR2FLAGS(WHITE)
#define BL_SELECTED   COLOR2FLAGS(RGB(11, 65, 244)) // deep blue


void bootloaderInitScreen()
{
  backlightEnable(BACKLIGHT_LEVEL_MAX);

  // TODO: load/decompress bitmaps
  extern void loadFonts();
  loadFonts();
}

static void bootloaderDrawTitle(unsigned int x, const char* text)
{
  lcd->drawText(x, 28, text, BL_FOREGROUND);
  lcd->drawSolidFilledRect(28, 56, 422, 2, BL_FOREGROUND);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(28, 234, 422, 2, BL_FOREGROUND);
}

static void bootloaderDrawBackground()
{
  // we have plenty of memory, let's cache that background
  static BitmapBuffer* _background = nullptr;

  if (!_background) {
    _background = new BitmapBuffer(BMP_RGB565, LCD_W, LCD_H);
    
    for (int i=0; i<LCD_W; i += BMP_BACKGROUND.width()) {
      for (int j=0; j<LCD_H; j += BMP_BACKGROUND.height()) {
        _background->drawBitmap(i, j, &BMP_BACKGROUND);
      }
    }
    _background->drawFilledRect(0, 0, LCD_W, LCD_H, SOLID,
                                COLOR2FLAGS(BLACK), OPACITY(4));
  }

  if (_background) {
    lcd->drawBitmap(0, 0, _background);
  }
  else {
    lcd->clear(BL_BACKGROUND);
  }
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
    bootloaderDrawBackground();
    
    if (st == ST_START) {
        bootloaderDrawTitle(88, BOOTLOADER_TITLE);

        lcd->drawBitmapPattern(90, 72, LBM_FLASH, BL_FOREGROUND);
        lcd->drawText(124,  75, "Write Firmware", BL_FOREGROUND);

      lcd->drawBitmapPattern(90, 107, LBM_EXIT, BL_FOREGROUND);
      lcd->drawText(124, 110, "Exit", BL_FOREGROUND);

      lcd->drawSolidRect(119, (opt == 0) ? 72 : 107, 270, 26, 2, BL_SELECTED);

      lcd->drawBitmap(60, 166, &BMP_PLUG_USB);
      lcd->drawText(195, 175, "Or plug in a USB cable", BL_FOREGROUND);
      lcd->drawText(195, 200, "for mass storage", BL_FOREGROUND);

      bootloaderDrawFooter();
      lcd->drawText(LCD_W / 2, 242, getFirmwareVersion(), CENTERED | BL_FOREGROUND);
    }
    else if (st == ST_USB) {

        lcd->drawBitmap(136, 98, &BMP_USB_PLUGGED);
        lcd->drawText(195, 128, "USB Connected", BL_FOREGROUND);
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle(126, "SD>FIRMWARE");
        lcd->drawBitmapPattern(87, 16, LBM_SD, BL_FOREGROUND);

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {

            LcdFlags color = BL_RED;

            if (st == ST_FLASH_DONE) {
                color = BL_GREEN;
                opt   = 100; // Completed > 100%
            }

            lcd->drawRect(70, 120, 340, 31, 2, SOLID, BL_SELECTED);
            lcd->drawSolidFilledRect(74, 124, (332 * opt) / 100, 23, color);
        }
        else if (st == ST_DIR_CHECK) {

            if (opt == FR_NO_PATH) {
                lcd->drawText(90, 168, "Directory is missing", BL_FOREGROUND);
            }
            else {
                lcd->drawText(90, 168, "Directory is empty", BL_FOREGROUND);
            }

            lcd->drawBitmapPattern(356, 158, LBM_ERROR, BL_RED);
        }
        else if (st == ST_FLASH_CHECK) {

            bootloaderDrawFilename(str, 0, true);

            if (opt == FC_ERROR) {
                lcd->drawText(94, 168, STR_INVALID_FIRMWARE, BL_FOREGROUND);
                lcd->drawBitmapPattern(356, 158, LBM_ERROR, BL_RED);
            }
            else if (opt == FC_OK) {
                VersionTag tag;
                extractFirmwareVersion(&tag);

                lcd->drawText(168, 158, "Version:", RIGHT | BL_FOREGROUND);
                lcd->drawText(174, 158, tag.version, BL_FOREGROUND);
                
                lcd->drawText(168, 178, "Radio:", RIGHT | BL_FOREGROUND);
                lcd->drawText(174, 178, tag.flavour, BL_FOREGROUND);
                
                lcd->drawBitmapPattern(356, 158, LBM_OK, BL_GREEN);
            }
        }
        
        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawBitmapPattern(28, 242, LBM_FLASH, BL_FOREGROUND);

            if (st == ST_FILE_LIST) {
                lcd->drawText(56, 244, "[ENT] to select file", BL_FOREGROUND);
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(56, 244, "Hold [ENT] long to flash", BL_FOREGROUND);
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(56, 244, "Writing Firmware ...", BL_FOREGROUND);
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(56, 244, "Writing Completed", BL_FOREGROUND);
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawBitmapPattern(305, 242, LBM_EXIT, BL_FOREGROUND);
            lcd->drawText(335, 244, "[RTN] to exit", BL_FOREGROUND);
        }        
    }

    // wait for next frame
    // while ((LTDC->CDSR & LTDC_CDSR_VSYNCS) == 0);
    // while ((LTDC->CDSR & LTDC_CDSR_VSYNCS) != 0);
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawBitmapPattern(94, 76 + (line * 25), LBM_FILE, BL_FOREGROUND);
    lcd->drawText(124, 75 + (line * 25), str, BL_FOREGROUND);

    if (selected) {
        lcd->drawSolidRect(119, 72 + (line * 25), 278, 26, 2, BL_SELECTED);
    }
}
