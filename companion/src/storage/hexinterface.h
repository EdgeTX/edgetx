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

#ifndef _HEXINTERFACE_H_
#define _HEXINTERFACE_H_

#include <inttypes.h>
#include <QTextStream>

class HexInterface {
  public:
    HexInterface(QTextStream &stream);

    int load(uint8_t * output, int maxsize);
    bool save(const uint8_t * data, const int size);

  protected:

    int getValueFromLine(const QString &line, int pos, int len=2);
    QString iHEXLine(const quint8 * data, quint32 addr, quint8 len);
    QString iHEXExtRec(quint8 bank);

    QTextStream & stream;
};

#endif // _HEXINTERFACE_H_
