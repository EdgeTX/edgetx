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

#include "updateexpresslrs.h"

UpdateExpressLRS::UpdateExpressLRS(QWidget * parent) :
  UpdateInterface(parent, CID_ExpressLRS, tr("ExpressLRS"), Repo::REPO_TYPE_GITHUB,
                  QString(GH_API_REPOS).append("/ExpressLRS/ExpressLRS"), "", 100)
{
  init(); // call after UpdateInterface ctor due to virtual functions
}

void UpdateExpressLRS::assetSettingsInit()
{
  if (!isSettingsIndexValid())
    return;

  g.component[id()].initAllAssets();

  {
  ComponentAssetData &cad = g.component[id()].asset[0];
  cad.desc("script");
  cad.processes(UPDFLG_Common_Asset &~ UPDFLG_Decompress);
  cad.flags(cad.processes() | UPDFLG_CopyFiles);
  cad.filterType(UpdateParameters::UFT_Pattern);
  cad.filter("^elrs.*\\.lua$");
  cad.maxExpected(1);
  cad.destSubDir("SCRIPTS/TOOLS");
  }

  qDebug() << "Asset settings initialised";
}
