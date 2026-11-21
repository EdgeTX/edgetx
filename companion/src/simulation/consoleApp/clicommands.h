#pragma once

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <QApplication>
#include <QString>
#include <atomic>
#include <cstdio>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "simulation/simulatorinterface.h"

extern qint32 currentChannels[16];
extern std::atomic<int> lastIntensity;
extern std::atomic<int64_t> lastChangeMs;

bool isHapticActive(int64_t tickDurationMs = 1500);

class StdoutWatcher
{
 public:
  static StdoutWatcher& instance()
  {
    static StdoutWatcher inst;
    return inst;
  }

  void start()
  {
    if (m_started.exchange(true)) return;

    fflush(stdout);
    setvbuf(stdout, nullptr, _IONBF, 0);

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
    std::thread reader([this, readFd]() {
      char buf[1024];
      while (true) {
#ifdef _WIN32
        int n = _read(readFd, buf, sizeof(buf));
#else
        ssize_t n = read(readFd, buf, sizeof(buf));
#endif
        if (n <= 0) break;

        if (m_origStdoutFd != -1) {
#ifdef _WIN32
          _write(m_origStdoutFd, buf, static_cast<unsigned int>(n));
#else
          (void)write(m_origStdoutFd, buf, static_cast<size_t>(n));
#endif
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
    });
    reader.detach();
  }

  void setCapturing(bool enabled)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_capturing.store(enabled);
    if (!enabled) {
      m_buffer.clear();
    }
  }

  bool contains(const std::string& text) const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_buffer.find(text) != std::string::npos;
  }

 private:
  std::atomic<bool> m_started{false};
  std::atomic<bool> m_capturing{true};
  int m_origStdoutFd{-1};
  mutable std::mutex m_mutex;
  std::string m_buffer;
};

void waitForLogOutput(SimulatorInterface* sim, const std::string& expectedText,
                      int timeoutMs, std::function<void()> onCompleted);

void startConsoleInputThread(QApplication& app, SimulatorInterface* sim,
                             const QString& radioName, const QString& sdPath);