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

#include "updatesounds.h"

UpdateSounds::UpdateSounds(QWidget * parent) :
  UpdateInterface(parent)
{
  setName("Sounds");
  setRepo(QString(GH_REPOS_EDGETX).append("/edgetx-sdcard-sounds"));

  /*
    file: sounds.json
  [
      {
          "language": "cs_CZ",
          "name": "Czech Female",
          "description": "Czech Female Voice (cs-CZ-Vlasta)",
          "directory": "cz"
      },

    TODO support multiple language sub variants
  //  eg edgetx-sdcard-sounds-en_gb-libby-2.7.0.zip
  ap.filter = QString("^edgetx-sdcard-sounds-%1").arg(language);  //return all variants of each language
  */

  UpdateParameters::AssetParams &ap = dfltParams->addAsset();
  //  only supports default for each language
  //  eg edgetx-sdcard-sounds-en-2.7.0.zip
  ap.filterType = UpdateParameters::UFT_Startswith;
  ap.filter = QString("edgetx-sdcard-sounds-%LANGUAGE%-");
  //  if user changes filter then more than one could be returned
  //ap.maxExpected = 1;
  ap.flags = dfltParams->data.flags | UPDFLG_CopyStructure;
}
