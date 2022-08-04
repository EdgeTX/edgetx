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

#include <cstdlib>
#include <algorithm>
#include "boards.h"
#include "helpers.h"
#include "opentxeeprom.h"
#include "customdebug.h"
#include "opentxinterface.h"

using namespace Board;

#define MAX_VIEWS(board)                      (HAS_LARGE_LCD(board) ? 2 : 256)
#define MAX_GYRO_ANALOGS(board, version)      (version >= 219 ? Boards::getCapability(board, Board::GyroAnalogs) : 0)

inline int MAX_SWITCHES(Board::Type board, int version)
{
  if (version <= 218) {
    if (IS_TARANIS_X7(board))
      return 6;
    if (IS_TARANIS_X9D(board) || IS_FAMILY_HORUS_OR_T16(board))
      return 8;
    if (IS_TARANIS_XLITE(board))
      return 4;
  }

  if (IS_TARANIS_X9D(board))
    return 9;

  if (IS_JUMPER_TPRO(board))
     return 10;

  if (IS_FAMILY_T12(board))
    return 8;

  if (IS_TARANIS_X7(board))
    return 8;

  return Boards::getCapability(board, Board::Switches);
}

inline int MAX_SWITCHES_SOURCE(Board::Type board, int version)
{
  if (IS_JUMPER_TPRO(board))  // 10 switches are allocated in EEprom but 6 are reserved for FS
  return Boards::getCapability(board, Board::Switches);
  else
    return MAX_SWITCHES(board, version);
}

inline int MAX_SWITCHES_POSITION(Board::Type board, int version)
{
    if (IS_JUMPER_TPRO(board))
    return Boards::getCapability(board, Board::SwitchPositions);
  else if (IS_HORUS_OR_TARANIS(board))
    return MAX_SWITCHES(board, version) * 3;
  else
    return Boards::getCapability(board, Board::SwitchPositions);
  }

inline int MAX_FUNCTIONSWITCHES(Board::Type board, int version)
{
  return Boards::getCapability(board, Board::FunctionSwitches);
}

inline int MAX_FUNCTIONSWITCHES_POSITION(Board::Type board, int version)
{
  return Boards::getCapability(board, Board::NumFunctionSwitchesPositions);
}

inline int POTS_CONFIG_SIZE(Board::Type board, int version)
{
  if (version >= 219 && IS_FAMILY_HORUS_OR_T16(board))
    return 16;

  return 8;
}

inline int MAX_POTS(Board::Type board, int version)
{
  if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board))
    return 3;
  if (version <= 220 && (IS_HORUS_X10(board) || IS_FAMILY_T16(board)))
    return 5;
  if (IS_FAMILY_T12(board))
    return 2;
  return Boards::getCapability(board, Board::Pots);
}

inline int MAX_FUNCTION_SWITCHES(Board::Type board, int version)
{
  if (IS_JUMPER_TPRO(board))
    return 6;
  return 0;
}

inline int MAX_POTS_STORAGE(Board::Type board, int version)
{
  if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board))
    return 3;
  if (version <= 220 && IS_FAMILY_HORUS_OR_T16(board) && !IS_FLYSKY_NV14(board))
    return 5;
  if (IS_FAMILY_T12(board))
    return 2;
  return Boards::getCapability(board, Board::Pots);
}

inline int MAX_POTS_SOURCES(Board::Type board, int version)
{
  if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board))
    return 3;
  if (version <= 220 && (IS_HORUS_X10(board) || IS_FAMILY_T16(board)))
    return 5;
  if (IS_FAMILY_T12(board))
    return 2;
  return Boards::getCapability(board, Board::Pots);
}

inline int MAX_XPOTS(Board::Type board, int version)
{
  if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board))
    return 3;
  if (version <= 220 && (IS_HORUS_X10(board) || IS_FAMILY_T16(board)))
    return 5;
  if (IS_FAMILY_T12(board))
    return 2;
  return Boards::getCapability(board, Board::MultiposPots);
}

inline int MAX_SLIDERS_STORAGE(Board::Type board, int version)
{
  if (version >= 219 && (IS_FAMILY_HORUS_OR_T16(board) && !IS_FLYSKY_NV14(board)))
    return 4;
  return Boards::getCapability(board, Board::Sliders);
}

inline int MAX_SLIDERS_SOURCES(Board::Type board, int version)
{
  if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board))
    return 2;
  return Boards::getCapability(board, Board::Sliders);
}

inline int SLIDERS_CONFIG_SIZE(Board::Type board, int version)
{
  if (version >= 219 && IS_FAMILY_HORUS_OR_T16(board))
    return 8;

  return 4;
}

inline int SWITCHES_CONFIG_SIZE(Board::Type board, int version)
{
  if (IS_TARANIS_X9E(board))
    return 64;

  if (IS_FAMILY_HORUS_OR_T16(board))
    return 32;

  if (version >= 219 && (IS_TARANIS_X9D(board) || IS_JUMPER_TPRO(board)))
    return 32;

  return 16;
}

inline int MAX_MOUSE_ANALOG_SOURCES(Board::Type board, int version)
{
  if (IS_FAMILY_HORUS_OR_T16(board) && !IS_FLYSKY_NV14(board))
    return 2;
  else
    return 0;
}

#define MAX_ROTARY_ENCODERS(board)            0
#define MAX_FLIGHT_MODES(board, version)      9
#define MAX_TIMERS(board, version)            3
#define MAX_MIXERS(board, version)            64
#define MAX_CHANNELS(board, version)          32
#define MAX_TRIMS(board)                      (Boards::getCapability(board, Board::NumTrims))
#define MAX_EXPOS(board, version)             (IS_HORUS_OR_TARANIS(board) ? 64 : 32)
#define MAX_LOGICAL_SWITCHES(board, version)  (version >= 218 ? 64 : 32)
#define MAX_CUSTOM_FUNCTIONS(board, version)  64
#define MAX_CURVES(board, version)            ((version >= 219 || HAS_LARGE_LCD(board)) ? 32 : 16)
#define MAX_GVARS(board, version)             9
#define MAX_SCRIPTS(board)                    (IS_FAMILY_HORUS_OR_T16(board) ? 9 : 7)
#define MAX_TELEMETRY_SENSORS(board, version) (version <= 218 ? 32 : ((IS_FAMILY_HORUS_OR_T16(board) || IS_TARANIS_X9(board) || IS_FLYSKY_NV14(board)) ? 60 : 40))
#define NUM_PPM_INPUTS(board, version)        16
#define ROTENC_COUNT(board, version)          ((IS_STM32(board) && version >= 218) ? 0 : 1)
#define MAX_AUX_TRIMS(board)                  ((IS_FAMILY_HORUS_OR_T16(board) && !IS_FLYSKY_NV14(board)) ? 2 : 0)
#define MAX_SOURCE_TYPE_SPECIAL(board, version)    SOURCE_TYPE_SPECIAL_COUNT

inline int switchIndex(int i, Board::Type board, unsigned int version)
{
  if (!IS_HORUS_OR_TARANIS(board))
    return (i<=3 ? i+3 : (i<=6 ? i-3 : i));
  else
    return i;
}

class SwitchesConversionTable: public ConversionTable {

  public:
    SwitchesConversionTable(Board::Type board, unsigned int version, bool timer=false)
    {
      int val = 0;
      int offset = 0;
      if (timer) {
        offset = 4;
        for (int i=0; i<5; i++) {
          addConversion(RawSwitch(SWITCH_TYPE_TIMER_MODE, i), val++);
        }
      }
      else {
        addConversion(RawSwitch(SWITCH_TYPE_NONE), val++);
      }

      for (int i=1; i<=MAX_SWITCHES_POSITION(board, version); i++) {
        int s = switchIndex(i, board, version);
        addConversion(RawSwitch(SWITCH_TYPE_SWITCH, s), val);
        addConversion(RawSwitch(SWITCH_TYPE_SWITCH, -s), -val+offset);
        val++;
      }

      for (int i=1; i<=MAX_FUNCTIONSWITCHES_POSITION(board, version); i++) {
        int s = switchIndex(i, board, version);
        addConversion(RawSwitch(SWITCH_TYPE_FUNCTIONSWITCH, s), val);
        addConversion(RawSwitch(SWITCH_TYPE_FUNCTIONSWITCH, -s), -val+offset);
        val++;
      }

      if (IS_HORUS_OR_TARANIS(board)) {
        for (int i=1; i<=MAX_XPOTS(board, version)*6; i++) {
          addConversion(RawSwitch(SWITCH_TYPE_MULTIPOS_POT, -i), -val+offset);
          addConversion(RawSwitch(SWITCH_TYPE_MULTIPOS_POT, i), val++);
        }
      }

      for (int i=1; i<=2*MAX_TRIMS(board); i++) {
        addConversion(RawSwitch(SWITCH_TYPE_TRIM, -i), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_TRIM, i), val++);
      }

      for (int i=1; i<=MAX_ROTARY_ENCODERS(board); i++) {
        addConversion(RawSwitch(SWITCH_TYPE_ROTARY_ENCODER, -i), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_ROTARY_ENCODER, i), val++);
      }

      for (int i=1; i<=MAX_LOGICAL_SWITCHES(board, version); i++) {
        addConversion(RawSwitch(SWITCH_TYPE_VIRTUAL, -i), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_VIRTUAL, i), val++);
      }

      addConversion(RawSwitch(SWITCH_TYPE_OFF), -val+offset);
      addConversion(RawSwitch(SWITCH_TYPE_ON), val++);

      addConversion(RawSwitch(SWITCH_TYPE_ONE, -1), -val+offset);
      addConversion(RawSwitch(SWITCH_TYPE_ONE, 1), val++);
      for (int i=1; i<=MAX_FLIGHT_MODES(board, version); i++) {
        addConversion(RawSwitch(SWITCH_TYPE_FLIGHT_MODE, -i), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_FLIGHT_MODE, i), val++);
      }

      if (version >= 218) {
        addConversion(RawSwitch(SWITCH_TYPE_TELEMETRY, -1), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_TELEMETRY, 1), val++);
        for (int i=1; i<=MAX_TELEMETRY_SENSORS(board, version); i++) {
          addConversion(RawSwitch(SWITCH_TYPE_SENSOR, -i), -val+offset);
          addConversion(RawSwitch(SWITCH_TYPE_SENSOR, i), val++);
        }
      }
      if (version >= 219) {
        addConversion(RawSwitch(SWITCH_TYPE_ACT, -1), -val+offset);
        addConversion(RawSwitch(SWITCH_TYPE_ACT, 1), val++);
      }
    }


  protected:

    void addConversion(const RawSwitch & sw, const int b)
    {
      ConversionTable::addConversion(sw.toValue(), b);
    }

    class Cache {
      public:
        Cache(Board::Type board, unsigned int version, unsigned long flags, SwitchesConversionTable * table):
          board(board),
          version(version),
          flags(flags),
          table(table)
        {
        }
        Board::Type board;
        unsigned int version;
        unsigned long flags;
        SwitchesConversionTable * table;
    };

    static std::list<Cache> internalCache;

  public:

    static SwitchesConversionTable * getInstance(Board::Type board, unsigned int version, unsigned long flags=0)
    {
      for (auto & element : internalCache) {
        if (element.board == board && element.version == version && element.flags == flags)
          return element.table;
      }

      Cache element(board, version, flags, new SwitchesConversionTable(board, version, flags));
      internalCache.push_back(element);
      return element.table;
    }
    static void Cleanup()
    {
      for (auto & element : internalCache) {
        delete element.table;
      }
      internalCache.clear();
    }
};

std::list<SwitchesConversionTable::Cache> SwitchesConversionTable::internalCache;

#define FLAG_NONONE       0x01
#define FLAG_NOSWITCHES   0x02
#define FLAG_NOTELEMETRY  0x04

class SourcesConversionTable: public ConversionTable {
  public:
    SourcesConversionTable(Board::Type board, unsigned int version, unsigned int variant, unsigned long flags=0)
    {
      int val=0;

      if (!(flags & FLAG_NONONE)) {
        addConversion(RawSource(SOURCE_TYPE_NONE), val++);
      }

      if (version >= 218 || IS_STM32(board)) {
        for (int i=0; i<32; i++) {
          addConversion(RawSource(SOURCE_TYPE_VIRTUAL_INPUT, i), val++);
        }
      }

      if (IS_STM32(board)) {
        for (int i = 0; i < MAX_SCRIPTS(board); i++) {
          for (int j = 0; j < 6; j++) {
            addConversion(RawSource(SOURCE_TYPE_LUA_OUTPUT, i * 16 + j), val++);
          }
        }
      }

      for (int i=0; i<CPN_MAX_STICKS + MAX_POTS_SOURCES(board, version) + MAX_SLIDERS_SOURCES(board, version) + MAX_MOUSE_ANALOG_SOURCES(board, version) + MAX_GYRO_ANALOGS(board, version); i++) {
        int offset = 0;
        if (version <= 218 && IS_HORUS_X10(board) && i >= CPN_MAX_STICKS + MAX_POTS_STORAGE(board, version))
          offset += 2;
        if (version <= 220 && (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) && i >= CPN_MAX_STICKS + MAX_POTS_STORAGE(board, version))
          offset += 2;

        addConversion(RawSource(SOURCE_TYPE_STICK, i + offset), val++);
      }

      for (int i=0; i<MAX_ROTARY_ENCODERS(board); i++) {
        addConversion(RawSource(SOURCE_TYPE_ROTARY_ENCODER, 0), val++);
      }

      addConversion(RawSource(SOURCE_TYPE_MAX), val++);

      for (int i=0; i<3; i++)
        addConversion(RawSource(SOURCE_TYPE_CYC, i), val++);

      for (int i=0; i<MAX_TRIMS(board); i++)
        addConversion(RawSource(SOURCE_TYPE_TRIM, i), val++);

      addConversion(RawSource(SOURCE_TYPE_SWITCH, 0), val++);

      if (!(flags & FLAG_NOSWITCHES)) {
        for (int i=1; i<MAX_SWITCHES_SOURCE(board, version); i++)
          addConversion(RawSource(SOURCE_TYPE_SWITCH, i), val++);
        for (int i=0; i<MAX_FUNCTIONSWITCHES(board, version); i++)
           addConversion(RawSource(SOURCE_TYPE_FUNCTIONSWITCH, i), val++);
        for (int i=0; i<MAX_LOGICAL_SWITCHES(board, version); i++)
          addConversion(RawSource(SOURCE_TYPE_CUSTOM_SWITCH, i), val++);
      }

      for (int i=0; i<NUM_PPM_INPUTS(board, version); i++) {
        addConversion(RawSource(SOURCE_TYPE_PPM, i), val++);
      }

      for (int i=0; i<MAX_CHANNELS(board, version); i++) {
        addConversion(RawSource(SOURCE_TYPE_CH, i), val++);
      }

      if (!(flags & FLAG_NOTELEMETRY)) {
        for (int i=0; i<MAX_GVARS(board, version); i++)
          addConversion(RawSource(SOURCE_TYPE_GVAR, i), val++);
        for (int i=0; i<MAX_SOURCE_TYPE_SPECIAL(board, version); i++)
          addConversion(RawSource(SOURCE_TYPE_SPECIAL, i), val++);
        for (int i=0; i<MAX_TELEMETRY_SENSORS(board, version)*3; ++i) {
          addConversion(RawSource(SOURCE_TYPE_TELEMETRY, i), val++);
        }
      }
    }

  protected:

    void addConversion(const RawSource & source, const int b)
    {
      ConversionTable::addConversion(source.toValue(), b);
    }

    class Cache {
      public:
        Cache(Board::Type board, unsigned int version, unsigned int variant, unsigned long flags, SourcesConversionTable * table):
          board(board),
          version(version),
          variant(variant),
          flags(flags),
          table(table)
        {
        }
        Board::Type board;
        unsigned int version;
        unsigned int variant;
        unsigned long flags;
        SourcesConversionTable * table;
    };
    static std::list<Cache> internalCache;

  public:

    static SourcesConversionTable * getInstance(Board::Type board, unsigned int version, unsigned int variant, unsigned long flags=0)
    {
      for (std::list<Cache>::iterator it=internalCache.begin(); it!=internalCache.end(); it++) {
        Cache & element = *it;
        if (element.board == board && element.version == version && element.variant == variant && element.flags == flags)
          return element.table;
      }

      Cache element(board, version, variant, flags, new SourcesConversionTable(board, version, variant, flags));
      internalCache.push_back(element);
      return element.table;
    }
    static void Cleanup()
    {
      for (std::list<Cache>::iterator it=internalCache.begin(); it!=internalCache.end(); it++) {
        Cache & element = *it;
        if (element.table)
          delete element.table;
      }
      internalCache.clear();
    }
};

std::list<SourcesConversionTable::Cache> SourcesConversionTable::internalCache;

void OpenTxEepromCleanup(void)
{
  SourcesConversionTable::Cleanup();
  SwitchesConversionTable::Cleanup();
}

template <int N>
class SwitchField: public ConversionField< SignedField<N> > {
  public:
    SwitchField(DataField * parent, RawSwitch & sw, Board::Type board, unsigned int version, unsigned long flags=0):
      ConversionField< SignedField<N> >(parent, _switch, SwitchesConversionTable::getInstance(board, version, flags), DataField::tr("Switch").toLatin1(),
          DataField::tr("Switch ").toLatin1() + sw.toString(board) + DataField::tr(" cannot be exported on this board!").toLatin1()),
      sw(sw),
      _switch(0),
      board(board)
    {
    }

    virtual ~SwitchField()
    {
    }

    void beforeExport() override
    {
      _switch = sw.toValue();
      ConversionField< SignedField<N> >::beforeExport();
    }

    void afterImport() override
    {
      ConversionField< SignedField<N> >::afterImport();
      sw = RawSwitch(_switch);
      qCDebug(eepromImport) << QString("imported %1: %2").arg(ConversionField< SignedField<N> >::internalField.getName()).arg(sw.toString(board));
    }

  protected:
    RawSwitch & sw;
    int _switch;
    Board::Type board;
};

template <int N>
class SourceField: public ConversionField< UnsignedField<N> > {
  public:
    SourceField(DataField * parent, RawSource & source, Board::Type board, unsigned int version, unsigned int variant, unsigned long flags=0):
      ConversionField< UnsignedField<N> >(parent, _source, SourcesConversionTable::getInstance(board, version, variant, flags),
            DataField::tr("Source").toLatin1(), DataField::tr("Source %1 cannot be exported on this board!").arg(source.toString())),
      source(source),
      _source(0)
    {
    }

    virtual ~SourceField()
    {
    }

    void beforeExport() override
    {
      _source = source.toValue();
      ConversionField< UnsignedField<N> >::beforeExport();
    }

    void afterImport() override
    {
      ConversionField< UnsignedField<N> >::afterImport();
      source = RawSource(_source);
      qCDebug(eepromImport) << QString("imported %1: %2").arg(ConversionField< UnsignedField<N> >::internalField.getName()).arg(source.toString());
    }

  protected:
    RawSource & source;
    unsigned int _source;
};


int smallGvarImport(int gvar)
{
  if (gvar < -10000) {
    gvar = 128 + gvar + 10000;
  }
  else if (gvar > 10000) {
    gvar = -128 +gvar - 10001;
  }
  return gvar;
}

int smallGvarExport(int gvar)
{
  if (gvar > 110) {
    gvar = gvar - 128 - 10000;
  }
  else if (gvar < -110) {
    gvar = gvar + 128 + 10001;
  }
  return gvar;
}

void splitGvarParam(const int gvar, int & _gvar, unsigned int & _gvarParam, const Board::Type board, const int version)
{
  if (gvar < -10000) {
    _gvarParam = 0;
    _gvar = 256 + gvar + 10000;
  }
  else if (gvar > 10000) {
    _gvarParam = 1;
    _gvar = gvar - 10001;
  }
  else {
    if (gvar < 0) _gvarParam = 1;
    else          _gvarParam = 0;
    _gvar = gvar;  // save routine throws away all unused bits; therefore no 2er complement compensation needed here
  }
}

void concatGvarParam(int & gvar, const int _gvar, const unsigned int _gvarParam, const Board::Type board, const int version)
{
  gvar = _gvar;
  if (gvar<0) gvar+=256;  // remove 2er complement, because 8bit part is in this case unsigned
  if (_gvarParam) {  // here is the real sign bit
    gvar|=-256;   // set all higher bits to simulate negative value
  }

  if (gvar>245) {
    gvar = gvar-256 - 10000;
  }
  else if (gvar<-245) {
    gvar = gvar+256 + 10001;
  }

}

void exportGvarParam(const int gvar, int & _gvar, int version)
{
  int GV1 = 1024;

  if (gvar < -10000) {
    _gvar = GV1 + gvar + 10000;
  }
  else if (gvar > 10000) {
    _gvar = GV1 + gvar - 10001;
  }
  else {
    _gvar = gvar;
  }
}

void importGvarParam(int & gvar, const int _gvar, int version)
{
  int GV1 = 1024;

  if (_gvar >= GV1) {
    gvar = 10001 + _gvar - GV1;
  }
  else if (_gvar >= GV1-9) {
    gvar = -10000 + _gvar - GV1;
  }
  else if (_gvar < -GV1) {
    gvar = -10000 + _gvar + GV1 + 1;
  }
  else if (_gvar < -GV1+9) {
    gvar = 10000 + _gvar + GV1 + 1;
  }
  else {
    gvar = _gvar;
  }

  // qDebug() << QString("import") << _gvar << gvar;
}

class CurveReferenceField: public TransformedField {
  public:
    CurveReferenceField(DataField * parent, CurveReference & curve, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this),
      curve(curve),
      _curve_type(0),
      _curve_value(0)
    {
      internalField.Append(new UnsignedField<8>(this, _curve_type));
      internalField.Append(new SignedField<8>(this, _curve_value));
    }

    void beforeExport() override
    {
      if (curve.value != 0) {
        _curve_type = (unsigned int)curve.type;
        _curve_value = smallGvarImport(curve.value);
      }
      else {
        _curve_type = 0;
        _curve_value = 0;
      }
    }

    void afterImport() override
    {
      curve.type = (CurveReference::CurveRefType)_curve_type;
      curve.value = smallGvarExport(_curve_value);
      qCDebug(eepromImport) << QString("imported CurveReference(%1)").arg(curve.toString());
    }

  protected:
    StructField internalField;
    CurveReference & curve;
    unsigned int _curve_type;
    int _curve_value;
};

static int exportHeliInversionWeight(int source) {
  return (source < 0) ? 1 : 0;
}

static int importHeliInversionWeight(int source) {
  return source ? -100: 100;
}

class HeliField: public StructField {
  public:
    HeliField(DataField * parent, SwashRingData & heli, Board::Type board, unsigned int version, unsigned int variant):
      StructField(parent, "Heli")
    {
      if (version >= 218 || IS_STM32(board)) {
        Append(new UnsignedField<8>(this, heli.type));
        Append(new UnsignedField<8>(this, heli.value));
        Append(new SourceField<8>(this, heli.collectiveSource, board, version, variant));
        Append(new SourceField<8>(this, heli.aileronSource, board, version, variant));
        Append(new SourceField<8>(this, heli.elevatorSource, board, version, variant));
        Append(new SignedField<8>(this, heli.collectiveWeight));
        Append(new SignedField<8>(this, heli.aileronWeight));
        Append(new SignedField<8>(this, heli.elevatorWeight));
      }
      else {
        Append(new ConversionField< SignedField<1> >(this, heli.elevatorWeight, exportHeliInversionWeight, importHeliInversionWeight));
        Append(new ConversionField< SignedField<1> >(this, heli.aileronWeight, exportHeliInversionWeight, importHeliInversionWeight));
        Append(new ConversionField< SignedField<1> >(this, heli.collectiveWeight, exportHeliInversionWeight, importHeliInversionWeight));
        Append(new UnsignedField<5>(this, heli.type));
        Append(new SourceField<8>(this, heli.collectiveSource, board, version, variant));
        //, FLAG_NOSWITCHES)); Fix shift in collective
        Append(new UnsignedField<8>(this, heli.value));
      }
    }
};

class FlightModeField: public TransformedField {
  public:
    FlightModeField(DataField * parent, FlightModeData & phase, int index, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "FlightMode"),
      phase(phase),
      index(index),
      board(board),
      version(version),
      rotencCount(ROTENC_COUNT(board, version))
    {
      if (version >= 218) {
        for (int i=0; i<CPN_MAX_STICKS+MAX_AUX_TRIMS(board); i++) {
          internalField.Append(new SignedField<11>(this, phase.trim[i]));
          internalField.Append(new UnsignedField<5>(this, trimMode[i]));
        }
      }
      else if (IS_TARANIS(board)) {
        for (int i=0; i<CPN_MAX_STICKS; i++) {
          internalField.Append(new SignedField<11>(this, phase.trim[i]));
          internalField.Append(new UnsignedField<5>(this, trimMode[i]));
        }
      }
      else {
        for (int i=0; i<CPN_MAX_STICKS; i++) {
          internalField.Append(new SignedField<16>(this, trimBase[i]));
        }
      }
      if (version >= 218) {
        if (version >= 220) {
          if (HAS_LARGE_LCD(board))
            internalField.Append(new CharField<10>(this, phase.name, "Flight mode name"));
          else
            internalField.Append(new CharField<6>(this, phase.name, "Flight mode name"));
        }
        else {
          if (HAS_LARGE_LCD(board))
            internalField.Append(new ZCharField<10>(this, phase.name, "Flight mode name"));
          else
            internalField.Append(new ZCharField<6>(this, phase.name, "Flight mode name"));
        }
        internalField.Append(new SwitchField<9>(this, phase.swtch, board, version));
        internalField.Append(new SpareBitsField<7>(this));
      }
      else {
        internalField.Append(new SwitchField<8>(this, phase.swtch, board, version));
        if (HAS_LARGE_LCD(board))
          internalField.Append(new ZCharField<10>(this, phase.name));
        else
          internalField.Append(new ZCharField<6>(this, phase.name));
      }
      internalField.Append(new UnsignedField<8>(this, phase.fadeIn));
      internalField.Append(new UnsignedField<8>(this, phase.fadeOut));

      if (version < 219) {
        for (int i = 0; i < rotencCount; i++) {
          internalField.Append(new SignedField<16>(this, phase.rotaryEncoders[i]));
        }
      }

      for (int i = 0; i < MAX_GVARS(board, version); i++) {
        internalField.Append(new SignedField<16>(this, phase.gvars[i]));
      }
    }

    void beforeExport() override
    {
      for (int i=0; i<CPN_MAX_STICKS+MAX_AUX_TRIMS(board); i++) {
        if (IS_HORUS_OR_TARANIS(board) || version >= 218) {
          if (phase.trimMode[i] < 0)
            trimMode[i] = TRIM_MODE_NONE;
          else
            trimMode[i] = 2*phase.trimRef[i] + phase.trimMode[i];
        }
        else {
          int trim;
          if (phase.trimMode[i] < 0)
            trim = 0;
          else if (phase.trimRef[i] != index)
            trim = 501 + phase.trimRef[i] - (phase.trimRef[i] > index ? 1 : 0);
          else
            trim = std::max(-500, std::min(500, phase.trim[i]));
          trimBase[i] = trim;
        }
      }
    }

    void afterImport() override
    {
      for (int i=0; i<CPN_MAX_STICKS+MAX_AUX_TRIMS(board); i++) {
        if (IS_HORUS_OR_TARANIS(board) || version >= 218) {
          if (trimMode[i] == TRIM_MODE_NONE) {
            phase.trimMode[i] = -1;
          }
          else {
            phase.trimMode[i] = trimMode[i] % 2;
            phase.trimRef[i] = trimMode[i] / 2;
          }
        }
        else {
          if (phase.swtch == RawSwitch(SWITCH_TYPE_NONE) && index > 0) {
            phase.trimRef[i] = 0;
            phase.trimMode[i] = 0;
            phase.trim[i] = 0;
          }
          else {
            int trim = trimBase[i];
            if (trim > 500) {
              phase.trimRef[i] = trim - 501;
              if (phase.trimRef[i] >= index)
                phase.trimRef[i] += 1;
              phase.trimMode[i] = 0;
              phase.trim[i] = 0;
            }
            else {
              phase.trimRef[i] = index/*own trim*/;
              phase.trimMode[i] = 0;
              phase.trim[i] = trim;
            }
          }
        }
      }
      qCDebug(eepromImport) << QString("imported %1: '%2'").arg(internalField.getName()).arg(phase.name);
    }

  protected:
    StructField internalField;
    FlightModeData & phase;
    int index;
    Board::Type board;
    unsigned int version;
    int rotencCount;
    int trimBase[CPN_MAX_TRIMS];
    int trimExt[CPN_MAX_TRIMS];
    unsigned int trimMode[CPN_MAX_TRIMS];
};

class MixField: public TransformedField {
  public:
    MixField(DataField * parent, MixData & mix, Board::Type board, unsigned int version, ModelData * model):
      TransformedField(parent, internalField),
      internalField(this, "Mix"),
      mix(mix),
      board(board),
      version(version),
      model(model)
    {
      if (version >= 218) {
        internalField.Append(new SignedField<11>(this, _weight));
        internalField.Append(new UnsignedField<5>(this, _destCh));
        internalField.Append(new SourceField<10>(this, mix.srcRaw, board, version, FLAG_NOTELEMETRY));
        internalField.Append(new UnsignedField<1>(this, (unsigned int &)mix.carryTrim));
        internalField.Append(new UnsignedField<2>(this, mix.mixWarn));
        internalField.Append(new UnsignedField<2>(this, (unsigned int &)mix.mltpx));
        internalField.Append(new SpareBitsField<1>(this));
        internalField.Append(new SignedField<14>(this, _offset));
        internalField.Append(new SwitchField<9>(this, mix.swtch, board, version));
        internalField.Append(new UnsignedField<9>(this, mix.flightModes));
        internalField.Append(new CurveReferenceField(this, mix.curve, board, version));
        internalField.Append(new UnsignedField<8>(this, mix.delayUp));
        internalField.Append(new UnsignedField<8>(this, mix.delayDown));
        internalField.Append(new UnsignedField<8>(this, mix.speedUp));
        internalField.Append(new UnsignedField<8>(this, mix.speedDown));
        if (version >= 220)
          internalField.Append(new CharField<6>(this, mix.name));
        else {
          if (IS_FAMILY_HORUS_OR_T16(board))
            internalField.Append(new ZCharField<6>(this, mix.name));
          else if (HAS_LARGE_LCD(board) && version <= 218)
            internalField.Append(new ZCharField<8>(this, mix.name));
          else
            internalField.Append(new ZCharField<6>(this, mix.name));
        }
      }
      else if (IS_TARANIS(board)) {
        internalField.Append(new UnsignedField<8>(this, _destCh));
        internalField.Append(new UnsignedField<9>(this, mix.flightModes));
        internalField.Append(new UnsignedField<2>(this, (unsigned int &)mix.mltpx));
        internalField.Append(new UnsignedField<1>(this, (unsigned int &)mix.carryTrim));
        internalField.Append(new UnsignedField<4>(this, mix.mixWarn));
        internalField.Append(new SignedField<16>(this, _weight));
        internalField.Append(new SourceField<10>(this, mix.srcRaw, board, version, FLAG_NOTELEMETRY));
        internalField.Append(new SignedField<14>(this, _offset));
        internalField.Append(new SwitchField<8>(this, mix.swtch, board, version));
        internalField.Append(new CurveReferenceField(this, mix.curve, board, version));
        internalField.Append(new UnsignedField<8>(this, mix.delayUp));
        internalField.Append(new UnsignedField<8>(this, mix.delayDown));
        internalField.Append(new UnsignedField<8>(this, mix.speedUp));
        internalField.Append(new UnsignedField<8>(this, mix.speedDown));
        internalField.Append(new ZCharField<8>(this, mix.name));
      }
      else {
        internalField.Append(new UnsignedField<5>(this, _destCh));
        internalField.Append(new UnsignedField<3>(this, mix.mixWarn));
        internalField.Append(new UnsignedField<9>(this, mix.flightModes));
        internalField.Append(new BoolField<1>(this, _curveMode));
        internalField.Append(new BoolField<1>(this, mix.noExpo));
        internalField.Append(new SignedField<3>(this, mix.carryTrim));
        internalField.Append(new UnsignedField<2>(this, (unsigned int &)mix.mltpx));
        internalField.Append(new SignedField<16>(this, _weight));
        internalField.Append(new SwitchField<8>(this, mix.swtch, board, version));
        internalField.Append(new SignedField<8>(this, _curveParam));
        internalField.Append(new UnsignedField<8>(this, mix.delayUp));
        internalField.Append(new UnsignedField<8>(this, mix.delayDown));
        internalField.Append(new UnsignedField<8>(this, mix.speedUp));
        internalField.Append(new UnsignedField<8>(this, mix.speedDown));
        internalField.Append(new SourceField<8>(this, mix.srcRaw, board, version, FLAG_NOTELEMETRY));
        internalField.Append(new SignedField<16>(this, _offset));
        internalField.Append(new ZCharField<6>(this, mix.name));
      }
    }

    void beforeExport() override
    {
      if (mix.destCh && mix.srcRaw.type != SOURCE_TYPE_NONE) {
        _destCh = mix.destCh - 1;
        if (mix.curve.type == CurveReference::CURVE_REF_CUSTOM) {
          _curveMode = true;
          if (mix.curve.value > 0) {
            _curveParam = 6 + mix.curve.value;
          }
          else {
            _curveParam = mix.curve.value;
          }
        }
        else if (mix.curve.type == CurveReference::CURVE_REF_FUNC) {
          _curveMode = true;
          _curveParam = mix.curve.value;
        }
        else if (mix.curve.type == CurveReference::CURVE_REF_DIFF) {
          _curveMode = false;
          _curveParam = smallGvarImport(mix.curve.value);
        }
      }
      else {
        mix.clear();
        _destCh = 0;
        _curveMode = false;
        _curveParam = 0;
      }

      exportGvarParam(mix.weight, _weight, version);
      if (version >= 214)
        exportGvarParam(mix.sOffset, _offset, version);
      else
        splitGvarParam(mix.sOffset, _offset, _offsetMode, board, version);
    }

    void afterImport() override
    {
      if (mix.srcRaw.type != SOURCE_TYPE_NONE) {
        mix.destCh = _destCh + 1;
        if (!IS_STM32(board) && version < 218) {
          if (!_curveMode)
            mix.curve = CurveReference(CurveReference::CURVE_REF_DIFF, smallGvarExport(_curveParam));
          else if (_curveParam > 6)
            mix.curve = CurveReference(CurveReference::CURVE_REF_CUSTOM, _curveParam-6);
          else if (_curveParam < 0)
            mix.curve = CurveReference(CurveReference::CURVE_REF_CUSTOM, _curveParam);
          else
            mix.curve = CurveReference(CurveReference::CURVE_REF_FUNC, _curveParam);
        }
      }

      importGvarParam(mix.weight, _weight, version);
      importGvarParam(mix.sOffset, _offset, version);

      qCDebug(eepromImport) << QString("imported %1: ch %2, name '%3'").arg(internalField.getName()).arg(mix.destCh).arg(mix.name);
    }

  protected:
    StructField internalField;
    MixData & mix;
    Board::Type board;
    unsigned int version;
    ModelData * model;
    unsigned int _destCh = 0;
    bool _curveMode = false;
    int _curveParam = 0;
    int _weight = 0;
    int _offset = 0;
    unsigned int _offsetMode = 0;
};

class InputField: public TransformedField {
  public:
    InputField(DataField * parent, ExpoData & expo, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "Input"),
      expo(expo),
      board(board),
      version(version)
    {
      if (version >= 218) {
        internalField.Append(new UnsignedField<2>(this, expo.mode, "Mode"));
        internalField.Append(new UnsignedField<14>(this, expo.scale, "Scale"));
        internalField.Append(new SourceField<10>(this, expo.srcRaw, board, version, 0));
        internalField.Append(new SignedField<6>(this, expo.carryTrim, "CarryTrim"));
        internalField.Append(new UnsignedField<5>(this, expo.chn, "Channel"));
        internalField.Append(new SwitchField<9>(this, expo.swtch, board, version));
        internalField.Append(new UnsignedField<9>(this, expo.flightModes));
        internalField.Append(new SignedField<8>(this, _weight, "Weight"));
        internalField.Append(new SpareBitsField<1>(this));
        if (version >= 220)
          internalField.Append(new CharField<6>(this, expo.name));
        else {
          if (IS_FAMILY_HORUS_OR_T16(board))
            internalField.Append(new ZCharField<6>(this, expo.name));
          else if (HAS_LARGE_LCD(board) && version <= 218)
            internalField.Append(new ZCharField<8>(this, expo.name));
          else
            internalField.Append(new ZCharField<6>(this, expo.name));
        }
        internalField.Append(new SignedField<8>(this, _offset, "Offset"));
        internalField.Append(new CurveReferenceField(this, expo.curve, board, version));
      }
      else if (IS_TARANIS(board)) {
        internalField.Append(new SourceField<10>(this, expo.srcRaw, board, version, 0));
        internalField.Append(new UnsignedField<14>(this, expo.scale, "Scale"));
        internalField.Append(new UnsignedField<8>(this, expo.chn, "Channel"));
        internalField.Append(new SwitchField<8>(this, expo.swtch, board, version));
        internalField.Append(new UnsignedField<16>(this, expo.flightModes));
        internalField.Append(new SignedField<8>(this, _weight, "Weight"));
        internalField.Append(new SignedField<6>(this, expo.carryTrim, "CarryTrim"));
        internalField.Append(new UnsignedField<2>(this, expo.mode, "Mode"));
        internalField.Append(new ZCharField<8>(this, expo.name));
        internalField.Append(new SignedField<8>(this, _offset, "Offset"));
        internalField.Append(new CurveReferenceField(this, expo.curve, board, version));
      }
      else {
        internalField.Append(new UnsignedField<2>(this, expo.mode, "Mode"));
        internalField.Append(new UnsignedField<3>(this, expo.chn, "Channel"));
        internalField.Append(new BoolField<2>(this, _curveMode));
        internalField.Append(new UnsignedField<9>(this, expo.flightModes, "Phases"));
        internalField.Append(new SwitchField<8>(this, expo.swtch, board, version));
        internalField.Append(new SignedField<8>(this, _weight, "Weight"));
        internalField.Append(new ZCharField<6>(this, expo.name));
        internalField.Append(new SignedField<8>(this, _curveParam));
      }
    }

    void beforeExport() override
    {
      _weight = smallGvarImport(expo.weight);

      if (IS_HORUS_OR_TARANIS(board) || version >= 218) {
        _offset = smallGvarImport(expo.offset);
      }

      if (!IS_TARANIS(board) && version < 218) {
        if (expo.curve.type==CurveReference::CURVE_REF_FUNC && expo.curve.value) {
          _curveMode = true;
          _curveParam = expo.curve.value;
        }
        else if (expo.curve.type==CurveReference::CURVE_REF_CUSTOM && expo.curve.value) {
          _curveMode = true;
          _curveParam = expo.curve.value+6;
        }
        else {
          _curveMode = false;
          _curveParam = smallGvarImport(expo.curve.value);
        }
      }
    }

    void afterImport() override
    {
      if (!IS_STM32(board) && expo.mode) {
        expo.srcRaw = RawSource(SOURCE_TYPE_STICK, expo.chn);
      }

      expo.weight = smallGvarExport(_weight);

      if (IS_STM32(board) || version >= 218) {
        expo.offset = smallGvarExport(_offset);
      }

      if (!IS_TARANIS(board) && version < 218) {
        if (!_curveMode)
          expo.curve = CurveReference(CurveReference::CURVE_REF_EXPO, smallGvarExport(_curveParam));
        else if (_curveParam > 6)
          expo.curve = CurveReference(CurveReference::CURVE_REF_CUSTOM, _curveParam-6);
        else
          expo.curve = CurveReference(CurveReference::CURVE_REF_FUNC, _curveParam);
      }
      qCDebug(eepromImport) << QString("imported %1: ch %2 name '%3'").arg(internalField.getName()).arg(expo.chn).arg(expo.name);
    }

  protected:
    StructField internalField;
    ExpoData & expo;
    Board::Type board;
    unsigned int version;
    bool _curveMode = false;
    int  _weight = 0;
    int  _offset = 0;
    int  _curveParam = 0;
};

class LimitField: public StructField {
  public:
    template <int shift, int GV1>
    static int exportLimitValue(int value)
    {
      if (value > 10000) {
        return -GV1 + value - 10001;
      }
      if (value < -10000) {
        return GV1 + value + 10000;
      }
      return value + shift;
    }

    template <int shift, int GV1>
    static int importLimitValue(int value)
    {
      if (value >= GV1) {
        return 10001 + value - GV1;
      }
      if (value >= GV1-9) {
        return -10000 + value - GV1;
      }
      if (value < -GV1) {
        return -10000 + value + GV1 + 1;
      }
      if (value < -GV1+9) {
        return 10000 + value + GV1 + 1;
      }
      return value - shift;
    }

    LimitField(DataField * parent, LimitData & limit, Board::Type board, unsigned int version):
      StructField(parent, "Limit")
    {
      if (version >= 218) {
        Append(new ConversionField< SignedField<11> >(this, limit.min, exportLimitValue<1000, 1024>, importLimitValue<1000, 1024>));
        Append(new ConversionField< SignedField<11> >(this, limit.max, exportLimitValue<-1000, 1024>, importLimitValue<-1000, 1024>));
        Append(new SignedField<10>(this, limit.ppmCenter));
        Append(new ConversionField< SignedField<11> >(this, limit.offset, exportLimitValue<0, 1024>, importLimitValue<0, 1024>));
        Append(new BoolField<1>(this, limit.symetrical));
        Append(new BoolField<1>(this, limit.revert));
        Append(new SpareBitsField<3>(this));
        Append(new SignedField<8>(this, limit.curve.value));
        if (version >= 220) {
          if (HAS_LARGE_LCD(board))
            Append(new CharField<6>(this, limit.name));
          else
            Append(new CharField<4>(this, limit.name));
        }
        else {
          if (HAS_LARGE_LCD(board))
            Append(new ZCharField<6>(this, limit.name));
          else
            Append(new ZCharField<4>(this, limit.name));
        }
      }
      else if (IS_TARANIS(board)) {
        Append(new ConversionField< SignedField<11> >(this, limit.min, exportLimitValue<1000, 1024>, importLimitValue<1000, 1024>));
        Append(new ConversionField< SignedField<11> >(this, limit.max, exportLimitValue<-1000, 1024>, importLimitValue<-1000, 1024>));
        Append(new SignedField<10>(this, limit.ppmCenter));
        Append(new ConversionField< SignedField<11> >(this, limit.offset, exportLimitValue<0, 1024>, importLimitValue<0, 1024>));
        Append(new BoolField<1>(this, limit.symetrical));
        Append(new BoolField<1>(this, limit.revert));
        Append(new SpareBitsField<3>(this));
        Append(new SignedField<8>(this, limit.curve.value));
        Append(new ZCharField<6>(this, limit.name));
      }
      else {
        Append(new ConversionField< SignedField<8> >(this, limit.min, +100, 10));
        Append(new ConversionField< SignedField<8> >(this, limit.max, -100, 10));
        Append(new SignedField<8>(this, limit.ppmCenter));
        Append(new SignedField<14>(this, limit.offset));
        Append(new BoolField<1>(this, limit.symetrical));
        Append(new BoolField<1>(this, limit.revert));
        if (HAS_LARGE_LCD(board)) {
          Append(new ZCharField<6>(this, limit.name));
        }
      }
    }
};

class CurvesField: public TransformedField {
  public:
    CurvesField(DataField * parent, CurveData * curves, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "Curves"),
      curves(curves),
      board(board),
      version(version),
      maxCurves(MAX_CURVES(board, version)),
      maxPoints(512)
    {
      for (int i=0; i<maxCurves; i++) {
        if (version >= 218) {
          internalField.Append(new UnsignedField<1>(this, (unsigned int &)curves[i].type));
          internalField.Append(new BoolField<1>(this, curves[i].smooth));
          internalField.Append(new ConversionField< SignedField<6> >(this, curves[i].count, -5));
          if (version >= 220)
            internalField.Append(new CharField<3>(this, curves[i].name));
          else
            internalField.Append(new ZCharField<3>(this, curves[i].name));
        }
        else if (IS_TARANIS(board)) {
          internalField.Append(new UnsignedField<3>(this, (unsigned int &)curves[i].type));
          internalField.Append(new BoolField<1>(this, curves[i].smooth));
          internalField.Append(new SpareBitsField<4>(this));
          internalField.Append(new ConversionField< SignedField<8> >(this, curves[i].count, -5));
        }
        else {
          internalField.Append(new SignedField<16>(this, _curves[i]));
        }
      }

      for (int i=0; i<maxPoints; i++) {
        internalField.Append(new SignedField<8>(this, _points[i]));
      }
    }

    void beforeExport() override
    {
      memset(_points, 0, sizeof(_points));

      int * cur = &_points[0];
      int offset = 0;

      for (int i=0; i<maxCurves; i++) {
        CurveData *curve = &curves[i];
        if (IS_STM32(board)) {
          offset += (curve->type == CurveData::CURVE_TYPE_CUSTOM ? curve->count * 2 - 2 : curve->count);
          if (offset > maxPoints) {
            setError(DataField::tr("OpenTX only accepts %1 points in all curves").arg(maxPoints));
            break;
          }
        }
        else {
          offset += (curve->type == CurveData::CURVE_TYPE_CUSTOM ? curve->count * 2 - 2 : curve->count) - 5;
          if (offset > maxPoints - 5 * maxCurves) {
            setError(DataField::tr("OpenTx only accepts %1 points in all curves").arg(maxPoints));
            break;
          }
          _curves[i] = offset;
        }
        for (int j=0; j<curve->count; j++) {
          *cur++ = curve->points[j].y;
        }
        if (curve->type == CurveData::CURVE_TYPE_CUSTOM) {
          for (int j=1; j<curve->count-1; j++) {
            *cur++ = curve->points[j].x;
          }
        }
      }
    }

    void afterImport() override
    {
      int * cur = &_points[0];

      for (int i=0; i<maxCurves; i++) {
        CurveData * curve = &curves[i];
        if (!IS_STM32(board) && version < 218) {
          int * next = &_points[5*(i+1) + _curves[i]];
          int size = next - cur;
          if (size % 2 == 0) {
            curve->count = (size / 2) + 1;
            curve->type = CurveData::CURVE_TYPE_CUSTOM;
          }
          else {
            curve->count = size;
            curve->type = CurveData::CURVE_TYPE_STANDARD;
          }
        }

        if (curve->count > 17) {
          qDebug() << "Wrong points count" << curve->count;
          curve->count = 0;
          for (int j=0; j<maxCurves; j++) {
            CurveData *curve = &curves[j];
            curve->clear(5);
          }
          return;
        }

        for (int j=0; j<curve->count; j++) {
          curve->points[j].y = *cur++;
        }

        if (curve->type == CurveData::CURVE_TYPE_CUSTOM) {
          curve->points[0].x = -100;
          for (int j=1; j<curve->count-1; j++)
            curve->points[j].x = *cur++;
          curve->points[curve->count-1].x = +100;
        }
        qCDebug(eepromImport) << QString("imported curve: %3 points").arg(curve->count);
      }
    }

  protected:
    StructField internalField;
    CurveData * curves;
    Board::Type board;
    unsigned int version;
    int maxCurves;
    int maxPoints;
    int _curves[CPN_MAX_CURVES] = {};
    int _points[CPN_MAX_CURVES*CPN_MAX_POINTS*2] = {};
};

class LogicalSwitchesFunctionsTable: public ConversionTable {

  public:
    LogicalSwitchesFunctionsTable(Board::Type board, unsigned int version)
    {
      int val=0;
      addConversion(LS_FN_OFF, val++);
      addConversion(LS_FN_VEQUAL, val++);
      addConversion(LS_FN_VALMOSTEQUAL, val++);
      addConversion(LS_FN_VPOS, val++);
      addConversion(LS_FN_VNEG, val++);
      val++; // later RANGE
      addConversion(LS_FN_APOS, val++);
      addConversion(LS_FN_ANEG, val++);
      addConversion(LS_FN_AND, val++);
      addConversion(LS_FN_OR, val++);
      addConversion(LS_FN_XOR, val++);
      addConversion(LS_FN_EDGE, val++);
      addConversion(LS_FN_EQUAL, val++);
      addConversion(LS_FN_GREATER, val++);
      addConversion(LS_FN_LESS, val++);
      addConversion(LS_FN_DPOS, val++);
      addConversion(LS_FN_DAPOS, val++);
      addConversion(LS_FN_TIMER, val++);
      addConversion(LS_FN_STICKY, val++);
    }
};

class LogicalSwitchField: public TransformedField {
  public:
    LogicalSwitchField(DataField * parent, LogicalSwitchData & csw, Board::Type board, unsigned int version, unsigned int variant, ModelData * model=nullptr):
      TransformedField(parent, internalField),
      internalField(this, "LogicalSwitch"),
      csw(csw),
      board(board),
      version(version),
      variant(variant),
      model(model),
      functionsConversionTable(board, version),
      sourcesConversionTable(SourcesConversionTable::getInstance(board, version, variant, 0)),
      switchesConversionTable(SwitchesConversionTable::getInstance(board, version))
    {
      if (version >= 218) {
        internalField.Append(new ConversionField< UnsignedField<8> >(this, csw.func, &functionsConversionTable, "Function"));
        internalField.Append(new SignedField<10>(this, v1));
        internalField.Append(new SignedField<10>(this, v3));
        internalField.Append(new ConversionField< SignedField<9> >(this, (int &)csw.andsw, switchesConversionTable, "AND switch"));
        internalField.Append(new SpareBitsField<1>(this)); // TODO extra switch mode
        internalField.Append(new SpareBitsField<2>(this));
        internalField.Append(new SignedField<16>(this, v2));
      }
      else {
        internalField.Append(new ConversionField< UnsignedField<6> >(this, csw.func, &functionsConversionTable, "Function"));
        internalField.Append(new SignedField<10>(this, v1));
        internalField.Append(new SignedField<16>(this, v2));
        internalField.Append(new SignedField<16>(this, v3));
      }

      int scale = 0;
      internalField.Append(new ConversionField< UnsignedField<8> >(this, csw.delay, 0, scale));
      internalField.Append(new ConversionField< UnsignedField<8> >(this, csw.duration, 0, scale));
      if (version == 217) {
        internalField.Append(new ConversionField< SignedField<8> >(this, (int &)csw.andsw, switchesConversionTable, "AND switch"));
      }
    }

    ~LogicalSwitchField() override
    = default;

    void beforeExport() override
    {
      if (csw.func == LS_FN_TIMER) {
        v1 = csw.val1;
        v2 = csw.val2;
      }
      else if (csw.func == LS_FN_EDGE) {
        switchesConversionTable->exportValue(csw.val1, v1);
        v2 = csw.val2;
        v3 = csw.val3;
      }
      else if ((csw.func >= LS_FN_AND && csw.func <= LS_FN_XOR) || csw.func == LS_FN_STICKY) {
        switchesConversionTable->exportValue(csw.val1, v1);
        switchesConversionTable->exportValue(csw.val2, v2);
      }
      else if (csw.func >= LS_FN_EQUAL && csw.func <= LS_FN_ELESS) {
        sourcesConversionTable->exportValue(csw.val1, v1);
        sourcesConversionTable->exportValue(csw.val2, v2);
      }
      else if (csw.func != LS_FN_OFF) {
        sourcesConversionTable->exportValue(csw.val1, v1);
        v2 = csw.val2;
      }
    }

    void afterImport() override
    {
      if (csw.func == LS_FN_TIMER) {
        csw.val1 = v1;
        csw.val2 = v2;
      }
      else if (csw.func == LS_FN_EDGE) {
        switchesConversionTable->importValue(v1, csw.val1);
        csw.val2 = v2;
        csw.val3 = v3;
      }
      else if ((csw.func >= LS_FN_AND && csw.func <= LS_FN_XOR) || csw.func == LS_FN_STICKY) {
        switchesConversionTable->importValue(v1, csw.val1);
        switchesConversionTable->importValue(v2, csw.val2);
      }
      else if (csw.func >= LS_FN_EQUAL && csw.func <= LS_FN_ELESS) {
        sourcesConversionTable->importValue((uint32_t)v1, csw.val1);
        sourcesConversionTable->importValue((uint32_t)v2, csw.val2);
      }
      else if (csw.func != LS_FN_OFF) {
        sourcesConversionTable->importValue((uint32_t)v1, csw.val1);
        csw.val2 = v2;
        RawSource val1(csw.val1);
      }
      qCDebug(eepromImport) << QString("imported %1: %2").arg(internalField.getName()).arg(csw.funcToString());
    }

  protected:
    StructField internalField;
    LogicalSwitchData & csw;
    Board::Type board;
    unsigned int version;
    unsigned int variant;
    ModelData * model;
    LogicalSwitchesFunctionsTable functionsConversionTable;
    SourcesConversionTable * sourcesConversionTable;
    SwitchesConversionTable * switchesConversionTable;
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
};

class CustomFunctionsConversionTable: public ConversionTable {

  public:
    CustomFunctionsConversionTable(Board::Type board, unsigned int version)
    {
      int val = 0;

      for (int i=0; i<MAX_CHANNELS(board, version); i++) {
        addConversion(i, val);
      }
      val++;

      addConversion(FuncTrainer, val);
      addConversion(FuncTrainerRUD, val);
      addConversion(FuncTrainerELE, val);
      addConversion(FuncTrainerTHR, val);
      addConversion(FuncTrainerAIL, val);
      addConversion(FuncTrainerChannels, val);
      val++;

      addConversion(FuncInstantTrim, val++);

      addConversion(FuncReset, val++);
      addConversion(FuncSetTimer1, val);
      addConversion(FuncSetTimer2, val);
      addConversion(FuncSetTimer3, val);
      val++;
      for (int i=0; i<MAX_GVARS(board, version); i++)
        addConversion(FuncAdjustGV1+i, val);
      val++;
      addConversion(FuncVolume, val++);
      addConversion(FuncSetFailsafe, val++);
      addConversion(FuncRangeCheckInternalModule, val);
      addConversion(FuncRangeCheckExternalModule, val++);
      addConversion(FuncBindInternalModule, val);
      addConversion(FuncBindExternalModule, val++);
      addConversion(FuncPlaySound, val++);
      addConversion(FuncPlayPrompt, val++);
      addConversion(FuncPlayValue, val++);
      addConversion(FuncReserve, val++);
      if (IS_STM32(board))
        addConversion(FuncPlayScript, val++);
      else
        addConversion(FuncReserve, val++);
      addConversion(FuncReserve, val++);
      addConversion(FuncBackgroundMusic, val++);
      addConversion(FuncBackgroundMusicPause, val++);
      addConversion(FuncVario, val++);
      addConversion(FuncPlayHaptic, val++);
      addConversion(FuncLogs, val++);
      addConversion(FuncBacklight, val++);
      if (IS_STM32(board)) {
        addConversion(FuncScreenshot, val++);
        addConversion(FuncRacingMode, val++);
        addConversion(FuncDisableTouch, val++);
      }
    }
};

template <int N>
class SwitchesWarningField: public TransformedField {
  public:
    SwitchesWarningField(DataField * parent, uint64_t & sw, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, _sw, "SwitchesWarning"),
      sw(sw),
      board(board),
      version(version)
    {
    }

    void beforeExport() override
    {
      _sw = sw;
    }

    void afterImport() override
    {
      sw = _sw;
      qCDebug(eepromImport) << QString("imported %1").arg(internalField.getName());
    }

  protected:
    BaseUnsignedField<uint64_t, N> internalField;
    uint64_t & sw;
    uint64_t _sw = 0;
    Board::Type board;
    unsigned int version;
};

class ArmCustomFunctionField: public TransformedField {
  public:
    ArmCustomFunctionField(DataField * parent, CustomFunctionData & fn, Board::Type board, unsigned int version, unsigned int variant):
      TransformedField(parent, internalField),
      internalField(this, "CustomFunction"),
      fn(fn),
      board(board),
      version(version),
      variant(variant),
      functionsConversionTable(board, version),
      sourcesConversionTable(SourcesConversionTable::getInstance(board, version, variant, 0))
    {
      memset(_param, 0, sizeof(_param));

      if (version >= 218) {
        internalField.Append(new SwitchField<9>(this, fn.swtch, board, version));
        internalField.Append(new ConversionField< UnsignedField<7> >(this, _func, &functionsConversionTable, "Function", DataField::tr("OpenTX on this board doesn't accept this function")));
      }
      else {
        internalField.Append(new SwitchField<8>(this, fn.swtch, board, version));
        internalField.Append(new ConversionField< UnsignedField<8> >(this, _func, &functionsConversionTable, "Function", DataField::tr("OpenTX on this board doesn't accept this function")));
      }

      if (IS_TARANIS(board))
        internalField.Append(new CharField<8>(this, _param, false));
      else
        internalField.Append(new CharField<6>(this, _param, false));

      internalField.Append(new SignedField<8>(this, _active));
    }

    static bool hasRepeatParam(const CustomFunctionData & fn)
    {
      return (fn.func == FuncPlaySound || fn.func == FuncPlayPrompt || fn.func == FuncPlayValue || fn.func == FuncPlayHaptic);
    }

    void beforeExport() override
    {
      if (fn.swtch.type != SWITCH_TYPE_NONE) {
        _func = fn.func;

        if (hasRepeatParam(fn))
          _active = fn.repeatParam;
        else
          _active = (fn.enabled ? 1 : 0);

        if (fn.func >= FuncOverrideCH1 && fn.func <= FuncOverrideCH32) {
          *((uint16_t *)_param) = fn.param;
          *((uint8_t *)(_param+3)) = fn.func - FuncOverrideCH1;
        }
        else if (fn.func >= FuncTrainer && fn.func <= FuncTrainerChannels) {
          *((uint8_t *)(_param+3)) = fn.func - FuncTrainer;
        }
        else if (fn.func >= FuncSetTimer1 && fn.func <= FuncSetTimer3) {
          *((uint16_t *)_param) = fn.param;
          *((uint8_t *)(_param+3)) = fn.func - FuncSetTimer1;
        }
        else if (fn.func >= FuncRangeCheckInternalModule && fn.func <= FuncRangeCheckExternalModule) {
          *((uint16_t *)_param) = fn.func - FuncRangeCheckInternalModule;
        }
        else if (fn.func >= FuncBindInternalModule && fn.func <= FuncBindExternalModule) {
          *((uint16_t *)_param) = fn.func - FuncBindInternalModule;
        }
        else if (fn.func == FuncPlayPrompt || fn.func == FuncBackgroundMusic || fn.func == FuncPlayScript) {
          memcpy(_param, fn.paramarm, sizeof(_param));
        }
        else if (fn.func >= FuncAdjustGV1 && fn.func <= FuncAdjustGVLast) {
          *((uint8_t *)(_param+2)) = fn.adjustMode;
          *((uint8_t *)(_param+3)) = fn.func - FuncAdjustGV1;
          unsigned int value;
          if (fn.adjustMode == 1)
            sourcesConversionTable->exportValue(fn.param, (int &)value);
          else if (fn.adjustMode == 2)
            value = RawSource(fn.param).index;
          else
            value = fn.param;
          *((uint16_t *)_param) = value;
        }
        else if (fn.func == FuncPlayValue || fn.func == FuncVolume || fn.func == FuncBacklight) {
          unsigned int value;
          sourcesConversionTable->exportValue(fn.param, (int &)value);
          *((uint16_t *)_param) = value;
        }
        else if (fn.func == FuncReset) {
          *((uint32_t *)_param) = fn.param;
        }
        else {
          *((uint32_t *)_param) = fn.param;
        }
      }
    }

    void afterImport() override
    {
      fn.func = (AssignFunc)_func;

      if (hasRepeatParam(fn))
        fn.repeatParam = _active;
      else
        fn.enabled = (_active & 0x01);

      unsigned int value=0, mode=0, index=0;
      value = *((uint16_t *)_param);
      mode = *((uint8_t *)(_param+2));
      index = *((uint8_t *)(_param+3));

      if (fn.func >= FuncOverrideCH1 && fn.func <= FuncOverrideCH32) {
        fn.func = AssignFunc(fn.func + index);
        fn.param = (int16_t)(uint16_t)value;
      }
      else if (fn.func >= FuncSetTimer1 && fn.func <= FuncSetTimer3) {
        fn.func = AssignFunc(fn.func + index);
        fn.param = (int)value;
      }
      else if (fn.func >= FuncTrainer && fn.func <= FuncTrainerChannels) {
        fn.func = AssignFunc(fn.func + index);
        fn.param = value;
      }
      else if (fn.func == FuncPlayPrompt || fn.func == FuncBackgroundMusic || fn.func == FuncPlayScript) {
        memcpy(fn.paramarm, _param, sizeof(fn.paramarm));
      }
      else if (fn.func == FuncVolume || fn.func == FuncBacklight) {
        sourcesConversionTable->importValue(value, (int &)fn.param);
      }
      else if (fn.func >= FuncAdjustGV1 && fn.func <= FuncAdjustGVLast) {
        fn.func = AssignFunc(fn.func + index);
        fn.adjustMode = mode;
        if (fn.adjustMode == 1)
          sourcesConversionTable->importValue(value, (int &)fn.param);
        else if (fn.adjustMode == 2)
          fn.param = RawSource(SOURCE_TYPE_GVAR, value).toValue();
        else
          fn.param = (int16_t)value;
      }
      else if (fn.func == FuncPlayValue) {
        sourcesConversionTable->importValue(value, (int &)fn.param);
      }
      else if (fn.func == FuncReset) {
        fn.param = value;
      }
      else {
        fn.param = value;
      }
      qCDebug(eepromImport) << QString("imported %1").arg(internalField.getName());
    }

  protected:
    StructField internalField;
    CustomFunctionData & fn;
    Board::Type board;
    unsigned int version;
    unsigned int variant;
    CustomFunctionsConversionTable functionsConversionTable;
    SourcesConversionTable * sourcesConversionTable;
    unsigned int _func = 0;
    char _param[10] = {};
    int _active = 0;
};

class FrskyScreenField: public DataField {
  public:
    FrskyScreenField(DataField * parent, FrSkyScreenData & screen, Board::Type board, unsigned int version, unsigned int variant):
      DataField(parent, "Frsky Screen"),
      screen(screen),
      board(board),
      version(version),
      none(this),
      bars(this),
      numbers(this),
      script(this)
    {
      for (int i=0; i<4; i++) {
        if (IS_TARANIS(board))
          bars.Append(new SourceField<16>(this, screen.body.bars[i].source, board, version, variant));
        else
          bars.Append(new SourceField<8>(this, screen.body.bars[i].source, board, version, variant));
        bars.Append(new SignedField<16>(this, screen.body.bars[i].barMin));
        bars.Append(new SignedField<16>(this, screen.body.bars[i].barMax));
      }

      int columns = (HAS_LARGE_LCD(board) ? 3 : 2);
      for (int i=0; i<4; i++) {
        for (int j=0; j<columns; j++) {
          if (IS_TARANIS(board))
            numbers.Append(new SourceField<16>(this, screen.body.lines[i].source[j], board, version, variant));
          else
            numbers.Append(new SourceField<8>(this, screen.body.lines[i].source[j], board, version, variant));
        }
      }
      for (unsigned i=numbers.size(); i<bars.size(); i++) {
        numbers.Append(new SpareBitsField<1>(this));
      }

      if (IS_TARANIS(board)) {
        script.Append(new CharField<8>(this, screen.body.script.filename, true, "Script name"));
        script.Append(new SpareBitsField<16*8>(this));
      }

      if (IS_TARANIS(board))
        none.Append(new SpareBitsField<24*8>(this));
      else
        none.Append(new SpareBitsField<20*8>(this));
    }

    void ExportBits(QBitArray & output) override
    {
      if (screen.type == TELEMETRY_SCREEN_SCRIPT)
        script.ExportBits(output);
      else if (screen.type == TELEMETRY_SCREEN_NUMBERS)
        numbers.ExportBits(output);
      else if (screen.type == TELEMETRY_SCREEN_BARS)
        bars.ExportBits(output);
      else
        none.ExportBits(output);
    }

    void ImportBits(const QBitArray & input) override
    {
      qCDebug(eepromImport) << QString("importing %1: type: %2").arg(name).arg(screen.type);

      // NOTA: screen.type should have been imported first!
      if (screen.type == TELEMETRY_SCREEN_SCRIPT)
        script.ImportBits(input);
      else if (screen.type == TELEMETRY_SCREEN_NUMBERS)
        numbers.ImportBits(input);
      else if (screen.type == TELEMETRY_SCREEN_BARS)
        bars.ImportBits(input);
      else
        none.ImportBits(input);
    }

    unsigned int size() override
    {
      // NOTA: screen.type should have been imported first!
      if (screen.type == TELEMETRY_SCREEN_SCRIPT)
        return script.size();
      else if (screen.type == TELEMETRY_SCREEN_NUMBERS)
        return numbers.size();
      else if (screen.type == TELEMETRY_SCREEN_BARS)
        return bars.size();
      else
        return none.size();
    }

  protected:
    FrSkyScreenData & screen;
    Board::Type board;
    unsigned int version;
    StructField none;
    StructField bars;
    StructField numbers;
    StructField script;
};

class RSSIConversionTable: public ConversionTable
{
  public:
    explicit RSSIConversionTable(int index)
    {
      addConversion(0, 2-index);
      addConversion(1, 3-index);
      addConversion(2, index ? 3 : 0);
      addConversion(3, 1-index);
    }

    RSSIConversionTable()
    = default;
};

class TelemetryVarioSourceConversionTable: public ConversionTable
{
  public:
    TelemetryVarioSourceConversionTable(Board::Type board, unsigned int version)
    {
      int val = 0;
      if (!IS_HORUS_OR_TARANIS(board)) {
        addConversion(TELEMETRY_VARIO_SOURCE_ALTI, val++);
        addConversion(TELEMETRY_VARIO_SOURCE_ALTI_PLUS, val++);
      }
      addConversion(TELEMETRY_VARIO_SOURCE_VSPEED, val++);
      addConversion(TELEMETRY_VARIO_SOURCE_A1, val++);
      addConversion(TELEMETRY_VARIO_SOURCE_A2, val++);
      if (IS_HORUS_OR_TARANIS(board)) {
        addConversion(TELEMETRY_VARIO_SOURCE_DTE, val++);
      }
    }
};

class TelemetryVoltsSourceConversionTable: public ConversionTable
{
  public:
    TelemetryVoltsSourceConversionTable(Board::Type board, unsigned int version)
    {
      int val = 0;
      addConversion(TELEMETRY_VOLTS_SOURCE_A1, val++);
      addConversion(TELEMETRY_VOLTS_SOURCE_A2, val++);
      addConversion(TELEMETRY_VOLTS_SOURCE_A3, val++);
      addConversion(TELEMETRY_VOLTS_SOURCE_A4, val++);
      addConversion(TELEMETRY_VOLTS_SOURCE_FAS, val++);
      addConversion(TELEMETRY_VOLTS_SOURCE_CELLS, val++);
    }
};

class ScreenTypesConversionTable: public ConversionTable
{
  public:
    ScreenTypesConversionTable(Board::Type board, unsigned int version)
    {
      int val = 0;
      addConversion(TELEMETRY_SCREEN_NONE, val++);
      addConversion(TELEMETRY_SCREEN_NUMBERS, val++);
      addConversion(TELEMETRY_SCREEN_BARS, val++);
    }
};

class TelemetryCurrentSourceConversionTable: public ConversionTable
{
  public:
    TelemetryCurrentSourceConversionTable(Board::Type board, unsigned int version)
    {
      int val = 0;
      addConversion(TELEMETRY_CURRENT_SOURCE_NONE, val++);
      addConversion(TELEMETRY_CURRENT_SOURCE_A1, val++);
      addConversion(TELEMETRY_CURRENT_SOURCE_A2, val++);
      addConversion(TELEMETRY_CURRENT_SOURCE_A3, val++);
      addConversion(TELEMETRY_CURRENT_SOURCE_A4, val++);
      addConversion(TELEMETRY_CURRENT_SOURCE_FAS, val++);
    }
};

class Frsky218Field: public StructField {
  public:
    Frsky218Field(DataField * parent, FrSkyData & frsky, RSSIAlarmData & rssiAlarms, Board::Type board, unsigned int version, unsigned int variant):
      StructField(parent, "FrSky"),
      telemetryVarioSourceConversionTable(board, version),
      screenTypesConversionTable(board, version),
      telemetryVoltsSourceConversionTable(board, version),
      telemetryCurrentSourceConversionTable(board, version)
    {
      if (version <= 218) {
        rssiConversionTable[0] = RSSIConversionTable(0);
        rssiConversionTable[1] = RSSIConversionTable(1);

        if (!IS_FAMILY_HORUS_OR_T16(board)) {
          Append(new UnsignedField<8>(this, frsky.voltsSource, "Volts Source"));
          Append(new UnsignedField<8>(this, frsky.altitudeSource, "Altitude Source"));

          for (int i = 0; i < 4; i++) {
            Append(new UnsignedField<2>(this, frsky.screens[i].type));
          }
          for (int i = 0; i < 4; i++) {
            Append(new FrskyScreenField(this, frsky.screens[i], board, version, variant));
          }
        }
      }

      Append(new UnsignedField<7>(this, frsky.varioSource, "Vario Source"));
      Append(new BoolField<1>(this, frsky.varioCenterSilent));
      Append(new SignedField<8>(this, frsky.varioCenterMax));
      Append(new SignedField<8>(this, frsky.varioCenterMin));
      Append(new SignedField<8>(this, frsky.varioMin));
      Append(new SignedField<8>(this, frsky.varioMax));
      if (version <= 218) {
        Append(new BoolField<1>(this, rssiAlarms.disabled));
        Append(new SpareBitsField<1>(this));
        Append(new ConversionField<SignedField<6> >(this, rssiAlarms.warning, -45));
        Append(new SpareBitsField<2>(this));
        Append(new ConversionField<SignedField<6> >(this, rssiAlarms.critical, -42));
      }
    }

  protected:
    RSSIConversionTable rssiConversionTable[2];
    TelemetryVarioSourceConversionTable telemetryVarioSourceConversionTable;
    ScreenTypesConversionTable screenTypesConversionTable;
    TelemetryVoltsSourceConversionTable telemetryVoltsSourceConversionTable;
    TelemetryCurrentSourceConversionTable telemetryCurrentSourceConversionTable;
};

class VarioField: public StructField {
  public:
    VarioField(DataField * parent, FrSkyData & frsky, Board::Type board, unsigned int version, unsigned int variant):
      StructField(parent, "Vario")
    {
      Append(new UnsignedField<7>(this, frsky.varioSource, "Vario Source"));
      Append(new BoolField<1>(this, frsky.varioCenterSilent));
      Append(new SignedField<8>(this, frsky.varioCenterMax));
      Append(new SignedField<8>(this, frsky.varioCenterMin));
      Append(new SignedField<8>(this, frsky.varioMin));
      Append(new SignedField<8>(this, frsky.varioMax));
    }
};

/*
 * TODO
 */
#if 0
class CustomScreenField: public StructField {
  public:
    CustomScreenField(CustomScreenData & customScreen):
      StructField("Custom Screen"),
      customScreen(customScreen)
    {
    }

  protected:
    CustomScreenData & customScreen;
};
#endif

class SensorField: public TransformedField {
  public:
    SensorField(DataField * parent, const ModelData & model, SensorData & sensor, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "Sensor"),
      sensor(sensor),
      model(model),
      version(version)
    {
      internalField.Append(new UnsignedField<16>(this, _id, "id/persistentValue"));
      internalField.Append(new UnsignedField<8>(this, _instance, "instance/formula"));
      if (version>= 220)
        internalField.Append(new CharField<4>(this, sensor.label));
      else
        internalField.Append(new ZCharField<4>(this, sensor.label));
      if (version >= 219) {
        internalField.Append(new UnsignedField<8>(this, _subid, "subid"));
        internalField.Append(new UnsignedField<2>(this, sensor.type, "type"));
        internalField.Append(new UnsignedField<6>(this, sensor.unit, "unit"));
        internalField.Append(new UnsignedField<2>(this, sensor.prec, "prec"));
        internalField.Append(new BoolField<1>(this, sensor.autoOffset));
        internalField.Append(new BoolField<1>(this, sensor.filter));
        internalField.Append(new BoolField<1>(this, sensor.logs));
        internalField.Append(new BoolField<1>(this, sensor.persistent));
        internalField.Append(new BoolField<1>(this, sensor.onlyPositive));
        internalField.Append(new SpareBitsField<1>(this));
      }
      else {
        internalField.Append(new UnsignedField<1>(this, sensor.type, "type"));
        internalField.Append(new UnsignedField<5>(this, sensor.unit, "unit"));
        internalField.Append(new UnsignedField<2>(this, sensor.prec, "prec"));
        internalField.Append(new BoolField<1>(this, sensor.autoOffset));
        internalField.Append(new BoolField<1>(this, sensor.filter));
        internalField.Append(new BoolField<1>(this, sensor.logs));
        internalField.Append(new BoolField<1>(this, sensor.persistent));
        internalField.Append(new BoolField<1>(this, sensor.onlyPositive));
        internalField.Append(new UnsignedField<3>(this, _subid, "subid"));
      }
      internalField.Append(new UnsignedField<32>(this, _param, "param"));
    }

    void beforeExport() override
    {
      if (sensor.type == SensorData::TELEM_TYPE_CUSTOM) {
        _id = sensor.id;
        _subid = sensor.subid;
        _instance = sensor.instance == 0 ? 0 : ((sensor.instance - 1) & 0x1F) | (sensor.rxIdx << 5) | (sensor.moduleIdx << 7);
        _ratio = sensor.ratio;
        _offset = sensor.offset;
      }
      else {
        _id = sensor.persistentValue;
        _instance = sensor.formula;
        if (sensor.formula == SensorData::TELEM_FORMULA_CELL)
          _param = (sensor.source) + (sensor.index << 8);
        else if (sensor.formula == SensorData::TELEM_FORMULA_ADD || sensor.formula == SensorData::TELEM_FORMULA_AVERAGE || sensor.formula == SensorData::TELEM_FORMULA_MULTIPLY || sensor.formula == SensorData::TELEM_FORMULA_MIN || sensor.formula == SensorData::TELEM_FORMULA_MAX)
          _param = ((uint8_t)sensor.sources[0]) + ((uint8_t)sensor.sources[1] << 8) + ((uint8_t)sensor.sources[2] << 16) + ((uint8_t)sensor.sources[3] << 24);
        else if (sensor.formula == SensorData::TELEM_FORMULA_DIST)
          _param = (sensor.gps) + (sensor.alt << 8);
        else if (sensor.formula == SensorData::TELEM_FORMULA_CONSUMPTION || sensor.formula == SensorData::TELEM_FORMULA_TOTALIZE)
          _param = (sensor.amps);
      }
    }

    void afterImport() override
    {
      if (sensor.type == SensorData::TELEM_TYPE_CUSTOM) {
        if (strlen(sensor.label) == 0) {
          sensor.clear();
        }
        else {
          sensor.id = _id;
          sensor.subid = _subid;
          sensor.instance = (_instance & 0x1F) + (version <= 218 ? 0 : 1); // 5 bits instance
          sensor.rxIdx = (_instance >> 5) & 0x03;    // 2 bits Rx idx
          sensor.moduleIdx = (_instance >> 7) & 0x1; // 1 bit module idx
          sensor.ratio = _ratio;
          sensor.offset = _offset;
        }
      }
      else {
        sensor.persistentValue = _id;
        sensor.formula = _instance;
        if (sensor.formula == SensorData::TELEM_FORMULA_CELL)
          (sensor.source = _sources[0], sensor.index = _sources[1]);
        else if (sensor.formula == SensorData::TELEM_FORMULA_ADD || sensor.formula == SensorData::TELEM_FORMULA_AVERAGE || sensor.formula == SensorData::TELEM_FORMULA_MULTIPLY || sensor.formula == SensorData::TELEM_FORMULA_MIN || sensor.formula == SensorData::TELEM_FORMULA_MAX)
          for (int i=0; i<4; ++i)
            sensor.sources[i] = _sources[i];
        else if (sensor.formula == SensorData::TELEM_FORMULA_DIST)
          (sensor.gps = _sources[0], sensor.alt = _sources[1]);
        else if (sensor.formula == SensorData::TELEM_FORMULA_CONSUMPTION || sensor.formula == SensorData::TELEM_FORMULA_TOTALIZE)
          sensor.amps = _sources[0];
      }

      if (version < 218) {
        if (sensor.unit > SensorData::UNIT_WATTS)
          sensor.unit++;
        if (sensor.unit > SensorData::UNIT_DEGREE)
          sensor.unit++;
      }

      if (version < 219) {
        if (sensor.unit > SensorData::UNIT_FLOZ) {
          sensor.unit += 11;
        }
      }

      qCDebug(eepromImport) << QString("imported %1").arg(internalField.getName());
    }

  protected:
    StructField internalField;
    SensorData & sensor;
    const ModelData& model;
    unsigned int version;
    unsigned int _id = 0;
    unsigned int _subid = 0;
    unsigned int _instance = 0;
    union {
      unsigned int _param = 0;
      uint8_t _sources[4];
      struct {
        uint16_t _ratio;
        int16_t _offset;
      };
    };
};

int exportPpmDelay(int delay) { return (delay - 300) / 50; }
int importPpmDelay(int delay) { return 300 + 50 * delay; }

class ModuleUnionField: public UnionField<unsigned int> {

  class PPMField: public UnionField::UnionMember, public StructField {
    public:
      PPMField(DataField * parent, ModuleData::PPM& ppm):
        StructField(parent, "PPM")
      {
        Append(new ConversionField< SignedField<6> >(parent, ppm.delay, exportPpmDelay, importPpmDelay));
        Append(new BoolField<1>(parent, ppm.pulsePol));
        Append(new BoolField<1>(parent, ppm.outputType));
        Append(new SignedField<8>(parent, ppm.frameLength));
      }

      bool select(const unsigned int & attr) const override
      {
        return true; // take what's left
      }

      DataField * getField() override
      {
        return this;
      }
  };

  class MultiField: public UnionField::TransformedMember {
    public:
      MultiField(DataField * parent, ModuleData& module):
        UnionField::TransformedMember(parent, internalField),
        internalField(this, "Multi"),
        module(module),
        rfProtExtra(0)
      {
        ModuleData::Multi & multi = module.multi;
        internalField.Append(new UnsignedField<3>(this, rfProtExtra));
        internalField.Append(new BoolField<1>(this, multi.disableTelemetry));
        internalField.Append(new BoolField<1>(this, multi.disableMapping));
        internalField.Append(new SpareBitsField<1>(this));
        internalField.Append(new BoolField<1>(this, multi.autoBindMode));
        internalField.Append(new BoolField<1>(this, multi.lowPowerMode));
        internalField.Append(new SignedField<8>(this, multi.optionValue));
      }

      bool select(const unsigned int & attr) const override
      {
        return attr == PULSES_MULTIMODULE;
      }

      void beforeExport() override
      {
        rfProtExtra = (module.multi.rfProtocol & 0x70) >> 4;
      }

      void afterImport() override
      {
        module.multi.rfProtocol = (rfProtExtra << 4) + (module.rfProtocol & 0xf);
      }

    private:
      StructField  internalField;
      ModuleData & module;
      unsigned int rfProtExtra;
  };

  class PxxField: public UnionField::TransformedMember {
    public:
      PxxField(DataField * parent, ModuleData& module, unsigned int version):
        UnionField::TransformedMember(parent, internalField),
        internalField(this, "Pxx"),
        module(module),
        version(version)
      {
        ModuleData::PXX& pxx = module.pxx;
        internalField.Append(new UnsignedField<2>(this, pxx.power));
        internalField.Append(new SpareBitsField<2>(this));
        internalField.Append(new BoolField<1>(this, pxx.receiverTelemetryOff));
        internalField.Append(new BoolField<1>(this, pxx.receiverHigherChannels));
        internalField.Append(new SignedField<2>(this, pxx.antennaMode));
        internalField.Append(new SpareBitsField<8>(this));
      }

      bool select(const unsigned int& attr) const override {
        return (attr >= PULSES_PXX_XJT_X16 && attr <= PULSES_PXX_XJT_LR12) ||
          attr==PULSES_PXX_DJT ||
          attr==PULSES_PXX_R9M ||
          attr==PULSES_PXX_R9M_LITE;
      }

      void beforeExport() override
      {
      }

      void afterImport() override
      {
        if (module.protocol == PULSES_PXX_XJT_X16) {
          if (version <= 218)
            module.protocol += module.rfProtocol;
          else
            module.protocol += module.subType;
        }
      }

    private:
      StructField internalField;
      ModuleData& module;
      unsigned int version;
  };

  class Afhds3Field: public UnionField::TransformedMember {
    public:
      Afhds3Field(DataField * parent, ModuleData& module):
        UnionField::TransformedMember(parent, internalField),
        internalField(this, "AFHDS3")
      {
        ModuleData::Afhds3& afhds3 = module.afhds3;
        internalField.Append(new UnsignedField<3>(this, minBindPower));
        internalField.Append(new UnsignedField<3>(this, afhds3.rfPower));
        internalField.Append(new UnsignedField<1>(this, emissionFCC));
        internalField.Append(new BoolField<1>(this, operationModeUnicast));
        internalField.Append(new BoolField<1>(this, operationModeUnicast));
        internalField.Append(new UnsignedField<16>(this, defaultFailSafeTimout));
        internalField.Append(new UnsignedField<16>(this, afhds3.rxFreq));
      }

      bool select(const unsigned int& attr) const override {
        return attr == PULSES_AFHDS3;
      }

      void beforeExport() override {}

      void afterImport() override {}

    private:
      StructField internalField;

      unsigned int minBindPower = 0;
      unsigned int emissionFCC = 0;
      unsigned int defaultFailSafeTimout = 1000;
      bool operationModeUnicast = true;
  };


  class AccessField: public UnionField::TransformedMember {
    public:
      AccessField(DataField * parent, ModuleData& module):
        UnionField::TransformedMember(parent, internalField),
        internalField(this, "Access"),
        module(module)
      {
        internalField.Append(new UnsignedField<3>(this, module.access.receivers));
        internalField.Append(new SpareBitsField<4>(this));
        internalField.Append(new UnsignedField<1>(this, module.access.racingMode));

        for (int i = 0; i < PXX2_MAX_RECEIVERS_PER_MODULE; i++)
          internalField.Append(new CharField<8>(this, receiverName[i]));

        memset(receiverName, 0, sizeof(receiverName));
      }

      bool select(const unsigned int & attr) const override
      {
        return attr >= PULSES_ACCESS_ISRM && attr <= PULSES_ACCESS_R9M_LITE_PRO;
      }

      void beforeExport() override
      {
        for (int i=0; i<PXX2_MAX_RECEIVERS_PER_MODULE; i++) {
          for (int pos=0; pos<PXX2_LEN_RX_NAME+1; pos++) {
            if (pos == PXX2_LEN_RX_NAME || module.access.receiverName[i][pos] == '\0') {
              memset(module.access.receiverName[i]+pos,'\0',PXX2_LEN_RX_NAME-pos);
              break;
            }
            receiverName[i][pos] = module.access.receiverName[i][pos];
          }
        }
      }

      void afterImport() override
      {
        if (module.protocol == PULSES_ACCESS_ISRM) {
          module.protocol += module.subType;
        }
        for (int i=0; i<PXX2_MAX_RECEIVERS_PER_MODULE; i++) {
          for (int pos=0; pos<PXX2_LEN_RX_NAME+1; pos++) {
            if (pos == PXX2_LEN_RX_NAME || receiverName[i][pos] == ' ' || receiverName[i][pos] == '\0') {
              module.access.receiverName[i][pos] = '\0';
              break;
            }
            module.access.receiverName[i][pos] = receiverName[i][pos];
          }
        }
      }

    private:
      StructField internalField;
      ModuleData & module;
      char receiverName[PXX2_MAX_RECEIVERS_PER_MODULE][PXX2_LEN_RX_NAME+1] = {};
  };

  class GhostField: public UnionField::TransformedMember {
    public:
      GhostField(DataField * parent, ModuleData& module):
        UnionField::TransformedMember(parent, internalField),
        internalField(this, "GHOST")
      {
        internalField.Append(new BoolField<1>(this, module.ghost.raw12bits));
        internalField.Append(new SpareBitsField<7>(this));
      }

      bool select(const unsigned int & attr) const override
      {
        return attr == PULSES_GHOST;
      }

      void beforeExport() override {}

      void afterImport() override {}

    private:
      StructField  internalField;
  };

  public:
    ModuleUnionField(DataField * parent, ModuleData & module, Board::Type board, unsigned int version):
      UnionField<unsigned int>(parent, module.protocol)
    {
      if (version >= 219) {
        Append(new AccessField(parent, module));
        Append(new Afhds3Field(parent, module));
        Append(new GhostField(parent, module));
      }
      Append(new PxxField(parent, module, version));
      Append(new MultiField(parent, module));
      Append(new PPMField(parent, module.ppm));
    }
};


class ModuleField: public TransformedField {
  public:
    ModuleField(DataField * parent, ModuleData & module, ModelData & model, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "Module"),
      module(module),
      protocolsConversionTable(board, version)
    {
      internalField.Append(new ConversionField<UnsignedField<4> >(this, module.protocol, &protocolsConversionTable, "Protocol", DataField::tr("OpenTX doesn't accept this radio protocol")));
      internalField.Append(new SignedField<4>(this, module.rfProtocol));
      internalField.Append(new UnsignedField<8>(this, module.channelsStart));
      internalField.Append(new ConversionField<SignedField<8> >(this, module.channelsCount, -8));
      internalField.Append(new UnsignedField<4>(this, module.failsafeMode));
      internalField.Append(new UnsignedField<3>(this, module.subType));
      internalField.Append(new BoolField<1>(this, module.invertedSerial));
      if (version <= 218) {
        for (int i = 0; i < 32; i++) {
          internalField.Append(new SignedField<16>(this, model.limitData[i].failsafe));
        }
      }

      internalField.Append(new ModuleUnionField(parent, module, board, version));
    }

    void beforeExport() override
    {
      if (module.protocol >= PULSES_LP45 && module.protocol <= PULSES_DSMX) {
        module.rfProtocol = module.protocol - PULSES_LP45;
      }
      else if (module.protocol == PULSES_ACCST_ISRM_D16 || module.protocol == PULSES_ACCESS_ISRM) {
        module.subType = module.protocol - PULSES_ACCESS_ISRM;
      }
      else if (module.protocol >= PULSES_PXX_XJT_X16 && module.protocol <= PULSES_PXX_XJT_LR12) {
        module.subType = module.protocol - PULSES_PXX_XJT_X16;
      }
      else if (module.protocol == PULSES_MULTIMODULE) {
        module.rfProtocol = module.multi.rfProtocol & 0x0F;
      }
    }

    void afterImport() override
    {
      if (module.protocol == PULSES_LP45) {
        module.protocol += module.rfProtocol;
      }
    }

  private:
    StructField              internalField;
    ModuleData&              module;
    ProtocolsConversionTable protocolsConversionTable;
};

class TimerField: public TransformedField {
  public:
    TimerField(DataField * parent, TimerData & timer, Board::Type board, unsigned int version):
      TransformedField(parent, internalField),
      internalField(this, "Timer"),
      timer(timer),
      board(board),
      version(version),
      switchesConversionTable(SwitchesConversionTable::getInstance(board, version))
    {
      if (version >= 220) {
        internalField.Append(new UnsignedField<22>(this, timer.val));
        internalField.Append(new SwitchField<10>(this, _swtch, board, version));
        internalField.Append(new SignedField<22>(this, timer.pvalue));
        internalField.Append(new UnsignedField<3>(this, _mode));
        internalField.Append(new UnsignedField<2>(this, timer.countdownBeep));
        internalField.Append(new BoolField<1>(this, timer.minuteBeep));
        internalField.Append(new UnsignedField<2>(this, timer.persistent));
        internalField.Append(new SignedField<2>(this, timer.countdownStart));
        if (HAS_LARGE_LCD(board))
          internalField.Append(new CharField<8>(this, timer.name, "Timer name"));
        else
          internalField.Append(new CharField<3>(this, timer.name, "Timer name"));
      }
      else if (version >= 218) {
        internalField.Append(new SignedField<9>(this, _mode_pre220));
        internalField.Append(new UnsignedField<23>(this, timer.val));
        internalField.Append(new SignedField<24>(this, timer.pvalue));
        internalField.Append(new UnsignedField<2>(this, timer.countdownBeep));
        internalField.Append(new BoolField<1>(this, timer.minuteBeep));
        internalField.Append(new UnsignedField<2>(this, timer.persistent));
        internalField.Append(new SignedField<2>(this, timer.countdownStart));
        internalField.Append(new SpareBitsField<1>(this));  // direction
        if (HAS_LARGE_LCD(board))
          internalField.Append(new ZCharField<8>(this, timer.name, "Timer name"));
        else
          internalField.Append(new ZCharField<3>(this, timer.name, "Timer name"));
      }
      else {
        internalField.Append(new SignedField<8>(this, _mode_pre220));
        internalField.Append(new UnsignedField<24>(this, timer.val));
        internalField.Append(new SignedField<24>(this, timer.pvalue));
        internalField.Append(new UnsignedField<2>(this, timer.countdownBeep));
        internalField.Append(new BoolField<1>(this, timer.minuteBeep));
        internalField.Append(new UnsignedField<2>(this, timer.persistent));
        internalField.Append(new SpareBitsField<3>(this));
        if (IS_TARANIS(board))
          internalField.Append(new ZCharField<8>(this, timer.name, "Timer name"));
        else
          internalField.Append(new ZCharField<3>(this, timer.name, "Timer name"));
      }
    }

    void beforeExport() override
    {
      _swtch = timer.swtch;
      _mode = timer.mode;
    }

    void afterImport() override
    {
      if (version >= 220) {
        timer.swtch = _swtch;
        timer.mode = _mode;
      }
      else {
        if (_mode_pre220 >= 0 and _mode_pre220 <= 4) {
          timer.swtch = _swtch;
          timer.mode = _mode_pre220;
          if (timer.mode >= TimerData::TIMERMODE_START)
            timer.mode += 1;
        }
        else {
          timer.mode = TimerData::TIMERMODE_ON;
          if (_mode_pre220 > 0)
            _mode_pre220 -= 4;
          switchesConversionTable->importValue(_mode_pre220, _sw1);
          timer.swtch = RawSwitch(_sw1);
        }

        timer.countdownStart = (timer.countdownStart * -1) - 1;
      }
    }

  protected:
    StructField internalField;
    TimerData & timer;
    Board::Type board;
    unsigned int version;
    SwitchesConversionTable * switchesConversionTable;
    int _mode_pre220 = 0;
    unsigned int _mode = 0;
    RawSwitch _swtch = RawSwitch(SWITCH_TYPE_NONE, 0);
    int _sw1;
};

class ZoneOptionValueUnionField: public UnionField<ZoneOptionValueEnum> {

  class ZOVUnsignedField: public UnionField::UnionMember, public StructField {
    public:
      ZOVUnsignedField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
        StructField(this, "ZOV unsigned")
      {
        Append(new UnsignedField<32>(this, option.value.unsignedValue));
      }

      bool select(const ZoneOptionValueEnum & attr) const override {
        return attr == ZOV_Unsigned;
      }

      DataField * getField() override
      {
        return this;
      }
};

  class ZOVSignedField: public UnionField::UnionMember, public StructField {
    public:
      ZOVSignedField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
        StructField(this, "ZOV signed")
      {
        Append(new SignedField<32>(this, option.value.signedValue));
      }

      bool select(const ZoneOptionValueEnum & attr) const override {
        return attr == ZOV_Signed;
      }

      DataField * getField() override
      {
        return this;
      }
   };

  class ZOVBoolField: public UnionField::UnionMember, public StructField {
    public:
      ZOVBoolField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
        StructField(this, "ZOV bool")
      {
        Append(new UnsignedField<32>(this, option.value.boolValue));
      }

      bool select(const ZoneOptionValueEnum & attr) const override {
        return attr == ZOV_Bool;
      }

      DataField * getField() override
      {
        return this;
      }
   };

  class ZOVStringField: public UnionField::UnionMember, public StructField {
    public:
      ZOVStringField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
        StructField(this, "ZOV string")
      {
        Append(new CharField<LEN_ZONE_OPTION_STRING>(this, option.value.stringValue));
      }

      bool select(const ZoneOptionValueEnum & attr) const override {
        return attr == ZOV_String;
      }

      DataField * getField() override
      {
        return this;
      }
   };

  public:
    ZoneOptionValueUnionField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
      UnionField<ZoneOptionValueEnum>(parent, option.type, "Zone Option Value Union")
    {
      Append(new ZOVUnsignedField(parent, option, board, version));
      Append(new ZOVSignedField(parent, option, board, version));
      Append(new ZOVBoolField(parent, option, board, version));
      Append(new ZOVStringField(parent, option, board, version));
    }
};

class ZoneOptionValueTypedField: public StructField {
  public:
    ZoneOptionValueTypedField(DataField * parent, ZoneOptionValueTyped & option, Board::Type board, unsigned int version):
      StructField(this, "Zone Option Value Typed")
    {
      Append(new SignedField<32>(this, (int &)option.type, "Zone Option Value Enum"));
      Append(new ZoneOptionValueUnionField(this, option, board, version));
    }
};

class WidgetPersistentDataField: public StructField {
  public:
    WidgetPersistentDataField(DataField * parent, WidgetPersistentData & persistentData, Board::Type board, unsigned int version):
      StructField(this, "Widget Persistent")
    {
      for (int i = 0; i < MAX_WIDGET_OPTIONS; i++) {
        Append(new ZoneOptionValueTypedField(this, persistentData.options[i], board, version));
      }
    }
};

class ZonePersistentDataField: public StructField {
  public:
    ZonePersistentDataField(DataField * parent, ZonePersistentData & persistentData, Board::Type board, unsigned int version):
      StructField(this, "Zone Persistent")
    {
      Append(new CharField<WIDGET_NAME_LEN>(this, persistentData.widgetName, "Widget name"));
      Append(new SpareBitsField<16>(this));   //  pad to word boundary
      Append(new WidgetPersistentDataField(this, persistentData.widgetData, board, version));
    }
};

template <class container>
class WidgetsContainerPersistentField: public StructField {
  public:
    WidgetsContainerPersistentField(DataField * parent, container & persistentData, int zonecnt, int optioncnt, Board::Type board, unsigned int version):
      StructField(this, "Widgets Container Persistent")
    {
      for (int i = 0; i < zonecnt; i++) {
        Append(new ZonePersistentDataField(this, persistentData.zones[i], board, version));
      }

      for (int i = 0; i < optioncnt; i++) {
        Append(new ZoneOptionValueTypedField(this, persistentData.options[i], board, version));
      }
    }
};

class CustomScreenField: public StructField {
  public:
    CustomScreenField(DataField * parent, RadioLayout::CustomScreenData & customScreen, Board::Type board, unsigned int version):
      StructField(this, "Custom Screen")
    {
      Append(new CharField<10>(this, customScreen.layoutId, "Layout id"));
      //Append(new SpareBitsField<16>(this));   //  pad to word boundary not required in this case
      Append(new WidgetsContainerPersistentField<LayoutPersistentData>(this, customScreen.layoutPersistentData, MAX_LAYOUT_ZONES, MAX_LAYOUT_OPTIONS, board, version));
      //dump();
    }
};

class TopBarField: public StructField {
  public:
    TopBarField(DataField * parent, TopBarPersistentData & topBar, Board::Type board, unsigned int version):
      StructField(parent, "Top Bar")
    {
      Append(new WidgetsContainerPersistentField<TopBarPersistentData>(this, topBar, IS_FLYSKY_NV14(board) ? 2 : 4, MAX_TOPBAR_OPTIONS, board, version));
      //dump();
    }
};

OpenTxModelData::OpenTxModelData(ModelData & modelData, Board::Type board, unsigned int version, unsigned int variant):
  TransformedField(nullptr, internalField),
  internalField(this, "ModelData"),
  modelData(modelData),
  board(board),
  version(version),
  variant(variant)
{
  sprintf(name, "Model %s", modelData.name);

  qDebug() << QString("OpenTxModelData::OpenTxModelData(name: %1, board: %2, ver: %3, var: %4)").arg(name).arg(board).arg(version).arg(variant);

  if (version >= 220) {
    if (IS_FAMILY_HORUS_OR_T16(board))
      internalField.Append(new CharField<15>(this, modelData.name, "Model name"));
    else if (HAS_LARGE_LCD(board))
      internalField.Append(new CharField<12>(this, modelData.name, "Model name"));
    else
      internalField.Append(new CharField<10>(this, modelData.name, "Model name"));
  }
  else {
    if (IS_FAMILY_HORUS_OR_T16(board))
      internalField.Append(new ZCharField<15>(this, modelData.name, "Model name"));
    else if (HAS_LARGE_LCD(board))
      internalField.Append(new ZCharField<12>(this, modelData.name, "Model name"));
    else
      internalField.Append(new ZCharField<10>(this, modelData.name, "Model name"));
  }

  internalField.Append(new UnsignedField<8>(this, modelData.moduleData[0].modelId));
  internalField.Append(new UnsignedField<8>(this, modelData.moduleData[1].modelId));

  if (HAS_LARGE_LCD(board)) {
    if (version >= 219 && IS_FAMILY_HORUS_OR_T16(board))
      internalField.Append(new CharField<14>(this, modelData.bitmap, true, "Model bitmap"));
    else
      internalField.Append(new CharField<10>(this, modelData.bitmap, true, "Model bitmap"));
  }

  for (int i = 0; i < MAX_TIMERS(board, version); i++) {
    internalField.Append(new TimerField(this, modelData.timers[i], board, version));
  }

  internalField.Append(new UnsignedField<3>(this, modelData.telemetryProtocol));
  internalField.Append(new BoolField<1>(this, modelData.thrTrim));
  internalField.Append(new BoolField<1>(this, modelData.noGlobalFunctions));
  internalField.Append(new UnsignedField<2>(this, modelData.trimsDisplay));
  internalField.Append(new BoolField<1>(this, modelData.frsky.ignoreSensorIds));
  internalField.Append(new SignedField<3>(this, modelData.trimInc));
  internalField.Append(new BoolField<1>(this, modelData.disableThrottleWarning));
  internalField.Append(new BoolField<1>(this, modelData.displayChecklist));
  internalField.Append(new BoolField<1>(this, modelData.extendedLimits));
  internalField.Append(new BoolField<1>(this, modelData.extendedTrims));
  internalField.Append(new BoolField<1>(this, modelData.throttleReversed));
  internalField.Append(new UnsignedField<16>(this, modelData.beepANACenter));

  for (int i=0; i<MAX_MIXERS(board, version); i++)
    internalField.Append(new MixField(this, modelData.mixData[i], board, version, &modelData));
  for (int i=0; i<MAX_CHANNELS(board, version); i++)
    internalField.Append(new LimitField(this, modelData.limitData[i], board, version));
  for (int i=0; i<MAX_EXPOS(board, version); i++)
    internalField.Append(new InputField(this, modelData.expoData[i], board, version));

  internalField.Append(new CurvesField(this, modelData.curves, board, version));

  for (int i=0; i<MAX_LOGICAL_SWITCHES(board, version); i++)
    internalField.Append(new LogicalSwitchField(this, modelData.logicalSw[i], board, version, variant, &modelData));
  for (int i=0; i<MAX_CUSTOM_FUNCTIONS(board, version); i++)
    internalField.Append(new ArmCustomFunctionField(this, modelData.customFn[i], board, version, variant));

  internalField.Append(new HeliField(this, modelData.swashRingData, board, version, variant));  // SwashRingData
  for (int i=0; i<MAX_FLIGHT_MODES(board, version); i++)
    internalField.Append(new FlightModeField(this, modelData.flightModeData[i], i, board, version));

  internalField.Append(new UnsignedField<8>(this, modelData.thrTraceSrc, "Throttle Source"));

  if (IS_FAMILY_HORUS_OR_T16(board))
    internalField.Append(new SwitchesWarningField<32>(this, modelData.switchWarningStates, board, version));
  else if (IS_TARANIS_X9E(board))
    internalField.Append(new SwitchesWarningField<64>(this, modelData.switchWarningStates, board, version));
  else if (version >= 219 && (IS_TARANIS_X9D(board) || IS_JUMPER_TPRO(board)))
    internalField.Append(new SwitchesWarningField<32>(this, modelData.switchWarningStates, board, version));
  else if (IS_TARANIS(board))
    internalField.Append(new SwitchesWarningField<16>(this, modelData.switchWarningStates, board, version));
  else
    internalField.Append(new SwitchesWarningField<8>(this, modelData.switchWarningStates, board, version));

  if (IS_TARANIS_X9E(board))
    internalField.Append(new UnsignedField<32>(this, modelData.switchWarningEnable));
  else if (version >= 219 && (IS_TARANIS_X9D(board) || IS_JUMPER_TPRO(board)))
    internalField.Append(new UnsignedField<16>(this, modelData.switchWarningEnable));
  else if (!IS_FAMILY_HORUS_OR_T16(board))
    internalField.Append(new UnsignedField<8>(this, modelData.switchWarningEnable));

  for (int i = 0; i < MAX_GVARS(board, version); i++) {
    if (version >= 218) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, modelData.gvarData[i].name, "GVar name"));
      else
        internalField.Append(new ZCharField<3>(this, modelData.gvarData[i].name, "GVar name"));
      internalField.Append(new UnsignedField<12>(this, (unsigned &)modelData.gvarData[i].min));
      internalField.Append(new UnsignedField<12>(this, (unsigned &)modelData.gvarData[i].max));
      internalField.Append(new BoolField<1>(this, modelData.gvarData[i].popup));
      internalField.Append(new UnsignedField<1>(this, modelData.gvarData[i].prec));
      internalField.Append(new UnsignedField<2>(this, modelData.gvarData[i].unit));
      internalField.Append(new SpareBitsField<4>(this));
    }
    else {
      internalField.Append(new ZCharField<3>(this, modelData.gvarData[i].name, "GVar name"));
      internalField.Append(new SpareBitsField<3*8>(this));
      internalField.Append(new BoolField<1>(this, modelData.gvarData[i].popup));
      internalField.Append(new SpareBitsField<7>(this));
    }
  }

  if (version <= 218) {
    internalField.Append(new Frsky218Field(this, modelData.frsky, modelData.rssiAlarms, board, version, variant));
  }
  else {
    internalField.Append(new VarioField(this, modelData.frsky, board, version, variant));
    internalField.Append(new UnsignedField<8>(this, modelData.rssiSource));

    if (IS_TARANIS_X9(board)) {
      internalField.Append(new UnsignedField<8>(this, modelData.frsky.voltsSource));
      internalField.Append(new UnsignedField<8>(this, modelData.frsky.altitudeSource));
    }

    internalField.Append(new BoolField<1>(this, modelData.rssiAlarms.disabled));
    internalField.Append(new SpareBitsField<1>(this));
    internalField.Append(new ConversionField<SignedField<6> >(this, modelData.rssiAlarms.warning, -45));
    internalField.Append(new SpareBitsField<2>(this));
    internalField.Append(new ConversionField<SignedField<6> >(this, modelData.rssiAlarms.critical, -42));
  }

  if (IS_STM32(board) && version <= 218) {
    internalField.Append(new SpareBitsField<3>(this));
    internalField.Append(new UnsignedField<3>(this, modelData.trainerMode));
    internalField.Append(new UnsignedField<2>(this, modelData.potsWarningMode));
  }
  else {
    internalField.Append(new SpareBitsField<3>(this));
    internalField.Append(new UnsignedField<3>(this, modelData.thrTrimSwitch));
    internalField.Append(new UnsignedField<2>(this, modelData.potsWarningMode));
  }

  int modulesCount = (version <= 218 ? 3 : 2);
  for (int module = 0; module < modulesCount; module++) {
    internalField.Append(new ModuleField(this, modelData.moduleData[module], modelData, board, version));
  }

  if (version >= 219) {
    for (int i = 0; i < 32; i++) {
      internalField.Append(new SignedField<16>(this, modelData.limitData[i].failsafe));
    }
  }

  if (version >= 219) {
    // Trainer PPM settings
    internalField.Append(new UnsignedField<3>(this,  modelData.trainerMode));
    internalField.Append(new SpareBitsField<5>(this));
    internalField.Append(new UnsignedField<8>(this, modelData.moduleData[2].channelsStart));
    internalField.Append(new ConversionField<SignedField<8> >(this, modelData.moduleData[2].channelsCount, -8));
    internalField.Append(new SignedField<8>(this, modelData.moduleData[2].ppm.frameLength));
    internalField.Append(new ConversionField<SignedField<6> >(this, modelData.moduleData[2].ppm.delay, exportPpmDelay, importPpmDelay));
    internalField.Append(new BoolField<1>(this, modelData.moduleData[2].ppm.pulsePol));
    internalField.Append(new SpareBitsField<1>(this));
  }

  if (IS_TARANIS(board) && version < 218) {
    for (int i = 0; i < MAX_CURVES(board, version); i++) {
      internalField.Append(new ZCharField<6>(this, modelData.curves[i].name, "Curve name"));
    }
  }

  if (IS_STM32(board)) {
    if (version >= 218) {
      for (int i = 0; i < MAX_SCRIPTS(board); i++) {
        ScriptData & script = modelData.scriptData[i];
        internalField.Append(new CharField<6>(this, script.filename, true, "Script filename"));
        if (version >= 220)
          internalField.Append(new CharField<6>(this, script.name, "Script name"));
        else
          internalField.Append(new ZCharField<6>(this, script.name, "Script name"));
        for (int j = 0; j < 6; j++) {
          internalField.Append(new SignedField<16>(this, script.inputs[j]));
        }
      }
    }
    else {
      for (int i = 0; i < 7; i++) {
        ScriptData & script = modelData.scriptData[i];
        internalField.Append(new CharField<8>(this, script.filename, true, "Script filename"));
        internalField.Append(new ZCharField<8>(this, script.name, "Script name"));
        for (int j=0; j<8; j++) {
          internalField.Append(new SignedField<8>(this, script.inputs[j]));
        }
      }
    }
  }

  for (int i = 0; i < 32; i++) {
    if (version >= 220) {
      if (HAS_LARGE_LCD(board))
        internalField.Append(new CharField<4>(this, modelData.inputNames[i], "Input name"));
      else
        internalField.Append(new CharField<3>(this, modelData.inputNames[i], "Input name"));
    }
    else {
      if (HAS_LARGE_LCD(board))
        internalField.Append(new ZCharField<4>(this, modelData.inputNames[i], "Input name"));
      else
        internalField.Append(new ZCharField<3>(this, modelData.inputNames[i], "Input name"));
    }
  }

  for (int i = 0; i < 8; i++) {
    if (i < Boards::getCapability(board, Board::Pots) + Boards::getCapability(board, Board::Sliders))
      internalField.Append(new BoolField<1>(this, modelData.potsWarnEnabled[i]));
    else
      internalField.Append(new SpareBitsField<1>(this));
  }

  for (int i = 0; i < MAX_POTS_STORAGE(board, version) + MAX_SLIDERS_STORAGE(board, version); i++) {
    internalField.Append(new SignedField<8>(this, modelData.potsWarnPosition[i]));
  }

  if (version <= 218 && IS_HORUS_X10(board)) {
    internalField.Append(new SpareBitsField<16>(this));
  }

  if (IS_SKY9X(board)) {
    internalField.Append(new SpareBitsField<8>(this));
    internalField.Append(new SpareBitsField<8>(this));
  }

  for (int i = 0; i < MAX_TELEMETRY_SENSORS(board, version); ++i) {
    internalField.Append(new SensorField(this, modelData, modelData.sensorData[i], board, version));
  }

  if (IS_TARANIS_X9E(board)) {
    internalField.Append(new UnsignedField<8>(this, modelData.toplcdTimer));
  }

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    if (version >= 220) {  //  data from earlier versions cannot be converted so fields initialised in afterImport
      for (int i = 0; i < MAX_CUSTOM_SCREENS; i++) {
        internalField.Append(new CustomScreenField(this, modelData.customScreens.customScreenData[i], board, version));
      }
      internalField.Append(new TopBarField(this, modelData.topBarData, board, version));
      internalField.Append(new UnsignedField<8>(this, modelData.view));
    }
  }
  else if (version >= 219) {
    for (int i = 0; i < 4; i++) {
      internalField.Append(new UnsignedField<2>(this, modelData.frsky.screens[i].type));
    }
    for (int i = 0; i < 4; i++) {
      internalField.Append(new FrskyScreenField(this, modelData.frsky.screens[i], board, version, variant));
    }
    internalField.Append(new SpareBitsField<8>(this)); // current view
  }

  if (version >= 220) {
    internalField.Append(new CharField<8>(this, modelData.registrationId, "ACCESS Registration ID"));
  }
  else if (version >= 219) {
    internalField.Append(new ZCharField<8>(this, modelData.registrationId, "ACCESS Registration ID"));
  }

  if (IS_JUMPER_TPRO(board)) {
    internalField.Append(new UnsignedField<16>(this, modelData.functionSwitchConfig));
    internalField.Append(new UnsignedField<16>(this, modelData.functionSwitchGroup));
    internalField.Append(new UnsignedField<16>(this, modelData.functionSwitchStartConfig));
    internalField.Append(new UnsignedField<8>(this, modelData.functionSwitchLogicalState));
    for (int i=0; i < Boards::getCapability(board, Board::FunctionSwitches); ++i) {
      internalField.Append(new ZCharField<3>(this, modelData.functionSwitchNames[i], "Function switch name"));
    }
  }
}

void OpenTxModelData::beforeExport()
{
  // qDebug() << QString("before export model") << modelData.name;

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    uint32_t newSwitchWarningStates = 0;
    for (int i = 0; i < MAX_SWITCHES(board, version); i++) {
      uint8_t value = (modelData.switchWarningStates >> (2*i)) & 0x03;
      if (!(modelData.switchWarningEnable & (1 << i))) {
        newSwitchWarningStates |= (value + 1) << (3*i);
      }
    }
    modelData.switchWarningStates = newSwitchWarningStates;
  }

  //  TODO remove when enum not radio specific requires eeprom change and conversion
  //  Note: this must mirror reverse afterImport
  if (!IS_FLYSKY_NV14(board))
      modelData.trainerMode -= 1;

  if (modelData.trainerMode > TRAINER_MODE_SLAVE_JACK) {
    if (!IS_TARANIS(board)) {
      modelData.trainerMode -= 2;
      if (!IS_RADIOMASTER_TX16S(board))
        modelData.trainerMode -= 1;
    }
  }
}

void OpenTxModelData::afterImport()
{
  qCDebug(eepromImport) << QString("OpenTxModelData::afterImport()") << modelData.name;

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    uint32_t newSwitchWarningStates = 0;
    for (int i = 0; i < MAX_SWITCHES(board, version); i++) {
      uint8_t value = (modelData.switchWarningStates >> (3*i)) & 0x07;
      if (value == 0)
        modelData.switchWarningEnable |= (1 << i);
      else
        newSwitchWarningStates |= ((value & 0x03) - 1) << (2*i);
    }
    modelData.switchWarningStates = newSwitchWarningStates;

    if (version < 220) {  //  re-initialise as no conversion possible
      const char * layoutId = "Layout2P1";  // currently all using same default though might change for NV14
      RadioLayout::init(layoutId, modelData.customScreens);
      memset(&modelData.topBarData, 0, sizeof(TopBarPersistentData));
    }
  }

  // EX1 / EX2
  if (version <= 218 && IS_HORUS_X10(board) && modelData.thrTraceSrc > 3) {
    modelData.thrTraceSrc += 2;
  }

  // EX3 / EX4
  if (version <= 220 && (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) &&
      modelData.thrTraceSrc > 5) {
    modelData.thrTraceSrc += 2;
  }

  //  TODO remove when enum not radio specific requires eeprom change and conversion
  //  Note: this must mirror reverse beforeExport
  if (!IS_FLYSKY_NV14(board))
      modelData.trainerMode += 1;

  if (modelData.trainerMode > TRAINER_MODE_SLAVE_JACK) {
    if (!IS_TARANIS(board)) {
      modelData.trainerMode += 2;
      if (!IS_RADIOMASTER_TX16S(board))
        modelData.trainerMode += 1;
    }
  }
}

OpenTxGeneralData::OpenTxGeneralData(GeneralSettings & generalData, Board::Type board, unsigned int version, unsigned int variant):
  TransformedField(nullptr, internalField),
  internalField(this, "General Settings"),
  generalData(generalData),
  board(board),
  version(version),
  inputsCount(CPN_MAX_STICKS + MAX_POTS_STORAGE(board, version) + MAX_SLIDERS_STORAGE(board, version) + MAX_MOUSE_ANALOG_SOURCES(board, version))
{
  qCDebug(eepromImport) << QString("OpenTxGeneralData::OpenTxGeneralData(board: %1, version:%2, variant:%3)").arg(board).arg(version).arg(variant);

  generalData.version = version;
  generalData.variant = variant;

  internalField.Append(new UnsignedField<8>(this, generalData.version));
  internalField.Append(new UnsignedField<16>(this, generalData.variant));

  for (int i=0, input=0; i<inputsCount; i++, input++) {
    if (version <= 218 && IS_HORUS_X10(board) && (i == CPN_MAX_STICKS + 3)) {
      input += 2;
    }
    if (version <= 220 && IS_FAMILY_HORUS_OR_T16(board) && (i == CPN_MAX_STICKS + 5)) { // make room for EXT3 and EXT4
      input += 2;
    }
    internalField.Append(new SignedField<16>(this, generalData.calibMid[input]));
    internalField.Append(new SignedField<16>(this, generalData.calibSpanNeg[input]));
    internalField.Append(new SignedField<16>(this, generalData.calibSpanPos[input]));
  }

  if (version <= 218 && IS_HORUS_X10(board)) {
    internalField.Append(new SpareBitsField<16*6>(this));
  }

  internalField.Append(new UnsignedField<16>(this, chkSum));

  if (!IS_FAMILY_HORUS_OR_T16(board) || (IS_FLYSKY_NV14(board))) {
    internalField.Append(new UnsignedField<8>(this, generalData.currModelIndex));
    internalField.Append(new UnsignedField<8>(this, generalData.contrast));
  }
  internalField.Append(new UnsignedField<8>(this, generalData.vBatWarn));
  internalField.Append(new SignedField<8>(this, generalData.txVoltageCalibration));

  internalField.Append(new SignedField<3>(this, generalData.backlightMode));
  if (version >= 219)
    internalField.Append(new SignedField<2>(this, generalData.antennaMode));
  else
    internalField.Append(new SpareBitsField<2>(this));
  internalField.Append(new BoolField<1>(this, generalData.rtcCheckDisable));
  if (IS_JUMPER_T18(board)) {
    internalField.Append(new BoolField<1>(this, generalData.keysBacklight));
    internalField.Append(new SpareBitsField<1>(this));
  }
  else {
    internalField.Append(new SpareBitsField<2>(this));
  }

  for (int i=0; i<4; i++) {
    internalField.Append(new SignedField<16>(this, generalData.trainer.calib[i]));
  }

  for (int i=0; i<4; i++) {
    internalField.Append(new UnsignedField<6>(this, generalData.trainer.mix[i].src));
    internalField.Append(new UnsignedField<2>(this, generalData.trainer.mix[i].mode));
    internalField.Append(new SignedField<8>(this, generalData.trainer.mix[i].weight));
  }

  internalField.Append(new UnsignedField<8>(this, generalData.view, 0, MAX_VIEWS(board)-1));

  internalField.Append(new SpareBitsField<2>(this)); // TODO buzzerMode?
  internalField.Append(new BoolField<1>(this, generalData.fai));
  internalField.Append(new SignedField<2>(this, (int &)generalData.beeperMode));
  internalField.Append(new BoolField<1>(this, generalData.flashBeep));
  internalField.Append(new BoolField<1>(this, generalData.disableMemoryWarning));
  internalField.Append(new BoolField<1>(this, generalData.disableAlarmWarning));

  internalField.Append(new UnsignedField<2>(this, generalData.stickMode));
  internalField.Append(new SignedField<5>(this, generalData.timezone));
  if (IS_HORUS_OR_TARANIS(board)) {
    internalField.Append(new BoolField<1>(this, generalData.adjustRTC));
  }
  else {
    internalField.Append(new SpareBitsField<1>(this));
  }

  internalField.Append(new UnsignedField<8>(this, generalData.inactivityTimer));
  internalField.Append(new UnsignedField<3>(this, generalData.internalModuleBaudrate));
  if (IS_FAMILY_HORUS_OR_T16(board))
    internalField.Append(new SpareBitsField<3>(this));
  else if (IS_TARANIS(board))
    internalField.Append(new SignedField<3>(this, generalData.splashDuration));
  else
    internalField.Append(new UnsignedField<3>(this, generalData.splashMode)); // TODO
  internalField.Append(new SignedField<2>(this, (int &)generalData.hapticMode));

  internalField.Append(new SignedField<8>(this, generalData.switchesDelay));

  internalField.Append(new UnsignedField<8>(this, generalData.backlightDelay));
  internalField.Append(new UnsignedField<8>(this, generalData.templateSetup));
  internalField.Append(new SignedField<8>(this, generalData.PPM_Multiplier));
  internalField.Append(new SignedField<8>(this, generalData.hapticLength));

  if (version < 218 || (!IS_TARANIS(board) && !IS_FAMILY_HORUS_OR_T16(board)) || IS_FLYSKY_NV14(board)) {
    internalField.Append(new UnsignedField<8>(this, generalData.reNavigation));
  }

  if ((!IS_TARANIS(board) && !IS_FAMILY_HORUS_OR_T16(board)) || IS_FLYSKY_NV14(board)) {
    internalField.Append(new UnsignedField<8>(this, generalData.stickReverse));
  }

  internalField.Append(new SignedField<3>(this, generalData.beeperLength));
  internalField.Append(new SignedField<3>(this, generalData.hapticStrength));
  internalField.Append(new UnsignedField<1>(this, generalData.gpsFormat));
  internalField.Append(new SpareBitsField<1>(this)); // unexpectedShutdown

  internalField.Append(new UnsignedField<8>(this, generalData.speakerPitch));

  internalField.Append(new ConversionField< SignedField<8> >(this, generalData.speakerVolume, -12, 0, 0, 23, "Volume"));

  internalField.Append(new SignedField<8>(this, generalData.vBatMin));
  internalField.Append(new SignedField<8>(this, generalData.vBatMax));

  internalField.Append(new UnsignedField<8>(this, generalData.backlightBright));
  if (version < 218) internalField.Append(new SignedField<8>(this, generalData.txCurrentCalibration));

  if (version < 218) internalField.Append(new SpareBitsField<8>(this));
  if (version < 218) internalField.Append(new UnsignedField<8>(this, generalData.mAhWarn));
  if (version < 218) internalField.Append(new UnsignedField<16>(this, generalData.mAhUsed));
  internalField.Append(new UnsignedField<32>(this, generalData.globalTimer));
  if (version < 218) internalField.Append(new SpareBitsField<8>(this));
  internalField.Append(new UnsignedField<4>(this, generalData.bluetoothBaudrate));
  internalField.Append(new UnsignedField<4>(this, generalData.bluetoothMode));
  if (version < 218) internalField.Append(new BoolField<8>(this, generalData.optrexDisplay));
  if (version < 218) internalField.Append(new UnsignedField<8>(this, generalData.sticksGain));


  if (version < 218) internalField.Append(new UnsignedField<8>(this, generalData.rotarySteps));
  internalField.Append(new UnsignedField<2>(this, generalData.countryCode));

  if (version >= 219 && IS_HORUS_OR_TARANIS(board)) {
    internalField.Append(new SignedField<3>(this, generalData.pwrOnSpeed, "Power On Speed"));
    internalField.Append(new SignedField<3>(this, generalData.pwrOffSpeed, "Power Off Speed"));
  }
  else {
    internalField.Append(new SpareBitsField<6>(this));
  }

  internalField.Append(new UnsignedField<1>(this, generalData.imperial));
  if (version >= 218) {
    internalField.Append(new BoolField<1>(this, generalData.noJitterFilter));
    internalField.Append(new BoolField<1>(this, generalData.disableRssiPoweroffAlarm));
    internalField.Append(new UnsignedField<2>(this, generalData.usbMode));
    internalField.Append(new UnsignedField<2>(this, generalData.jackMode));
    internalField.Append(new BoolField<1>(this, generalData.sportPower));
  }
  else {
    internalField.Append(new SpareBitsField<7>(this));
  }

  internalField.Append(new CharField<2>(this, generalData.ttsLanguage, true, "TTS language"));
  if (version >= 218) {
    internalField.Append(new SignedField<4>(this, generalData.beepVolume));
    internalField.Append(new SignedField<4>(this, generalData.wavVolume));
    internalField.Append(new SignedField<4>(this, generalData.varioVolume));
    internalField.Append(new SignedField<4>(this, generalData.backgroundVolume));
  }
  else {
    internalField.Append(new SignedField<8>(this, generalData.beepVolume));
    internalField.Append(new SignedField<8>(this, generalData.wavVolume));
    internalField.Append(new SignedField<8>(this, generalData.varioVolume));
  }
  internalField.Append(new SignedField<8>(this, generalData.varioPitch));
  internalField.Append(new SignedField<8>(this, generalData.varioRange));
  internalField.Append(new SignedField<8>(this, generalData.varioRepeat));
  if (version < 218) internalField.Append(new SignedField<8>(this, generalData.backgroundVolume));

  if (version >= 218) {
    for (int i=0; i<MAX_CUSTOM_FUNCTIONS(board, version); i++) {
      internalField.Append(new ArmCustomFunctionField(this, generalData.customFn[i], board, version, variant));
    }
  }

  if (IS_STM32(board)) {
    if (version >= 218) {
      internalField.Append(new UnsignedField<4>(this, generalData.serialPort[GeneralSettings::SP_AUX1]));
      if (IS_FAMILY_HORUS_OR_T16(board) && version >= 219) {
        internalField.Append(new UnsignedField<4>(this, generalData.serialPort[GeneralSettings::SP_AUX2]));
      }
      else {
        for (uint8_t i=0; i<SLIDERS_CONFIG_SIZE(board,version); i++) {
          internalField.Append(new UnsignedField<1>(this, generalData.sliderConfig[i]));
        }
      }
    }
    else {
      internalField.Append(new UnsignedField<6>(this, generalData.serialPort[GeneralSettings::SP_AUX1]));
      if (IS_TARANIS_X9E(board)) {
        internalField.Append(new UnsignedField<1>(this, generalData.sliderConfig[2]));
        internalField.Append(new UnsignedField<1>(this, generalData.sliderConfig[3]));
      }
      else {
        internalField.Append(new SpareBitsField<2>(this));
      }
    }

    if (IS_FAMILY_HORUS_OR_T16(board)) {
      for (int i=0; i<SWITCHES_CONFIG_SIZE(board, version) / 2; i++) {
        if (i < MAX_SWITCHES(board, version))
          internalField.Append(new UnsignedField<2>(this, generalData.switchConfig[i]));
        else
          internalField.Append(new SpareBitsField<2>(this));
      }
    }

    for (int i=0; i<POTS_CONFIG_SIZE(board, version) / 2; i++) {
      int offset = 0;
      // 2 new pots for Horus from 219 on
      if (version <= 218 && IS_FAMILY_HORUS_OR_T16(board) && (i >= 3))
        offset += 2;

      if (i < MAX_POTS_STORAGE(board, version))
        internalField.Append(new UnsignedField<2>(this, generalData.potConfig[i+offset]));
      else
        internalField.Append(new SpareBitsField<2>(this));
    }

    if (IS_FAMILY_HORUS_OR_T16(board) && version >= 219) {
      for (int i=0; i<SLIDERS_CONFIG_SIZE(board,version); i++) {
        if (i < MAX_SLIDERS_STORAGE(board, version))
          internalField.Append(new UnsignedField<1>(this, generalData.sliderConfig[i]));
        else
          internalField.Append(new SpareBitsField<1>(this));
      }
    }

    if (!IS_FAMILY_HORUS_OR_T16(board)) {
      internalField.Append(new UnsignedField<8>(this, generalData.backlightColor));
    }
  }
  else if (IS_SKY9X(board) && version >= 218) {
    internalField.Append(new SignedField<8>(this, generalData.txCurrentCalibration));
    internalField.Append(new SpareBitsField<8>(this));
    internalField.Append(new UnsignedField<8>(this, generalData.mAhWarn));
    internalField.Append(new UnsignedField<16>(this, generalData.mAhUsed));
    internalField.Append(new SpareBitsField<8>(this));
    internalField.Append(new BoolField<8>(this, generalData.optrexDisplay));
    internalField.Append(new UnsignedField<8>(this, generalData.sticksGain));
    internalField.Append(new UnsignedField<8>(this, generalData.rotarySteps));
  }

  if (IS_TARANIS_X9E(board))
    internalField.Append(new SpareBitsField<64>(this)); // switchUnlockStates
  else if (version >= 219 && (IS_TARANIS_X9D(board) || IS_JUMPER_TPRO(board)))
    internalField.Append(new SpareBitsField<32>(this)); // switchUnlockStates
  else if (IS_TARANIS(board))
    internalField.Append(new SpareBitsField<16>(this)); // switchUnlockStates

  if (version == 217) {
    for (int i = 0; i < MAX_CUSTOM_FUNCTIONS(board, version); i++) {
      internalField.Append(new ArmCustomFunctionField(this, generalData.customFn[i], board, version, variant));
    }
  }

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    for (int i = 0; i < MAX_SWITCHES(board, version); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.switchName[i], "Switch name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.switchName[i], "Switch name"));
    }
    for (int i = 0; i < CPN_MAX_STICKS; ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.stickName[i], "Stick name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.stickName[i], "Stick name"));
    }
    for (int i = 0; i < MAX_POTS_STORAGE(board, version); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.potName[i], "Pot name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.potName[i], "Pot name"));
    }
    for (int i = 0; i < MAX_SLIDERS_STORAGE(board, version); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.sliderName[i], "Slider name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.sliderName[i], "Slider name"));
    }
    if (version <= 218 && IS_HORUS_X10(board)) {
      internalField.Append(new SpareBitsField<48>(this)); // DUMMY_ANAS
    }
    internalField.Append(new CharField<17>(this, generalData.currModelFilename, true, "Current model filename"));
  }
  else if (IS_TARANIS(board)) {
    for (int i = 0; i < SWITCHES_CONFIG_SIZE(board, version) / 2; i++) {
      if (i < MAX_SWITCHES(board, version))
        internalField.Append(new UnsignedField<2>(this, generalData.switchConfig[i]));
      else
        internalField.Append(new SpareBitsField<2>(this));
    }
    for (int i = 0; i < MAX_SWITCHES(board, version); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.switchName[i], "Switch name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.switchName[i], "Switch name"));
    }
    for (int i = 0; i < CPN_MAX_STICKS; ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.stickName[i], "Stick name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.stickName[i], "Stick name"));
    }
    for (int i = 0; i < Boards::getCapability(board, Board::Pots); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.potName[i], "Pot name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.potName[i], "Pot name"));
    }
    for (int i = 0; i < Boards::getCapability(board, Board::Sliders); ++i) {
      if (version >= 220)
        internalField.Append(new CharField<3>(this, generalData.sliderName[i], "Slider name"));
      else
        internalField.Append(new ZCharField<3>(this, generalData.sliderName[i], "Slider name"));
    }
  }

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    internalField.Append(new SpareBitsField<1>(this));
    internalField.Append(new UnsignedField<7>(this, generalData.backlightOffBright));
    if (version >= 220)
      internalField.Append(new CharField<10>(this, generalData.bluetoothName, "Bluetooth name"));
    else
      internalField.Append(new ZCharField<10>(this, generalData.bluetoothName, "Bluetooth name"));
  }
  else if (IS_TARANIS_X9E(board) || (version >= 219 && (IS_TARANIS_X7(board) || IS_TARANIS_X9D(board) || IS_TARANIS_XLITE(board)))) {
    internalField.Append(new SpareBitsField<8>(this));
    if (version >= 220)
      internalField.Append(new CharField<10>(this, generalData.bluetoothName, "Bluetooth name"));
    else
      internalField.Append(new ZCharField<10>(this, generalData.bluetoothName, "Bluetooth name"));
  }

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    if (version >= 220) {   //  data from earlier versions cannot be converted so fields initialised in afterImport
      internalField.Append(new CharField<8>(this, generalData.themeData.themeName, true, "Theme name"));
      for (int i = 0; i < MAX_THEME_OPTIONS; i++) {
        internalField.Append(new ZoneOptionValueTypedField(this, generalData.themeData.themePersistentData.options[i], board, version));
      }
    }
  }

  if (version >= 220) {
    internalField.Append(new CharField<8>(this, generalData.registrationId, "ACCESS Registration ID"));
  }
  else if (version >= 219) {
    internalField.Append(new ZCharField<8>(this, generalData.registrationId, "ACCESS Registration ID"));
  }

  if (version >= 219 && IS_TARANIS_XLITES(board)) {
    internalField.Append(new SignedField<8>(this, generalData.gyroMax, "Gyro full scale"));
    internalField.Append(new SignedField<8>(this, generalData.gyroOffset, "Gyro Offset"));
  }

  if (version >= 220) {
    internalField.Append(new SignedField<2>(this, generalData.uartSampleMode, "Uart Sample Mode"));
    internalField.Append(new UnsignedField<2>(this, generalData.rotEncMode));
    //internalField.Append(new SpareBitsField<6>(this));  // may need padding to end of 8 byte boundary
  }
}

void OpenTxGeneralData::beforeExport()
{
  uint16_t sum = 0;

  int count = 0;
  for (int i = 0; i < inputsCount; i++) {
    sum += generalData.calibMid[i];
    if (++count == 12) break;
    sum += generalData.calibSpanNeg[i];
    if (++count == 12) break;
    sum += generalData.calibSpanPos[i];
    if (++count == 12) break;
  }

  chkSum = sum;

  if (Boards::getCapability((Board::Type)generalData.variant,
                            Board::SportMaxBaudRate) >= 400000)
    generalData.internalModuleBaudrate =
        (generalData.internalModuleBaudrate + moduleBaudratesList.size() - 1) %
        moduleBaudratesList.size();
}

void OpenTxGeneralData::afterImport()
{
  if (IS_FAMILY_HORUS_OR_T16(board)) {
    if (version < 220) {    //  re-initialise as no conversion possible
      const char * themeName = IS_FLYSKY_NV14(board) ? "FlySky" : "EdgeTX";
      RadioTheme::init(themeName, generalData.themeData);
    }
  }

  if (Boards::getCapability((Board::Type)generalData.variant,
                            Board::SportMaxBaudRate) >= 400000)
    generalData.internalModuleBaudrate =
        (generalData.internalModuleBaudrate + 1) % moduleBaudratesList.size();
}
