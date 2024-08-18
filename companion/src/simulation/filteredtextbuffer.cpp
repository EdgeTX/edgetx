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

#include "filteredtextbuffer.h"

#include <QDebug>
#include <QElapsedTimer>

/*
 *
 * FIFOBufferDevice
 *
 */

FIFOBufferDevice::FIFOBufferDevice(QObject * parent) :
  QBuffer(parent),
  m_hasOverflow(false)
{
  setDataBufferMaxSize(20 * 1024);
}

void FIFOBufferDevice::setDataBufferMaxSize(int size)
{
  QWriteLocker locker(&m_dataRWLock);
  if (buffer().capacity() > size) {
    trimData(buffer().capacity() - size);
    buffer().squeeze();
  }
  buffer().reserve(size);
}

// Remove data from beginning of storage array.
// NOT thread-safe, lock data before use (this avoids needing a recursive mutex)
int FIFOBufferDevice::trimData(int len)
{
  if (!isWritable() || len <= 0)
    return 0;

  len = qMin(len, buffer().size());
  buffer().remove(0, len);
  QIODevice::seek(0);
  return len;
}

int FIFOBufferDevice::nextLineLen() const
{
  QReadLocker locker(&m_dataRWLock);
  return buffer().indexOf('\n', 0) + 1;
}

qint64 FIFOBufferDevice::writeData(const char * data, qint64 len)
{
  if (!isWritable())
    return -1;

  QWriteLocker locker(&m_dataRWLock);

  // Handle overflow
  if (size() + len > getDataBufferMaxSize()) {
    m_hasOverflow = true;
    const int tlen = trimData(size() + len - getDataBufferMaxSize());
    emit bufferOverflow(qint64(tlen));
  }
  else if (m_hasOverflow) {
    m_hasOverflow = false;
    emit bufferOverflow(0);
  }

  // Always write to end of stream.
  if (!QIODevice::seek(size()))
    return -1;

  // Save the data
  len = QBuffer::writeData(data, len);
  // Make sure we're always ready for reading, this makes bytesAvailable() (et.al.) return correct result.
  QIODevice::seek(0);

  return len;
}

qint64 FIFOBufferDevice::readData(char * data, qint64 len)
{
  len = qMin(len, qint64(buffer().size()));
  if (!isReadable() || len <= 0)
    return 0;

  QWriteLocker locker(&m_dataRWLock);
  // Always take data from top
  if (QIODevice::seek(0)) {
    memcpy(data, buffer().constData(), len);
    buffer().remove(0, len);
  }
  else {
    len = -1;
  }
  return len;
}

qint64 FIFOBufferDevice::readLine(char * data, qint64 maxSize)
{
  if (maxSize < 2)
    return -1;

  int len = nextLineLen();
  if (len <= 0)
    return 0;

  len = qMin(len, int(maxSize));
  return readData(data, qint64(len));
}

QByteArray FIFOBufferDevice::readLine(qint64 maxSize)
{
  if (maxSize < 0)
    return QByteArray();

  int len = nextLineLen();
  if (len <= 0)
    return QByteArray();

  if (maxSize > 1)
    len = qMin(len, int(maxSize - 1));

  QByteArray ba(len, 0);
  len = int(readData(ba.data(), qint64(len)));

  if (len < 0)
    len = 0;
  if (len < ba.size())
    ba.resize(len);

  return ba;
}


/*
 *
 * FilteredTextBuffer
 *
 */

FilteredTextBuffer::FilteredTextBuffer(QObject * parent) :
  FIFOBufferDevice(parent),
  m_inBuffer(new FIFOBufferDevice(this)),
  m_bufferFlushTimer(new QTimer(this)),
  m_lineFilterEnable(false),
  m_lineFilterExclusive(false)
{
  m_bufferFlushTimer->setSingleShot(true);
  setInputBufferMaxSize(5 * 1024);
  setInputBufferTimeout(1500);

  connect(m_inBuffer, &FIFOBufferDevice::readyRead, this, &FilteredTextBuffer::processInputBuffer, Qt::QueuedConnection);
  connect(m_inBuffer, &FIFOBufferDevice::bytesWritten, this, &FilteredTextBuffer::onInputBufferWrite, Qt::QueuedConnection);
  connect(m_inBuffer, &FIFOBufferDevice::bufferOverflow, this, &FilteredTextBuffer::onInputBufferOverflow, Qt::QueuedConnection);
  connect(m_bufferFlushTimer, &QTimer::timeout, this, &FilteredTextBuffer::processInputBuffer);
  connect(this, &FilteredTextBuffer::timerStart, m_bufferFlushTimer, static_cast<void (QTimer::*)(void)>(&QTimer::start));
  connect(this, &FilteredTextBuffer::timerStop, m_bufferFlushTimer, &QTimer::stop);
}

FilteredTextBuffer::~FilteredTextBuffer()
{
  if (m_bufferFlushTimer) {
    disconnect(m_bufferFlushTimer, 0, this, 0);
    disconnect(this, 0, m_bufferFlushTimer, 0);
  }
  if (m_inBuffer) {
    closeInputBuffer();
    disconnect(m_inBuffer, 0, this, 0);
  }
}

void FilteredTextBuffer::setInputBufferMaxSize(int size)
{
  m_inBuffer->setDataBufferMaxSize(size);
}

void FilteredTextBuffer::setInputBufferTimeout(int ms)
{
  m_bufferFlushTimer->setInterval(ms);
}

void FilteredTextBuffer::setLineFilterExpr(const QRegularExpression & expr)
{
  if (!expr.pattern().isEmpty() && expr.isValid())
    m_lineFilter = expr;
  else
    setLineFilterEnabled(false);
}

void FilteredTextBuffer::setLineFilterEnabled(bool enable)
{
  if (enable && !m_inBuffer->isOpen())
    openInputBuffer();
  else if (!enable)
    closeInputBuffer();

  m_lineFilterEnable = enable;
}

void FilteredTextBuffer::setLineFilterExclusive(bool exclusive)
{
  m_lineFilterExclusive = exclusive;
}

void FilteredTextBuffer::setLineFilter(bool enable, bool exclusive, const QRegularExpression & expr)
{
  setLineFilterEnabled(enable);
  setLineFilterExclusive(exclusive);
  setLineFilterExpr(expr);
}

qint64 FilteredTextBuffer::writeDataSuper(const char * data, qint64 len)
{
  if (len < 0)
    len = qstrlen(data);

  return FIFOBufferDevice::writeData(data, len);
}

qint64 FilteredTextBuffer::writeData(const char * data, qint64 len)
{
  if (!isWritable() || len < 0)
    return -1;
  if (!len)
    return 0;

  const qint64 maxSz = qint64(getInputBufferMaxSize());
  // if filter is disabled, invalid, or input buffer failure, write directly to output buffer
  if (!m_inBuffer->isOpen() || len > maxSz)
    return writeDataSuper(data, len);

  // check for input buffer overflow
  if (m_inBuffer->bytesAvailable() + len > maxSz) {
    flushInputBuffer();
    onInputBufferOverflow(m_inBuffer->bytesAvailable() + len - maxSz);
  }

  return m_inBuffer->write(data, len);
}

void FilteredTextBuffer::flushInputBuffer()
{
  emit timerStop();
  if (m_inBuffer->bytesAvailable()) {
    const QByteArray text = m_inBuffer->readAll();
    writeDataSuper(text.constData(), qint64(text.size()));
  }
}

void FilteredTextBuffer::closeInputBuffer()
{
  if (!m_inBuffer->isOpen())
    return;

  flushInputBuffer();
  m_inBuffer->close();
}

bool FilteredTextBuffer::openInputBuffer()
{
  if (m_inBuffer->isOpen())
    closeInputBuffer();

  return m_inBuffer->open(ReadWrite | Text);
}

void FilteredTextBuffer::processInputBuffer()
{
  if (!m_lineFilterEnable) {
    return closeInputBuffer();
  }
  else if (sender() == m_bufferFlushTimer) {
    return flushInputBuffer();
  }

  emit timerStop();
  while (m_lineFilterEnable && m_inBuffer->canReadLine()) {
    const QByteArray text = m_inBuffer->readLine();
    // If line matches inclusive filter or does not match exclusive filter, write it to output buffer.
    if (!text.isEmpty() && m_lineFilterExclusive != QString(text).contains(m_lineFilter))
      writeDataSuper(text.constData(), qint64(text.size()));
  }
  // restart timer if unread bytes still remain
  if (m_inBuffer->bytesAvailable() > 0)
    emit timerStart();
}

void FilteredTextBuffer::onInputBufferWrite(qint64)
{
  emit timerStart();
}

void FilteredTextBuffer::onInputBufferOverflow(const qint64 len)
{
  static QElapsedTimer reportTimer;

  if (len <= 0) {
    reportTimer.invalidate();
  }
  else if (!reportTimer.isValid() || reportTimer.elapsed() > 1000 * 15) {
    qWarning("Input data buffer overflow by %lld bytes!", len);
    reportTimer.start();
  }
}
