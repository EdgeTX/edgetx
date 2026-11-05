// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#define PWR_SWITCH_GPIO 42u
#define PWR_ON_GPIO 43u
void pwrInit();
void pwrOn();
void pwrOff();
bool pwrPressed();
bool pwrOffPressed();
void pwrResetHandler();
