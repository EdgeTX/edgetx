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

#ifndef _WARNINGS_H_
#define _WARNINGS_H_

#include "appdata.h"

#include <QtCore>
#include <QCheckBox>
#include <QMessageBox>

#define CPN_STR_MSG_WELCOME   QCoreApplication::translate("Companion", \
  "<p><b>Welcome to EdgeTX %1.</b></p>" \
  "<p>As the first step, please configure the initial Radio Profile by selecting your Radio Type, Menu Language, and Build Options.</p>" \
  "<p>You may also want to take this time to review the other available options in the displayed Settings dialog.</p>" \
  "<p>After saving your settings, we recommend you download the latest firmware for your radio by using the <i>File -&gt; Download</i> menu option.</p>" \
  "<p>Please visit <a href='https://edgetx.org'>edgetx.org</a> for latest news, updates and documentation. Thank you for choosing EdgeTX!</p>" \
  "- The EdgeTX Team.")

#define CPN_STR_MSG_UPGRADED  QCoreApplication::translate("Companion", \
  "<p><b>Thank you for upgrading to EdgeTX %1.</b></p>" \
  "<p>This is a major upgrade that adds and modifies a lot of things, so please make sure that you read release notes carefully" \
  "  to learn about the changes, and thoroughly check each of your models for proper function.</p>" \
  "<p>Please visit <a href='https://edgetx.org'>edgetx.org</a> for release notes and other documentation.</p>" \
  "- The EdgeTX Team.")

#define CPN_STR_MSG_NO_RADIO_TYPE  QCoreApplication::translate("Companion", \
  "<p>The radio type in the selected profile does not exist. Using the default type instead.</p> <p><b>Please update your profile settings!</b></p>")

class AppMessages
{
  Q_DECLARE_TR_FUNCTIONS("AppMessages")

  public:
    // These are used for saving "[don't] show this message again" user preferences.
    enum MessageCodes {
      MSG_NONE          = 0,
      MSG_WELCOME       = 0x01,
      MSG_UPGRADED      = 0x02,
      MSG_RESERVED      = 0x04,  // reserved due to legacy code
      MSG_NO_RADIO_TYPE = 0x08,
      MSG_ENUM_END
    };

    static void displayMessage(int id, QWidget * parent = nullptr)
    {
      QString infoTxt;
      QMessageBox::Icon icon = QMessageBox::Information;
      bool transient = false;

      switch (id) {
        case MSG_WELCOME:
          infoTxt = CPN_STR_MSG_WELCOME.arg(VERSION);
          break;

        case MSG_UPGRADED:
          infoTxt = CPN_STR_MSG_UPGRADED.arg(VERSION);
          break;

        case MSG_NO_RADIO_TYPE:
          infoTxt = CPN_STR_MSG_NO_RADIO_TYPE;
          icon = QMessageBox::Warning;
          transient = true;
          break;

        default:
          return;
      }

      QMessageBox msgBox(parent);
      msgBox.setWindowTitle(CPN_STR_APP_NAME);
      msgBox.setIcon(icon);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setText(infoTxt);

      if (!transient)
        msgBox.setCheckBox(new QCheckBox(QObject::tr("Show this message again at next startup?")));

      msgBox.exec();

      if (transient || (msgBox.checkBox() && !msgBox.checkBox()->isChecked()))
        g.warningId(g.warningId() & ~id);
    }

};

#endif // _WARNINGS_H_

