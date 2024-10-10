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

#include "updatethemes.h"

UpdateThemes::UpdateThemes(QWidget * parent) :
  UpdateInterface(parent, CID_Themes, tr("Themes"), Repo::REPO_TYPE_GITHUB,
                  QString(GH_API_REPOS_EDGETX).append("/themes"))
{
  init(); // call after UpdateInterface ctor due to virtual functions
}

void UpdateThemes::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  ComponentAssetData &cad = g.component[id()].asset[0];
  cad.desc("files");
  cad.processes(UPDFLG_Common_Asset);
  cad.flags(cad.processes() | UPDFLG_CopyStructure);
  cad.filterType(UpdateParameters::UFT_Startswith);
  cad.filter("edgetx-themes");
  cad.maxExpected(1);

  qDebug() << "Asset settings initialised";
}
