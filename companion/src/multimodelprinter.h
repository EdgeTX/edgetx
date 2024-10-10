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

#ifndef _MULTIMODELPRINTER_H_
#define _MULTIMODELPRINTER_H_

#include <QObject>
#include <QTextDocument>
#include "eeprominterface.h"
#include "modelprinter.h"

class MultiModelPrinter: public QObject
{
  Q_OBJECT

  public:
    MultiModelPrinter(Firmware * firmware);
    virtual ~MultiModelPrinter();

    void setModel(int idx, const ModelData * model, const GeneralSettings * generalSettings);
    void setModel(int idx, const ModelData * model);
    void clearModels();
    QString print(QTextDocument * document);

  protected:
    class MultiColumns {
      public:
        MultiColumns(int count);
        ~MultiColumns();
        bool isEmpty();
        QString print();
        void append(const QString & str);
        void appendTitle(const QString & name);
        void append(int idx, const QString & str);
        template <class T> void append(int idx, T val);
        void beginCompare();
        void endCompare();
        void appendLineBreak();
        void appendSectionTableStart();
        void appendTableEnd();
        void appendRowStart(const QString & title = "", const unsigned int titlewidth = 0);
        void appendRowEnd();
        void appendCellStart(const unsigned int width = 0, const bool bold = false);
        void appendCellEnd(const bool bold = false);
        void appendLabelCell(const QString & str, const unsigned int width = 0, const QString & align = "left", const QString & color = "");
        void appendValueCell(const QString & str, const unsigned int width = 0, const QString & align = "left", const QString & color = "");
        void appendRow(const QStringList & strl, const unsigned int width = 0, const QString & align = "left", const QString & color = "");
        void appendRowHeader(const QStringList & strl, const unsigned int width = 0, const QString & align = "left", const QString & color = "");
        void appendRowBlank();
        void appendFieldLabel(const QString & lbl = "");
        void appendFieldSeparator(const bool sep = false);

      private:
        int count;
        QString * columns;
        QString * compareColumns;
    };

    Firmware * firmware;
    GeneralSettings defaultSettings;
    QMap<int, QPair<const ModelData *, ModelPrinter *> > modelPrinterMap;

    QString printTitle(const QString & label);
    QString printSetup();
    QString printModules();
    QString printHeliSetup();
    QString printFlightModes();
    QString printOutputs();
    QString printInputs();
    QString printMixers();
    QString printCurves(QTextDocument * document);
    QString printGvars();
    QString printLogicalSwitches();
    QString printSpecialFunctions();
    QString printTelemetry();
    QString printTimers();
    QString printSensors();
    QString printTelemetryScreens();
    QString printGlobalFunctions();
    QString printChecklist();
    QString printFunctionSwitches();
};

#endif // _MULTIMODELPRINTER_H_
