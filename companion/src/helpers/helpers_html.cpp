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
#include "helpers_html.h"
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QTextStream>

QString tdAlign(const QString & s, const QString & align, const QString & color, bool bold)
{
  QString str = s;
  if (bold) str = "<b>" + str + "</b>";
  if (!color.isEmpty()) str = "<font color='" + color + "'>" + str + "</font>";
  return "<td align='" + align + "'>" + str + "</td>";
}

QString doTC(const QString & s, const QString & color, bool bold)
{
  return tdAlign(s, "center", color, bold);
}

QString doTR(const QString & s, const QString & color, bool bold)
{
  return tdAlign(s, "right", color, bold);
}

QString doTL(const QString & s, const QString & color, bool bold)
{
  return tdAlign(s, "left", color, bold);
}

QString fv(const QString & name, const QString & value, const QString & color)
{
  return "<b>" + name + ": </b><font color='" + color + "'>" + value + "</font><br>";
}

QString doTableCell(const QString & s, const unsigned int width, const QString & align, const QString & color, bool bold)
{
  QString prfx = "<td";
  if (width)
    prfx.append(QString(" width='%1%'").arg(QString::number(width)));
  if (!align.isEmpty())
    prfx.append(QString(" align='%1'").arg(align));
  prfx.append(">");

  QString str = s;
  if (bold)
    str = "<b>" + str + "</b>";
  if (!color.isEmpty())
    str = QString("<font color='%1'>%2</font>").arg(color).arg(str);

  str =  prfx + str + "</td>";
  return str;
}

QString doTableRow(const QStringList & strl, const unsigned int width, const QString & align, const QString & color, bool bold)
{
  QString str;
  for (int i=0; i<strl.count(); i++) {
    str.append(doTableCell(strl.at(i), width, align, color, bold));
  }
  return "<tr>" + str + "</tr>";
}

QString doTableBlankRow()
{
  return "<tr></tr>";
}

Stylesheet::Stylesheet(const QString & name):
  mName(name)
{
  init();
}

Stylesheet::Stylesheet(const QString & name, const StyleType styleType):
  mName(name)
{
  init();
  mResult = load(styleType);
}

Stylesheet::~Stylesheet()
{
}

void Stylesheet::init()
{
  mResult = false;
  mErrormsg = "";
  mText = "";
  QStringList p = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
  mCustomPath = p[0];
  mCustomFile = p[0] + "/" + mName;
  mDefaultFile = THEMES_DEFAULT_PATH + mName;
}

bool Stylesheet::load(const StyleType styleType)
{
  mResult = false;
  switch (styleType) {
  case STYLE_TYPE_NONE:
  case STYLE_TYPE_DEFAULT:
    mResult = read(mDefaultFile);
  case STYLE_TYPE_CUSTOM:
    mResult = read(mCustomFile);
  case STYLE_TYPE_EFFECTIVE:
    mResult = read(mCustomFile);
    if(!mResult)
      mResult = read(mDefaultFile);
  }
  return mResult;
}

bool Stylesheet::read(const QString & path)
{
  mResult = false;
  mText = "";

  QFile file(path);
  if (file.exists()) {
    if (file.open(QFile::ReadOnly | QFile::Text)) {
      QTextStream in(&file);
      if (in.status()==QTextStream::Ok) {
        mText = in.readAll();
        if (in.status()==QTextStream::Ok)
          mResult = true;
        else
          mText = "";
      }
      file.close();
    }
  }
  if (mResult)
    mErrormsg = tr("Style sheet data read from '%1'").arg(QDir::toNativeSeparators(path));
  else
    mErrormsg = tr("Style sheet data unable to be read from '%1'").arg(QDir::toNativeSeparators(path));
  qDebug() << mErrormsg;
  return mResult;
}

bool Stylesheet::update()
{
  mResult = false;
  mErrormsg = "";
  QDir path(mCustomPath);
  if (!path.exists()) {
    QDir dir;
    if (!dir.mkpath(mCustomPath))
      mErrormsg = tr("Cannot create folder '%1'").arg(QDir::toNativeSeparators(mCustomPath));
  }
  if (path.exists()) {
    QFile file(mCustomFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      mErrormsg = tr("Cannot open file for writing '%1': Error: %2").arg(QDir::toNativeSeparators(mCustomFile), file.errorString());
    }
    else {
      QTextStream out(&file);
      if (out.status()==QTextStream::Ok) {
        out << mText;
        if (!(out.status()==QTextStream::Ok)) {
          mErrormsg = tr("Cannot write to file '%1': Error: %2").arg(QDir::toNativeSeparators(mCustomFile), file.errorString());
          if (!file.flush()) {
            mErrormsg = tr("Cannot flush buffer for file '%1': Error: %2").arg(QDir::toNativeSeparators(mCustomFile), file.errorString());
          }
        }
        else {
          mResult = true;
          mErrormsg = tr("Style sheet written to '%1'").arg(QDir::toNativeSeparators(mCustomFile));
        }
      }
    }
    file.close();
  }
  qDebug() << mErrormsg;
  return mResult;
}

bool Stylesheet::deleteCustom()
{
  QFile file(mCustomFile);
  mResult = file.remove();
  if (mResult)
    mErrormsg = tr("Custom style sheet deleted: '%1'").arg(QDir::toNativeSeparators(mCustomFile));
  else
    mErrormsg = tr("Unable to delete custom style sheet: '%1'").arg(QDir::toNativeSeparators(mCustomFile));
  qDebug() << mErrormsg;
  return mResult;
}

QString Stylesheet::name()
{
  return mName;
}

QString Stylesheet::text()
{
  return mText;
}

void Stylesheet::setText(const QString & text)
{
  mText = text;
}

QString Stylesheet::errormsg()
{
  return mErrormsg;
}

