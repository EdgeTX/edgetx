/*
 * Copyright (C) EdgeTX
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

#include "eldb.h"

#include <cli.h>
#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <board_common.h>
#include <stamp.h>

#include "eldp.pb.h"
#include "encoders.h"
#include "rs.h"

void eldbReceive(uint8_t *buf, size_t bufLen, size_t dataLen) {
   if (dataLen != 0) {
      uint8_t buffer[128];

      edgetx_eldp_Response message = edgetx_eldp_Response_init_zero;

      pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

      message.content.systemInfo = edgetx_eldp_SystemInfo_init_default;
      message.content.systemInfo.osName.funcs.encode = &encode_string;
      message.content.systemInfo.osName.arg = (void*)"EdgeTX";
      message.content.systemInfo.version.major = VERSION_MAJOR;
      message.content.systemInfo.version.minor = VERSION_MINOR;
      message.content.systemInfo.version.patch = this_function_exists_in_c();
      message.content.systemInfo.has_version = true;
      message.content.systemInfo.versionTag.funcs.encode = &encode_string;
      #if defined(VERSION_TAG)
      message.content.systemInfo.versionTag.arg = (void*)VERSION_TAG;
      #else
      message.content.systemInfo.versionTag.arg = (void*)VERSION_SUFFIX;
      #endif
      message.content.systemInfo.codename.funcs.encode = &encode_string;
      message.content.systemInfo.codename.arg = (void*)CODENAME;
      message.content.systemInfo.gitTag.funcs.encode = &encode_string;
      message.content.systemInfo.gitTag.arg = (void*)GIT_STR;
      message.content.systemInfo.deviceIdentifier.funcs.encode = &encode_string;
      message.content.systemInfo.deviceIdentifier.arg = (void*)FLAVOUR;
      message.content.systemInfo.batteryVoltage = getBatteryVoltage();
      message.content.systemInfo.has_batteryVoltage = true;

      message.which_content = edgetx_eldp_Response_systemInfo_tag;

      size_t message_length;
      bool status;

      status = pb_encode(&stream, edgetx_eldp_Response_fields, &message);
      message_length = stream.bytes_written;

      cliELDPSend(buffer, message_length, PB_GET_ERROR(&stream));
   }
   // TODO: Implement
}