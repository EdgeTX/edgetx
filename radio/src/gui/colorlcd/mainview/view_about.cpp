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

#include "edgetx.h"
#include "stamp.h"

#if defined(VERSION_TAG)
const std::string about_str =
    "EdgeTX"
    " (" VERSION_TAG
    ")\n"
    "\"" CODENAME "\"";
#else
const std::string about_str =
    "EdgeTX"
    " (" VERSION "-" VERSION_SUFFIX ")";
#endif
const std::string copyright_str = "Copyright (C) " BUILD_YEAR " EdgeTX";
const std::string edgetx_url = "https://edgetx.org";

AboutUs::AboutUs() :
    BaseDialog(STR_ABOUT_US, true, 220, LV_SIZE_CONTENT)
{
  new StaticText(form, {0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                 about_str + "\n" + copyright_str,
                 COLOR_THEME_SECONDARY1_INDEX, CENTERED);

  auto qrBox = new Window(form, {0, 0, LV_PCT(100), QR_SZ});
  auto qr = new QRCode(qrBox, 0, 0, QR_SZ, edgetx_url);
  lv_obj_center(qr->getLvObj());
}
