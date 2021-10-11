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

#pragma once

#include <QString>
#include <QStringList>
#include <QPainter>
#include <QTextDocument>
#include "eeprominterface.h"

QString changeColor(const QString & input, const QString & to, const QString & from = "grey");
QString addFont(const QString & input, const QString & color = "", const QString & size = "", const QString & face = "");
void debugHtml(const QString & html);
QString formatTitle(const QString & name);

class CurveImage
{
  public:
    CurveImage();
    void drawCurve(const CurveData & curve, QColor color);
    const QImage & get() const { return image; }

  protected:
    int size;
    QImage image;
    QPainter painter;
};

class ModelPrinter: public QObject
{
  Q_OBJECT

  public:
    ModelPrinter(Firmware * firmware, const GeneralSettings & generalSettings, const ModelData & model);
    virtual ~ModelPrinter();

    QString printBoolean(const bool val, const int typ);
    QString printEEpromSize();
    QString printTrimIncrementMode();
    QString printThrottle();
    QString printFlightModeSwitch(const RawSwitch & swtch);
    QString printFlightModeName(int index);
    QString printFlightModes(unsigned int flightModes);
    QString printInputFlightModes(unsigned int flightModes);
    QString printModule(int idx);
    QString printCenterBeep();
    QString printHeliSwashType();
    QString printTrim(int flightModeIndex, int stickIndex);
    QString printGlobalVar(int flightModeIndex, int gvarIndex);
    QString printRotaryEncoder(int flightModeIndex, int reIndex);
    QString printInputName(int idx);
    QString printInputLine(int idx);
    QString printInputLine(const ExpoData & ed);
    QString printMixerLine(const MixData & md, bool showMultiplex, int highlightedSource = 0);
    QString printLogicalSwitchLine(int idx);
    QString printCustomFunctionLine(int idx, bool gfunc = false);
    QString printChannelName(int idx);
    QString printCurveName(int idx);
    QString printCurve(int idx);
    QString createCurveImage(int idx, QTextDocument * document);
    QString printGlobalVarUnit(int idx);
    QString printGlobalVarPrec(int idx);
    QString printGlobalVarMin(int idx);
    QString printGlobalVarMax(int idx);
    QString printGlobalVarPopup(int idx);
    QString printOutputValueGVar(int val);
    QString printOutputOffset(int idx);
    QString printOutputMin(int idx);
    QString printOutputMax(int idx);
    QString printOutputRevert(int idx);
    QString printOutputCurve(int idx);
    QString printOutputPpmCenter(int idx);
    QString printOutputSymetrical(int idx);
    QString printSettingsOther();
    QString printModuleType(int idx);
    QString printThrottleSource(int idx);
    QString printTrimsDisplayMode();
    QString printSettingsTrim();
    QString printSwitchWarnings();
    QString printPotWarnings();
    QString printPotsWarningMode();
    QString printPxxPower(int power);
    QString printFailsafe(int idx);
    QString printFailsafeMode(unsigned int fsmode);
    QString printFailsafeValue(int val);
    QString printPPMFrameLength(int ppmFL);
    QString printTimeValue(const int value, const unsigned int mask);
    QString printTelemetryProtocol(unsigned int val);
    QString printLabelValue(const QString & lbl, const QString & val, const bool sep = false);
    QString printLabelValues(const QString & lbl, const QStringList & vals, const bool sep = false);
    QString printRssiAlarmsDisabled(bool mb);
    QString printVarioSource(unsigned int val);
    QString printVarioCenterSilent(bool mb);
    QString printVoltsSource(unsigned int val);
    QString printCurrentSource(unsigned int val);
    QString printMahPersistent(bool mb);
    QString printIgnoreSensorIds(bool mb);
    QString printTelemetryScreenType(unsigned int val);
    QString printTelemetryScreen(unsigned int idx, unsigned int line, unsigned int width);
    QString printChecklist();

  private:
    Firmware * firmware;
    const GeneralSettings & generalSettings;
    const ModelData & model;

};
