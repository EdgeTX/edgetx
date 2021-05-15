#include "edgetx.h"
#include "../../common/arm/stm32/bootloader/boot.h"
#include "../../common/arm/stm32/bootloader/bin_files.h"
#define SELECTED_COLOR (INVERS | DEFAULT_COLOR)
#define DEFAULT_PADDING 28
#define DOUBLE_PADDING  56
#define MESSAGE_TOP     (LCD_H - (2*DOUBLE_PADDING))
//#include "bmp_plug_usb.lbm"
//#include "bmp_usb_plugged.lbm"

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
  lcdColorTable[DEFAULT_BGCOLOR_INDEX] = WHITE;
  lcdColorTable[LINE_COLOR_INDEX]      = RED;
  lcdColorTable[BARGRAPH1_COLOR_INDEX] = RED;
  lcdColorTable[BARGRAPH2_COLOR_INDEX] = RGB(73, 219, 62); // green
 
  backlightEnable(BACKLIGHT_LEVEL_MAX);
}

static void bootloaderDrawTitle(const char* text)
{
    lcd->drawText(LCD_W/2, DEFAULT_PADDING, text, CENTERED | DEFAULT_COLOR);
    lcd->drawSolidFilledRect(DEFAULT_PADDING, DOUBLE_PADDING, LCD_W - DOUBLE_PADDING, 2, DEFAULT_COLOR);
}

static void bootloaderDrawFooter()
{
    lcd->drawSolidFilledRect(DEFAULT_PADDING, LCD_H - (DOUBLE_PADDING + 4), LCD_W - DOUBLE_PADDING, 2, DEFAULT_COLOR);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char* str)
{
#if defined(COLORLCD)
    lcdNextLayer();
#endif
    // clear screen
    lcd->drawSolidFilledRect(0, 0, LCD_W-1, LCD_H-1, DEFAULT_BGCOLOR);
    int center = LCD_W/2;
    if (st == ST_START) {

        bootloaderDrawTitle("NV14 BOOTLOADER");
        
        lcd->drawBitmapPattern(50, 72, LBM_FLASH, DEFAULT_COLOR);
        lcd->drawText(84,  75, "Write Firmware");

        lcd->drawBitmapPattern(50, 107, LBM_EXIT, DEFAULT_COLOR);
        lcd->drawText(84, 110, "Exit");

        lcd->drawSolidRect(79, (opt == 0) ? 72 : 107, LCD_W - 79 - 28, 26, 2, LINE_COLOR);
        
        //lcd->drawBitmap(60, 166, &BMP_PLUG_USB);
        lcd->drawText(center, 175, "Or plug in a USB cable", CENTERED | DEFAULT_COLOR);
        lcd->drawText(center, 200, "for mass storage", CENTERED | DEFAULT_COLOR);

        bootloaderDrawFooter();
        lcd->drawText(center, LCD_H - DOUBLE_PADDING, "Current Firmware:", CENTERED | DEFAULT_COLOR);
        lcd->drawText(center, LCD_H - DEFAULT_PADDING, getOtherVersion(nullptr), CENTERED | DEFAULT_COLOR);
    }
    else if (st == ST_USB) {
        //lcd->drawBitmap(136, 98, &BMP_USB_PLUGGED);
        lcd->drawText(center, 128, "USB Connected", CENTERED | DEFAULT_COLOR);
    }
    else if (st == ST_FILE_LIST || st == ST_DIR_CHECK || st == ST_FLASH_CHECK ||
             st == ST_FLASHING || st == ST_FLASH_DONE) {

        bootloaderDrawTitle("SD>FIRMWARE");
        lcd->drawBitmapPattern(DEFAULT_PADDING, 16, LBM_SD, DEFAULT_COLOR);

        if (st == ST_FLASHING || st == ST_FLASH_DONE) {

            LcdFlags color = BARGRAPH1_COLOR; // red

            if (st == ST_FLASH_DONE) {
                color = BARGRAPH2_COLOR/* green */;
                opt   = 100; // Completed > 100%
            }

            lcd->drawRect(DEFAULT_PADDING, 120, LCD_W - DOUBLE_PADDING, 31, 2);
            lcd->drawSolidFilledRect(DEFAULT_PADDING+4, 124, ((LCD_W - DOUBLE_PADDING - 8) * opt) / 100, 23, color);
        }
        else if (st == ST_DIR_CHECK) {

            if (opt == FR_NO_PATH) {
                lcd->drawText(20, MESSAGE_TOP, "Directory is missing");
            }
            else {
                lcd->drawText(20, MESSAGE_TOP, "Directory is empty");
            }

            lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_ERROR, BARGRAPH1_COLOR);
        }
        else if (st == ST_FLASH_CHECK) {

            bootloaderDrawFilename(str, 0, true);

            if (opt == FC_ERROR) {
                lcd->drawText(20, MESSAGE_TOP, STR_INVALID_FIRMWARE);
                lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_ERROR, BARGRAPH1_COLOR);
            }
            else if (opt == FC_OK) {
                VersionTag tag;
                extractFirmwareVersion(&tag);

                lcd->drawText(LCD_W/4 + DEFAULT_PADDING, MESSAGE_TOP, "Version:", RIGHT);
                lcd->drawText(LCD_W/4 + 6 + DEFAULT_PADDING, MESSAGE_TOP, tag.version);
                
                lcd->drawText(LCD_W/4 + DEFAULT_PADDING, MESSAGE_TOP + DEFAULT_PADDING, "Radio:", RIGHT);
                lcd->drawText(LCD_W/4 + 6 + DEFAULT_PADDING, MESSAGE_TOP + DEFAULT_PADDING, tag.flavour);
                
                lcd->drawBitmapPattern(LCD_W - DOUBLE_PADDING, MESSAGE_TOP-10, LBM_OK, BARGRAPH2_COLOR);
            }
        }
        
        bootloaderDrawFooter();

        if ( st != ST_DIR_CHECK && (st != ST_FLASH_CHECK || opt == FC_OK)) {

            lcd->drawBitmapPattern(DEFAULT_PADDING, LCD_H - DOUBLE_PADDING - 2, LBM_FLASH, DEFAULT_COLOR);

            if (st == ST_FILE_LIST) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "[R TRIM] to select file");
            }
            else if (st == ST_FLASH_CHECK && opt == FC_OK) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Hold [R TRIM] long to flash");
            }
            else if (st == ST_FLASHING) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Writing Firmware ...");
            }
            else if (st == ST_FLASH_DONE) {
                lcd->drawText(DOUBLE_PADDING, LCD_H - DOUBLE_PADDING, "Writing Completed");
            }
        }

        if (st != ST_FLASHING) {
            lcd->drawBitmapPattern(DEFAULT_PADDING, LCD_H - DEFAULT_PADDING - 2, LBM_EXIT, DEFAULT_COLOR);
            lcd->drawText(DOUBLE_PADDING, LCD_H - DEFAULT_PADDING, "[L TRIM] to exit");
        }        
    }
}

void bootloaderDrawFilename(const char* str, uint8_t line, bool selected)
{
    lcd->drawBitmapPattern(DEFAULT_PADDING, 76 + (line * 25), LBM_FILE, DEFAULT_COLOR);
    lcd->drawText(DEFAULT_PADDING + 30, 75 + (line * 25), str);

    if (selected) {
        lcd->drawSolidRect(DEFAULT_PADDING + 25, 72 + (line * 25), LCD_W - (DEFAULT_PADDING + 25) - 28, 26, 2, LINE_COLOR);
    }
}
