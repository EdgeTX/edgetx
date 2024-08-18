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

#ifndef _FIRMWAREINTERFACE_H_
#define _FIRMWAREINTERFACE_H_

#include <QtCore>
#include <QString>
#include <QImage>
#include <QByteArray>

#define SPLASH_WIDTH (128)
#define SPLASH_HEIGHT (64)
#define SPLASHX9D_WIDTH (212)
#define SPLASHX9D_HEIGHT (64)
#define SPLASH_SIZE_MAX   (SPLASHX9D_WIDTH*SPLASHX9D_HEIGHT/2)
#define ERSPLASH_MARKER "Splash"
#define ERSPLASH_OFFSET (10)
#define ERSKY9X_SPS "SPS"
#define ERSKY9X_SPE "SPE"
#define ERSKY9X_OFFSET (7)

// 3kB - width and height bytes
#define RLE_SPLASH_MAX_SIZE (3070)

class FirmwareInterface
{
  Q_DECLARE_TR_FUNCTIONS("FirmwareInterface")

  public:
    FirmwareInterface(const QString & filename, QDialog* parentDialog = nullptr);
    inline QString getDate() { return date; }
    inline QString getTime() { return time; }
    int getSize() { return flashSize; }
    QString getFlavour() const;
    bool isHardwareCompatible(const FirmwareInterface &previousFirmware) const;
    inline QString getVersion() { return version; }
    inline int getEEpromVersion() { return eepromVersion; }
    inline int getEEpromVariant() { return eepromVariant; }
    inline QString getEEpromId() { return eepromId; }
    QImage getSplash();
    bool setSplash(const QImage & newsplash);
    bool hasSplash();
    int getSplashWidth();
    uint getSplashHeight();
    QImage::Format getSplashFormat();
    unsigned int save(const QString & fileName);
    bool isValid();

  private:
    QDialog* parentDialog;
    QByteArray flash;
    uint flashSize;
    QString seekString(const QString & string);
    QString seekLabel(const QString & label);
    void seekSplash();
    bool seekSplash(QByteArray sps, QByteArray spe, int size);
    bool seekSplash(QByteArray splash);
    QString filename;
    QString date;
    QString time;
    QString flavour;
    QString version;
    QString eepromId;
    int eepromVersion;
    int eepromVariant;
    QByteArray splash;
    uint splashOffset;
    uint splashSize;
    uint splashWidth;
    uint splashHeight;
    QImage::Format splash_format;
    bool isValidFlag;
};

#endif // _FIRMWAREINTERFACE_H_
