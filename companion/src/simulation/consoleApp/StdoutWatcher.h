/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <QObject>
#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace SimulatorCli
{

class StdoutWatcher
{
 public:
  static StdoutWatcher& instance();

  StdoutWatcher(const StdoutWatcher&) = delete;
  StdoutWatcher& operator=(const StdoutWatcher&) = delete;

  void start();
  void setCapturing(bool enabled);
  bool contains(const std::string& text) const;
  void writeDirect(const std::string& text);

 private:
  StdoutWatcher() = default;
  void readerLoop(int readFd);
  void writeRawLocked(const char* data, size_t size);

  std::atomic<bool> m_started{false};
  std::atomic<bool> m_capturing{true};
  int m_origStdoutFd{-1};
  bool m_atLineStart{true};
  mutable std::mutex m_mutex;
  mutable std::mutex m_writeMutex;
  std::string m_buffer;
};

void waitForLogOutput(QObject* context, const std::string& expectedText,
                      int timeoutMs, std::function<void()> onCompleted);

}  // namespace SimulatorCli