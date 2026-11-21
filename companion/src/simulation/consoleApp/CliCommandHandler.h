/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <QCoreApplication>
#include <atomic>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#include "SimulatorController.h"

namespace SimulatorCli
{

class CliCommandHandler
{
 public:
  CliCommandHandler(SimulatorController& controller, QCoreApplication& app);
  ~CliCommandHandler();

  CliCommandHandler(const CliCommandHandler&) = delete;
  CliCommandHandler& operator=(const CliCommandHandler&) = delete;

  void startAsync();
  bool processLine(const std::string& line);
  void printBanner() const;

 private:
  void readerLoop(std::shared_ptr<std::atomic<bool>> alive);

  void handleStick(const std::string& cmd, std::stringstream& ss);
  void handlePot(std::stringstream& ss);
  void handle6Pos(std::stringstream& ss);
  void handleSwitch(std::stringstream& ss);
  void handleScreenshot(std::stringstream& ss);

  void printChannels() const;
  void printHaptic() const;
  void printSwitches() const;

  void scheduleAck(const std::string& cmd, int delayMs) const;
  void printAck(const std::string& cmd) const;
  void printOut(const std::string& text) const;
  void printErr(const std::string& text) const;

  static std::string toUpper(std::string str);

  SimulatorController& m_controller;
  QCoreApplication& m_app;

  std::shared_ptr<std::atomic<bool>> m_alive;
  mutable std::mutex m_coutMutex;
};

}  // namespace SimulatorCli