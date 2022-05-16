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

#include "helpers.h"
#include "helpers_html.h"
#include "multimodelprinter.h"
#include "appdata.h"
#include <algorithm>

MultiModelPrinter::MultiColumns::MultiColumns(int count):
  count(count),
  compareColumns(NULL)
{
  columns = new QString[count];
}

MultiModelPrinter::MultiColumns::~MultiColumns()
{
  delete[] columns;
}

void MultiModelPrinter::MultiColumns::append(const QString & str)
{
  for (int i=0; i<count; i++) {
    append(i, str);
  }
}

void MultiModelPrinter::MultiColumns::appendTitle(const QString & name)
{
  append("<b>" + name + "</b>&nbsp;");
}

void MultiModelPrinter::MultiColumns::append(int idx, const QString & str)
{
  if (compareColumns)
    compareColumns[idx].append(str);
  else
    columns[idx].append(str);
}

void MultiModelPrinter::MultiColumns::beginCompare()
{
  compareColumns = new QString[count];
}

void MultiModelPrinter::MultiColumns::endCompare()
{
  for (int i=0; i<count; i++) {
    QString style = "";
    if (i==0 && count>1 && compareColumns[0]!=compareColumns[1])
      style = "mpc-diff1";
    else if (i>0 && compareColumns[i]!=compareColumns[0])
      style = "mpc-diff2";
    if (style!="")
      columns[i].append(QString("<span class='%1'>%2</span>").arg(style).arg(compareColumns[i]));
    else
      columns[i].append(compareColumns[i]);
  }
  delete[] compareColumns;
  compareColumns = NULL;
}

template <class T>
void MultiModelPrinter::MultiColumns::append(int idx, T val)
{
  append(idx, QString("%1").arg(val));
}

QString MultiModelPrinter::MultiColumns::print()
{
  QString result = "<tr>";
  for (int i=0; i<count; i++) {
    result.append(QString("<td width='%1%'>%2</td>").arg(100.0/count).arg(columns[i]));
  }
  result.append("</tr>");
  return result;
}

bool MultiModelPrinter::MultiColumns::isEmpty()
{
  for (int i=0; i<count; i++) {
    if (!columns[i].isEmpty())
      return false;
  }
  return true;
}

void MultiModelPrinter::MultiColumns::appendLineBreak()
{
  append("<br/>");
}

void MultiModelPrinter::MultiColumns::appendSectionTableStart()
{
  append("<table cellspacing='0' cellpadding='1' width='100%' border='0' style='border-collapse:collapse'>");
}

void MultiModelPrinter::MultiColumns::appendTableEnd()
{
  append("</table>");
}

void MultiModelPrinter::MultiColumns::appendRowStart(const QString & title, const unsigned int titlewidth)
{
  append("<tr>");
  if (!(title.isEmpty()) || (titlewidth))
    appendLabelCell(QString(title), titlewidth);
}

void MultiModelPrinter::MultiColumns::appendRowEnd()
{
  append("</tr>");
}

void MultiModelPrinter::MultiColumns::appendCellStart(const unsigned int width, const bool bold)
{
  QString str = "<td";
  if (width)
    str.append(QString(" width='%1%'").arg(QString::number(width)));
  str.append(">");
  str.append(bold ? "<b>" : "");
  append(str);
}

void MultiModelPrinter::MultiColumns::appendCellEnd(const bool bold)
{
  QString str;
  str.append(bold ? "</b>" : "");
  str.append("</td>");
  append(str);
}

void MultiModelPrinter::MultiColumns::appendLabelCell(const QString & str, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableCell(str, width, align, color, true));
}

void MultiModelPrinter::MultiColumns::appendValueCell(const QString & str, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableCell(str, width, align, color, false));
}

void MultiModelPrinter::MultiColumns::appendRow(const QStringList & strl, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableRow(strl, width, align, color, false));
}

void MultiModelPrinter::MultiColumns::appendRowHeader(const QStringList & strl, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableRow(strl, width, align, color, true));
}

void MultiModelPrinter::MultiColumns::appendRowBlank()
{
  append(doTableBlankRow());
}

void MultiModelPrinter::MultiColumns::appendFieldLabel(const QString & lbl)
{
  if (!lbl.isEmpty())
    appendTitle(QString("%1:").arg(lbl));
}

void MultiModelPrinter::MultiColumns::appendFieldSeparator(const bool sep)
{
  if (sep)
    append(";&nbsp;");
}

#define COMPARE(what) \
  columns.beginCompare(); \
  for (int cc=0; cc < modelPrinterMap.size(); cc++) { \
    ModelPrinter * modelPrinter = modelPrinterMap.value(cc).second; \
    const ModelData * model = modelPrinterMap.value(cc).first; \
    (void)(model); (void)(modelPrinter); \
    columns.append(cc, (what)); \
  } \
  columns.endCompare();

#define COMPARECELL(what) \
  columns.appendCellStart(); \
  COMPARE(what); \
  columns.appendCellEnd();

#define COMPARECELLWIDTH(what, width) \
  columns.appendCellStart(width); \
  COMPARE(what); \
  columns.appendCellEnd();

#define LABELCOMPARECELL(lbl, what, width) \
  columns.appendCellStart(width); \
  columns.appendFieldLabel(lbl); \
  COMPARE(what); \
  columns.appendCellEnd();

#define ROWLABELCOMPARECELL(lbl, lblw, what, width) \
  columns.appendRowStart(lbl, lblw); \
  columns.appendCellStart(width); \
  COMPARE(what); \
  columns.appendCellEnd(); \
  columns.appendRowEnd();

#define COMPARESTRING(lbl, what, sep) \
  columns.appendFieldLabel(lbl); \
  COMPARE(what); \
  columns.appendFieldSeparator(sep);

QString MultiModelPrinter::printTitle(const QString & label)
{
  return QString("<tr><td class=mpc-section-title colspan='%1'>").arg(modelPrinterMap.count()) + label + "</td></tr>";
}

MultiModelPrinter::MultiModelPrinter(Firmware * firmware):
  firmware(firmware)
{
}

MultiModelPrinter::~MultiModelPrinter()
{
  clearModels();
}

void MultiModelPrinter::setModel(int idx, const ModelData * model, const GeneralSettings * generalSettings)
{
  if (modelPrinterMap.contains(idx) && modelPrinterMap.value(idx).second) {
    // free existing model printer
    delete modelPrinterMap.value(idx).second;
    modelPrinterMap[idx].second = NULL;
  }

  QPair<const ModelData *, ModelPrinter *> pair(model, new ModelPrinter(firmware, *generalSettings, *model));
  modelPrinterMap.insert(idx, pair);  // QMap.insert will replace any existing key
}

void MultiModelPrinter::setModel(int idx, const ModelData * model)
{
  setModel(idx, model, &defaultSettings);
}

void MultiModelPrinter::clearModels()
{
  for(int i=0; i < modelPrinterMap.size(); i++) {
    if (modelPrinterMap.value(i).second)
      delete modelPrinterMap.value(i).second;
  }
  modelPrinterMap.clear();
}

QString MultiModelPrinter::print(QTextDocument * document)
{
  if (document) document->clear();
  Stylesheet css(MODEL_PRINT_CSS);
  if (css.load(Stylesheet::StyleType::STYLE_TYPE_EFFECTIVE))
    document->setDefaultStyleSheet(css.text());
  QString str = "<table cellspacing='0' cellpadding='3' width='100%'>";   // attributes not settable via QT stylesheet
  str.append(printSetup());
  if (firmware->getCapability(HasDisplayText))
    str.append(printChecklist());
  if (firmware->getCapability(Timers)) {
    str.append(printTimers());
  }
  if (Boards::getCapability(firmware->getBoard(), Board::FunctionSwitches)) {
    str.append(printFunctionSwitches());
  }
  
  str.append(printModules());
  if (firmware->getCapability(Heli))
    str.append(printHeliSetup());
  if (firmware->getCapability(FlightModes))
    str.append(printFlightModes());
  str.append(printInputs());
  str.append(printMixers());
  str.append(printOutputs());
  str.append(printCurves(document));
  if (firmware->getCapability(Gvars) && !firmware->getCapability(GvarsFlightModes))
    str.append(printGvars());
  str.append(printLogicalSwitches());
  if (firmware->getCapability(GlobalFunctions))
    str.append(printGlobalFunctions());
  str.append(printSpecialFunctions());
  if (firmware->getCapability(Telemetry)) {
    str.append(printTelemetry());
    str.append(printSensors());
    if (firmware->getCapability(TelemetryCustomScreens)) {
      str.append(printTelemetryScreens());
    }
  }
  str.append("</table>");
  return str;
}

QString MultiModelPrinter::printSetup()
{
  QString str = printTitle(tr("General"));

  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  ROWLABELCOMPARECELL(tr("Name"), 20, model->name, 80);
  if (!IS_FAMILY_HORUS_OR_T16(firmware->getBoard())) {
    ROWLABELCOMPARECELL(tr("EEprom Size"), 0, modelPrinter->printEEpromSize(), 0);
  }
  if (firmware->getCapability(HasModelImage)) {
    ROWLABELCOMPARECELL(tr("Model Image"), 0, model->bitmap, 0);
  }
  ROWLABELCOMPARECELL(tr("Throttle"), 0, modelPrinter->printThrottle(), 0);
  ROWLABELCOMPARECELL(tr("Trims"), 0, modelPrinter->printSettingsTrim(), 0);
  ROWLABELCOMPARECELL(tr("Center Beep"), 0, modelPrinter->printCenterBeep(), 0);
  ROWLABELCOMPARECELL(tr("Switch Warnings"), 0, modelPrinter->printSwitchWarnings(), 0);
  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    ROWLABELCOMPARECELL(tr("Pot Warnings"), 0, modelPrinter->printPotWarnings(), 0);
  }
  ROWLABELCOMPARECELL(tr("Other"), 0, modelPrinter->printSettingsOther(), 0);
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printTimers()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  columns.appendRowHeader(QStringList() << tr("Timers") << tr("Time") << tr("Mode") << tr("Switch") << tr("Countdown") << tr("Start") << tr("Min.call") << tr("Persist"));

  for (int i=0; i<firmware->getCapability(Timers); i++) {
    columns.appendRowStart();
    columns.appendCellStart(20, true);
    COMPARE(model->timers[i].nameToString(i));
    columns.appendCellEnd(true);
    COMPARECELLWIDTH(model->timers[i].valToString(), 10);
    COMPARECELLWIDTH(model->timers[i].modeToString(), 10);
    COMPARECELLWIDTH(modelPrinter->printFlightModeSwitch(model->timers[i].swtch), 5);
    COMPARECELLWIDTH(model->timers[i].countdownBeepToString(), 10);
    COMPARECELLWIDTH(model->timers[i].countdownStartToString(), 10);
    COMPARECELLWIDTH(DataHelpers::boolToString(model->timers[i].minuteBeep, DataHelpers::BOOL_FMT_YESNO), 10);
    COMPARECELLWIDTH(model->timers[i].persistentToString(false), 10);
    COMPARECELLWIDTH(model->timers[i].showElapsedToString(), 12);
    columns.appendRowEnd();
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printModules()
{
  QString str = printTitle(tr("Modules"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(NumModules); i++) {
    columns.appendRowStart();
    columns.appendCellStart(20, true);
    COMPARE(modelPrinter->printModuleType(i));
    columns.appendCellEnd(true);
    COMPARECELLWIDTH(modelPrinter->printModule(i), 80);
    columns.appendRowEnd();
  }
  if (firmware->getCapability(ModelTrainerEnable))
    columns.appendRowStart(tr("Trainer port"));
    COMPARECELL(modelPrinter->printModule(-1));
    columns.appendRowEnd();
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printHeliSetup()
{
  bool heliEnabled = false;
  for (int k=0; k < modelPrinterMap.size(); k++) {
    heliEnabled =  heliEnabled || modelPrinterMap.value(k).first->swashRingData.type != HELI_SWASH_TYPE_NONE;
  }

  if (!heliEnabled)
    return "";

  QString str = printTitle(tr("Helicopter"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  columns.appendRowStart(tr("Swash"), 20);
  LABELCOMPARECELL(tr("Type"), modelPrinter->printHeliSwashType(), 20);
  LABELCOMPARECELL(tr("Ring"), model->swashRingData.value, 20);
  columns.appendRowEnd();

  columns.appendRowBlank();
  columns.appendRowHeader(QStringList() << "" << tr("Input") << tr("Weight"));
  columns.appendRowStart(tr("Long. cyc"));
  COMPARECELL(model->swashRingData.elevatorSource.toString(model));
  COMPARECELL(model->swashRingData.elevatorWeight);
  columns.appendRowEnd();

  columns.appendRowStart(tr("Lateral cyc"));
  COMPARECELL(model->swashRingData.aileronSource.toString(model));
  COMPARECELL(model->swashRingData.aileronWeight)
  columns.appendRowEnd();

  columns.appendRowStart(tr("Collective"));
  COMPARECELL(model->swashRingData.collectiveSource.toString(model));
  COMPARECELL(model->swashRingData.collectiveWeight)
  columns.appendRowEnd();

  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printFlightModes()
{
  QString str = printTitle(tr("Flight modes"));
  // Trims
  {
    MultiColumns columns(modelPrinterMap.size());
    columns.appendSectionTableStart();
    QStringList hd = QStringList() << tr("Flight mode") << tr("Switch") << tr("F.In") << tr("F.Out");
    for (int i=0; i < getBoardCapability(getCurrentBoard(), Board::NumTrims); i++) {
      hd << RawSource(SOURCE_TYPE_TRIM, i).toString();
    }
    columns.appendRowHeader(hd);
    int wd = 80/(getBoardCapability(getCurrentBoard(), Board::NumTrims) + 3);
    for (int i=0; i<firmware->getCapability(FlightModes); i++) {
      columns.appendRowStart();
      columns.appendCellStart(20,true);
      COMPARE(modelPrinter->printFlightModeName(i));
      columns.appendCellEnd(true);
      COMPARECELLWIDTH(modelPrinter->printFlightModeSwitch(model->flightModeData[i].swtch), wd);
      COMPARECELLWIDTH(model->flightModeData[i].fadeIn, wd);
      COMPARECELLWIDTH(model->flightModeData[i].fadeOut, wd);
      for (int k=0; k < getBoardCapability(getCurrentBoard(), Board::NumTrims); k++) {
        COMPARECELLWIDTH(modelPrinter->printTrim(i, k), wd);
      }
      columns.appendRowEnd();
    }

    columns.appendTableEnd();
    str.append(columns.print());
  }

  // GVars and Rotary Encoders
  int gvars = firmware->getCapability(Gvars);
  if ((gvars && firmware->getCapability(GvarsFlightModes)) || firmware->getCapability(RotaryEncoders)) {
    MultiColumns columns(modelPrinterMap.size());
    columns.appendSectionTableStart();
    QStringList hd = QStringList() << tr("Global vars");
    if (firmware->getCapability(GvarsFlightModes)) {
      for (int i=0; i<gvars; i++) {
        hd << tr("GV%1").arg(i+1);
      }
    }
    for (int i=0; i<firmware->getCapability(RotaryEncoders); i++) {
      hd << tr("RE%1").arg(i+1);
    }
    columns.appendRowHeader(hd);
    int wd = 80/gvars;
    if (firmware->getCapability(GvarsFlightModes)) {
      columns.appendRowStart(tr("Name"), 20);
      for (int i=0; i<gvars; i++) {
        COMPARECELLWIDTH(model->gvarData[i].name, wd);
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Unit"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarUnit(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Prec"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarPrec(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Min"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarMin(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Max"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarMax(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Popup"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarPopup(i));
      }
      columns.appendRowEnd();
    }

    columns.appendRowHeader(QStringList() << tr("Flight mode"));

    for (int i=0; i<firmware->getCapability(FlightModes); i++) {
      columns.appendRowStart();
      columns.appendCellStart(0,true);
      COMPARE(modelPrinter->printFlightModeName(i));
      columns.appendCellEnd(true);
      if (firmware->getCapability(GvarsFlightModes)) {
        for (int k=0; k<gvars; k++) {
          COMPARECELL(modelPrinter->printGlobalVar(i, k));
        }
      }
      for (int k=0; k<firmware->getCapability(RotaryEncoders); k++) {
        COMPARECELL(modelPrinter->printRotaryEncoder(i, k));
      }
      columns.appendRowEnd();
    }
    columns.appendTableEnd();
    str.append(columns.print());
  }

  return str;
}

QString MultiModelPrinter::printOutputs()
{
  QString str = printTitle(tr("Outputs"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  QStringList hd = QStringList() << tr("Channel") << tr("Subtrim") << tr("Min") << tr("Max") << tr("Direct");
  if (IS_HORUS_OR_TARANIS(firmware->getBoard()))
    hd << tr("Curve");
  if (firmware->getCapability(PPMCenter))
    hd << tr("PPM");
  if (firmware->getCapability(SYMLimits))
    hd << tr("Linear");
  columns.appendRowHeader(hd);
  int cols = 4;
  if (IS_HORUS_OR_TARANIS(firmware->getBoard()))
    cols++;
  if (firmware->getCapability(PPMCenter))
    cols++;
  if (firmware->getCapability(SYMLimits))
    cols++;
  int wd = 80/cols;
  for (int i=0; i<firmware->getCapability(Outputs); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++)
      count = std::max(count, modelPrinterMap.value(k).first->mixes(i).size());
    if (!count)
      continue;
    columns.appendRowStart();
    columns.appendCellStart(20, true);
    COMPARE(modelPrinter->printChannelName(i));
    columns.appendCellEnd(true);
    COMPARECELLWIDTH(modelPrinter->printOutputOffset(i), wd);
    COMPARECELLWIDTH(modelPrinter->printOutputMin(i), wd);
    COMPARECELLWIDTH(modelPrinter->printOutputMax(i), wd);
    COMPARECELLWIDTH(modelPrinter->printOutputRevert(i), wd);
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      COMPARECELLWIDTH(modelPrinter->printOutputCurve(i), wd);
    }
    if (firmware->getCapability(PPMCenter)) {
      COMPARECELLWIDTH(modelPrinter->printOutputPpmCenter(i), wd);
    }
    if (firmware->getCapability(SYMLimits)) {
      COMPARECELLWIDTH(modelPrinter->printOutputSymetrical(i), wd);
    }
    columns.appendRowEnd();
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printGvars()
{
  QString str = printTitle(tr("Global Variables"));
  int gvars = firmware->getCapability(Gvars);
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  QStringList hd;
  for (int i=0; i<gvars; i++) {
    hd << tr("GV%1").arg(i+1);
  }
  columns.appendRowHeader(hd);

  for (int i=0; i<gvars; i++) {
    COMPARECELL(model->flightModeData[0].gvars[i]);
  }
  columns.appendRowEnd();
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printInputs()
{
  QString str = printTitle(tr("Inputs"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<std::max(4, firmware->getCapability(VirtualInputs)); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      count = std::max(count, modelPrinterMap.value(k).first->expos(i).size());
    }
    if (count > 0) {
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printInputName(i));
      columns.appendCellEnd(true);
      columns.appendCellStart(80);
      for (int j=0; j<count; j++) {
        if (j > 0)
          columns.appendLineBreak();
        COMPARE(j<model->expos(i).size() ? modelPrinter->printInputLine(*model->expos(i)[j]) : "");
      }
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printMixers()
{
  QString str = printTitle(tr("Mixers"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(Outputs); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      count = std::max(count, modelPrinterMap.value(k).first->mixes(i).size());
    }
    if (count > 0) {
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printChannelName(i));
      columns.appendCellEnd(true);
      columns.appendCellStart(80);
      for (int j=0; j<count; j++) {
        if (j > 0)
          columns.appendLineBreak();
        COMPARE((j < model->mixes(i).size()) ? modelPrinter->printMixerLine(*model->mixes(i)[j], (j>0)) : "&nbsp;");
      }
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printCurves(QTextDocument * document)
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(NumCurves); i++) {
    bool curveEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->curves[i].isEmpty()) {
        curveEmpty = false;
        break;
      }
    }
    if (!curveEmpty) {
      count++;
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printCurveName(i));
      columns.appendCellEnd(true);
      COMPARECELL(modelPrinter->printCurve(i));
      columns.appendRowEnd();
      columns.appendRowStart("", 20);
      columns.appendCellStart();
      for (int k=0; k < modelPrinterMap.size(); k++)
        columns.append(k, QString("<br/><img src='%1' border='0' /><br/>").arg(modelPrinterMap.value(k).second->createCurveImage(i, document)));
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Curves")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printLogicalSwitches()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  int count = 0;
  for (int i=0; i<firmware->getCapability(LogicalSwitches); i++) {
    bool lsEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->logicalSw[i].isEmpty()) {
        lsEmpty = false;
        break;
      }
    }
    if (!lsEmpty) {
      count++;
      columns.appendRowStart(tr("L%1").arg(i+1), 20);
      COMPARECELL(modelPrinter->printLogicalSwitchLine(i));
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Logical Switches")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printSpecialFunctions()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i < firmware->getCapability(CustomFunctions); i++) {
    bool sfEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->customFn[i].isEmpty()) {
        sfEmpty = false;
        break;
      }
    }
    if (!sfEmpty) {
      count++;
      columns.appendRowStart(tr("SF%1").arg(i+1), 20);
      COMPARECELL(modelPrinter->printCustomFunctionLine(i));
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Special Functions")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printTelemetry()
{
  QString str = printTitle(tr("Telemetry"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();

  // Protocol
  columns.appendRowStart(tr("Protocol"), 20);
  COMPARECELL(modelPrinter->printTelemetryProtocol(model->telemetryProtocol));
  columns.appendRowEnd();

  // RSSI alarms
  {
    columns.appendRowStart(tr("RSSI Alarms"), 20);
    columns.appendCellStart(80);
    COMPARESTRING("", tr("Low"), false);
    columns.append(": ");
    COMPARESTRING("", QString("&lt; %1").arg(QString::number(model->rssiAlarms.warning, 10)), true);
    COMPARESTRING("", tr("Critical"), false);
    columns.append(": ");
    COMPARESTRING("", QString("&lt; %1").arg(QString::number(model->rssiAlarms.critical, 10)), true);
    COMPARESTRING(tr("Telemetry audio"), modelPrinter->printRssiAlarmsDisabled(model->rssiAlarms.disabled), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  // Altimetry
  if (firmware->getCapability(HasVario)) {
    columns.appendRowStart(tr("Altimetry"), 20);
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      LABELCOMPARECELL(tr("Vario source"), SensorData::sourceToString(model, model->frsky.varioSource), 80);
    }
    else {
      LABELCOMPARECELL(tr("Vario source"), modelPrinter->printVarioSource(model->frsky.varioSource), 80);
    }
    columns.appendRowEnd();
    columns.appendRowStart("", 20);
    columns.appendCellStart(80);
    columns.appendTitle(tr("Vario limits >"));
    if (firmware->getCapability(HasVarioSink)) {
      COMPARESTRING(tr("Sink max"), QString::number(model->frsky.varioMin - 10), true);
      COMPARESTRING(tr("Sink min"), QString::number((float(model->frsky.varioCenterMin) / 10.0) - 0.5), true);
    }
    COMPARESTRING(tr("Climb min"), QString::number((float(model->frsky.varioCenterMax) / 10.0) + 0.5), true);
    COMPARESTRING(tr("Climb max"), QString::number(model->frsky.varioMax + 10), true);
    COMPARESTRING(tr("Center silent"), modelPrinter->printVarioCenterSilent(model->frsky.varioCenterSilent), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  // Top Bar
  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    columns.appendRowStart(tr("Top Bar"), 20);
    columns.appendCellStart(80);
    COMPARESTRING(tr("Volts source"), SensorData::sourceToString(model, model->frsky.voltsSource), true);
    COMPARESTRING(tr("Altitude source"), SensorData::sourceToString(model, model->frsky.altitudeSource), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  ROWLABELCOMPARECELL("Multi sensors", 0, modelPrinter->printIgnoreSensorIds(!model->frsky.ignoreSensorIds), 0);

  // Various
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printSensors()
{
  MultiColumns columns(modelPrinterMap.size());
  QString str;
  int count = 0;
  columns.appendSectionTableStart();
  columns.appendRowHeader(QStringList() << tr("Name") << tr("Type") << tr("Parameters"));
  for (unsigned i = 0; i < CPN_MAX_SENSORS; ++i) {
    bool tsEmpty = true;
    for (int k = 0; k < modelPrinterMap.size(); k++) {
      if (modelPrinterMap.value(k).first->sensorData[i].isAvailable()) {
        tsEmpty = false;
        break;
      }
    }
    if (!tsEmpty) {
      count++;
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(model->sensorData[i].nameToString(i));
      columns.appendCellEnd(true);
      COMPARECELLWIDTH(model->sensorData[i].isAvailable() ? model->sensorData[i].typeToString() : "", 15);
      COMPARECELLWIDTH(model->sensorData[i].paramsToString(model), 65);
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Telemetry Sensors")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printTelemetryScreens()
{
  MultiColumns columns(modelPrinterMap.size());
  QString str;
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(TelemetryCustomScreens); ++i) {
    bool tsEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!(modelPrinterMap.value(k).first->frsky.screens[i].type == TELEMETRY_SCREEN_NONE)) {
        tsEmpty = false;
        break;
      }
    }
    if (!tsEmpty) {
      count++;
      columns.appendRowStart();
      LABELCOMPARECELL(QString("%1").arg(i+1), modelPrinter->printTelemetryScreenType(model->frsky.screens[i].type), 20);
      columns.appendRowEnd();
      for (int l=0; l<4; l++) {
        COMPARE(modelPrinter->printTelemetryScreen(i, l, 80));
      }
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Telemetry Screens")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printGlobalFunctions()
{
  QString str;
  QString txt;
  int count = 0;
  int idx = -1;
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();

  for (int k=0; k < modelPrinterMap.size(); k++) {
    if (!modelPrinterMap.value(k).first->noGlobalFunctions) {
      idx = k;
      break;
    }
  }

  if (idx > -1) {
    ModelPrinter * modelPrinter = modelPrinterMap.value(idx).second;
    (void)(modelPrinter);

    for (int i=0; i < firmware->getCapability(GlobalFunctions); i++) {
      txt = modelPrinter->printCustomFunctionLine(i, true);
      if (!txt.isEmpty()) {
        count++;
        ROWLABELCOMPARECELL(tr("GF%1").arg(i+1), 20, modelPrinter->printCustomFunctionLine(i, true), 80);
      }
    }
    columns.appendTableEnd();
    if (count > 0) {
      str.append(printTitle(tr("Global Functions")));
      str.append(columns.print());
    }
  }
  return str;
}

QString MultiModelPrinter::printChecklist()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  bool isChecklist = false;
  for (int k=0; k < modelPrinterMap.size(); k++) {
    if (modelPrinterMap.value(k).first->displayChecklist) {
      isChecklist = true;
      break;
    }
  }
  if (isChecklist) {
    columns.appendSectionTableStart();
    ROWLABELCOMPARECELL(tr("Checklist"), 20, modelPrinter->printChecklist(), 80);
    columns.appendTableEnd();
    str.append(columns.print());
  }
  return str;
}


 QString MultiModelPrinter::printFunctionSwitches()
 {
   QString str;
   MultiColumns columns(modelPrinterMap.size());
   columns.appendSectionTableStart();

   int numFS = Boards::getCapability(firmware->getBoard(), Board::FunctionSwitches);
   int colwidth = 80 / numFS;

   QStringList headings = { tr("Function Switches") };
   for (int i = 0; i < numFS; i++) {
     headings << tr("Switch %1").arg(i + 1);
   }
   columns.appendRowHeader(headings);

   columns.appendRowStart(tr("Name"), 20);

   for (int i = 0; i < numFS; i++) {
     COMPARECELLWIDTH(model->functionSwitchNames[i], colwidth);
   }

   columns.appendRowEnd();
   columns.appendRowStart(tr("Type"), 20);

   for (int i = 0; i < numFS; i++) {
     COMPARECELLWIDTH(model->funcSwitchConfigToString(model->getFuncSwitchConfig((unsigned int)i)), colwidth);
   }

   columns.appendRowEnd();
   columns.appendRowStart(tr("Start"), 20);

   for (int i = 0; i < numFS; i++) {
     COMPARECELLWIDTH(model->funcSwitchStartToString(model->getFuncSwitchStart((unsigned int)i)), colwidth);
   }

   columns.appendRowEnd();
   columns.appendRowStart(tr("Group"), 20);

   for (int i = 0; i < numFS; i++) {
     COMPARECELLWIDTH(model->getFuncSwitchGroup((unsigned int)i), colwidth);
   }

   columns.appendRowEnd();
   columns.appendRowStart(tr("Always On"), 20);

   for (int i = 0; i < numFS; i++) {
     COMPARECELLWIDTH((model->getFuncSwitchAlwaysOnGroup((unsigned int)i) == 0 ? "No" : "Yes"), colwidth);
   }

   columns.appendRowEnd();

   columns.appendTableEnd();
   str.append(columns.print());
   return str;
 }
