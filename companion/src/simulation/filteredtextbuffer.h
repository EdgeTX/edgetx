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

#include <QBuffer>
#include <QRegularExpression>
#include <QReadWriteLock>
#include <QTimer>

/*
 * FIFOBufferDevice implements a thread-safe, asynchronous, buffered, FIFO I/O device based on QBuffer (which is a QIODevice).
 * Data is removed from the beginning after each read operation (read(), readAll(), readLine(), etc).
 * Also unlike QBuffer, it will NOT grow in an unlmited fashion. Even if never read, size is constrained to dataBufferMaxSize.
 * Default maximum buffer size 20KB.
 */
class FIFOBufferDevice : public QBuffer
{
  Q_OBJECT

  public:
    explicit FIFOBufferDevice(QObject * parent = Q_NULLPTR);

    //! @returns FIFO buffer limit in bytes.
    inline int getDataBufferMaxSize() const { return buffer().capacity(); }
    //! @param size in bytes.
    void setDataBufferMaxSize(int size);

    virtual inline qint64 bytesAvailable() const override { return size() || QIODevice::bytesAvailable(); }
    virtual inline bool canReadLine()      const override { return nextLineLen() > 0; }
    // reimplemented from QIODevice for efficiency
    qint64 readLine(char * data, qint64 maxSize);
    QByteArray readLine(qint64 maxSize = 0);

  signals:
    //! @param len is overflow size, <= 0 means overflow has cleared
    void bufferOverflow(qint64 len);

  protected:
    int trimData(int len);
    int nextLineLen() const;
    virtual qint64 writeData(const char * data, qint64 len) override;
    virtual qint64 readData(char * data, qint64 len) override;

    mutable QReadWriteLock m_dataRWLock;
    bool m_hasOverflow;
};


/*
 * FilteredTextBuffer implements a FIFOBufferDevice which can, optionally, have a line filter applied to it.
 * If no filter is set, it acts exactly like its parent class.
 * If a filter is set, incoming data is buffered until a full line (\n terminated) is available. The line is then
 *   compared against the filter, and either added to the normal output buffer or discarded.
 * If a full line is not found after a specified timeout (1500ms by default) then any data in the input buffer is
 *   flushed anyway.  The same is true if the input buffer overflows (max. 5KB by default).
 * Another FIFOBufferDevice is used as the input buffer.
 */
class FilteredTextBuffer : public FIFOBufferDevice
{
  Q_OBJECT

  public:
    explicit FilteredTextBuffer(QObject * parent = Q_NULLPTR);
    ~FilteredTextBuffer();

    inline int getInputBufferMaxSize() const { return m_inBuffer->getDataBufferMaxSize(); }
    inline int getInputBufferTimeout() const { return m_bufferFlushTimer->interval(); }

  public slots:
    //! @param size in bytes. Input buffer will be flushed if grows over this size.
    void setInputBufferMaxSize(int size);
    //! @param ms how often to flush the input buffer when less than whole lines are present.
    void setInputBufferTimeout(int ms);
    void setLineFilterExpr(const QRegularExpression & expr);
    void setLineFilterEnabled(bool enable);
    void setLineFilterExclusive(bool exclusive);
    void setLineFilter(bool enable, bool exclusive, const QRegularExpression & expr);

  signals:
    // these are used internally to toggle the input buffer timeout timer (we use signals for thread safety)
    void timerStop();
    void timerStart();

  protected:
    qint64 writeDataSuper(const char * data, qint64 len = -1);
    virtual qint64 writeData(const char * data, qint64 len) override;
    void flushInputBuffer();
    void closeInputBuffer();
    bool openInputBuffer();
    void processInputBuffer();
    void onInputBufferWrite(qint64);
    void onInputBufferOverflow(const qint64 len);

    FIFOBufferDevice * m_inBuffer;
    QTimer * m_bufferFlushTimer;
    QRegularExpression m_lineFilter;
    bool m_lineFilterEnable;
    bool m_lineFilterExclusive;
};
