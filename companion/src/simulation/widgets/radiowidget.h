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

#include <QGridLayout>
#include <QDebug>
#include <QLabel>
#include <QWidget>

#define RADIO_WIDGET_STATE_VERSION    1

class RadioUiAction;

class RadioWidget : public QWidget
{
  Q_OBJECT

  public:

    // These are saved to persistent user settings in RadioWidgetState,
    //   so only append entires here, do not change index value if it can be avoided.
    enum RadioWidgetType {
      RADIO_WIDGET_NONE = 0,
      RADIO_WIDGET_SWITCH,
      RADIO_WIDGET_KNOB,
      RADIO_WIDGET_FADER,
      RADIO_WIDGET_TRIM,    // trim axis, usually 2 buttons & slider
      RADIO_WIDGET_STICK,   // actually one axis of a stick
      RADIO_WIDGET_KEY,     // UI key/pushbutton
      RADIO_WIDGET_ENUM_COUNT
    };

    enum RadioTrimWidgetButtonStates { RADIO_TRIM_BTN_ON = 1024, RADIO_TRIM_BTN_OFF = -1024 };

    struct RadioWidgetState {
      public:
        RadioWidgetState(quint8 type = 0, qint8 index = 0, qint16 value = 0, quint16 flags = 0) :
          type(type), index(index), value(value), flags(flags),
          _version(RADIO_WIDGET_STATE_VERSION)
        { }

        quint8 type;
        qint8 index;
        qint16 value;
        quint16 flags;

        friend QDataStream & operator << (QDataStream &out, const RadioWidgetState & o);
        friend QDataStream & operator >> (QDataStream &in, RadioWidgetState & o);
        friend QDebug operator << (QDebug d, const RadioWidgetState & o);

      private:
        quint8 _version = RADIO_WIDGET_STATE_VERSION;  // structure definition version
    };

    explicit RadioWidget(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    explicit RadioWidget(RadioUiAction * action = NULL, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    explicit RadioWidget(const QString & labelText, int value = 0, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    virtual int getValue() const;
    virtual int getIndex() const;
    virtual int getType()  const;
    virtual QByteArray getStateData() const;
    virtual RadioWidgetState getState() const;
    virtual RadioUiAction * getAction() const;

  public slots:

    virtual void setIndex(const int & index);
    virtual void setType(const RadioWidgetType & type);
    virtual void setValue(const int & value);
    virtual void setValueQual(const RadioWidgetType & type, const int & index, const int & value);
    virtual void chgValueQual(const RadioWidgetType & type, const int & index, const int offset, const bool state) {}
    virtual void setFlags(const quint16 & flags);
    virtual void setShowLabel(const bool show);
    virtual void setLabelText(const QString & labelText, bool showLabel = true);
    virtual void setStateData(const RadioWidgetState & state);
    virtual void changeVisibility(bool visible);
    virtual void setAction(RadioUiAction * action);

  signals:

    void valueChange(const RadioWidgetType type, const int index, int value);
    void valueChanged(const int value);
    void flagsChanged(const quint16 flags);

  protected:

    void addLayout();
    void addLabel();
    void setWidget(QWidget * widget = NULL, Qt::Alignment align = Qt::AlignHCenter);
    virtual void onActionToggled(int index, bool active);
    virtual void onActionTriggered(int index, bool active);

    QGridLayout * m_gridLayout;
    QWidget * m_controlWidget;
    QLabel * m_nameLabel;
    RadioUiAction * m_action;

    int m_value;
    int m_index;
    quint16 m_flags;
    bool m_valueReset;
    bool m_showLabel;
    QString m_labelText;
    RadioWidgetType m_type;

};

Q_DECLARE_METATYPE(RadioWidget::RadioWidgetState)
