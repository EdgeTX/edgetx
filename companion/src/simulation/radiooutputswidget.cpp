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

#include "radiooutputswidget.h"
#include "ui_radiooutputswidget.h"

#include "appdata.h"
#include "constants.h"
#include "eeprominterface.h"
#include "radiodata.h"
#include "simulator.h"

#include <QScrollBar>

extern AppData g;  // ensure what "g" means

const quint16 RadioOutputsWidget::m_savedViewStateVersion = 2;

RadioOutputsWidget::RadioOutputsWidget(SimulatorInterface * simulator, Firmware * firmware, QWidget *parent) :
  QWidget(parent),
  m_simulator(simulator),
  m_firmware(firmware),
  m_radioProfileId(g.sessionId()),
  ui(new Ui::RadioOutputsWidget)
{
  ui->setupUi(this);

  restoreState();

  // link the channels and mixes display horizontal scroll
  connect(ui->channelsScroll->horizontalScrollBar(), &QScrollBar::sliderMoved, ui->mixersScroll->horizontalScrollBar(), &QScrollBar::setValue);
  connect(ui->mixersScroll->horizontalScrollBar(), &QScrollBar::sliderMoved, ui->channelsScroll->horizontalScrollBar(), &QScrollBar::setValue);

  connect(m_simulator, &SimulatorInterface::channelOutValueChange, this, &RadioOutputsWidget::onChannelOutValueChange);
  connect(m_simulator, &SimulatorInterface::channelMixValueChange, this, &RadioOutputsWidget::onChannelMixValueChange);
  connect(m_simulator, &SimulatorInterface::virtualSwValueChange, this, &RadioOutputsWidget::onVirtSwValueChange);
  connect(m_simulator, &SimulatorInterface::gVarValueChange, this, &RadioOutputsWidget::onGVarValueChange);
  connect(m_simulator, &SimulatorInterface::phaseChanged, this, &RadioOutputsWidget::onPhaseChanged);
}

RadioOutputsWidget::~RadioOutputsWidget()
{
  //stop();
  saveState();
  delete ui;
}

void RadioOutputsWidget::changeEvent(QEvent *e)
{
  QWidget::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

void RadioOutputsWidget::start()
{
  setupChannelsDisplay(false);
  setupChannelsDisplay(true);
  setupGVarsDisplay();
  setupLsDisplay();
}

//void RadioOutputsWidget::stop()
//{
//}

void RadioOutputsWidget::restart()
{
  //stop();
  start();
}

void RadioOutputsWidget::saveState()
{
  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  stream << m_savedViewStateVersion
         << ui->btnLogiSw->isChecked() << ui->btnGlobalVars->isChecked()  << ui->btnChannels->isChecked()
         << ui->splitter->saveState();
  // view state version 2
  stream << ui->btnMixes->isChecked();

  SimulatorOptions opts = g.profile[m_radioProfileId].simulatorOptions();
  opts.radioOutputsState = state;
  g.profile[m_radioProfileId].simulatorOptions(opts);
}

void RadioOutputsWidget::restoreState()
{
  quint16 ver = 0;
  QByteArray splitterState;
  bool ls = true, gv = true, ch = true, mx = false;
  QByteArray state = g.profile[m_radioProfileId].simulatorOptions().radioOutputsState;
  QDataStream stream(state);

  stream >> ver;
  if (ver && ver <= m_savedViewStateVersion) {
    stream >> ls >> gv >> ch >> splitterState;
    if (ver >= 2)
      stream >> mx;
  }

  ui->btnLogiSw->setChecked(ls);
  ui->btnGlobalVars->setChecked(gv);
  ui->btnChannels->setChecked(ch);
  ui->btnMixes->setChecked(mx);
  if (!splitterState.isEmpty())
    ui->splitter->restoreState(splitterState);
}

void RadioOutputsWidget::setupChannelsDisplay(bool mixes)
{
  int outputs = std::min(32, m_firmware->getCapability(Capability(Outputs)));

  // delete old widgets if already exist
  QWidget * oldChanW = nullptr;
  if (mixes) {
    m_mixesMap.clear();
    oldChanW = ui->mixersScroll->takeWidget();
  }
  else {
    m_channelsMap.clear();
    oldChanW = ui->channelsScroll->takeWidget();
  }
  if (oldChanW)
    oldChanW->deleteLater();

  if (!outputs)
    return;

  QWidget * channelsWidget = new QWidget();
  QGridLayout * channelsLayout = new QGridLayout(channelsWidget);
  channelsLayout->setHorizontalSpacing(4);
  channelsLayout->setVerticalSpacing(3);
  channelsLayout->setContentsMargins(5, 5, 5, 5);

  if (mixes)
    ui->mixersScroll->setWidget(channelsWidget);
  else
    ui->channelsScroll->setWidget(channelsWidget);

  // populate outputs
  int column = 0;
  for (int i = 0; i < outputs; i++) {
    QLabel * label = new QLabel(channelsWidget);
    label->setText(" " + RawSource(SOURCE_TYPE_CH, i + 1).toString() + " ");
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    channelsLayout->addWidget(label, 0, column, 1, 1);

    QSlider * slider = new QSlider(channelsWidget);
    slider->setEnabled(false);
    slider->setMinimum(-1024);
    slider->setMaximum(1024);
    slider->setPageStep(128);
    slider->setTracking(false);
    slider->setOrientation(Qt::Vertical);
    slider->setInvertedAppearance(false);
    slider->setTickPosition(QSlider::TicksRight);
    slider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    channelsLayout->addWidget(slider, 2, column, 1, 1);
    channelsLayout->setAlignment(slider, Qt::AlignHCenter);

    QLabel * value = new QLabel(channelsWidget);
    value->setMinimumSize(QSize(value->fontMetrics().size(Qt::TextSingleLine, "-100.0").width(), 0));
    value->setAlignment(Qt::AlignCenter);
    value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    channelsLayout->addWidget(value, 1, column, 1, 1);

    ++column;

    if (mixes)
      m_mixesMap.insert(i, QPair<QLabel *, QSlider *>(value, slider));
    else
      m_channelsMap.insert(i, QPair<QLabel *, QSlider *>(value, slider));
  }
}

void RadioOutputsWidget::setupGVarsDisplay()
{
  int gvars = m_firmware->getCapability(Capability(Gvars));
  int fmodes = m_firmware->getCapability(Capability(FlightModes));

  // delete old widgets if already exist
  m_globalVarsMap.clear();

  QWidget * oldGv = ui->globalVarsScroll->takeWidget();
  if (oldGv)
    oldGv->deleteLater();

  if (!gvars)
    return;

  QWidget * gvarsWidget = new QWidget();
  QGridLayout * gvarsLayout = new QGridLayout(gvarsWidget);
  gvarsLayout->setContentsMargins(5, 5, 5, 5);
  gvarsLayout->setHorizontalSpacing(6);
  gvarsLayout->setVerticalSpacing(3);
  ui->globalVarsScroll->setWidget(gvarsWidget);

  QPalette::ColorRole bgrole = QPalette::AlternateBase;
  const bool isAir = Boards::isAir();
  for (int fm=0; fm < fmodes; fm++) {
    QLabel * label = new QLabel(gvarsWidget);
    label->setText(QString("%1%2").arg(isAir ? tr("FM") : tr("DM")).arg(fm));
    label->setAlignment(Qt::AlignCenter);
    label->setBackgroundRole(bgrole);
    gvarsLayout->addWidget(label, 0, fm+1);
  }
  QHash<int, QLabel *> fmMap;
  for (int gv=0; gv < gvars; gv++) {
    bgrole = ((gv % 2) ? QPalette::Window : QPalette::AlternateBase);
    QLabel * label = new QLabel(gvarsWidget);
    label->setText(QString(tr("GV%1")).arg(gv+1));
    label->setAutoFillBackground(true);
    label->setBackgroundRole(bgrole);
    gvarsLayout->addWidget(label, gv+1, 0);
    for (int fm=0; fm < fmodes; fm++) {
      QLabel * value = new QLabel(gvarsWidget);
      value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      value->setAutoFillBackground(true);
      value->setBackgroundRole(bgrole);
      value->setText("0");
      value->setStyleSheet("padding-right: .06em;");
      gvarsLayout->addWidget(value, gv+1, fm+1);

      fmMap.insert(fm, value);
    }
    m_globalVarsMap.insert(gv, fmMap);
  }
}

void RadioOutputsWidget::setupLsDisplay()
{
  int switches = m_firmware->getCapability(LogicalSwitches);

  // delete old widgets if already exist
  m_logicSwitchMap.clear();

  QWidget * oldLsW = ui->logicalSwitchesScroll->takeWidget();
  if (oldLsW)
    oldLsW->deleteLater();

  if (!switches)
    return;

  QWidget * logicalSwitches = new QWidget();
  logicalSwitches->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  QGridLayout * logicalSwitchesLayout = new QGridLayout(logicalSwitches);
  logicalSwitchesLayout->setHorizontalSpacing(3);
  logicalSwitchesLayout->setVerticalSpacing(2);
  logicalSwitchesLayout->setContentsMargins(5, 5, 5, 5);
  ui->logicalSwitchesScroll->setWidget(logicalSwitches);

  // populate logical switches
  int rows = switches / (switches > 16 ? 4 : 2);
  for (int i=0; i < switches; i++) {
    logicalSwitchesLayout->addWidget(createLogicalSwitch(logicalSwitches, i), i / rows, i % rows, 1, 1);
  }
}

QWidget * RadioOutputsWidget::createLogicalSwitch(QWidget * parent, int switchNo)
{
  QLabel * swtch = new QLabel(parent);
  swtch->setAutoFillBackground(true);
  swtch->setFrameStyle(QFrame::Panel | QFrame::Raised);
  swtch->setLineWidth(2);
  swtch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QFont font = swtch->font();
  font.setBold(true);
  swtch->setFont(font);
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  swtch->setMinimumWidth(swtch->fontMetrics().width("99") + 10);
#else
  swtch->setMinimumWidth(swtch->fontMetrics().horizontalAdvance("99") + 10);
#endif
  font.setBold(false);
  swtch->setFont(font);
  swtch->setText(QString("%1").arg(switchNo+1, 2, 10, QChar('0')));
  swtch->setAlignment(Qt::AlignCenter);
  m_logicSwitchMap.insert(switchNo, swtch);
  return swtch;
}

void RadioOutputsWidget::onChannelOutValueChange(quint8 index, qint32 value, qint32 limit)
{
  if (m_channelsMap.contains(index)) {
    QPair<QLabel *, QSlider *> ch = m_channelsMap.value(index);
    if (ch.second->maximum() != limit) {
      ch.second->setMaximum(limit);
      ch.second->setMinimum(-limit);
    }
    ch.first->setText(QString("%1%").arg(calcRESXto100(value)));
    ch.second->setValue(qMin(limit, qMax(-limit, value)));
  }
}

void RadioOutputsWidget::onChannelMixValueChange(quint8 index, qint32 value, qint32 limit)
{
  if (m_mixesMap.contains(index)) {
    QPair<QLabel *, QSlider *> ch = m_mixesMap.value(index);
    if (ch.second->maximum() != limit) {
      ch.second->setMaximum(limit);
      ch.second->setMinimum(-limit);
    }
    ch.first->setText(QString("%1%").arg(calcRESXto100(value)));
    ch.second->setValue(qMin(limit, qMax(-limit, value)));
  }
}

void RadioOutputsWidget::onVirtSwValueChange(quint8 index, qint32 value)
{
  if (!m_logicSwitchMap.contains(index))
    return;

  QLabel * ls = m_logicSwitchMap.value(index);
  ls->setBackgroundRole(value ? QPalette::Dark : QPalette::Window);
  ls->setForegroundRole(value ? QPalette::BrightText : QPalette::WindowText);
  ls->setFrameShadow(value ? QFrame::Sunken : QFrame::Raised);
  QFont font = ls->font();
  font.setBold((bool)value);
  ls->setFont(font);
  //qDebug() << index << value;
}

void RadioOutputsWidget::onGVarValueChange(quint8 index, qint32 value)
{
  if (!m_globalVarsMap.contains(index))
    return;

  QHash<int, QLabel *> fmMap = m_globalVarsMap.value(index);
  SimulatorInterface::gVarMode_t gv(value);
  QLabel * lbl;

  if (fmMap.contains(gv.mode) && (lbl = fmMap.value(gv.mode))) {
    GVarData gvar;
    gvar.prec = gv.prec;
    gvar.unit = gv.unit;
    lbl->setText(QString::number(gv.value * gvar.multiplierGet(), 'f', gv.prec) + gvar.unitToString());
  }
  //qDebug() << index << value << gv.mode << gv.value << gv.prec << gv.unit;
}

void RadioOutputsWidget::onPhaseChanged(qint32 phase, const QString &)
{
  QPalette::ColorRole fgrole, bgrole;
  QLabel * lbl;
  QFont font;
  QHash<int, QHash<int, QLabel *> >::const_iterator gv;
  QHash<int, QLabel *>::const_iterator fm;

  for (gv = m_globalVarsMap.constBegin(); gv != m_globalVarsMap.constEnd(); ++gv) {
    for (fm = gv.value().constBegin(); fm != gv.value().constEnd(); ++fm) {
      lbl = fm.value();
      font = lbl->font();
      if (fm.key() == phase) {
        fgrole =  QPalette::BrightText;
        bgrole = QPalette::Dark;
        font.setBold(true);
      }
      else {
        fgrole =  QPalette::WindowText;
        bgrole = ((gv.key() % 2) ? QPalette::Window : QPalette::AlternateBase);
        font.setBold(false);
      }
      lbl->setForegroundRole(fgrole);
      lbl->setBackgroundRole(bgrole);
      lbl->setFont(font);
    }
  }
  //qDebug() << phase;
}
