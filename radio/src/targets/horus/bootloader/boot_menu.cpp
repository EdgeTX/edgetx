#include "opentx.h"
#include "../../common/arm/stm32/bootloader/boot.h"
#include "../../common/arm/stm32/bootloader/bin_files.h"

#define SELECTED_COLOR (INVERS | DEFAULT_COLOR)

const uint8_t __bmp_plug_usb_rle[] {
#include "bmp_plug_usb.lbm"
};
RLEBitmap BMP_PLUG_USB(BMP_ARGB4444, __bmp_plug_usb_rle);

const uint8_t __bmp_usb_plugged_rle[] {
#include "bmp_usb_plugged.lbm"
};
RLEBitmap BMP_USB_PLUGGED(BMP_ARGB4444, __bmp_usb_plugged_rle);

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

void bootloaderInitScreen()
{
  lcdColorTable[DEFAULT_COLOR_INDEX]      = BLACK;
  lcdColorTable[DEFAULT_BGCOLOR_INDEX]    = WHITE;
  lcdColorTable[LINE_COLOR_INDEX]      = RED;
  lcdColorTable[BARGRAPH1_COLOR_INDEX] = RED;
  lcdColorTable[BARGRAPH2_COLOR_INDEX] = RGB(73, 219, 62); // green
 
  backlightEnable(BACKLIGHT_LEVEL_MAX);

  // TODO: load/decompress bitmaps
  extern void loadFonts();
  loadFonts();
}

static void bootloaderDrawTitle(unsigned int x, const char* text)
{
  lcd->drawText(x, 28, text);
  lcd->drawSolidFilledRect(28, 56, 422, 2, DEFAULT_COLOR);
}

static void bootloaderDrawFooter()
{
  lcd->drawSolidFilledRect(28, 234, 422, 2, DEFAULT_COLOR);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
#if defined(COLORLCD)
  lcdNextLayer();
#endif

    lcd->clear(DEFAULT_BGCOLOR);
    
    if (st == ST_START) {
        bootloaderDrawTitle(88, BOOTLOADER_TITLE);

        lcd->drawBitmapPattern(90, 72, LBM_FLASH, DEFAULT_COLOR);
      lcd->drawText(124,  75, "Write Firmware");

      lcd->drawBitmapPattern(90, 107, LBM_EXIT, DEFAULT_COLOR);
      lcd->drawText(124, 110, "Exit");

      lcd->drawSolidRect(119, (opt == 0) ? 72 : 107, 270, 26, 2, LINE_COLOR);

      lcd->drawBitmap(60, 166, &BMP_PLUG_USB);
      lcd->drawText(195, 175, "Or plug in a USB cable");
      lcd->drawText(195, 200, "for mass storage");

      bootloaderDrawFooter();
      lcd->drawText(LCD_W / 2, 242, getFirmwareVersion(), CENTERED);
    }
    else if (st == ST_USB) {

        lcd->drawBitmap(136, 98, &BMP_USB_PLUGGED);
        lcd->drawText(195, 128, "USB Connected");
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle(126, "SD>FIRMWARE");
        lcd->drawBitmapPattern(87, 16, LBM_SD, DEFAULT_COLOR);

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {

            LcdFlags color = BARGRAPH1_COLOR; // red

            if (st == ST_FLASH_DONE) {
                color = BARGRAPH2_COLOR/* green */;
                opt   = 100; // Completed > 100%
            }

            lcd->drawRect(70, 120, 340, 31, 2);
            lcd->drawSolidFilledRect(74, 124, (332 * opt) / 100, 23, color);
        }
        else if (st == ST_DIR_CHECK) {

            if (opt == FR_NO_PATH) {
                lcd->drawText(90, 168, "Directory is missing");
            }
            else {
                lcd->drawText(90, 168, "Directory is empty");
            }

            lcd->drawBitmapPattern(356, 158, LBM_ERROR, BARGRAPH1_COLOR);
        }
        else if (st == ST_FLASH_CHECK) {

            bootloaderDrawFilename(str, 0, true);

            if (opt == FC_ERROR) {
                lcd->drawText(94, 168, STR_INVALID_FIRMWARE);
                lcd->drawBitmapPattern(356, 158, LBM_ERROR, BARGRAPH1_COLOR);
            }
            else if (opt == FC_OK) {
                VersionTag tag;
                extractFirmwareVersion(&tag);

                lcd->drawText(168, 158, "Version:", RIGHT);
                lcd->drawText(174, 158, tag.version);
                
                lcd->drawText(168, 178, "Radio:", RIGHT);
                lcd->drawText(174, 178, tag.flavour);
                
                lcd->drawBitmapPattern(356, 158, LBM_OK, BARGRAPH2_COLOR);
            }
        }
        
        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawBitmapPattern(28, 242, LBM_FLASH, DEFAULT_COLOR);

            if (st == ST_FILE_LIST) {
                lcd->drawText(56, 244, "[ENT] to select file");
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(56, 244, "Hold [ENT] long to flash");
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(56, 244, "Writing Firmware ...");
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(56, 244, "Writing Completed");
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawBitmapPattern(305, 242, LBM_EXIT, DEFAULT_COLOR);
            lcd->drawText(335, 244, "[RTN] to exit");
        }        
    }
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawBitmapPattern(94, 76 + (line * 25), LBM_FILE, DEFAULT_COLOR);
    lcd->drawText(124, 75 + (line * 25), str);

    if (selected) {
        lcd->drawSolidRect(119, 72 + (line * 25), 278, 26, 2, LINE_COLOR);
    }
}
