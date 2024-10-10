/*
 * Copyright (C) EdgeTx
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include <stdint.h>

enum ChannelsProtocols {
  PROTOCOL_CHANNELS_UNINITIALIZED,
  PROTOCOL_CHANNELS_NONE,
  PROTOCOL_CHANNELS_PPM,
  PROTOCOL_CHANNELS_PXX1,
  PROTOCOL_CHANNELS_DSM2,
  PROTOCOL_CHANNELS_CROSSFIRE,
  PROTOCOL_CHANNELS_MULTIMODULE,
  PROTOCOL_CHANNELS_SBUS,
  PROTOCOL_CHANNELS_PXX2,
  PROTOCOL_CHANNELS_AFHDS2A,
  PROTOCOL_CHANNELS_AFHDS3,
  PROTOCOL_CHANNELS_GHOST,
  PROTOCOL_CHANNELS_DSMP
};

struct etx_proto_driver_t {

    // Protocol implemented by this module driver
    ChannelsProtocols protocol;
    
    // Init module communication
    void* (*init)(uint8_t module);

    // De-Init module communication
    void (*deinit)(void* ctx);

    // Send the next pulse frame
    void (*sendPulses)(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels);

    // Process input data byte (telemetry)
    void (*processData)(void* ctx, uint8_t data, uint8_t* buffer, uint8_t* len);

    // Process input data byte (telemetry)
    void (*processFrame)(void* ctx, uint8_t* frame, uint8_t flen, uint8_t* buf, uint8_t* len);

    // Some module settings may have been modified
    void (*onConfigChange)(void* ctx);

    // Module state mode has been modified
    // void (*onModeChange)(void* ctx, uint8_t old_mode, uint8_t new_mode);
};
