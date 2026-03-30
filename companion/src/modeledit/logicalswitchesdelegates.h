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

#include <QStyledItemDelegate>

class LogicalSwitchesTableModel;

class LogicalSwitchFunctionDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchFunctionDelegate(QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
};

class LogicalSwitchV1Delegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchV1Delegate(LogicalSwitchesTableModel * tableModel, QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
  private:
    LogicalSwitchesTableModel * m_tableModel;
};

class LogicalSwitchV2Delegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchV2Delegate(LogicalSwitchesTableModel * tableModel, QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
  private:
    LogicalSwitchesTableModel * m_tableModel;
};

class LogicalSwitchComboDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchComboDelegate(LogicalSwitchesTableModel * tableModel, QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
  private:
    LogicalSwitchesTableModel * m_tableModel;
};

class LogicalSwitchNumericDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchNumericDelegate(unsigned int maskBit, QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
  private:
    unsigned int m_maskBit;
};

class LogicalSwitchPersistDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    explicit LogicalSwitchPersistDelegate(QObject * parent = nullptr);
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
};
