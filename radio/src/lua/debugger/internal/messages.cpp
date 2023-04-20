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
#include <stamp.h>

#include "eldp.pb.h"
#include "encode_decode.h"

size_t eldbMakeSystemInfoMessage(uint8_t *targetBuf, size_t targetBufLen) {
    edgetx_eldp_Response message = edgetx_eldp_Response_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(targetBuf, targetBufLen);

    message.has_systemInfo = true;

    message.systemInfo.has_osName = true;
    strcpy(message.systemInfo.osName, "EdgeTX");
    message.systemInfo.has_version = true;
    message.systemInfo.version.major = VERSION_MAJOR;
    message.systemInfo.version.minor = VERSION_MINOR;
    message.systemInfo.version.patch = VERSION_REVISION;
    message.systemInfo.has_versionTag = true;
    #if defined(VERSION_TAG)
    strcpy(message.systemInfo.versionTag, VERSION_TAG);
    #else
    strcpy(message.systemInfo.versionTag, VERSION_SUFFIX);
    #endif
    message.systemInfo.has_codename = true;
    strcpy(message.systemInfo.codename, CODENAME);
    message.systemInfo.has_gitTag = true;
    strcpy(message.systemInfo.gitTag, GIT_STR);
    message.systemInfo.has_deviceIdentifier = true;
    strcpy(message.systemInfo.deviceIdentifier, FLAVOUR);

    pb_encode(&stream, edgetx_eldp_Response_fields, &message);
    return stream.bytes_written;
}

size_t eldbMakeErrorMessage(
    uint8_t *targetBuf,
    size_t targetBufLen,
    edgetx_eldp_Error_Type type,
    const char *msg
) {
    edgetx_eldp_Response message = edgetx_eldp_Response_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(targetBuf, targetBufLen);

    message.has_error = true;

    message.error.has_type = true;
    message.error.type = type;
    if (msg != nullptr) {
        message.error.message.funcs.encode = &encodeString;
        message.error.message.arg = (void*)msg;
    }
    pb_encode(&stream, edgetx_eldp_Response_fields, &message);
    return stream.bytes_written;
}