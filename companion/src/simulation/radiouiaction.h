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
#include <QWidget>
#include <QKeyEvent>

/*
 * This class is somewhat like a QAction but specific for the radio UI.
 * Actions can have one or more keyboard shortcuts associated with them (currently single-key only, w/out modifiers).
 */
class RadioUiAction : public QObject
{
  Q_OBJECT

  public:
    /*
     * @param index    Typically this is the hardware array index corresponding to button on current radio model,
     *                   but it could be anything. Use -1 or any other negative value for non-hardware indices.
     * @param key      An optional Qt:Key code for shortcut (zero for none).
     * @param parent   Parent widget, required for handling keyboard events.
     * @param text     Optional title for this action. The text and description are currently used in generated help text.
     * @param descript Optional longer description text for this action.
     */
    RadioUiAction(int index = -1, int key = 0, const QString &text = "", const QString &descript = "", QWidget * parent = nullptr):
      m_hwIndex(index),
      m_active(false),
      m_keys(QList<int>()),
      m_text(text),
      m_description(descript),
      m_parent(parent)
    {
      addKey(key);
    }
    /*
     * @param keys QList of Qt:Key codes to use as shortcuts.
     *   [See above for other params.]
     */
    RadioUiAction(int index, QList<int> keys, const QString &text = "", const QString &descript = "", QWidget * parent = nullptr):
      RadioUiAction(index, 0, text, descript, parent)
    {
      addKeys(keys);
    }

    void addKey(const int & key)
    {
      if (key > 0 && !m_keys.contains(key)) {
        m_keys.append(key);
        addShortcut();
      }
    }

    void addKeys(const QList<int> & keys)
    {
      foreach (int key, keys)
        addKey(key);
    }

    void addShortcut()
    {
      if (m_parent) {
        m_parent->removeEventFilter(this);
        if (m_keys.size())
          m_parent->installEventFilter(this);
      }
    }

    // override
    void setParent(QObject * parent)
    {
      QObject::setParent(parent);
      QWidget * w = qobject_cast<QWidget *>(parent);
      if (w && w != m_parent) {
        m_parent = w;
        addShortcut();
      }
    }

    void setDescription(const QString & description)    { m_description = description; }
    void setText(QPair<QString, QString> name_descript) { setText(name_descript.first, name_descript.second); }

    void setText(const QString & text, const QString & description = "")
    {
      m_text = text;
      if (!description.isEmpty())
        setDescription(description);
    }

    int     getIndex()       const { return m_hwIndex; }
    bool    isActive()       const { return m_active; }
    QString getDescription() const { return m_description; }
    QString getText()        const { return m_text; }

    bool eventFilter(QObject * obj, QEvent * event)
    {
      if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((!keyEvent->modifiers() || keyEvent->modifiers() == Qt::KeypadModifier) && m_keys.contains(keyEvent->key())) {
          trigger(event->type() == QEvent::KeyPress);
          return true;
        }
      }
      return QObject::eventFilter(obj, event);
    }

  public slots:

    // "toggle" is the most basic way to set the action status and emits only one "toggled" signal
    bool toggle(bool active = true)
    {
      if (active != m_active) {
        m_active = active;
        emit toggled(m_hwIndex, active);
        return true;
      }
      return false;
    }

    // "trigger" indicates a user-initiated action and emits more signals than "toggle" does
    void trigger(bool active = true)
    {
      if (toggle(active)) {
        emit triggered(m_hwIndex, active);
        if (active) {
          emit pushed(m_hwIndex);
          emit pushed();
        }
      }
    }

  private:

    int m_hwIndex;
    bool m_active;
    QList<int> m_keys;
    QString m_text;
    QString m_description;
    QWidget * m_parent;


  signals:
    void toggled(int index, bool active);    // on programmatic or user interaction change
    void triggered(int index, bool active);  // on user interaction change only
    void pushed(int index);                  // only emitted on user interaction && when 'active' is true
    void pushed();
};
