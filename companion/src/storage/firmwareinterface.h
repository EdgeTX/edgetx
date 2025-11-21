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

#pragma once

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
    enum FirmwareType{
      FIRMWARE_TYPE_BIN,
      FIRMWARE_TYPE_UF2
    };

    struct UF2_Block {
      // 32 byte header
      uint32_t magicStart0;   // 0x0A324655 ("UF2\n")
      uint32_t magicStart1;   // 0x9E5D5157
      uint32_t flags;         // see cpp defines
      uint32_t targetAddr;    // Address in flash where the data should be written
      uint32_t payloadSize;   // Number of bytes used in data (often 256)
      uint32_t blockNo;       // Sequential block number; starts at 0
      uint32_t numBlocks;     // Total number of blocks in file
      uint32_t fileSize;      // or familyID;
      uint8_t  data[476];     // Data, padded with zeros
      uint32_t magicEnd;      // 0x0AB16F30
    };

    explicit FirmwareInterface(const QString &filename, QDialog* parent = nullptr);
    explicit FirmwareInterface(const QByteArray &flashData, QDialog* parent = nullptr);

    inline QString getDate() { return date; }
    inline QString getTime() { return time; }
    inline const QByteArray& getFlash() { return flash; }
    inline int getSize() { return flashSize; }
    inline QString getFlavour() const { return flavour; }
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
    FirmwareType getType() { return type; }
    const QString typeFileExtn() const { return (type == FIRMWARE_TYPE_UF2 ? "uf2" : "bin"); }

  private:
    QDialog* parentDialog;
    QByteArray flash;
    uint flashSize = 0;
    QString filename;
    QString date;
    QString time;
    QString flavour;
    QString version;
    QString eepromId;
    int eepromVersion = 0;
    int eepromVariant = 0;
    QByteArray splash;
    uint splashOffset = 0;
    uint splashSize = 0;
    uint splashWidth = 0;
    uint splashHeight = 0;
    QImage::Format splashFormat;
    bool isValidFlag = false;
    FirmwareType type;

    void initFlash(const QByteArray & flashData);
    QString seekString(const QString & string);
    QString seekLabel(const QString & label);
    void seekSplash();
    bool seekSplash(QByteArray sps, QByteArray spe, int size);
    bool seekSplash(QByteArray splash);
    bool concatUF2Payloads(QByteArray & flashData);
};
