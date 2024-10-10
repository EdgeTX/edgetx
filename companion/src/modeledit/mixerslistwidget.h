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

#ifndef _MIXERSLISTWIDGET_H_
#define _MIXERSLISTWIDGET_H_

#include <QListWidget>
#include <QProxyStyle>
#include <QStyleOption>
#include <QStyledItemDelegate>
#include <QTextDocument>

class MixersListWidget : public QListWidget
{
    Q_OBJECT

  public:
    explicit MixersListWidget(QWidget *parent, bool expo);
    void keyPressEvent(QKeyEvent *event);
    void dropEvent(QDropEvent *event);

  signals:
    void mimeDropped(int index, const QMimeData *data, Qt::DropAction action);
    void keyWasPressed(QKeyEvent *event);

  public slots:
    void addItem(QListWidgetItem *item, const unsigned & rowId, bool topLevel = false, bool hasSib = false);
    bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
    void zoomView();

  protected:
    virtual QStringList mimeTypes() const;

  private:
    QString itemMimeFmt;
    bool expo;

};

/**
  @brief Provides sizing and painting of items for our mixes and inputs lists

  This class draws HTML formated strings into QListWidget class
*/
class MixersDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    inline MixersDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

  private:
    void SetupDocument(QTextDocument & doc, const QStyleOptionViewItem & options, const QModelIndex & index) const;

};

/**
  @brief Provides custom painting of items on a per-element basis, eg. to change style of drop indicator
*/
class MixerItemViewProxyStyle: public QProxyStyle
{
  public:
    explicit MixerItemViewProxyStyle(QStyle * style = nullptr) : QProxyStyle(style) {}

    virtual void drawPrimitive(PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = nullptr) const override;
};

#endif // _MIXERSLISTWIDGET_H_
