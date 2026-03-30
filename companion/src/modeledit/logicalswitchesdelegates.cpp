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

#include "logicalswitchesdelegates.h"
#include "logicalswitchestablemodel.h"
#include "eeprominterface.h"
#include "filtereditemmodels.h"

#include <TimerEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSignalBlocker>

// Helper: populate function combobox with ordered function list
static void populateFunctionCB(QComboBox * b)
{
  int order[] = {
    LS_FN_OFF,
    LS_FN_VEQUAL,
    LS_FN_VALMOSTEQUAL,
    LS_FN_VPOS,
    LS_FN_VNEG,
    LS_FN_APOS,
    LS_FN_ANEG,
    LS_FN_AND,
    LS_FN_OR,
    LS_FN_XOR,
    LS_FN_EDGE,
    LS_FN_EQUAL,
    LS_FN_NEQUAL,
    LS_FN_GREATER,
    LS_FN_LESS,
    LS_FN_EGREATER,
    LS_FN_ELESS,
    LS_FN_DPOS,
    LS_FN_DAPOS,
    LS_FN_TIMER,
    LS_FN_STICKY
  };

  b->clear();
  for (int i = 0; i < LS_FN_MAX; i++) {
    int func = order[i];
    if (func == LS_FN_NEQUAL || func == LS_FN_EGREATER || func == LS_FN_ELESS)
      continue;
    b->addItem(LogicalSwitchData(func).funcToString(), func);
  }
  b->setMaxVisibleItems(10);
}

// Helper: configure a timer-param QDoubleSpinBox
static void setupTimerSpinBox(QDoubleSpinBox * sb, int timer, double minimum = 0.0, double maximum = 175.0)
{
  sb->setDecimals(1);
  sb->setMinimum(minimum);
  sb->setMaximum(maximum);
  float value = ValToTim(timer);
  if (value >= 60)
    sb->setSingleStep(1);
  else if (value >= 2)
    sb->setSingleStep(0.5);
  else
    sb->setSingleStep(0.1);
  sb->setValue(value);
}

// ---- LogicalSwitchFunctionDelegate ----

LogicalSwitchFunctionDelegate::LogicalSwitchFunctionDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{
}

QWidget * LogicalSwitchFunctionDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
  auto * cb = new QComboBox(parent);
  populateFunctionCB(cb);
  connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cb]() {
    const_cast<LogicalSwitchFunctionDelegate*>(this)->commitData(cb);
  });
  return cb;
}

void LogicalSwitchFunctionDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QComboBox *>(editor);
  if (!cb) return;
  QSignalBlocker blocker(cb);
  cb->setCurrentIndex(cb->findData(index.data(Qt::EditRole).toInt()));
}

void LogicalSwitchFunctionDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QComboBox *>(editor);
  if (!cb) return;
  model->setData(index, cb->currentData().toUInt());
}

void LogicalSwitchFunctionDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

// ---- LogicalSwitchV1Delegate ----

LogicalSwitchV1Delegate::LogicalSwitchV1Delegate(LogicalSwitchesTableModel * tableModel, QObject * parent)
  : QStyledItemDelegate(parent), m_tableModel(tableModel)
{
}

QWidget * LogicalSwitchV1Delegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
  unsigned int mask = index.data(LogicalSwitchesTableModel::VisibilityMaskRole).toUInt();

  if (mask & LogicalSwitchesTableModel::VALUE1_VISIBLE) {
    // TIMER family: spinbox
    auto * sb = new QDoubleSpinBox(parent);
    int val1 = index.data(Qt::EditRole).toInt();
    setupTimerSpinBox(sb, val1, 0.1);
    connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, sb]() {
      const_cast<LogicalSwitchV1Delegate*>(this)->commitData(sb);
    });
    return sb;
  }

  if (mask & LogicalSwitchesTableModel::SOURCE1_VISIBLE) {
    auto * cb = new QComboBox(parent);
    int family = index.data(LogicalSwitchesTableModel::FunctionFamilyRole).toInt();
    if (family == LS_FAMILY_VOFS || family == LS_FAMILY_VCOMP)
      cb->setModel(m_tableModel->rawSourceModel());
    else
      cb->setModel(m_tableModel->rawSwitchModel());
    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cb]() {
      const_cast<LogicalSwitchV1Delegate*>(this)->commitData(cb);
    });
    return cb;
  }

  return nullptr;
}

void LogicalSwitchV1Delegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  int val1 = index.data(Qt::EditRole).toInt();

  if (auto * sb = qobject_cast<QDoubleSpinBox *>(editor)) {
    QSignalBlocker blocker(sb);
    setupTimerSpinBox(sb, val1, 0.1);
  }
  else if (auto * cb = qobject_cast<QComboBox *>(editor)) {
    QSignalBlocker blocker(cb);
    int family = index.data(LogicalSwitchesTableModel::FunctionFamilyRole).toInt();
    if (family == LS_FAMILY_VBOOL || family == LS_FAMILY_STICKY || family == LS_FAMILY_EDGE)
      cb->setCurrentIndex(cb->findData(val1));
    else
      cb->setCurrentIndex(cb->findData(RawSource(val1).toValue()));
  }
}

void LogicalSwitchV1Delegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  if (auto * sb = qobject_cast<QDoubleSpinBox *>(editor)) {
    model->setData(index, TimToVal(sb->value()));
  }
  else if (auto * cb = qobject_cast<QComboBox *>(editor)) {
    model->setData(index, cb->currentData().toInt());
  }
}

void LogicalSwitchV1Delegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

// ---- LogicalSwitchV2Delegate ----

LogicalSwitchV2Delegate::LogicalSwitchV2Delegate(LogicalSwitchesTableModel * tableModel, QObject * parent)
  : QStyledItemDelegate(parent), m_tableModel(tableModel)
{
}

QWidget * LogicalSwitchV2Delegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
  unsigned int mask = index.data(LogicalSwitchesTableModel::VisibilityMaskRole).toUInt();
  int family = index.data(LogicalSwitchesTableModel::FunctionFamilyRole).toInt();

  if (mask & LogicalSwitchesTableModel::VALUE_TO_VISIBLE) {
    // VOFS with timer source: TimerEdit
    auto * te = new TimerEdit(parent);
    connect(te, &TimerEdit::editingFinished, this, [this, te]() {
      const_cast<LogicalSwitchV2Delegate*>(this)->commitData(te);
    });
    return te;
  }

  if ((mask & LogicalSwitchesTableModel::VALUE3_VISIBLE) && family == LS_FAMILY_EDGE) {
    // EDGE: container with two spinboxes
    auto * container = new QWidget(parent);
    auto * layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    auto * dsbOffset = new QDoubleSpinBox(container);
    dsbOffset->setObjectName("dsbOffset");
    auto * dsbOffset2 = new QDoubleSpinBox(container);
    dsbOffset2->setObjectName("dsbOffset2");

    layout->addWidget(dsbOffset);
    layout->addWidget(dsbOffset2);

    connect(dsbOffset, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, container]() {
      const_cast<LogicalSwitchV2Delegate*>(this)->commitData(container);
    });
    connect(dsbOffset2, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, container]() {
      const_cast<LogicalSwitchV2Delegate*>(this)->commitData(container);
    });
    return container;
  }

  if (mask & LogicalSwitchesTableModel::SOURCE2_VISIBLE) {
    auto * cb = new QComboBox(parent);
    if (family == LS_FAMILY_VCOMP)
      cb->setModel(m_tableModel->rawSourceModel());
    else
      cb->setModel(m_tableModel->rawSwitchModel());
    connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cb]() {
      const_cast<LogicalSwitchV2Delegate*>(this)->commitData(cb);
    });
    return cb;
  }

  if (mask & LogicalSwitchesTableModel::VALUE2_VISIBLE) {
    // VOFS non-timer or TIMER family
    auto * sb = new QDoubleSpinBox(parent);
    sb->setAccelerated(true);

    if (family == LS_FAMILY_TIMER) {
      QVariantMap vals = index.data(Qt::EditRole).toMap();
      setupTimerSpinBox(sb, vals["val2"].toInt(), 0.1);
    }
    else {
      // VOFS: configure from source range
      int val1 = index.sibling(index.row(), LogicalSwitchesTableModel::COL_V1).data(Qt::EditRole).toInt();
      QVariantMap vals = index.data(Qt::EditRole).toMap();
      LogicalSwitchData & lsw = m_tableModel->modelData()->logicalSw[index.row()];
      RawSource source(val1);
      RawSourceRange range = source.getRange(m_tableModel->modelData(), m_tableModel->generalSettingsRef(), lsw.getRangeFlags());
      sb->setMinimum(range.min);
      sb->setMaximum(range.max);
      sb->setDecimals(range.decimals);
      sb->setSingleStep(range.step);
      double value = range.toDisplay(vals["val2"].toInt());
      value = range.validateDisplay(value);
      sb->setValue(value);
      if (!range.unit.isEmpty())
        sb->setSuffix(" " + range.unit);
    }

    connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, sb]() {
      const_cast<LogicalSwitchV2Delegate*>(this)->commitData(sb);
    });
    return sb;
  }

  return nullptr;
}

void LogicalSwitchV2Delegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  QVariantMap vals = index.data(Qt::EditRole).toMap();
  int family = index.data(LogicalSwitchesTableModel::FunctionFamilyRole).toInt();

  if (auto * te = qobject_cast<TimerEdit *>(editor)) {
    QSignalBlocker blocker(te);
    int val1 = index.sibling(index.row(), LogicalSwitchesTableModel::COL_V1).data(Qt::EditRole).toInt();
    LogicalSwitchData & lsw = m_tableModel->modelData()->logicalSw[index.row()];
    RawSource source(val1);
    RawSourceRange range = source.getRange(m_tableModel->modelData(), m_tableModel->generalSettingsRef(), lsw.getRangeFlags());
    te->setTimeRange(range.min, range.max);
    te->setSingleStep(range.step);
    te->setPageStep(range.step * 60);
    te->setShowSeconds(range.step != 60);
    te->setTime((int)range.toDisplay(vals["val2"].toInt()));
  }
  else if (editor->objectName().isEmpty() && qobject_cast<QComboBox *>(editor)) {
    auto * cb = qobject_cast<QComboBox *>(editor);
    QSignalBlocker blocker(cb);
    cb->setCurrentIndex(cb->findData(vals["val2"].toInt()));
  }
  else if (auto * sb = qobject_cast<QDoubleSpinBox *>(editor)) {
    QSignalBlocker blocker(sb);
    if (family == LS_FAMILY_TIMER) {
      setupTimerSpinBox(sb, vals["val2"].toInt(), 0.1);
    }
    else {
      // VOFS
      int val1 = index.sibling(index.row(), LogicalSwitchesTableModel::COL_V1).data(Qt::EditRole).toInt();
      LogicalSwitchData & lsw = m_tableModel->modelData()->logicalSw[index.row()];
      RawSource source(val1);
      RawSourceRange range = source.getRange(m_tableModel->modelData(), m_tableModel->generalSettingsRef(), lsw.getRangeFlags());
      sb->setMinimum(range.min);
      sb->setMaximum(range.max);
      sb->setDecimals(range.decimals);
      sb->setSingleStep(range.step);
      double value = range.validateDisplay(range.toDisplay(vals["val2"].toInt()));
      sb->setValue(value);
      sb->setSuffix(range.unit.isEmpty() ? QString() : QString(" ") + range.unit);
    }
  }
  else if (editor->findChild<QDoubleSpinBox *>("dsbOffset")) {
    // EDGE container
    auto * dsb1 = editor->findChild<QDoubleSpinBox *>("dsbOffset");
    auto * dsb2 = editor->findChild<QDoubleSpinBox *>("dsbOffset2");
    if (dsb1 && dsb2) {
      QSignalBlocker b1(dsb1);
      QSignalBlocker b2(dsb2);
      int val2 = vals["val2"].toInt();
      int val3 = vals["val3"].toInt();
      setupTimerSpinBox(dsb1, val2, 0.0);
      setupTimerSpinBox(dsb2, val2 + val3, ValToTim(TimToVal(dsb1->value()) - 1), 275.0);
      dsb2->setSuffix(val3 ? "" : tr(" (infinite)"));
    }
  }
}

void LogicalSwitchV2Delegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  int family = index.data(LogicalSwitchesTableModel::FunctionFamilyRole).toInt();

  if (auto * te = qobject_cast<TimerEdit *>(editor)) {
    // VOFS timer: convert time back to raw
    int val1 = index.sibling(index.row(), LogicalSwitchesTableModel::COL_V1).data(Qt::EditRole).toInt();
    LogicalSwitchData & lsw = m_tableModel->modelData()->logicalSw[index.row()];
    RawSource source(val1);
    RawSourceRange range = source.getRange(m_tableModel->modelData(), m_tableModel->generalSettingsRef(), lsw.getRangeFlags());
    QVariantMap m;
    m["val2"] = range.toRaw(te->timeInSeconds());
    m["val3"] = lsw.val3;
    model->setData(index, m);
  }
  else if (editor->objectName().isEmpty() && qobject_cast<QComboBox *>(editor)) {
    auto * cb = qobject_cast<QComboBox *>(editor);
    QVariantMap m;
    m["val2"] = cb->currentData().toInt();
    m["val3"] = 0;
    model->setData(index, m);
  }
  else if (auto * sb = qobject_cast<QDoubleSpinBox *>(editor)) {
    QVariantMap m;
    if (family == LS_FAMILY_TIMER) {
      m["val2"] = TimToVal(sb->value());
    }
    else {
      // VOFS
      int val1 = index.sibling(index.row(), LogicalSwitchesTableModel::COL_V1).data(Qt::EditRole).toInt();
      LogicalSwitchData & lsw = m_tableModel->modelData()->logicalSw[index.row()];
      RawSource source(val1);
      RawSourceRange range = source.getRange(m_tableModel->modelData(), m_tableModel->generalSettingsRef(), lsw.getRangeFlags());
      m["val2"] = range.toRaw(sb->value());
    }
    m["val3"] = 0;
    model->setData(index, m);
  }
  else if (editor->findChild<QDoubleSpinBox *>("dsbOffset")) {
    // EDGE container
    auto * dsb1 = editor->findChild<QDoubleSpinBox *>("dsbOffset");
    auto * dsb2 = editor->findChild<QDoubleSpinBox *>("dsbOffset2");
    if (dsb1 && dsb2) {
      int val2 = TimToVal(dsb1->value());
      int val3 = TimToVal(dsb2->value()) - val2;
      QVariantMap m;
      m["val2"] = val2;
      m["val3"] = val3;
      model->setData(index, m);
    }
  }
}

void LogicalSwitchV2Delegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

// ---- LogicalSwitchComboDelegate (AND Switch) ----

LogicalSwitchComboDelegate::LogicalSwitchComboDelegate(LogicalSwitchesTableModel * tableModel, QObject * parent)
  : QStyledItemDelegate(parent), m_tableModel(tableModel)
{
}

QWidget * LogicalSwitchComboDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
  unsigned int mask = index.data(LogicalSwitchesTableModel::VisibilityMaskRole).toUInt();
  if (!(mask & LogicalSwitchesTableModel::LINE_ENABLED))
    return nullptr;

  auto * cb = new QComboBox(parent);
  cb->setModel(m_tableModel->rawSwitchModel());
  connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, cb]() {
    const_cast<LogicalSwitchComboDelegate*>(this)->commitData(cb);
  });
  return cb;
}

void LogicalSwitchComboDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QComboBox *>(editor);
  if (!cb) return;
  QSignalBlocker blocker(cb);
  cb->setCurrentIndex(cb->findData(RawSwitch(index.data(Qt::EditRole).toInt()).toValue()));
}

void LogicalSwitchComboDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QComboBox *>(editor);
  if (!cb) return;
  model->setData(index, cb->currentData().toInt());
}

void LogicalSwitchComboDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

// ---- LogicalSwitchNumericDelegate (Duration / Delay) ----

LogicalSwitchNumericDelegate::LogicalSwitchNumericDelegate(unsigned int maskBit, QObject * parent)
  : QStyledItemDelegate(parent), m_maskBit(maskBit)
{
}

QWidget * LogicalSwitchNumericDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
  unsigned int mask = index.data(LogicalSwitchesTableModel::VisibilityMaskRole).toUInt();
  if (!(mask & m_maskBit))
    return nullptr;

  auto * sb = new QDoubleSpinBox(parent);
  sb->setRange(0, 25);
  sb->setSingleStep(0.1);
  sb->setDecimals(1);
  connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, sb]() {
    const_cast<LogicalSwitchNumericDelegate*>(this)->commitData(sb);
  });
  return sb;
}

void LogicalSwitchNumericDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  auto * sb = qobject_cast<QDoubleSpinBox *>(editor);
  if (!sb) return;
  QSignalBlocker blocker(sb);
  sb->setValue(index.data(Qt::EditRole).toDouble());
}

void LogicalSwitchNumericDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  auto * sb = qobject_cast<QDoubleSpinBox *>(editor);
  if (!sb) return;
  model->setData(index, sb->value());
}

void LogicalSwitchNumericDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}

// ---- LogicalSwitchPersistDelegate ----

LogicalSwitchPersistDelegate::LogicalSwitchPersistDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{
}

QWidget * LogicalSwitchPersistDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const
{
  unsigned int mask = index.data(LogicalSwitchesTableModel::VisibilityMaskRole).toUInt();
  if (!(mask & LogicalSwitchesTableModel::PERSIST_ENABLED))
    return nullptr;

  auto * cb = new QCheckBox(parent);
  connect(cb, &QCheckBox::stateChanged, this, [this, cb]() {
    const_cast<LogicalSwitchPersistDelegate*>(this)->commitData(cb);
  });
  return cb;
}

void LogicalSwitchPersistDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QCheckBox *>(editor);
  if (!cb) return;
  QSignalBlocker blocker(cb);
  cb->setChecked(index.data(Qt::EditRole).toBool());
}

void LogicalSwitchPersistDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
  auto * cb = qobject_cast<QCheckBox *>(editor);
  if (!cb) return;
  model->setData(index, cb->isChecked(), Qt::CheckStateRole);
}

void LogicalSwitchPersistDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}
