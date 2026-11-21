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
      } else if (label == "S1" || name == "S1" ||
                 (map.s1 == 255 && (label == "P1" || name == "P1"))) {
        map.s1 = analogIdx;
      } else if (label == "S2" || name == "S2" ||
                 (map.s2 == 255 && (label == "P2" || name == "P2"))) {
        map.s2 = analogIdx;
      } else if (label == "LS" || name == "LS" || label == "SL1" ||
                 name == "SL1") {
        map.ls = analogIdx;
      } else if (label == "RS" || name == "RS" || label == "SL2" ||
                 name == "SL2") {
        map.rs = analogIdx;
      }

      ++analogIdx;
    }
  }

  // 2. Parse function switches (SW1..SW6, marked with "is_cfs": true)
  const QJsonArray switches = root.value("switches").toArray();
  for (int i = 0; i < switches.size(); ++i) {
    const QJsonObject swObj = switches.at(i).toObject();
    const bool isCfs = swObj.value("is_cfs").toBool(false);
    const QString swName = swObj.value("name").toString().toUpper();

    if (isCfs || swName.startsWith("SW")) {
      map.firstFuncSwitch = static_cast<uint8_t>(i);
      break;
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