#include "radioconfig.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

static QJsonObject loadRadioJson(const QString& radioName)
{
  QString base = QFileInfo(radioName.trimmed()).completeBaseName().toLower();
  if (base.startsWith("edgetx-")) {
    base = base.mid(7);
  }

  const QString targetFileName = base + ".json";
  QString foundPath;

  QDirIterator it(":/", QStringList() << "*.json", QDir::Files,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QString path = it.next();
    if (QFileInfo(path).fileName().compare(targetFileName,
                                           Qt::CaseInsensitive) == 0) {
      foundPath = path;
      break;
    }
  }

  if (foundPath.isEmpty()) {
    std::cerr << "[WARN] Hardware definition JSON not found for radio: "
              << radioName.toStdString() << "\n";
    return {};
  }

  QFile file(foundPath);
  if (!file.open(QIODevice::ReadOnly)) {
    std::cerr << "[WARN] Failed to open hardware definition: "
              << foundPath.toStdString() << "\n";
    return {};
  }

  QJsonParseError parseError;
  const QJsonDocument doc =
      QJsonDocument::fromJson(file.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    std::cerr << "[WARN] Failed to parse JSON " << foundPath.toStdString()
              << ": " << parseError.errorString().toStdString() << "\n";
    return {};
  }

  return doc.object();
}

AnalogMap getAnalogMapForRadio(const QString& radioName)
{
  AnalogMap map;
  const QJsonObject root = loadRadioJson(radioName);
  if (root.isEmpty()) {
    return map;
  }

  map.s1 = 255;

  // 1. Parse ADC inputs (STICK -> 0..3, FLEX -> pots / sliders / 6POS)
  const QJsonArray inputs =
      root.value("adc_inputs").toObject().value("inputs").toArray();

  uint8_t analogIdx = 0;
  for (const QJsonValue& val : inputs) {
    const QJsonObject obj = val.toObject();
    const QString type = obj.value("type").toString().toUpper();

    if (type == "STICK") {
      ++analogIdx;
    } else if (type == "FLEX") {
      const QString name = obj.value("name").toString().toUpper();
      const QString label = obj.value("label").toString().toUpper();
      const QString def = obj.value("default").toString().toUpper();

      if (def == "MULTIPOS" || label == "6P" || label == "6POS" ||
          name == "6P" || name == "6POS") {
        map.pos6 = analogIdx;
      } else if (def == "SLIDER" || name == "SL1" || name == "SL2" ||
                 name == "LS" || name == "RS" || label == "LS" ||
                 label == "RS" || label == "SL1" || label == "SL2") {
        if (name == "SL2" || name == "RS" || label == "SL2" || label == "RS" ||
            map.ls != 255) {
          map.rs = analogIdx;
        } else {
          map.ls = analogIdx;
        }
      } else if (name == "P1" || name == "S1" || label == "P1" ||
                 label == "S1") {
        map.s1 = analogIdx;
      } else if (name == "P2" || name == "S2" || label == "P2" ||
                 label == "S2") {
        map.s2 = analogIdx;
      }

      ++analogIdx;
    }
  }

  // 2. Parse function switches block (SW1..SW6)
  const QJsonArray switches = root.value("switches").toArray();
  for (int i = 0; i < switches.size(); ++i) {
    const QJsonObject swObj = switches.at(i).toObject();
    const QString swName = swObj.value("name").toString().toUpper();

    if (swName == "SW1") {
      map.firstFuncSwitch = static_cast<uint8_t>(i);
      break;
    }
  }

  return map;
}

KeyMap getKeyMapForRadio(const QString& radioName)
{
  KeyMap map;
  const QJsonObject root = loadRadioJson(radioName);
  if (root.isEmpty()) {
    return map;
  }

  const QJsonArray keys = root.value("keys").toArray();
  for (int i = 0; i < keys.size(); ++i) {
    const QJsonObject obj = keys.at(i).toObject();
    const QString key = obj.value("key").toString().toUpper();
    const QString name = obj.value("name").toString().toUpper();
    const QString label = obj.value("label").toString().toUpper();
    const uint8_t idx = static_cast<uint8_t>(i);

    if (key == "KEY_EXIT" || name == "EXIT" || label == "RTN") {
      map.rtn = idx;
    } else if (key == "KEY_PAGEDN" || key == "KEY_PAGE" || name == "PAGEDN" ||
               label == "PAGE>") {
      map.pageNext = idx;
    } else if (key == "KEY_PAGEUP" || name == "PAGEUP" || label == "PAGE<" ||
               label == "<PAGE") {
      map.pagePrev = idx;
    } else if (key == "KEY_SYS" || name == "SYS" || label == "SYS") {
      map.sys = idx;
    } else if (key == "KEY_TELE" || name == "TELE" || label == "TELE") {
      map.tele = idx;
    } else if (key == "KEY_MODEL" || name == "MDL" || label == "MDL") {
      map.mdl = idx;
    } else if (key == "KEY_ENTER" || name == "ENTER" || label == "ENTER" ||
               label == "ENT") {
      map.enter = idx;
    }
  }

  return map;
}

LcdSpec getLcdSpecForRadio(const QString& radioName)
{
  const QJsonObject root = loadRadioJson(radioName);
  if (root.contains("display")) {
    const QJsonObject display = root.value("display").toObject();
    const int width = display.value("lcd_w").toInt(128);
    const int height = display.value("lcd_h").toInt(64);
    const int depth = display.value("lcd_depth").toInt(1);
    return {width, height, depth};
  }

  return {128, 64, 1};
}