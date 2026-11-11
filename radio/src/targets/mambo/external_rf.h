// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>

struct MamboExternalDiagnostics {
  uint32_t txFrames = 0, rxBytes = 0, uartErrors = 0, idleEvents = 0;
  uint32_t parseEvents = 0, validFrames = 0, crcErrors = 0, headerErrors = 0;
  uint8_t lastTxType = 0, lastRxType = 0, pins = 0;
  bool powered = false;
};
void mamboExternalTxObserved(uint8_t type);
void mamboExternalRxObserved(uint8_t byte, bool error);
void mamboExternalIdleObserved();
void mamboExternalParseObserved();
void mamboExternalFrameObserved(uint8_t type);
void mamboExternalCrcErrorObserved();
void mamboExternalHeaderErrorObserved();
MamboExternalDiagnostics mamboExternalDiagnostics();
