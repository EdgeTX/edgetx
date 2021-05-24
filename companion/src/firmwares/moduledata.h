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

#ifndef MODULEDATA_H
#define MODULEDATA_H

#include "constants.h"

#include <QtCore>

class Firmware;
class RadioDataConversionState;

enum PulsesProtocol {
  PULSES_OFF,
  PULSES_PPM,
  PULSES_SILV_A,
  PULSES_SILV_B,
  PULSES_SILV_C,
  PULSES_CTP1009,
  PULSES_LP45,
  PULSES_DSM2,
  PULSES_DSMX,
  PULSES_PPM16,
  PULSES_PPMSIM,
  PULSES_PXX_XJT_X16,
  PULSES_PXX_XJT_D8,
  PULSES_PXX_XJT_LR12,
  PULSES_PXX_DJT,
  PULSES_CROSSFIRE,
  PULSES_MULTIMODULE,
  PULSES_PXX_R9M,
  PULSES_PXX_R9M_LITE,
  PULSES_PXX_R9M_LITE_PRO,
  PULSES_SBUS,
  PULSES_ACCESS_ISRM,
  PULSES_ACCST_ISRM_D16,
  PULSES_ACCESS_R9M,
  PULSES_ACCESS_R9M_LITE,
  PULSES_ACCESS_R9M_LITE_PRO,
  PULSES_XJT_LITE_X16,
  PULSES_XJT_LITE_D8,
  PULSES_XJT_LITE_LR12,
  PULSES_AFHDS3,
  PULSES_GHOST,
  PULSES_PROTOCOL_LAST
};

enum MultiModuleRFProtocols {
  MODULE_SUBTYPE_MULTI_FLYSKY=0,
  MODULE_SUBTYPE_MULTI_FIRST=MODULE_SUBTYPE_MULTI_FLYSKY,
  MODULE_SUBTYPE_MULTI_HUBSAN,
  MODULE_SUBTYPE_MULTI_FRSKY,
  MODULE_SUBTYPE_MULTI_HISKY,
  MODULE_SUBTYPE_MULTI_V2X2,
  MODULE_SUBTYPE_MULTI_DSM2,
  MODULE_SUBTYPE_MULTI_DEVO,
  MODULE_SUBTYPE_MULTI_YD717,
  MODULE_SUBTYPE_MULTI_KN,
  MODULE_SUBTYPE_MULTI_SYMAX,
  MODULE_SUBTYPE_MULTI_SLT,
  MODULE_SUBTYPE_MULTI_CX10,
  MODULE_SUBTYPE_MULTI_CG023,
  MODULE_SUBTYPE_MULTI_BAYANG,
  MODULE_SUBTYPE_MULTI_ESky,
  MODULE_SUBTYPE_MULTI_MT99XX,
  MODULE_SUBTYPE_MULTI_MJXQ,
  MODULE_SUBTYPE_MULTI_SHENQI,
  MODULE_SUBTYPE_MULTI_FY326,
  MODULE_SUBTYPE_MULTI_FUTABA,
  MODULE_SUBTYPE_MULTI_J6PRO,
  MODULE_SUBTYPE_MULTI_FQ777,
  MODULE_SUBTYPE_MULTI_ASSAN,
  MODULE_SUBTYPE_MULTI_HONTAI,
  MODULE_SUBTYPE_MULTI_OLRS,
  MODULE_SUBTYPE_MULTI_FS_AFHDS2A,
  MODULE_SUBTYPE_MULTI_Q2X2,
  MODULE_SUBTYPE_MULTI_WK_2X01,
  MODULE_SUBTYPE_MULTI_Q303,
  MODULE_SUBTYPE_MULTI_GW008,
  MODULE_SUBTYPE_MULTI_DM002,
  MODULE_SUBTYPE_MULTI_CABELL,
  MODULE_SUBTYPE_MULTI_ESKY150,
  MODULE_SUBTYPE_MULTI_H83D,
  MODULE_SUBTYPE_MULTI_CORONA,
  MODULE_SUBTYPE_MULTI_CFLIE,
  MODULE_SUBTYPE_MULTI_HITEC,
  MODULE_SUBTYPE_MULTI_WFLY,
  MODULE_SUBTYPE_MULTI_BUGS,
  MODULE_SUBTYPE_MULTI_BUGS_MINI,
  MODULE_SUBTYPE_MULTI_TRAXXAS,
  MODULE_SUBTYPE_MULTI_NCC1701,
  MODULE_SUBTYPE_MULTI_E01X,
  MODULE_SUBTYPE_MULTI_V911S,
  MODULE_SUBTYPE_MULTI_GD00X,
  MODULE_SUBTYPE_MULTI_V761,
  MODULE_SUBTYPE_MULTI_KF606,
  MODULE_SUBTYPE_MULTI_REDPINE,
  MODULE_SUBTYPE_MULTI_POTENSIC,
  MODULE_SUBTYPE_MULTI_ZSX,
  MODULE_SUBTYPE_MULTI_HEIGHT,
  MODULE_SUBTYPE_MULTI_SCANNER,
  MODULE_SUBTYPE_MULTI_FRSKYX_RX,
  MODULE_SUBTYPE_MULTI_AFHDS2A_RX,
  MODULE_SUBTYPE_MULTI_HOTT,
  MODULE_SUBTYPE_MULTI_FX816,
  MODULE_SUBTYPE_MULTI_BAYANG_RX,
  MODULE_SUBTYPE_MULTI_PELIKAN,
  MODULE_SUBTYPE_MULTI_TIGER,
  MODULE_SUBTYPE_MULTI_XK,
  MODULE_SUBTYPE_MULTI_XN297DUMP,
  MODULE_SUBTYPE_MULTI_FRSKYX2,
  MODULE_SUBTYPE_MULTI_FRSKY_R9,
  MODULE_SUBTYPE_MULTI_PROPEL,
  MODULE_SUBTYPE_MULTI_FRSKYL,
  MODULE_SUBTYPE_MULTI_SKYARTEC,
  MODULE_SUBTYPE_MULTI_ESKY150V2,
  MODULE_SUBTYPE_MULTI_DSM_RX,
  MODULE_SUBTYPE_MULTI_JJRC345,
  MODULE_SUBTYPE_MULTI_Q90C,
  MODULE_SUBTYPE_MULTI_KYOSHO,
  MODULE_SUBTYPE_MULTI_RLINK,
  MODULE_SUBTYPE_MULTI_ELRS,
  MODULE_SUBTYPE_MULTI_REALACC,
  MODULE_SUBTYPE_MULTI_OMP,
  MODULE_SUBTYPE_MULTI_MLINK,
  MODULE_SUBTYPE_MULTI_WFLY2,
  MODULE_SUBTYPE_MULTI_E016HV2,
  MODULE_SUBTYPE_MULTI_LAST = MODULE_SUBTYPE_MULTI_E016HV2
};

enum TrainerProtocol {
  TRAINER_MASTER_JACK,
  TRAINER_SLAVE_JACK,
  TRAINER_MASTER_SBUS_MODULE,
  TRAINER_MASTER_CPPM_MODULE,
  TRAINER_MASTER_SBUS_BATT_COMPARTMENT
};

enum ModuleSubtypeR9M {
  MODULE_SUBTYPE_R9M_FCC,
  MODULE_SUBTYPE_R9M_EU,
  MODULE_SUBTYPE_R9M_EUPLUS,
  MODULE_SUBTYPE_R9M_AUPLUS,
  MODULE_SUBTYPE_R9M_LAST=MODULE_SUBTYPE_R9M_AUPLUS
};

constexpr int PXX2_MAX_RECEIVERS_PER_MODULE = 3;
constexpr int PXX2_LEN_RX_NAME              = 8;

class ModuleData {
  Q_DECLARE_TR_FUNCTIONS(ModuleData)

  public:
    ModuleData()
    {
      clear();
    }

    unsigned int modelId;
    unsigned int protocol;   // type in datastructs.h
    int          rfProtocol; // rfProtocol in datastructs.h

    unsigned int subType;
    bool         invertedSerial;
    unsigned int channelsStart;
    int          channelsCount; // 0=8 channels
    unsigned int failsafeMode;

    struct PPM {
      int delay;
      bool pulsePol;           // false = positive
      bool outputType;         // false = open drain, true = push pull
      int frameLength;
    } ppm;

    struct Multi {
      unsigned int rfProtocol;
      bool disableTelemetry;
      bool disableMapping;
      bool autoBindMode;
      bool lowPowerMode;
      int optionValue;
    } multi;

    struct Afhds3 {
      unsigned int rxFreq;
      unsigned int rfPower;
    } afhds3;

    struct PXX {
      unsigned int power;          // 0 10 mW, 1 100 mW, 2 500 mW, 3 1W
      bool receiverTelemetryOff;     // false = receiver telem enabled
      bool receiverHigherChannels;  // false = pwm out 1-8, true 9-16
      int antennaMode;       // false = internal antenna, true = external antenna
    } pxx;

    struct Access {
      unsigned int receivers;
      char         receiverName[PXX2_MAX_RECEIVERS_PER_MODULE][PXX2_LEN_RX_NAME+1];
      unsigned int racingMode;
    } access;

    void clear() { memset(this, 0, sizeof(ModuleData)); }
    void convert(RadioDataConversionState & cstate);
    bool isPxx2Module() const;
    bool supportRxNum() const;
    QString polarityToString() const { return ppm.pulsePol ? tr("Positive") : tr("Negative"); }
    QString rfProtocolToString() const;
    QString subTypeToString(int type = -1) const;
    QString powerValueToString(Firmware * fw) const;
    static QString indexToString(int index, Firmware * fw);
    static QString protocolToString(unsigned protocol);
    static QStringList powerValueStrings(enum PulsesProtocol protocol, int subType, Firmware * fw);
    bool hasFailsafes(Firmware * fw) const;
    int getMaxChannelCount();
};

#endif // MODULEDATA_H
