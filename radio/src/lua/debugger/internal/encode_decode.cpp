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

#include "encode_decode.h"
#include <cli.h>
#include <pb_common.h>
#include <pb_encode.h>
#include <pb_decode.h>

bool encodeString(pb_ostream_t *stream, const pb_field_t *field,
                   void *const *arg) {
    if (!pb_encode_tag_for_field(stream, field)) return false;

    return pb_encode_string(stream, *(uint8_t* const*)arg, strlen((char*)*arg));
}

bool decodeString(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    uint8_t buffer[1024] = {0};
    
    // TODO: we could read block-by-block to avoid the huge ass buffer...
    if (stream->bytes_left > sizeof(buffer) - 1)
        return false;
    
    if (!pb_read(stream, buffer, stream->bytes_left))
        return false;
    
    strcpy((char*)*arg, (const char*)buffer);
    return true;
}