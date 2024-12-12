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

#include "helpers.h"
#include "modelprinter.h"
#include "multiprotocols.h"
#include "boards.h"
#include "helpers_html.h"
#include "appdata.h"
#include "adjustmentreference.h"
#include "curveimage.h"
#include "sourcenumref.h"

#include <QApplication>
#include <QPainter>
#include <QFile>
#include <QUrl>
#include <QTextStream>

extern AppData g;

QString changeColor(const QString & input, const QString & to, const QString & from)
{
  QString result = input;
  return result.replace("color="+from, "color="+to);
}

ModelPrinter::ModelPrinter(Firmware * firmware, const GeneralSettings & generalSettings, const ModelData & model):
  firmware(firmware),
  generalSettings(generalSettings),
  model(model)
{
}

ModelPrinter::~ModelPrinter()
{
}

QString formatTitle(const QString & name)
{
  return QString("<b>" + name + "</b>&nbsp;");
}

void debugHtml(const QString & html)
{
  QFile file("foo.html");
  file.open(QIODevice::Truncate | QIODevice::WriteOnly);
  file.write(html.toUtf8());
  file.close();
}

QString addFont(const QString & input, const QString & color, const QString & size, const QString & face)
{
  QString colorStr;
  if (!color.isEmpty()) {
    colorStr = "color=" + color;
  }
  QString sizeStr;
  if (!size.isEmpty()) {
    sizeStr = "size=" + size;
  }
  QString faceStr;
  if (!face.isEmpty()) {
    faceStr = "face='" + face + "'";
  }
  return "<font " + sizeStr + " " + faceStr + " " + colorStr + ">" + input + "</font>";
}

QString ModelPrinter::printLabelValue(const QString & lbl, const QString & val, const bool sep) {
  return QString("<b>%1:</b> %2%3 ").arg(lbl, val, (sep ? ";" : ""));
}

QString ModelPrinter::printLabelValues(const QString & lbl, const QStringList & vals, const bool sep) {
  QString str;
  if (vals.count() > 1)
    str.append("(");
  for (int i=0;i<vals.count();i++) {
    str.append(vals.at(i));
    if (i<(vals.count()-1))
      str.append(", ");
  }
  if (vals.count() > 1)
    str.append(")");

  return printLabelValue(lbl, str, sep);
}

#define MASK_TIMEVALUE_HRSMINS 1
#define MASK_TIMEVALUE_ZEROHRS 2
#define MASK_TIMEVALUE_PADSIGN 3

QString ModelPrinter::printTimeValue(const int value, const unsigned int mask)
{
  QString result;
  int sign = 1;
  int val = value;
  if (val < 0) {
    val = -val;
    sign = -1;
  }
  result = (sign < 0 ? QString("-") : ((mask & MASK_TIMEVALUE_PADSIGN) ? QString(" ") : QString("")));
  if (mask & MASK_TIMEVALUE_HRSMINS) {
    int hours = val / 3600;
    if (hours > 0 || (mask & MASK_TIMEVALUE_ZEROHRS)) {
      val -= hours * 3600;
      result.append(QString("%1:").arg(hours, 2, 10, QLatin1Char('0')));
    }
  }
  int minutes = val / 60;
  int seconds = val % 60;
  result.append(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
  return result;
}

#define BOOLEAN_ENABLEDISABLE 1
#define BOOLEAN_TRUEFALSE 2
#define BOOLEAN_YESNO 3
#define BOOLEAN_YN 4
#define BOOLEAN_ONOFF 5

QString ModelPrinter::printBoolean(const bool val, const int typ)
{
  switch (typ) {
    case BOOLEAN_ENABLEDISABLE:
      return (val ? tr("Enable") : tr("Disable"));
    case BOOLEAN_TRUEFALSE:
      return (val ? tr("True") : tr("False"));
    case BOOLEAN_YESNO:
      return (val ? tr("Yes") : tr("No"));
    case BOOLEAN_YN:
      return (val ? tr("Y") : tr("N"));
    case BOOLEAN_ONOFF:
      return (val ? tr("ON") : tr("OFF"));
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printChannelName(int idx)
{
  QString str = RawSource(SOURCE_TYPE_CH, idx + 1).toString(&model, &generalSettings);
  if (firmware->getCapability(ChannelsName)) {
    str = str.leftJustified(firmware->getCapability(ChannelsName) + 5, ' ', false);
  }
  str.append(' ');
  return str.toHtmlEscaped();
}

QString ModelPrinter::printTrimIncrementMode()
{
  switch (model.trimInc) {
    case -2:
      return tr("Exponential");
    case -1:
      return tr("Extra Fine");
    case 0:
      return tr("Fine");
    case 1:
      return tr("Medium");
    case 2:
      return tr("Coarse");
    default:
      return tr("Unknown");
  }
}

QString ModelPrinter::printModule(int idx)
{
  QStringList str;
  QString result;
  ModuleData module = model.moduleData[(idx<0 ? CPN_MAX_MODULES : idx)];
  if (idx < 0) {
    str << printLabelValue(tr("Mode"), model.trainerModeToString());
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      if (model.trainerMode == TRAINER_MODE_SLAVE_JACK) {
        str << printLabelValue(tr("Channels"), QString("%1-%2").arg(module.channelsStart + 1).arg(module.channelsStart + module.channelsCount));
        str << printLabelValue(tr("Frame length"), QString("%1ms").arg(printPPMFrameLength(module.ppm.frameLength)));
        str << printLabelValue(tr("PPM delay"), QString("%1us").arg(module.ppm.delay));
        str << printLabelValue(tr("Polarity"), module.polarityToString());
      }
    }
    result = str.join(" ");
  }
  else {
    str << printLabelValue(tr("Protocol"), ModuleData::protocolToString(module.protocol));
    if (module.protocol) {
      if (module.protocol == PULSES_PPM || module.protocol == PULSES_SBUS)
        str << printLabelValue(tr("Sub Type"), module.subTypeToString());
      str << printLabelValue(tr("Channels"), QString("%1-%2").arg(module.channelsStart + 1).arg(module.channelsStart + module.channelsCount));
      if (module.protocol == PULSES_PPM || module.protocol == PULSES_SBUS) {
        str << printLabelValue(tr("Frame length"), QString("%1ms").arg(printPPMFrameLength(module.ppm.frameLength)));
        str << printLabelValue(tr("Polarity"), module.polarityToString());
        if (module.protocol == PULSES_PPM)
          str << printLabelValue(tr("Delay"), QString("%1us").arg(module.ppm.delay));
      }
      else {
        if (!(module.protocol == PULSES_PXX_XJT_D8 || module.protocol == PULSES_CROSSFIRE ||
              module.protocol == PULSES_GHOST || module.protocol == PULSES_SBUS || module.protocol == PULSES_FLYSKY_AFHDS2A)) {
          str << printLabelValue(tr("Receiver"), QString::number(module.modelId));
        }
        if (module.protocol == PULSES_MULTIMODULE) {
          str << printLabelValue(tr("Radio protocol"), module.rfProtocolToString());
          str << printLabelValue(tr("Subtype"), module.subTypeToString());
          str << printLabelValue(tr("Option value"), QString::number(module.multi.optionValue));
        }
        if (module.protocol == PULSES_PXX_R9M) {
          str << printLabelValue(tr("Sub Type"), module.subTypeToString());
          str << printLabelValue(tr("RF Output Power"), module.powerValueToString(firmware));
        }
        if (module.protocol == PULSES_FLYSKY_AFHDS2A) {
          str << printLabelValue(tr("Options"), module.afhds2aMode1ToString() + " " + module.afhds2aMode2ToString());
        }
        if (module.protocol == PULSES_FLYSKY_AFHDS3) {
          str << printLabelValue(tr("Type"), module.afhds3PhyModeToString() + " " + module.afhds3EmiToString());
        }
        if (module.protocol == PULSES_GHOST) {
          str << printLabelValue(tr("Raw 12 bits"), printBoolean(module.ghost.raw12bits, BOOLEAN_YN));
        }
        if (module.protocol == PULSES_CROSSFIRE) {
          str << printLabelValue(tr("Arming mode"), module.crsfArmingModeToString());
          if (module.crsf.crsfArmingMode == ModuleData::CRSF_ARMING_MODE_SWITCH) {
            str << printLabelValue(tr("Switch"), RawSwitch(module.crsf.crsfArmingTrigger).toString());
          }
        }
      }
    }
    result = str.join(" ");
    if (((PulsesProtocol)module.protocol == PulsesProtocol::PULSES_PXX_XJT_X16 || (PulsesProtocol)module.protocol == PulsesProtocol::PULSES_PXX_R9M)
       && firmware->getCapability(HasFailsafe))
      result.append("<br/>" + printFailsafe(idx));
  }
  return result;
}

QString ModelPrinter::printHeliSwashType ()
{
  switch (model.swashRingData.type) {
    case HELI_SWASH_TYPE_90:
        return tr("90");
      case HELI_SWASH_TYPE_120:
        return tr("120");
      case HELI_SWASH_TYPE_120X:
        return tr("120X");
      case HELI_SWASH_TYPE_140:
        return tr("140");
      case HELI_SWASH_TYPE_NONE:
        return tr("Off");
      default:
        return CPN_STR_UNKNOWN_ITEM;
    }
}

QString ModelPrinter::printCenterBeep()
{
  QStringList strl;
  Board::Type board = getCurrentBoard();
  int inputs = Boards::getBoardCapability(board, Board::Inputs);

  for (int i = 0; i < inputs + firmware->getCapability(RotaryEncoders); i++) {
    if (model.beepANACenter & (0x01 << i)) {
      RawSource src((i < inputs) ? SOURCE_TYPE_INPUT : SOURCE_TYPE_ROTARY_ENCODER, (i < inputs) ? i : inputs - i);
      strl << src.toString(&model, &generalSettings);
    }
  }

  return (strl.isEmpty() ? tr("None") : strl.join(" "));
}

QString ModelPrinter::printTrim(int flightModeIndex, int stickIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.trimMode[stickIndex] == -1) {
    return tr("OFF");
  } else if (fm.trimMode[stickIndex] == CPN_MAX_FLIGHT_MODES * 2) {
    return tr("3POS");
  }
  else {
    if (fm.trimRef[stickIndex] == flightModeIndex) {
      return QString("%1").arg(fm.trim[stickIndex]);
    }
    else {
      if (fm.trimMode[stickIndex] == 0) {
        return tr("FM%1").arg(fm.trimRef[stickIndex]);
      }
      else {
        if (fm.trim[stickIndex] < 0)
          return tr("FM%1%2").arg(fm.trimRef[stickIndex]).arg(fm.trim[stickIndex]);
        else
          return tr("FM%1+%2").arg(fm.trimRef[stickIndex]).arg(fm.trim[stickIndex]);
      }
    }
  }
}

QString ModelPrinter::printGlobalVar(int flightModeIndex, int gvarIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.gvars[gvarIndex] <= 1024) {
    return QString("%1").arg(fm.gvars[gvarIndex] * model.gvarData[gvarIndex].multiplierGet());
  }
  else {
    int num = fm.gvars[gvarIndex] - 1025;
    if (num >= flightModeIndex) num++;
    return tr("FM%1").arg(num);
  }
}

QString ModelPrinter::printRotaryEncoder(int flightModeIndex, int reIndex)
{
  const FlightModeData & fm = model.flightModeData[flightModeIndex];

  if (fm.rotaryEncoders[reIndex] <= 1024) {
    return QString("%1").arg(fm.rotaryEncoders[reIndex]);
  }
  else {
    int num = fm.rotaryEncoders[reIndex] - 1025;
    if (num >= flightModeIndex) num++;
    return tr("FM%1").arg(num);
  }
}

QString ModelPrinter::printInputName(int idx)
{
  RawSourceType srcType = (firmware->getCapability(VirtualInputs) ? SOURCE_TYPE_VIRTUAL_INPUT : SOURCE_TYPE_INPUT);
  return RawSource(srcType, idx + 1).toString(&model, &generalSettings).toHtmlEscaped();
}

QString ModelPrinter::printInputLine(int idx)
{
  return printInputLine(model.expoData[idx]);
}

QString ModelPrinter::printInputLine(const ExpoData & input)
{
  QString str = "&nbsp;";

  switch (input.mode) {
    case (1): str += "&lt;-&nbsp;"; break;
    case (2): str += "-&gt;&nbsp;"; break;
    default:  str += "&nbsp;&nbsp;&nbsp;"; break;
  }

  if (firmware->getCapability(VirtualInputs)) {
    str += input.srcRaw.toString(&model, &generalSettings).toHtmlEscaped();
  }

  if (input.srcRaw.type == SOURCE_TYPE_TELEMETRY && input.scale != 0){
    RawSourceRange range = input.srcRaw.getRange(&model, generalSettings);
    str += " " + tr("Scale(%1)").arg(input.scale * range.step).toHtmlEscaped();
  }

  str += " " + tr("Weight(%1)").arg(SourceNumRef(input.weight).toString(&model, &generalSettings)).toHtmlEscaped();
  if (input.curve.value)
    str += " " + input.curve.toString(&model, true, &generalSettings).toHtmlEscaped();

  QString flightModesStr = printFlightModes(input.flightModes);
  if (!flightModesStr.isEmpty())
    str += " " + flightModesStr.toHtmlEscaped();

  if (input.swtch.type != SWITCH_TYPE_NONE)
    str += " " + tr("Switch(%1)").arg(input.swtch.toString(getCurrentBoard(), &generalSettings, &model)).toHtmlEscaped();


  if (firmware->getCapability(VirtualInputs)) {
    if ((input.srcRaw.isStick() && input.carryTrim == CARRYTRIM_STICK_OFF) || (!input.srcRaw.isStick() && input.carryTrim == CARRYTRIM_DEFAULT))
      str += " " + tr("No Trim");
    else if (input.carryTrim != CARRYTRIM_DEFAULT)
      str += " " + input.carryTrimToString().toHtmlEscaped();
  }

  if (input.offset)
    str += " " + tr("Offset(%1)").arg(SourceNumRef(input.offset).toString(&model, &generalSettings)).toHtmlEscaped();
  if (firmware->getCapability(HasExpoNames) && input.name[0])
    str += QString(" [%1]").arg(input.name).toHtmlEscaped();

  return str;
}

QString ModelPrinter::printMixerLine(const MixData & mix, bool showMultiplex, int highlightedSource)
{
  QString str = "&nbsp;";

  if (showMultiplex) {
    switch(mix.mltpx) {
      case (1): str += "*="; break;
      case (2): str += ":="; break;
      default:  str += "+="; break;
    }
  }
  else {
    str += "&nbsp;&nbsp;";
  }
  // highlight source if needed
  QString source = mix.srcRaw.toString(&model, &generalSettings).toHtmlEscaped();
  if ( (mix.srcRaw.type == SOURCE_TYPE_CH) && (mix.srcRaw.index + 1 == (int)highlightedSource) ) {
    source = "<b>" + source + "</b>";
  }
  str += "&nbsp;" + source;

  str += " " + tr("Weight(%1)").arg(SourceNumRef(mix.weight).toString(&model, &generalSettings)).toHtmlEscaped();

  QString flightModesStr = printFlightModes(mix.flightModes);
  if (!flightModesStr.isEmpty())
    str += " " + flightModesStr.toHtmlEscaped();

  if (mix.swtch.type != SWITCH_TYPE_NONE)
    str += " " + tr("Switch(%1)").arg(mix.swtch.toString(getCurrentBoard(), &generalSettings, &model)).toHtmlEscaped();

  if (mix.carryTrim > 0)
    str += " " + tr("NoTrim");
  else if (mix.carryTrim < 0)
    str += " " + RawSource(SOURCE_TYPE_TRIM, (-(mix.carryTrim)-1) + 1).toString(&model, &generalSettings);

  if (firmware->getCapability(HasNoExpo) && mix.noExpo)
    str += " " + tr("No DR/Expo").toHtmlEscaped();
  if (mix.sOffset)
    str += " " + tr("Offset(%1)").arg(SourceNumRef(mix.sOffset).toString(&model, &generalSettings)).toHtmlEscaped();
  if (mix.curve.value)
    str += " " + mix.curve.toString(&model, true, &generalSettings).toHtmlEscaped();
  int scale = firmware->getCapability(SlowScale);
  if (scale == 0) scale = 1;
  if (mix.delayPrec) {
    scale = scale * 10;
    str += " " + tr("Delay precision(0.00)").toHtmlEscaped();
  }
  if (mix.delayDown || mix.delayUp)
    str += " " + tr("Delay(u%1:d%2)").arg((double)mix.delayUp / scale).arg((double)mix.delayDown / scale).toHtmlEscaped();
  scale = firmware->getCapability(SlowScale);
  if (scale == 0) scale = 1;
  if (mix.speedPrec) {
    scale = scale * 10;
    str += " " + tr("Slow precision(0.00)").toHtmlEscaped();
  }
  if (mix.speedDown || mix.speedUp)
    str += " " + tr("Slow(u%1:d%2)").arg((double)mix.speedUp / scale).arg((double)mix.speedDown / scale).toHtmlEscaped();
  if (mix.mixWarn)
    str += " " + tr("Warn(%1)").arg(mix.mixWarn).toHtmlEscaped();
  if (firmware->getCapability(HasMixerNames) && mix.name[0])
    str += QString(" [%1]").arg(mix.name).toHtmlEscaped();
  return str;
}

QString ModelPrinter::printFlightModeSwitch(const RawSwitch & swtch)
{
  return swtch.toString(getCurrentBoard(), &generalSettings);
}

QString ModelPrinter::printFlightModeName(int index)
{
  return model.flightModeData[index].nameToString(index);
}

QString ModelPrinter::printFlightModes(unsigned int flightModes)
{
  int numFlightModes = firmware->getCapability(FlightModes);
  if (numFlightModes && flightModes) {
    if (flightModes == (unsigned int)(1 << numFlightModes) - 1) {
      return tr("Disabled in all flight modes");
    }
    else {
      QStringList list;
      for (int i = 0; i < numFlightModes; i++) {
        if (!(flightModes & (1 << i))) {
          list << printFlightModeName(i);
        }
      }
      return (list.size() > 1 ? tr("Flight modes") : tr("Flight mode")) + QString("(%1)").arg(list.join(", "));
    }
  }
  else
    return "";
}

QString ModelPrinter::printInputFlightModes(unsigned int flightModes)
{
  int numFlightModes = firmware->getCapability(FlightModes);
  if (numFlightModes && flightModes) {
    if (flightModes == (unsigned int)(1 << numFlightModes) - 1) {
      return tr("None");
    }
    else {
      QStringList list;
      for (int i = 0; i < numFlightModes; i++) {
        if (!(flightModes & (1 << i))) {
          list << printFlightModeName(i);
        }
      }
      return QString("%1").arg(list.join(" "));
    }
  }
  else
    return tr("All");
}

QString ModelPrinter::printLogicalSwitchLine(int idx)
{
  QString result = "";
  const LogicalSwitchData & ls = model.logicalSw[idx];

  if (ls.isEmpty())
    return result;

  QString sw1Name;
  QString sw2Name;

  if (ls.andsw!=0) {
    result +="( ";
  }
  switch (ls.getFunctionFamily()) {
    case LS_FAMILY_EDGE:
      sw1Name = RawSwitch(ls.val1).toString(getCurrentBoard(), &generalSettings);
      result += tr("Edge") + QString("(%1, [%2:%3])").arg(sw1Name).arg(ValToTim(ls.val2)).arg(ls.val3 < 0 ?
                tr("instant") : (ls.val3 == 0 ? tr("infinite") : QString("%1").arg(ValToTim(ls.val2 + ls.val3))));
      break;
    case LS_FAMILY_STICKY:
      sw1Name = RawSwitch(ls.val1).toString(getCurrentBoard(), &generalSettings);
      sw2Name = RawSwitch(ls.val2).toString(getCurrentBoard(), &generalSettings);
      result += tr("Sticky") + QString("(%1, %2)").arg(sw1Name).arg(sw2Name);
      if (ls.lsPersist)
        result += tr(" Persistent");
      break;
    case LS_FAMILY_TIMER:
      result += tr("Timer") + QString("(%1, %2)").arg(ValToTim(ls.val1)).arg(ValToTim(ls.val2));
      break;
    case LS_FAMILY_VOFS: {
      RawSource source = RawSource(ls.val1);
      RawSourceRange range = source.getRange(&model, generalSettings);
      QString res;
      if (ls.val1)
        res += source.toString(&model, &generalSettings);
      else
        res += "0";
      res.remove(" ");
      if (ls.func == LS_FN_APOS || ls.func == LS_FN_ANEG)
        res = "|" + res + "|";
      else if (ls.func == LS_FN_DAPOS)
        res = "|d(" + res + ")|";
      else if (ls.func == LS_FN_DPOS)
        result = "d(" + res + ")";
      result += res;
      if (ls.func == LS_FN_VEQUAL)
        result += " = ";
      else if (ls.func == LS_FN_APOS || ls.func == LS_FN_VPOS || ls.func == LS_FN_DPOS || ls.func == LS_FN_DAPOS)
        result += " &gt; ";
      else if (ls.func == LS_FN_ANEG || ls.func == LS_FN_VNEG)
        result += " &lt; ";
      else if (ls.func == LS_FN_VALMOSTEQUAL)
        result += " ~ ";
      else
        result += tr(" missing");
      result += QString::number(range.step * (ls.val2 /*TODO+ source.getRawOffset(model)*/) + range.offset);
      result += range.unit;
      break;
    }
    case LS_FAMILY_VBOOL:
      sw1Name = RawSwitch(ls.val1).toString(getCurrentBoard(), &generalSettings);
      sw2Name = RawSwitch(ls.val2).toString(getCurrentBoard(), &generalSettings);
      result += sw1Name;
      switch (ls.func) {
        case LS_FN_AND:
          result += " AND ";
          break;
        case LS_FN_OR:
          result += " OR ";
          break;
        case LS_FN_XOR:
          result += " XOR ";
          break;
       default:
          result += " bar ";
          break;
      }
      result += sw2Name;
      break;

    case LS_FAMILY_VCOMP:
      if (ls.val1)
        result += RawSource(ls.val1).toString(&model, &generalSettings);
      else
        result += "0";
      switch (ls.func) {
        case LS_FN_EQUAL:
        case LS_FN_VEQUAL:
          result += " = ";
          break;
        case LS_FN_NEQUAL:
          result += " != ";
          break;
        case LS_FN_GREATER:
          result += " &gt; ";
          break;
        case LS_FN_LESS:
          result += " &lt; ";
          break;
        case LS_FN_EGREATER:
          result += " &gt;= ";
          break;
        case LS_FN_ELESS:
          result += " &lt;= ";
          break;
        default:
          result += " foo ";
          break;
      }
      if (ls.val2) {
        RawSource source = RawSource(ls.val2);
        RawSourceRange range = source.getRange(&model, generalSettings);
        result += source.toString(&model, &generalSettings);
        result += range.unit;
      }
      else
        result += "0";
      break;
  }

  if (ls.andsw != 0) {
    result +=" ) AND ";
    result += RawSwitch(ls.andsw).toString(getCurrentBoard(), &generalSettings);
  }

  if (firmware->getCapability(LogicalSwitchesExt)) {
    if (ls.duration)
      result += " " + tr("Duration") + QString("(%1s)").arg(ls.duration/10.0);
    if (ls.delay)
      result += " " + tr("Delay") + QString("(%1s)").arg(ls.delay/10.0);
  }

  return result;
}

QString ModelPrinter::printCurveName(int idx)
{
  return model.curves[idx].nameToString(idx).toHtmlEscaped();
}

QString ModelPrinter::printCurve(int idx)
{
 const CurveData & curve = model.curves[idx];
  return QString("%1   %2").arg(curve.typeToString()).arg(curve.pointsToString());
}

QString ModelPrinter::createCurveImage(int idx, QTextDocument * document)
{
  CurveImage image;
  image.drawCurve(model.curves[idx], colors[idx]);
  QString filename = QString("mydata://curve-%1-%2.png").arg((uint64_t)this).arg(idx);
  if (document)
    document->addResource(QTextDocument::ImageResource, QUrl(filename), image.get());
  // qDebug() << "ModelPrinter::createCurveImage()" << idx << filename;
  return filename;
}

QString ModelPrinter::printGlobalVarUnit(int idx)
{
  return model.gvarData[idx].unitToString().toHtmlEscaped();
}

QString ModelPrinter::printGlobalVarPrec(int idx)
{
  return model.gvarData[idx].precToString().toHtmlEscaped();
}

QString ModelPrinter::printGlobalVarMin(int idx)
{
  return QString::number(model.gvarData[idx].getMinPrec());
}

QString ModelPrinter::printGlobalVarMax(int idx)
{
  return QString::number(model.gvarData[idx].getMaxPrec());
}

QString ModelPrinter::printGlobalVarPopup(int idx)
{
  return printBoolean(model.gvarData[idx].popup, BOOLEAN_YN);
}

QString ModelPrinter::printOutputValueGVar(int val)
{
  QString result = "";
  if (abs(val) > 10000) {
    if (val < 0)
      result = "-";
    result.append(RawSource(SOURCE_TYPE_GVAR, (abs(val)-10001) + 1).toString(&model));
  }
  else {
    if (val >= 0)
      result = "+";
    result.append(QString::number((qreal)val/10, 'f', 1) + "%");
  }
  return result;
}

QString ModelPrinter::printOutputOffset(int idx)
{
  return printOutputValueGVar(model.limitData[idx].offset);
}

QString ModelPrinter::printOutputMin(int idx)
{
  return printOutputValueGVar(model.limitData[idx].min);
}

QString ModelPrinter::printOutputMax(int idx)
{
  return printOutputValueGVar(model.limitData[idx].max);
}

QString ModelPrinter::printOutputRevert(int idx)
{
  return model.limitData[idx].revertToString();
}

QString ModelPrinter::printOutputPpmCenter(int idx)
{
  return QString::number(model.limitData[idx].ppmCenter + 1500);
}

QString ModelPrinter::printOutputCurve(int idx)
{
  return CurveReference(CurveReference::CURVE_REF_CUSTOM, model.limitData[idx].curve.value).toString(&model, false);
}

QString ModelPrinter::printOutputSymetrical(int idx)
{
  return printBoolean(model.limitData[idx].symetrical, BOOLEAN_YN);
}

QString ModelPrinter::printSettingsOther()
{
  QStringList str;
  str << printLabelValue(tr("Extended Limits"), printBoolean(model.extendedLimits, BOOLEAN_YESNO));
  if (firmware->getCapability(HasDisplayText))
    str << printLabelValue(tr("Display Checklist"), printBoolean(model.displayChecklist, BOOLEAN_YESNO));
  if (firmware->getCapability(GlobalFunctions))
    str << printLabelValue(tr("Global Functions"), printBoolean(!model.noGlobalFunctions, BOOLEAN_YESNO));
  return str.join(" ");
}

QString ModelPrinter::printSwitchWarnings()
{
  QStringList str;
  Board::Type board = getCurrentBoard();
  uint64_t switchStates = model.switchWarningStates;
  uint64_t value;

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    Board::SwitchInfo switchInfo = Boards::getSwitchInfo(i);
    if (switchInfo.type == Board::SWITCH_NOT_AVAILABLE || switchInfo.type == Board::SWITCH_TOGGLE) {
      continue;
    }
    if (!(model.switchWarningEnable & (1 << i))) {
      if (IS_HORUS_OR_TARANIS(board)) {
        value = (switchStates >> (2 * i)) & 0x03;
      }
      else {
        value = (i == 0 ? switchStates & 0x3 : switchStates & 0x1);
        switchStates >>= (i == 0 ? 2 : 1);
      }
      str += RawSwitch(SWITCH_TYPE_SWITCH, 1 + i * 3 + value).toString(board, &generalSettings, &model);
    }
  }
  return (str.isEmpty() ? tr("None") : str.join(" ")) ;
}

QString ModelPrinter::printPotWarnings()
{
  Board::Type board = getCurrentBoard();
  QStringList str = { printLabelValue(tr("Mode"), printPotsWarningMode()) };

  if (model.potsWarningMode) {
    for (int i = Boards::getCapability(board, Board::Sticks); i < Boards::getCapability(board, Board::Inputs); i++) {
      if (generalSettings.isInputAvailable(i) && (generalSettings.isInputPot(i) || generalSettings.isInputSlider(i))) {
        if (model.potsWarnEnabled[i]) {
          RawSource src(SOURCE_TYPE_INPUT, i);
          str += src.toString(&model, &generalSettings);
        }
      }
    }
  }

  return str.join(" ");
}

QString ModelPrinter::printPotsWarningMode()
{
  switch (model.potsWarningMode) {
    case 0:
      return tr("OFF");
    case 1:
      return tr("Manual");
    case 2:
      return tr("Auto");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printFailsafe(int idx)
{
  QStringList strl;
  ModuleData module = model.moduleData[idx];
  strl << printLabelValue(tr("Failsafe Mode"), printFailsafeMode(module.failsafeMode));
  if (module.failsafeMode == FAILSAFE_CUSTOM) {
    for (int i = 0; i < module.channelsCount; i++) {
      strl << printLabelValue(printChannelName(module.channelsStart + i).trimmed(), printFailsafeValue(model.limitData[i].failsafe));
    }
  }
  return strl.join(" ");
}

QString ModelPrinter::printFailsafeValue(int val)
{
  switch (val) {
    case 2000:
      return tr("Hold");
    case 2001:
      return tr("No Pulse");
    default:
      return QString("%1%").arg(QString::number(divRoundClosest(val * 1000, 1024) / 10.0));
  }
}

QString ModelPrinter::printFailsafeMode(unsigned int fsmode)
{
  switch (fsmode) {
    case FAILSAFE_NOT_SET:
      return tr("Not set");
    case FAILSAFE_HOLD:
      return tr("Hold");
    case FAILSAFE_CUSTOM:
      return tr("Custom");
    case FAILSAFE_NOPULSES:
      return tr("No pulses");
    case FAILSAFE_RECEIVER:
      return tr("Receiver");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printSettingsTrim()
{
  QStringList str;
  str << printLabelValue(tr("Step"), printTrimIncrementMode());
  str << printLabelValue(tr("Display"), printTrimsDisplayMode());
  str << printLabelValue(tr("Extended"), printBoolean(model.extendedTrims, BOOLEAN_YESNO));
  Board::Type board = firmware->getBoard();
  if (IS_FLYSKY_EL18(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_PL18(board)) {
    str << printLabelValue(tr("Hats Mode"), printHatsMode());
  }
  return str.join(" ");
}

QString ModelPrinter::printThrottleSource(int idx)
{
  Board::Type board = firmware->getBoard();
  int pscnt = Boards::getCapability(board, Board::Pots) + Boards::getCapability(board, Board::Sliders);

  if (idx == 0)
    return "THR";
  else if (idx <= pscnt)
    return Boards::getInputName(idx + Boards::getCapability(board, Board::Sticks) - 1, board);
  else if (idx <= pscnt + getCurrentFirmware()->getCapability(Outputs))
    return RawSource(SOURCE_TYPE_CH, idx - pscnt - 1).toString(&model, &generalSettings);

  return QString(CPN_STR_UNKNOWN_ITEM);
}

QString ModelPrinter::printTrimsDisplayMode()
{
  switch (model.trimsDisplay) {
    case 0:
      return tr("Never");
    case 1:
      return tr("On Change");
    case 2:
      return tr("Always");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printHatsMode()
{
  switch (model.hatsMode) {
    case GeneralSettings::HATSMODE_TRIMS_ONLY:
      return tr("Trims only");
    case GeneralSettings::HATSMODE_KEYS_ONLY:
      return tr("Keys only");
    case GeneralSettings::HATSMODE_SWITCHABLE:
      return tr("Switchable");
    case GeneralSettings::HATSMODE_GLOBAL:
      return tr("Global");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printModuleType(int idx)
{
  return ModuleData::indexToString(idx, firmware);
}

QString ModelPrinter::printPxxPower(int power)
{
  static const char *strings[] = {
    "10mW", "100mW", "500mW", "3W"
  };
  return CHECK_IN_ARRAY(strings, power);
}

QString ModelPrinter::printThrottle()
{
  QStringList result;
  result << printLabelValue(tr("Source"), printThrottleSource(model.thrTraceSrc));
  result << printLabelValue(tr("Trim idle only"), printBoolean(model.thrTrim, BOOLEAN_YESNO));
  result << printLabelValue(tr("Warning"), printBoolean(!model.disableThrottleWarning, BOOLEAN_YESNO));
  result << printLabelValue(tr("Reversed"), printBoolean(model.throttleReversed, BOOLEAN_YESNO));
  int thrTrim = model.thrTrimSwitch;
  if (thrTrim == 0) thrTrim = 2;
  else if (thrTrim == 2) thrTrim = 0;
  result << printLabelValue(tr("Trim source"), RawSource(SOURCE_TYPE_TRIM, thrTrim + 1).toString(&model, &generalSettings));
  return result.join(" ");
}

QString ModelPrinter::printPPMFrameLength(int ppmFL)
{
  double result = (((double)ppmFL * 5) + 225) / 10;
  return QString::number(result);
}

QString ModelPrinter::printTelemetryProtocol(unsigned int val)
{
  switch (val) {
    case 0:
      return tr("FrSky S.PORT");
    case 1:
      return tr("FrSky D");
    case 2:
      return tr("FrSky D (cable)");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printRssiAlarmsDisabled(bool mb)
{
  return printBoolean(!mb, BOOLEAN_ENABLEDISABLE);
}

QString ModelPrinter::printVarioSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_VARIO_SOURCE_ALTI:
      return tr("Alti");
    case TELEMETRY_VARIO_SOURCE_ALTI_PLUS:
      return tr("Alti+");
    case TELEMETRY_VARIO_SOURCE_VSPEED:
      return tr("VSpeed");
    case TELEMETRY_VARIO_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_VARIO_SOURCE_A2:
      return tr("A2");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printVarioCenterSilent(bool mb)
{
  return printBoolean(mb, BOOLEAN_YESNO);
}

QString ModelPrinter::printVoltsSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_VOLTS_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_VOLTS_SOURCE_A2:
      return tr("A2");
    case TELEMETRY_VOLTS_SOURCE_A3:
      return tr("A3");
    case TELEMETRY_VOLTS_SOURCE_A4:
      return tr("A4");
    case TELEMETRY_VOLTS_SOURCE_FAS:
      return tr("FAS");
    case TELEMETRY_VOLTS_SOURCE_CELLS:
      return tr("Cells");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printCurrentSource(unsigned int val)
{
  switch (val) {
    case TELEMETRY_CURRENT_SOURCE_NONE:
      return tr("None");
    case TELEMETRY_CURRENT_SOURCE_A1:
      return tr("A1");
    case TELEMETRY_CURRENT_SOURCE_A2:
      return tr("A2");
    case TELEMETRY_CURRENT_SOURCE_A3:
      return tr("A3");
    case TELEMETRY_CURRENT_SOURCE_A4:
      return tr("A4");
    case TELEMETRY_CURRENT_SOURCE_FAS:
      return tr("FAS");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printMahPersistent(bool mb)
{
  return printBoolean(mb, BOOLEAN_YESNO);
}

QString ModelPrinter::printIgnoreSensorIds(bool mb)
{
  return printBoolean(mb, BOOLEAN_ENABLEDISABLE);
}

QString ModelPrinter::printTelemetryScreenType(unsigned int val)
{
  switch (val) {
    case TelemetryScreenEnum::TELEMETRY_SCREEN_NONE:
      return tr("None");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_NUMBERS:
      return tr("Numbers");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_BARS:
      return tr("Bars");
    case TelemetryScreenEnum::TELEMETRY_SCREEN_SCRIPT:
      return tr("Script");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

QString ModelPrinter::printTelemetryScreen(unsigned int idx, unsigned int line, unsigned int width)
{
  QStringList strl;
  QStringList hd;
  FrSkyScreenData screen = model.frsky.screens[idx];
  hd << "";  // blank 1st column
  strl << "";
  if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_NUMBERS) {
    if (line == 0) {
      for (int c=0; c<firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
        hd << tr("Source");
      }
    }
    for (int c=0; c<firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
      RawSource source = screen.body.lines[line].source[c];
      strl << source.toString(&model, &generalSettings);
    }
  }
  else if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_BARS) {
    if (line == 0) {
      hd << tr("Source") << tr("Min") << tr("Max");
    }
    RawSource source = screen.body.bars[line].source;
    RawSourceRange range = source.getRange(&model, generalSettings);
    strl << source.toString(&model, &generalSettings);
    QString unit;
    QString minstr;
    QString maxstr;
    if (source.type == SOURCE_TYPE_TIMER) {
      minstr = printTimeValue((float)screen.body.bars[line].barMin, MASK_TIMEVALUE_HRSMINS);
      maxstr = printTimeValue((float)screen.body.bars[line].barMax, MASK_TIMEVALUE_HRSMINS);
    }
    else {
      minstr = QString::number(range.getValue(screen.body.bars[line].barMin));
      maxstr = QString::number(range.getValue(screen.body.bars[line].barMax));
      unit = range.unit;
    }
    strl << QString("%1%2").arg(minstr).arg(unit);
    strl << QString("%1%2").arg(maxstr).arg(unit);
  }
  else if (screen.type == TelemetryScreenEnum::TELEMETRY_SCREEN_SCRIPT && line == 0) {
    hd << tr("Filename");
    strl << QString("%1.lua").arg(screen.body.script.filename);
  }
  return (hd.count() > 1 ? doTableRow(hd, width / hd.count(), "left", "", true) : "" ) + doTableRow(strl, width / strl.count());
}

QString ModelPrinter::printChecklist()
{
  if (!model.displayChecklist)
    return "";
  QString str = tr("Error: Unable to open or read file!");
  QFile file(Helpers::getChecklistFilePath(&model));
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream in(&file);
    if (in.status() == QTextStream::Ok) {
      str = in.readAll();
      str.replace("\n", "<br />");
      str.remove("\r");
    }
    file.close();
  }
  return str;
}
