/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "view_about.h"

#include "opentx.h"
#include "stamp.h"

const uint8_t _mask_qrcode[] = {
#include "mask_qrcode.lbm"
};
STATIC_LZ4_BITMAP(mask_qrcode);

#if defined(VERSION_TAG)
const std::string about_str = "EdgeTX" " (" VERSION_TAG ")\n" "\"" CODENAME "\"" ;
#else
const std::string about_str = "EdgeTX" " (" VERSION "-" VERSION_SUFFIX ")";
#endif
const std::string copyright_str = "Copyright (C) 2022 EdgeTX";

AboutUs::AboutUs() :
    MessageDialog(MainWindow::instance(), STR_ABOUT_US, "", "",
                  CENTERED | FONT(BOLD) | COLOR_THEME_SECONDARY1, CENTERED)
{
  constexpr int TOP_PADDING = 35;
  constexpr coord_t ABOUT_WIDTH = 220;

#if LCD_H > LCD_W
#if defined(VERSION_TAG)
  constexpr int NUM_LINES = 4;
  constexpr int DIALOG_HEIGHT = 280;
#else
  constexpr int NUM_LINES = 3;
  constexpr int DIALOG_HEIGHT = 260;
#endif
  content->setRect({(LCD_W - ABOUT_WIDTH) / 2, (LCD_H - DIALOG_HEIGHT) / 2,
                    ABOUT_WIDTH, DIALOG_HEIGHT});
#else
  constexpr int NUM_LINES = 3;
  content->setRect({(LCD_W - ABOUT_WIDTH) / 2, 10, ABOUT_WIDTH, LCD_H - 20});
#endif

  messageWidget->setTop(content->top() + TOP_PADDING);
  messageWidget->setHeight(NUM_LINES * PAGE_LINE_HEIGHT);

  messageWidget->setText(about_str + "\n" + copyright_str);

  qrcode = BitmapBuffer::load8bitMaskOnBackground(
      mask_qrcode, COLOR_THEME_SECONDARY1, COLOR_THEME_SECONDARY3);
  new StaticBitmap(content,
                   rect_t{content->width() / 2 - qrcode->width() / 2,
                          TOP_PADDING + (NUM_LINES * PAGE_LINE_HEIGHT),
                          qrcode->width(), qrcode->height()},
                   qrcode);
}

AboutUs::~AboutUs() { delete qrcode; }
