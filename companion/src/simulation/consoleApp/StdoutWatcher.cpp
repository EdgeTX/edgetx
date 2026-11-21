/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "StdoutWatcher.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <QElapsedTimer>
#include <QTimer>
#include <cstdio>
#include <iostream>
#include <thread>

namespace SimulatorCli
{

StdoutWatcher& StdoutWatcher::instance()
{
  static StdoutWatcher inst;
  return inst;
}

void StdoutWatcher::start()
{
  if (m_started.exchange(true)) return;

  std::fflush(stdout);
  std::setvbuf(stdout, nullptr, _IONBF, 0);

  int pipeFds[2];
#ifdef _WIN32
  m_origStdoutFd = _dup(_fileno(stdout));
  if (m_origStdoutFd == -1 || _pipe(pipeFds, 4096, _O_BINARY) != 0) {
    return;
  }
  _dup2(pipeFds[1], _fileno(stdout));
  _close(pipeFds[1]);
#else
  m_origStdoutFd = dup(STDOUT_FILENO);
  if (m_origStdoutFd == -1 || pipe(pipeFds) != 0) {
    return;
  }
  dup2(pipeFds[1], STDOUT_FILENO);
  close(pipeFds[1]);
#endif

  const int readFd = pipeFds[0];
  std::thread(&StdoutWatcher::readerLoop, this, readFd).detach();
}

void StdoutWatcher::setCapturing(bool enabled)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_capturing.store(enabled);
  if (!enabled) {
    m_buffer.clear();
  }
}

bool StdoutWatcher::contains(const std::string& text) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_buffer.find(text) != std::string::npos;
}

void waitForLogOutput(QObject* context, const std::string& expectedText,
                      int timeoutMs, std::function<void()> onCompleted)
{
  if (StdoutWatcher::instance().contains(expectedText)) {
    onCompleted();
    return;
  }

  if (!context) {
    onCompleted();
    return;
  }

  auto* timer = new QTimer(context);
  QElapsedTimer elapsed;
  elapsed.start();

  QObject::connect(timer, &QTimer::timeout, context,
                   [timer, expectedText, timeoutMs, elapsed, onCompleted]() {
                     if (StdoutWatcher::instance().contains(expectedText)) {
                       timer->stop();
                       timer->deleteLater();
                       onCompleted();
                     } else if (elapsed.hasExpired(timeoutMs)) {
                       std::cerr
                           << "[WARN] Timeout waiting for: " << expectedText
                           << "\n"
                           << std::flush;
                       timer->stop();
                       timer->deleteLater();
                       onCompleted();
                     }
                   });

  timer->start(50);
}

void StdoutWatcher::writeRawLocked(const char* data, size_t size)
{
  if (m_origStdoutFd == -1) {
    std::cout.write(data, static_cast<std::streamsize>(size));
    std::cout.flush();
    return;
  }

  const char* ptr = data;
  size_t remaining = size;
  while (remaining > 0) {
#ifdef _WIN32
    int written =
        _write(m_origStdoutFd, ptr, static_cast<unsigned int>(remaining));
#else
    ssize_t written = write(m_origStdoutFd, ptr, remaining);
#endif
    if (written <= 0) break;
    if (static_cast<size_t>(written) >= remaining) break;
    ptr += written;
    remaining -= static_cast<size_t>(written);
  }
}

void StdoutWatcher::readerLoop(int readFd)
{
  char buf[1024];
  while (true) {
#ifdef _WIN32
    int n = _read(readFd, buf, sizeof(buf));
#else
    ssize_t n = read(readFd, buf, sizeof(buf));
#endif
    if (n <= 0) break;

    {
      std::lock_guard<std::mutex> lock(m_writeMutex);
      writeRawLocked(buf, static_cast<size_t>(n));
      m_atLineStart = (buf[n - 1] == '\n');
    }

    if (m_capturing.load()) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_buffer.append(buf, static_cast<size_t>(n));
    }
  }

#ifdef _WIN32
  _close(readFd);
#else
  close(readFd);
#endif
}

void StdoutWatcher::writeDirect(const std::string& text)
{
  if (text.empty()) return;

  std::lock_guard<std::mutex> lock(m_writeMutex);
  if (!m_atLineStart) {
    writeRawLocked("\n", 1);
  }
  writeRawLocked(text.data(), text.size());
  m_atLineStart = (text.back() == '\n');
}
}  // namespace SimulatorCli