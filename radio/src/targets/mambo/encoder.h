// SPDX-License-Identifier: GPL-2.0-only
#pragma once

namespace mambo {
// Call after rotaryEncoderInit has configured the GPIO and timer interrupt.
void encoderStartPolling();
}  // namespace mambo
