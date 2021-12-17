/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _OPENTXEEPROM_H_
#define _OPENTXEEPROM_H_

#include <inttypes.h>
#include "eeprominterface.h"
#include "eepromimportexport.h"
#include <qbytearray.h>

#define GVARS_VARIANT                  0x0001
#define FRSKY_VARIANT                  0x0002
#define M128_VARIANT                   0x8000
#define TARANIS_X9E_VARIANT            0x8000
#define TARANIS_X7_VARIANT             0x4000
#define TARANIS_XLITE_VARIANT          0x2000
#define TARANIS_XLITES_VARIANT         0x1000
#define TARANIS_X9LITE_VARIANT         0x0800
#define TARANIS_X9LITES_VARIANT        0x0801
#define JUMPER_T12_VARIANT             0x4001
#define RADIOMASTER_TX12_VARIANT       0x4002
#define JUMPER_TLITE_VARIANT           0x4003
#define RADIOMASTER_T8_VARIANT         0x4004
#define RADIOMASTER_ZORRO_VARIANT      0x4006

class OpenTxGeneralData: public TransformedField {
  public:
    OpenTxGeneralData(GeneralSettings & generalData, Board::Type board, unsigned int version, unsigned int variant=0);

    virtual const QString & getName() { return internalField.getName(); }

    QStringList errors()
    {
      return _errors;
    }

  protected:
    virtual void beforeExport();
    virtual void afterImport();

    virtual void setError(const QString & error)
    {
      qWarning() << error;
      _errors << error;
    }

    StructField internalField;
    GeneralSettings & generalData;
    Board::Type board;
    unsigned int version;
    int inputsCount;
    unsigned int chkSum;
    QStringList _errors;
};

class ProtocolsConversionTable: public ConversionTable
{
  public:
    ProtocolsConversionTable(Board::Type board, int version)
    {
      int val = 0;
      addConversion(PULSES_OFF, val++);
      addConversion(PULSES_PPM, val++);

      addConversion(PULSES_PXX_XJT_X16, val);
      addConversion(PULSES_PXX_XJT_D8, val);
      addConversion(PULSES_PXX_XJT_LR12, val++);

      if (version >= 219) {
        addConversion(PULSES_ACCESS_ISRM, val);
        addConversion(PULSES_ACCST_ISRM_D16, val++);
      }

      addConversion(PULSES_LP45, val);
      addConversion(PULSES_DSM2, val);
      addConversion(PULSES_DSMX, val++);

      addConversion(PULSES_CROSSFIRE, val++);
      addConversion(PULSES_MULTIMODULE, val++);

      addConversion(PULSES_PXX_R9M, val++);
      addConversion(PULSES_ACCESS_R9M, val++);
      addConversion(PULSES_PXX_R9M_LITE, val++);
      addConversion(PULSES_ACCESS_R9M_LITE, val++);
      addConversion(PULSES_GHOST, val++);
      addConversion(PULSES_ACCESS_R9M_LITE_PRO, val++);

      addConversion(PULSES_SBUS, val++);

      addConversion(PULSES_XJT_LITE_X16, val);
      addConversion(PULSES_XJT_LITE_D8, val);
      addConversion(PULSES_XJT_LITE_LR12, val++);

      if (version >= 219) {
        addConversion(PULSES_AFHDS3, val++);
      }
    }
};

class ChannelsConversionTable: public ConversionTable
{
  public:
    ChannelsConversionTable()
    {
      for (int i=4; i<=16; i+= 2)
        addConversion(i, -4+i/2);
    }
};


class OpenTxModelData: public TransformedField {
  public:
    OpenTxModelData(ModelData & modelData, Board::Type board, unsigned int version, unsigned int variant);

    virtual const QString & getName() { return internalField.getName(); }

    QStringList errors()
    {
      return _errors;
    }

  protected:
    virtual void beforeExport();
    virtual void afterImport();

    virtual void setError(const QString & error)
    {
      qWarning() << error;
      _errors << error;
    }

    StructField internalField;
    ModelData & modelData;
    Board::Type board;
    unsigned int version;
    unsigned int variant;
    char name[256];
    int subprotocols[CPN_MAX_MODULES+1/*trainer*/];
    ChannelsConversionTable channelsConversionTable;
    QStringList _errors;
};

void OpenTxEepromCleanup(void);
#endif // _OPENTXEEPROM_H_
