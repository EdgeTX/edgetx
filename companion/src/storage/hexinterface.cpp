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

#include "hexinterface.h"

HexInterface::HexInterface(QTextStream &stream):
  stream(stream)
{
}

int HexInterface::getValueFromLine(const QString &line, int pos, int len)
{
    bool ok;
    int hex = line.mid(pos,len).toInt(&ok, 16);
    return ok ? hex : -1;
}

int HexInterface::load(uint8_t *data, int maxsize)
{
  int result = 0;
  int offset = 0;
  while (!stream.atEnd()) {
    QString line = stream.readLine();

    if(line.left(1)!=":") continue;
    
    int byteCount = getValueFromLine(line,1);
    int address = getValueFromLine(line,3,4);
    int recType = getValueFromLine(line,7);
    if (recType==0x02) {
        offset+=0x010000;
    }
    if(byteCount<0 || address<0 || recType<0)
      return 0;

    QByteArray ba;
    ba.clear();

    quint8 chkSum = 0;
    chkSum -= byteCount;
    chkSum -= recType;
    chkSum -= address & 0xFF;
    chkSum -= address >> 8;
    for(int i=0; i<byteCount; i++)
    {
        quint8 v = getValueFromLine(line,(i*2)+9) & 0xFF;
        chkSum -= v;
        ba.append(v);
    }

    quint8 retV = getValueFromLine(line,(byteCount*2)+9) & 0xFF;
    if(chkSum!=retV)
      return 0;

    if (address+offset + byteCount <= maxsize) {
      if (recType == 0x00) { //data record - ba holds record
        memcpy(&data[address+offset],ba.data(),byteCount);
        result = std::max(result, address+offset+byteCount);
      }
    }
    else {
      return 0;
    }
  }
  
  return result;
}


bool HexInterface::save(const uint8_t * data, const int size)
{
  int addr = 0;
  int nextbank = 1;
  while (addr < size) {
    if (addr>(nextbank*0x010000)-1) {
      stream << iHEXExtRec(nextbank) << "\n";
      nextbank++;
    }
    int llen = 32;
    if ((size - addr) < llen)
      llen = size - addr;
    stream << iHEXLine(data, addr, llen) << "\n";
    addr += llen;
  }
  stream << ":00000001FF\n"; // write EOF
  return true;
}

QString HexInterface::iHEXLine(const quint8 * data, quint32 addr, quint8 len)
{
  unsigned int bankaddr;
  bankaddr=addr&0xffff;
  QString str = QString(":%1%2000").arg(len, 2, 16, QChar('0')).arg(bankaddr, 4, 16, QChar('0')); //write start, bytecount (32), address and record type
  quint8 chkSum = 0;
  chkSum = -len; //-bytecount; recordtype is zero
  chkSum -= bankaddr & 0xFF;
  chkSum -= bankaddr >> 8;
  for (int j = 0; j < len; j++) {
    str += QString("%1").arg(data[addr + j], 2, 16, QChar('0'));
    chkSum -= data[addr + j];
  }

  str += QString("%1").arg(chkSum, 2, 16, QChar('0'));
  return str.toUpper(); // output to file and lf;
}

QString HexInterface::iHEXExtRec(quint8 bank)
{
  QString str = QString(":02000002"); //write record type 2 record
  quint8 chkSum = 0;
  chkSum = -2; //-bytecount; recordtype is zero
  chkSum -= 2; // type 2 record type
  str += QString("%1000").arg((bank&0x0f)<<4,1,16,QChar('0'));
  chkSum -= ((bank&0x0f)<<4); // type 2 record type
  str += QString("%1").arg(chkSum, 2, 16, QChar('0'));
  return str.toUpper(); // output to file and lf;
}

