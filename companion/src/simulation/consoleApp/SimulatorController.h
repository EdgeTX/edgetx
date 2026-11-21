/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <QImage>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QThread>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "RadioProfile.h"

class SimulatorInterface;

namespace SimulatorCli
{

enum class SwitchPos : int8_t { UP = -1, CENTER = 0, DOWN = 1 };

class SimulatorController : public QObject
{
 public:
  SimulatorController(const QString& radioName, const QString& sdPath,
                      QObject* parent = nullptr);
  ~SimulatorController() override;

  SimulatorController(const SimulatorController&) = delete;
  SimulatorController& operator=(const SimulatorController&) = delete;

  bool start(std::function<void()> onReady = nullptr);
  bool restart(std::function<void()> onFinished = nullptr);
  void stop();
  void waitForShutdown();

  bool setStick(const std::string& stickName, double percent);
  bool setPot(const std::string& potName, double percent);
  bool set6Pos(int pos);
  bool setSwitch(char swLetter, SwitchPos pos);
  void pressKey(int key, int durationMs = 100);
  void invokeDelayed(int delayMs, std::function<void()> fn) const;

  const RadioProfile& profile() const { return m_profile; }
  std::array<int32_t, 16> channels() const;
  std::vector<SwitchPos> switches() const;
  bool isHapticActive(int64_t tickDurationMs = 1500) const;

  QImage grabLcdImage() const;
  bool saveScreenshot(const QString& filePath) const;

 private:
  void setupConnections();
  void bootAndWaitForReady(bool restoreState, int settleDelayMs,
                           std::function<void()> onCompleted);
  void setAnalogInternal(uint8_t axis, int16_t rawValue);

  template <typename Func>
  void invokeAsync(Func&& fn) const
  {
    if (!m_sim) return;
    QMetaObject::invokeMethod(m_sim, std::forward<Func>(fn),
                              Qt::QueuedConnection);
  }

  static int64_t nowMs();

  QString m_radioName;
  QString m_sdPath;
  RadioProfile m_profile;

  QThread* m_simuThread = nullptr;
  SimulatorInterface* m_sim = nullptr;

  std::array<std::atomic<int32_t>, 16> m_channels{};
  std::array<std::atomic<int16_t>, MAX_ANALOGS> m_analogs{};
  std::array<std::atomic<int8_t>, MAX_SWITCHES> m_switches{};

  std::atomic<int> m_lastIntensity{0};
  std::atomic<int64_t> m_lastChangeMs{0};
  std::atomic<bool> m_isRestarting{false};
};

}  // namespace SimulatorCli