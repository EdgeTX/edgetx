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

#include "messages.h"

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <board_common.h>
#include <stamp.h>

#include "eldp.pb.h"
#include "encode_decode.h"

bool eldbDecodeRequest(uint8_t *buf, size_t len, pb_istream_t *target_stream, edgetx_eldp_Request* target) {
    pb_istream_t stream = pb_istream_from_buffer(buf, len);
        
    bool result = pb_decode(&stream, edgetx_eldp_Request_fields, target);
    *target_stream = stream;
    return result;
}

size_t eldbMakeSystemInfoMessage(uint8_t *targetBuf, size_t targetBufLen) {
    edgetx_eldp_Response message = edgetx_eldp_Response_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(targetBuf, targetBufLen);

    message.which_content = edgetx_eldp_Response_systemInfo_tag;

    message.content.systemInfo = edgetx_eldp_SystemInfo_init_default;
    message.content.systemInfo.osName.funcs.encode = &encodeString;
    message.content.systemInfo.osName.arg = (void*)"EdgeTX";
    message.content.systemInfo.version.major = VERSION_MAJOR;
    message.content.systemInfo.version.minor = VERSION_MINOR;
    message.content.systemInfo.version.patch = VERSION_REVISION;
    message.content.systemInfo.has_version = true;
    message.content.systemInfo.versionTag.funcs.encode = &encodeString;
    #if defined(VERSION_TAG)
    message.content.systemInfo.versionTag.arg = (void*)VERSION_TAG;
    #else
    message.content.systemInfo.versionTag.arg = (void*)VERSION_SUFFIX;
    #endif
    message.content.systemInfo.codename.funcs.encode = &encodeString;
    message.content.systemInfo.codename.arg = (void*)CODENAME;
    message.content.systemInfo.gitTag.funcs.encode = &encodeString;
    message.content.systemInfo.gitTag.arg = (void*)GIT_STR;
    message.content.systemInfo.deviceIdentifier.funcs.encode = &encodeString;
    message.content.systemInfo.deviceIdentifier.arg = (void*)FLAVOUR;
    message.content.systemInfo.batteryVoltage = getBatteryVoltage();
    message.content.systemInfo.has_batteryVoltage = true;

    pb_encode(&stream, edgetx_eldp_Response_fields, &message);
    return stream.bytes_written;
}

size_t eldbMakeErrorMessage(
    uint8_t *targetBuf,
    size_t targetBufLen,
    edgetx_eldp_Error_Type type,
    char *msg
) {
    edgetx_eldp_Response message = edgetx_eldp_Response_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(targetBuf, targetBufLen);

    message.which_content = edgetx_eldp_Response_error_tag;

    message.content.error.has_type = true;
    message.content.error.type = type;
    if (msg != nullptr) {
        message.content.error.message.funcs.encode = &encodeString;
        message.content.error.message.arg = (void*)msg;
    }
    pb_encode(&stream, edgetx_eldp_Response_fields, &message);
    return stream.bytes_written;
}