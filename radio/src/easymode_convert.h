/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include "easymode.h"

// Generate standard EdgeTX mixData/expoData/limitData from easy mode config.
// Clears existing mixes/inputs first, then generates new ones.
void easyModeApply(const EasyModeData& em);

// Clear the easyMode flag (convert to expert mode).
// The generated mixes are kept as-is.
void easyModeConvertToExpert();
