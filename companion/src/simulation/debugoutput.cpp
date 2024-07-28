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

#include "debugoutput.h"
#include "ui_debugoutput.h"

#include "appdata.h"
#include "filteredtextbuffer.h"

#include <AppDebugMessageHandler>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollBar>
#include <QThread>
#include <QDebug>

#define DEBUG_OUTPUT_STATE_VERSION    1

extern AppData g;  // ensure what "g" means

const quint16 DebugOutput::m_savedViewStateVersion = 1;

DebugOutput::DebugOutput(QWidget * parent, SimulatorInterface *simulator):
  QWidget(parent),
  ui(new Ui::DebugOutput),
  m_simulator(simulator),
  m_dataBufferDevice(NULL),
  m_radioProfileId(g.sessionId()),
  m_filterEnable(false),
  m_filterExclude(false)
{
  ui->setupUi(this);

#ifdef __APPLE__
  ui->console->setFont(QFont("Courier", 13));
#endif

  // TODO : allow selecting multiple filters, but needs to be efficient at output stage

  QStringList stockFilters;
  stockFilters << "/^(lua[A-Z]|script).*/i";
  stockFilters << "/(error|warning|-(E|W)-)/i";
  stockFilters << "!^(GC Use|(play|load|write|find(True)?)File|convert(To|From)Simu|\\t(not found|found( in map|\\:))|eeprom |f_[a-z]+\\(|(push|(p|P)op(up)?|chain)? ?Menu( .+ display)?|RamBackup).*$";

  foreach (const QString & fltr, stockFilters)
    ui->filterText->addItem(fltr, "no_delete");

  ui->filterText->setValidator(new DebugOutputFilterValidator(ui->filterText));
  ui->filterText->installEventFilter(new DeleteComboBoxItemEventFilter(this));

  ui->actionShowFilterHelp->setIcon(SimulatorIcon("info"));
  ui->actionWordWrap->setIcon(SimulatorIcon("word_wrap"));
  ui->actionClearScr->setIcon(SimulatorIcon("eraser"));

  ui->btnFilter->setDefaultAction(ui->actionToggleFilter);
  ui->btnShowFilterHelp->setDefaultAction(ui->actionShowFilterHelp);
  ui->btnWordWrap->setDefaultAction(ui->actionWordWrap);
  ui->btnClearScr->setDefaultAction(ui->actionClearScr);

  m_dataBufferDevice = new FilteredTextBuffer();
  m_dataBufferDevice->setDataBufferMaxSize(DEBUG_OUTPUT_WIDGET_OUT_BUFF_SIZE);
  m_dataBufferDevice->setInputBufferMaxSize(DEBUG_OUTPUT_WIDGET_INP_BUFF_SIZE);
  m_dataBufferDevice->open(QIODevice::ReadWrite | QIODevice::Text);

  connect(m_dataBufferDevice, &FilteredTextBuffer::readyRead, this, &DebugOutput::processBytesReceived, Qt::QueuedConnection);
  connect(m_dataBufferDevice, &FilteredTextBuffer::bufferOverflow, this, &DebugOutput::onDataBufferOverflow, Qt::QueuedConnection);
  connect(this, &DebugOutput::filterChanged, m_dataBufferDevice, &FilteredTextBuffer::setLineFilter);
  connect(this, &DebugOutput::filterEnabledChanged, m_dataBufferDevice, &FilteredTextBuffer::setLineFilterEnabled);
  connect(this, &DebugOutput::filterExprChanged, m_dataBufferDevice, &FilteredTextBuffer::setLineFilterExpr);
  connect(this, &DebugOutput::filterExclusiveChanged, m_dataBufferDevice, &FilteredTextBuffer::setLineFilterExclusive);

  restoreState();

  ui->bufferSize->setValue(ui->console->maximumBlockCount());

  connect(ui->actionToggleFilter, &QAction::toggled, this, &DebugOutput::onFilterToggled);
  connect(ui->filterText, &QComboBox::currentTextChanged, this, &DebugOutput::onFilterTextChanged);

  if (AppDebugMessageHandler::instance()) {
    // send application Info/Warning/Error events to our data collector
    m_dataBufferDevice->setProperty("level", 1);
    AppDebugMessageHandler::instance()->addOutputDevice(m_dataBufferDevice);
  }

  // send firmware TRACE events to our data collector
  m_simulator->addTracebackDevice(m_dataBufferDevice);
}

DebugOutput::~DebugOutput()
{
  if (m_dataBufferDevice) {
    if (AppDebugMessageHandler::instance())
      AppDebugMessageHandler::instance()->removeOutputDevice(m_dataBufferDevice);
    m_simulator->removeTracebackDevice(m_dataBufferDevice);
    disconnect(m_dataBufferDevice, 0, this, 0);
    disconnect(this, 0, m_dataBufferDevice, 0);
    delete m_dataBufferDevice;
    m_dataBufferDevice = Q_NULLPTR;
  }

  saveState();

  delete ui;
}

void DebugOutput::saveState()
{
  QStringList filters;
  for (int i = 0; i < ui->filterText->count(); ++i) {
    if (!ui->filterText->itemText(i).isEmpty() && ui->filterText->itemData(i).toString() != "no_delete")
      filters << ui->filterText->itemText(i);
  }
  g.simuDbgFilters(filters);

  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  stream << m_savedViewStateVersion
         << (qint16)ui->filterText->currentIndex() << (qint32)ui->console->maximumBlockCount()
         << m_filterEnable << ui->actionWordWrap->isChecked();

  SimulatorOptions opts = g.profile[m_radioProfileId].simulatorOptions();
  opts.dbgConsoleState = state;
  g.profile[m_radioProfileId].simulatorOptions(opts);
}

void DebugOutput::restoreState()
{
  quint16 ver = 0;
  qint16 fci = -1;
  qint32 mbc = 10000;
  bool flten = false, wwen = false;
  QByteArray state = g.profile[m_radioProfileId].simulatorOptions().dbgConsoleState;
  QDataStream stream(state);

  stream >> ver;
  if (ver && ver <= m_savedViewStateVersion)
    stream >> fci >> mbc >> flten >> wwen;

  ui->filterText->insertItems(0, g.simuDbgFilters());
  ui->filterText->setCurrentIndex(fci);
  ui->console->setMaximumBlockCount(mbc);
  ui->actionWordWrap->setChecked(wwen);

  onFilterToggled(flten);
}

void DebugOutput::processBytesReceived()
{
  static QByteArray text;
  if (text.capacity() < 512)
    text.reserve(512);
  const QTextCursor savedCursor(ui->console->textCursor());
  const int sbValue = ui->console->verticalScrollBar()->value();
  const bool sbAtBottom = (sbValue == ui->console->verticalScrollBar()->maximum());

  while (m_dataBufferDevice && m_dataBufferDevice->bytesAvailable() > 0) {
    text = m_dataBufferDevice->read(qint64(text.capacity()));
    if (text.isEmpty())
      break;
    ui->console->moveCursor(QTextCursor::End);
    ui->console->textCursor().insertText(QString(text));
    if (sbAtBottom) {
      ui->console->moveCursor(QTextCursor::End);
      ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
    }
    else {
      ui->console->setTextCursor(savedCursor);
      ui->console->verticalScrollBar()->setValue(sbValue);
    }
    QCoreApplication::processEvents();
  }
}

void DebugOutput::onDataBufferOverflow(const qint64 len)
{
  static QElapsedTimer reportTimer;

  if (len <= 0) {
    reportTimer.invalidate();
  }
  else if (!reportTimer.isValid() || reportTimer.elapsed() > 1000 * 30) {
    qWarning("Data buffer overflow by %lld bytes!", len);
    reportTimer.start();
  }
}

/*
 * UI handlers
 */

void DebugOutput::onFilterStateChanged()
{
  const QString fText = ui->filterText->currentText();
  if (fText.isEmpty()) {
    onFilterToggled(false);
    return;
  }

  QRegularExpression filterRegEx = makeRegEx(fText, &m_filterExclude);

  if (!m_filterEnable || filterRegEx.isValid())
    ui->filterText->setStyleSheet("");
  else if (m_filterEnable)
    ui->filterText->setStyleSheet("background-color: rgba(255, 205, 185, 200);");

  if (filterRegEx.isValid())
    emit filterChanged(m_filterEnable, m_filterExclude, filterRegEx);
  else
    onFilterToggled(false);
}

void DebugOutput::onFilterTextChanged(const QString &)
{
  onFilterStateChanged();
}

void DebugOutput::onFilterToggled(bool enable)
{
  if (enable != m_filterEnable) {
    m_filterEnable = enable;
    if (ui->actionToggleFilter->isChecked() != enable)
      ui->actionToggleFilter->setChecked(enable);
    if (enable)
      onFilterStateChanged();
    else
      emit filterEnabledChanged(false);
  }
}

void DebugOutput::on_bufferSize_editingFinished()
{
  ui->console->setMaximumBlockCount(ui->bufferSize->value());
}

void DebugOutput::on_actionWordWrap_toggled(bool checked)
{
  ui->console->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void DebugOutput::on_actionClearScr_triggered()
{
  ui->console->clear();
}

void DebugOutput::on_actionShowFilterHelp_triggered()
{
  // TODO : find some place better for this.
  QString help = tr( \
    "<html><head><style>kbd {background-color: palette(alternate-base); font-size: large; white-space: nowrap;}</style></head><body>"
    "<p>The filter supports two syntax types: basic matching with common wildcards as well as full Perl-style (<code>pcre</code>) Regular Expressions.</p>"
    "<p>By default a filter will only show lines which match (<b>inclusive</b>). To make an <b>exclusive</b> filter which removes matching lines, "
      "prefix the filter expression with a <kbd>!</kbd> (exclamation mark).</p>"
    "<p>To use <b>Regular Expressions</b> (RegEx), prefix the filter text with a <kbd>/</kbd> (slash) or <kbd>^</kbd> (up caret). "
      "<ul>"
      "<li>Put the <kbd>/</kbd> or <kbd>^</kbd> after the exclusive <kbd>!</kbd> indicator if you're using one.</li>"
      "<li>By default the match is case-sensitive. To make it insensitive, add the typical <kbd>/i</kbd> (slash i) operator at the end of your RegEx.</li>"
      "<li>If you use a caret (^) to denote a RegEx, it will become part of the Reg. Ex. (that is, matches from start of line).</li>"
      "<li>If the RegEx is invalid, the filter edit field should show a red border and you will not be able to enable the filter.</li>"
      "<li>A useful resource for testing REs (with a full reference) can be found at <a href=\"http://www.regexr.com/\">http://www.regexr.com/</a></li>"
    "</ul></p>"
    "<p>To use <b>basic matching</b> just type any text."
    "<ul>"
      "<li>Wildcards: <kbd>*</kbd> (asterisk) matches zero or more of any character(s), and <kbd>?</kbd> (question mark) matches any single character.</li>"
      "<li>The match is always case-insensitive.</li>"
      "<li>The match always starts from the beginning of a log line. To ignore characters at the start, use a leading <kbd>*</kbd> wildcard.</li>"
      "<li>A trailing <kbd>*</kbd> is always implied (that is, matches anything to the end of the log line). To avoid this, use a RegEx.</li>"
      "<li>You can match literal wildcard characters by prefixing them with a <kbd>\\</kbd> (backslash) character (eg. \"foo\\*bar\" matches \"foo*bar\").</li>"
    "</ul></p>"
    "<p>After <b>editing text</b>, press ENTER or TAB key (or click anywhere outside the box) to update the filter.</p>"
    "<p>To <b>remove an entry</b> from the filter selector list, first choose it, and while in the line editor press <kbd>Shift-Delete</kbd> (or <kbd>Shift-Backspace</kbd>) key combination. "
      "The default filters cannot be removed. Up to 50 filters are stored.</p>"
    "</body></html>"
  );
  QMessageBox * msgbox = new QMessageBox(QMessageBox::NoIcon, tr("Debug Console Filter Help"), help, QMessageBox::Ok, this);
  msgbox->exec();
}


// static
QRegularExpression DebugOutput::makeRegEx(const QString & input, bool * isExlusive)
{
  QString output(input);
  QRegularExpression re;
  QRegularExpression::PatternOptions reFlags = QRegularExpression::DontCaptureOption;

  if (input.left(1) == "!") {
    output.remove(0, 1);
    if (isExlusive)
      *isExlusive = true;
  }
  else if (isExlusive) {
    *isExlusive = false;
  }
  // regex?
  if (output.left(1) == "/" || output.left(1) == "^") {
    if (output.left(1) == "/")
      output.remove(0, 1);
    // check for case-insensitive flag at end ("/.../i")
    if (output.endsWith("/i")) {
      output.chop(2);
      reFlags |= QRegularExpression::CaseInsensitiveOption;
    }
    else if (output.endsWith("/")) {
      output.chop(1);
    }
    re.setPattern(output);
  }
  // no, convert arbitrary string to regex
  else {
    output.replace(QRegExp("^\\\\/"), "/");  // remove escape before fwd-slash ("\/...")
    // escape all special chars except * and ?
    output.replace(QRegExp("(\\\\|\\.|\\+|\\^|\\$|\\||\\)|\\(|\\]|\\[|\\}|\\{)"), "\\\\1");
    output.replace("\\*", "\x30").replace("\\?", "\x31");  // save escaped wildcard chars
    output.replace("*", ".*").replace("?", ".");  // convert common wildcards
    output.replace("\x30", "\\\\*").replace("\x31", "\\\\?");  // replace escaped wildcard chars
    output.prepend("^"); // match from start of line; .append("$");  // match whole line
    reFlags |= QRegularExpression::CaseInsensitiveOption;
    re.setPattern(output);
  }
  re.setPatternOptions(reFlags);
  return re;
}


/*
 * Filter input validator for RegEx syntax.
 */

QValidator::State DebugOutputFilterValidator::validate(QString & input, int &) const
{
  QRegularExpression re = DebugOutput::makeRegEx(input);
  if (re.isValid())
    return QValidator::Acceptable;
  else
    return QValidator::Intermediate;
}

/*
 * Event filter for editable QComboBox to allow deleting items with Shift-Delete/Backspace
 */

bool DeleteComboBoxItemEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  QComboBox * cb = dynamic_cast<QComboBox *>(obj);
  if (cb && cb->isEditable() && cb->currentIndex() > -1 && cb->currentData().toString() != "no_delete") {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      if ((keyEvent->key() == Qt::Key::Key_Delete || keyEvent->key() == Qt::Key::Key_Backspace) && keyEvent->modifiers() == Qt::ShiftModifier) {
        cb->removeItem(cb->currentIndex());
        cb->setCurrentIndex(-1);
        return true;
      }
    }
  }
  return QObject::eventFilter(obj, event);
}
