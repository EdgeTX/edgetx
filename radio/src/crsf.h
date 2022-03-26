#pragma once

#include "dataconstants.h"
#include "crc.h"
#include "telemetry/crossfire.h"

#define CRSF_BAUDRATE         420000

void processCrsfInput();
void crsfTrainerPauseCheck();

