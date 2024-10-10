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

#include "mixerslistwidget.h"

#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QTextFrame>

#define GRP_IS_PARENT      0x01
#define GRP_HAS_SIBLING    0x02
#define GRP_HAS_DATA       0x04

#define GroupIdRole        (Qt::UserRole+1)  // destination Input/Channel, for alternate row colors
#define GroupHeaderRole    (Qt::UserRole+2)  // bitfield of row metadata (see addItem())

static QFont defaultFont;
static bool fontInit = false;

MixersListWidget::MixersListWidget(QWidget * parent, bool expo) :
  QListWidget(parent),
  expo(expo)
{
  if (!fontInit) {
    fontInit = true;
    defaultFont = font();
    defaultFont.setFamily("Courier");
    defaultFont.setStyleHint(QFont::TypeWriter);
    defaultFont.setPointSize(defaultFont.pointSize() + 3);
  }

  setFont(defaultFont);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setDragEnabled(true);
  setAcceptDrops(true);
  setDragDropMode(QAbstractItemView::DragDrop);
  setDropIndicatorShown(true);
  setItemDelegate(new MixersDelegate(this));        // set custom paint handler
  setStyle(new MixerItemViewProxyStyle());          // custom element style painter

  itemMimeFmt = (expo ? "application/x-companion-expo-item" : "application/x-companion-mix-item");

  QAction * zin = new QAction(tr("Increase font size"), this);
  zin->setShortcut(QKeySequence::ZoomIn);
  zin->setData(1);
  addAction(zin);
  QAction * zout = new QAction(tr("Decrease font size"), this);
  zout->setShortcut(QKeySequence::ZoomOut);
  zout->setData(-1);
  addAction(zout);
  QAction * zrst = new QAction(tr("Default font size"), this);
  zrst->setShortcut(tr("Ctrl+0"));
  zrst->setData(0);
  addAction(zrst);
  connect(zin, &QAction::triggered, this, &MixersListWidget::zoomView);
  connect(zout, &QAction::triggered, this, &MixersListWidget::zoomView);
  connect(zrst, &QAction::triggered, this, &MixersListWidget::zoomView);
}

void MixersListWidget::keyPressEvent(QKeyEvent *event)
{
  emit keyWasPressed(event);
}

void MixersListWidget::addItem(QListWidgetItem * item, const unsigned & rowId, bool topLevel, bool hasSib)
{
  Qt::ItemFlags f = item->flags();
  f |= Qt::ItemIsDropEnabled;

  quint8 hdrRole = 0;
  if (topLevel)
    hdrRole |= GRP_IS_PARENT;
  if (hasSib)
    hdrRole |= GRP_HAS_SIBLING;
  if (item->data(Qt::UserRole).toByteArray().at(0) > -1)
    hdrRole |= GRP_HAS_DATA;
  else
    f &= ~Qt::ItemIsDragEnabled;  // prevent drag of empty items

  item->setData(GroupIdRole, rowId);
  item->setData(GroupHeaderRole, hdrRole);
  item->setFlags(f);

  QListWidget::addItem(item);
  //qDebug() << rowId << topLevel << hasSib << item->data(GroupHeaderRole).toUInt();
}

/**
    @brief Override to give us different mime type for mixers and inputs
*/
QStringList MixersListWidget::mimeTypes () const
{
  return QStringList() << itemMimeFmt;
}

void MixersListWidget::dropEvent(QDropEvent * event)
{
  if (event->proposedAction() == Qt::IgnoreAction)
    return;

  if (event->mimeData() && event->mimeData()->hasFormat(itemMimeFmt)) {
    if (event->source() != this) {
      // force a copy action if dragging from another window
      event->setDropAction(Qt::CopyAction);
      event->accept();
    }
    else {
      event->acceptProposedAction();
    }
    dropMimeData(indexAt(event->pos()).row(), event->mimeData(), event->dropAction());
  }
}

bool MixersListWidget::dropMimeData(int index, const QMimeData * data, Qt::DropAction action )
{
  // qDebug() << "index:" << index << action << "formats" << data->formats();
  QByteArray dropData = data->data(itemMimeFmt);
  if (dropData.isEmpty() )
    return false;

  QDataStream stream(&dropData, QIODevice::ReadOnly);
  QByteArray qba;

  while (!stream.atEnd()) {
    int r,c;
    QMap<int, QVariant> v;
    stream >> r >> c >> v;
    qba.append(v.values().at(1).toByteArray().mid(1));
  }

  if(qba.size()) {
    QMimeData mimeData;
    mimeData.setData(QString(itemMimeFmt).remove("-item"), qba);
    emit mimeDropped(index, &mimeData, action);
  }

  return true;
}

void MixersListWidget::zoomView()
{
  static int origSz = QFontInfo(font()).pixelSize();
  int fsz = origSz;
  int step = qobject_cast<QAction *>(sender())->data().toInt();
  if (step)
    fsz = QFontInfo(font()).pixelSize() + step;
  if (fsz < 8)
    return;

  QFont fnt(font());
  fnt.setPixelSize(fsz);
  setFont(fnt);
  defaultFont = fnt;
}


/*
 * MixersDelegate
*/

/**
    @brief Paints our HTML formated list item text
*/
void MixersDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItem options = option;
  initStyleOption(&options, index);

  QTextDocument doc;
  SetupDocument(doc, options, index);
  options.text.clear();

  QAbstractTextDocumentLayout::PaintContext ctx;
  ctx.palette = (options.widget ? options.widget->palette() : QApplication::palette("QListWidgetItem"));

  QBrush brush = ((index.data(GroupIdRole).toUInt() % 2) ? ctx.palette.base() : ctx.palette.alternateBase());
  options.backgroundBrush = brush;

  if (!(index.model()->data(index, GroupHeaderRole).toUInt() & GRP_HAS_DATA)) {
    // lighter text for empty lines
    ctx.palette.setBrush(QPalette::Text, ctx.palette.mid());
  }
#ifndef Q_OS_WIN
  // Linux and OS X show a solid dark blue color on selected items
  else if ((options.state & QStyle::State_Selected)) {
    ctx.palette.setBrush(QPalette::Text, ctx.palette.highlightedText());
  }
#endif

  QStyle * style = (options.widget ? options.widget->style() : QApplication::style());
  style->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

  QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &options, options.widget);
#ifdef Q_OS_LINUX
  // Workaround for buggy Linux native styles (https://github.com/opentx/opentx/issues/5633)
  if (options.widget && textRect.width() < options.widget->geometry().width() / 2)
    textRect.setWidth(options.widget->geometry().width());
#endif
  ctx.clip = textRect.translated(-textRect.topLeft());
  painter->save();
  painter->translate(textRect.topLeft());
  painter->setClipRect(ctx.clip);
  doc.documentLayout()->draw(painter, ctx);
  painter->restore();
}


/**
    @brief Returns needed size for our HTML formated list item text
*/
QSize MixersDelegate::sizeHint (const QStyleOptionViewItem & option, const QModelIndex & index) const
{
  QStyleOptionViewItem options = option;
  initStyleOption(&options, index);

  QTextDocument doc;
  SetupDocument(doc, options, index);
  return QSize(doc.idealWidth(), doc.size().height());
}

void MixersDelegate::SetupDocument(QTextDocument & doc, const QStyleOptionViewItem & options, const QModelIndex & index) const
{
  //setup html document
  quint8 hdrRole = index.model()->data(index, GroupHeaderRole).toUInt();

  doc.setDefaultFont(options.font);
  doc.setHtml(options.text);

  // adjust margins to visually group items per input/channel
  QTextFrame *tf = doc.rootFrame();
  QTextFrameFormat tff = tf->frameFormat();

  if (!(hdrRole & GRP_HAS_DATA)) {
    tff.setTopMargin(0.75f);
    tff.setBottomMargin(0.75f);
  }
  else {
    if (!(hdrRole & GRP_IS_PARENT))
      tff.setTopMargin(0.5f);
    if (hdrRole & GRP_HAS_SIBLING)
      tff.setBottomMargin(0.5f);
  }
  tf->setFrameFormat(tff);
}


/*
 * MixerItemViewProxyStyle
*/

void MixerItemViewProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const
{
  painter->save();
  if (element == QStyle::PE_IndicatorItemViewItemDrop) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    // set a wider stroke for the drop indicator
    QPen pen(painter->pen());
    pen.setWidthF(2.25f);
    painter->setPen(pen);
  }
  QProxyStyle::drawPrimitive(element, option, painter, widget);
  painter->restore();
}
