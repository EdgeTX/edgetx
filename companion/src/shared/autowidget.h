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

#include <functional>

#include <QWidget>
#include <QString>
#include <QList>
#include <QAbstractItemModel>

class GenericPanel;

// Base class for all Auto name prefixed widgets
// Note: ths class cannot be a Qt object as it will trigger a compile time ambiguity in the inheriting class
class AutoWidget
{
  friend class GenericPanel;

  public:
    explicit AutoWidget();
    ~AutoWidget();

    // buddy Auto widgets have their bindings mirroring the parent
    // example buddies are AutoLabel and AutoComboBox
    void addBuddyWidget(AutoWidget * wgt);
    void addBuddyWidgets(QList<AutoWidget *> wgts);
    void clearBuddyWidget(AutoWidget * wgt);
    void clearBuddyWidgets();

    // mark those unsupported by the base Qt widget as deleted in the child widget
    // these are not marked as virtual to support deletion
    void setBindEnabled(std::function<bool()> pred);
    void setBindModel(std::function<QAbstractItemModel*()> fn);
    void setBindPostChanged(std::function<void()> fn);
    void setBindText(std::function<QString()> fn);
    void setBindVisible(std::function<bool()> pred);

  protected:
    virtual void setAutoModel(QAbstractItemModel * model) {};
    virtual void setAutoText(QString text) {};

    virtual void updateValue() = 0;

    void applyBindings();
    void clearBindEnabled();
    void clearBindVisible();
    void clearBuddyBinds(AutoWidget * wgt);
    bool lock();
    bool panelLock();
    void runPostChanged();
    void setAutoEnabled(bool enabled);
    void setAutoVisible(bool visible);
    void setLock(bool lock);
    void setPanel(GenericPanel * panel);


  private:
    GenericPanel *m_panel;
    bool m_lock;

    QList<AutoWidget *> m_buddyWidgets;

    std::function<bool()> m_enabled;
    std::function<QAbstractItemModel*()> m_model;
    std::function<QString()> m_text;
    std::function<bool()> m_visible;
    std::function<void()> m_postChanged;

    void dataChanged();
};
