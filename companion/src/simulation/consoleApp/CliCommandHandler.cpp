/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "CliCommandHandler.h"

#include <QDateTime>
#include <QMetaObject>
#include <QString>
#include <cctype>
#include <iostream>
#include <thread>
#include "StdoutWatcher.h"

namespace SimulatorCli
{

CliCommandHandler::CliCommandHandler(SimulatorController& controller,
                                     QCoreApplication& app) :
    m_controller(controller),
    m_app(app),
    m_alive(std::make_shared<std::atomic<bool>>(true))
{
}

CliCommandHandler::~CliCommandHandler() { m_alive->store(false); }

void CliCommandHandler::startAsync()
{
  auto alive = m_alive;
  std::thread([this, alive]() { readerLoop(alive); }).detach();
}

void CliCommandHandler::readerLoop(std::shared_ptr<std::atomic<bool>> alive)
{
  std::string line;
  while (alive->load() && std::getline(std::cin, line)) {
    if (!alive->load()) break;
    if (!processLine(line)) {
      break;
    }
  }
}

bool CliCommandHandler::processLine(const std::string& rawLine)
{
  std::string line = rawLine;
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }

  std::stringstream ss(line);
  std::string rawCmd;
  if (!(ss >> rawCmd)) {
    return true;
  }

  const std::string cmd = toUpper(rawCmd);
  int ackDelayMs = 300;
  bool validCommand = true;

  if (cmd == "EXIT") {
    printAck(cmd);
    QMetaObject::invokeMethod(&m_app, &QCoreApplication::quit,
                              Qt::QueuedConnection);
    return false;
  }

  if (cmd == "RESTART") {
    const bool started =
        m_controller.restart([this, line]() { printAck(line); });
    if (!started) {
      printErr("[WARN] Restart already in progress\n");
    }
    return true;
  }

  if (cmd == "CHANNELS") {
    printChannels();
  } else if (cmd == "HAPTIC") {
    printHaptic();
  } else if (cmd == "THROTTLE" || cmd == "YAW" || cmd == "PITCH" ||
             cmd == "ROLL") {
    handleStick(cmd, ss);
  } else if (cmd == "POT" || cmd == "KNOB") {
    handlePot(ss);
  } else if (cmd == "6POS" || cmd == "POS6") {
    handle6Pos(ss);
    ackDelayMs = 500;
  } else if (cmd == "SW" || cmd == "SWITCH") {
    handleSwitch(ss);
  } else if (cmd == "SCREENSHOT" || cmd == "SHOT") {
    handleScreenshot(ss);
  } else if (cmd == "KEY") {
    int keyId = 0;
    if (ss >> keyId) {
      m_controller.pressKey(keyId);
    } else {
      validCommand = false;
    }
  } else {
    validCommand = false;
    printErr("[ERR] Unknown command: " + cmd + "\n");
  }

  if (validCommand) {
    scheduleAck(line, ackDelayMs);
  }

  return true;
}

void CliCommandHandler::printBanner() const
{
  printOut(
      "\n===================================================================\n"
      ">>> EdgeTX Simulator CLI Ready! Available commands:\n"
      "-------------------------------------------------------------------\n"
      "  THROTTLE <0..100>              ex: THROTTLE 0, THROTTLE 50\n"
      "  ROLL|PITCH|YAW <-100..100>    ex: PITCH -20, ROLL 50, YAW 0\n"
      "  SW <A..H> <UP|MID|DOWN>        ex: SW A UP, SW B CENTER, SW G DOWN\n"
      "  POT <S1|S2|LS|RS> <-100..100> ex: POT S1 0, POT S2 -100\n"
      "  6POS <1..6>                    ex: 6POS 1, 6POS 4\n"
      "  KEY <id>                       ex: KEY 1, KEY 2\n"
      "  SCREENSHOT [file]              Save LCD screenshot to PNG file\n"
      "  CHANNELS                       Print current 16 channel outputs\n"
      "  HAPTIC                         Check vibration status (ON / OFF)\n"
      "  RESTART                        Restart simulator firmware\n"
      "  EXIT                           Shutdown simulator and exit\n"
      "==================================================================="
      "\n\n");
}

void CliCommandHandler::handleStick(const std::string& cmd,
                                    std::stringstream& ss)
{
  double percent = 0.0;
  if (!(ss >> percent)) return;

  m_controller.setStick(cmd, percent);
}

void CliCommandHandler::handlePot(std::stringstream& ss)
{
  std::string potName;
  double percent = 0.0;
  if (!(ss >> potName >> percent)) return;

  potName = toUpper(potName);
  if (!m_controller.setPot(potName, percent)) {
    printErr("Pot " + potName + " is not available on this radio\n");
  }
}

void CliCommandHandler::handle6Pos(std::stringstream& ss)
{
  int pos = 1;
  if (!(ss >> pos)) return;

  if (!m_controller.set6Pos(pos)) {
    printErr("6POS is not available on this radio\n");
  }
}

void CliCommandHandler::handleSwitch(std::stringstream& ss)
{
  std::string sw, pos;
  if (!(ss >> sw >> pos)) {
    printSwitches();
    return;
  }

  const char swLetter = static_cast<char>(std::toupper(sw.back()));
  pos = toUpper(pos);

  SwitchPos state = SwitchPos::CENTER;
  if (pos == "UP") {
    state = SwitchPos::UP;
  } else if (pos == "DOWN") {
    state = SwitchPos::DOWN;
  } else if (pos == "CENTER" || pos == "MID") {
    state = SwitchPos::CENTER;
  }

  m_controller.setSwitch(swLetter, state);
}

void CliCommandHandler::handleScreenshot(std::stringstream& ss)
{
  std::string filePath;
  if (!(ss >> filePath)) {
    filePath = "screenshot.png";
  }

  const QString qFilePath = QString::fromStdString(filePath);
  if (m_controller.saveScreenshot(qFilePath)) {
    printOut("Screenshot saved to: " + filePath + "\n");
  } else {
    printOut("Failed to save screenshot to: " + filePath + "\n");
  }
}

void CliCommandHandler::printChannels() const
{
  const auto ch = m_controller.channels();
  QString channelsStr = "CHANNELS ";
  for (size_t i = 0; i < ch.size(); ++i) {
    channelsStr += QString::number(i + 1) + ":" + QString::number(ch[i]) + " ";
  }
  printOut(channelsStr.trimmed().toStdString() + "\n");
}

void CliCommandHandler::printHaptic() const
{
  printOut(std::string("HAPTIC ") +
           (m_controller.isHapticActive() ? "ON\n" : "OFF\n"));
}

void CliCommandHandler::printSwitches() const
{
  const auto swStates = m_controller.switches();
  std::string header;
  std::string states;

  header.reserve(swStates.size());
  states.reserve(swStates.size());

  for (size_t i = 0; i < swStates.size(); ++i) {
    header += static_cast<char>('A' + i);

    if (swStates[i] == SwitchPos::UP) {
      states += '^';
    } else if (swStates[i] == SwitchPos::DOWN) {
      states += 'v';
    } else {
      states += '-';
    }
  }

  printOut(header + "\n" + states + "\n");
}

void CliCommandHandler::scheduleAck(const std::string& cmd, int delayMs) const
{
  m_controller.invokeDelayed(delayMs, [this, cmd]() { printAck(cmd); });
}

void CliCommandHandler::printAck(const std::string& cmd) const
{
  const QString ts =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  printOut("[ACK] " + ts.toStdString() + " " + cmd + "\n");
}

void CliCommandHandler::printOut(const std::string& text) const
{
  StdoutWatcher::instance().writeDirect(text);
}

void CliCommandHandler::printErr(const std::string& text) const
{
  std::lock_guard<std::mutex> lock(m_coutMutex);
  std::cerr << text << std::flush;
}

std::string CliCommandHandler::toUpper(std::string str)
{
  for (char& c : str) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return str;
}

}  // namespace SimulatorCli