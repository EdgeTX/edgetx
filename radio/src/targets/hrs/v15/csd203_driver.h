#pragma once

#include <stdint.h>

void csd203DriverInit(void);
void csd203DriverInvalidate(void);

bool csd203DriverEnsureReady(void);
bool csd203DriverReadVoltage(uint16_t &voltageMv);
bool csd203DriverReadCurrent(int16_t &currentMa);
