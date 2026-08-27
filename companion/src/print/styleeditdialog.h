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

#ifndef _STYLEEDITDIALOG_H_
#define _STYLEEDITDIALOG_H_

#include "helpers_html.h"
#include <QtWidgets>

namespace Ui {
  class StyleEditDialog;
}

class StyleEditDialog : public QDialog
{
  Q_OBJECT

  public:
    StyleEditDialog(QWidget * parent, const QString stylesheet);
    ~StyleEditDialog();

  private slots:
    void update();
    void changed();
    void resetToDefault();

  private:
    Ui::StyleEditDialog *ui;
    Stylesheet mStylesheet;
    bool mDirty;
};

#endif // _STYLEEDITDIALOG_H_
