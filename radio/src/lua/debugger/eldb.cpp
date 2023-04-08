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
#include "internal/messages.h"
#include "internal/session.h"
#include "internal/encode_decode.h"
#include "eldp.pb.h"

void eldbReceive(uint8_t *rxBuf, size_t rxBufLen, size_t dataLen) {
   if (dataLen != 0) { // there actually IS some valuable data
      uint8_t txBuf[128];
      edgetx_eldp_Request request = edgetx_eldp_Request_init_zero;

      pb_istream_t stream;

      if (eldbDecodeRequest(rxBuf, dataLen, &stream, &request)) { // success
         if (request.which_content == edgetx_eldp_Request_startDebug_tag && !eldbIsStarted) {
            char targetName[32];
            request.content.startDebug.targetName.funcs.decode = &decodeString;
            request.content.startDebug.targetName.arg = targetName;

            eldbStartSession(targetName);
         } else if (request.which_content == edgetx_eldp_Request_startDebug_tag && eldbIsStarted) {
            size_t len = eldbMakeErrorMessage(
               txBuf,
               sizeof(txBuf),
               edgetx_eldp_Error_Type_ALREADY_STARTED,
               nullptr
            );
            cliELDPSend(txBuf, len);
         } else if (request.which_content != edgetx_eldp_Request_startDebug_tag && eldbIsStarted) {
            size_t len = eldbMakeSystemInfoMessage(txBuf, sizeof(txBuf));
            
            cliELDPSend(txBuf, len);
            // TODO: Make it redirect to the current running session
         } else {
            size_t len = eldbMakeErrorMessage(
               txBuf,
               sizeof(txBuf),
               edgetx_eldp_Error_Type_NOT_STARTED_YET,
               nullptr
            );
            cliELDPSend(txBuf, len);
         }
      } else {
         size_t len = eldbMakeErrorMessage(
            txBuf,
            sizeof(txBuf),
            edgetx_eldp_Error_Type_BAD_MESSAGE,
            nullptr
         );
         cliELDPSend(txBuf, len);
      }

      // TODO: Do proper error handling
   }
}