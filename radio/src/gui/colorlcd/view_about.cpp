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

constexpr coord_t ABOUT_WIDTH = 220;
#if defined(VERSION_TAG)
const std::string about_str = "EdgeTX" " " "\"" CODENAME "\" (" VERSION_TAG ")";
#else
const std::string about_str = "EdgeTX" " (" VERSION "-" VERSION_SUFFIX ")";
#endif
const std::string copyright_str = "Copyright (C) 2022 EdgeTX";

AboutUs::AboutUs() :
  MessageDialog(MainWindow::instance(), STR_ABOUT_US, "")
{
  content->setRect({(LCD_W - ABOUT_WIDTH) / 2, 20, ABOUT_WIDTH, LCD_H - 40});

  messageWidget->setTextFlags(CENTERED | FONT(BOLD) | COLOR_THEME_SECONDARY1);
  messageWidget->setTop(content->top() + 40);
  messageWidget->setHeight(2*PAGE_LINE_HEIGHT);

  messageWidget->setText(about_str + "\n" + copyright_str);

  qrcode = BitmapBuffer::load8bitMaskOnBackground(
      mask_qrcode, COLOR_THEME_SECONDARY1, COLOR_THEME_SECONDARY3);
  new StaticBitmap(content,
                   rect_t{ content->width() / 2 - qrcode->width() / 2, 80,
                       qrcode->width(), qrcode->height()},
                   qrcode);
}

AboutUs::~AboutUs()
{
  delete qrcode;
}
