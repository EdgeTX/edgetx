/*
 * Copyright (C) EdgeTX
 *
 * (c) www.olliw.eu, OlliW, OlliW42
 *
 * Based on code named
 *   opentx - http://github.com/opentx/opentx
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

#include "opentx.h"

// -- Generate MAVLink messages --
// these should never be called directly, should only be called by the task handler

//STorM32 specific
void MavlinkTelem::generateCmdDoQShotConfigure(uint8_t tsystem, uint8_t tcomponent, uint8_t mode, uint8_t shot_state)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_QSHOT_DO_CONFIGURE,
      mode, shot_state, 0,0,0,0,0
      );
}

//STorM32 specific
void MavlinkTelem::generateQShotStatus(uint8_t mode, uint8_t shot_state)
{
  fmav_msg_qshot_status_pack(
      &_msg_out, _my_sysid, _my_compid,
      mode, shot_state,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateButtonChange(uint8_t button_state)
{
  fmav_msg_button_change_pack(
      &_msg_out, _my_sysid, _my_compid,
      time_boot_ms(),
      0,
      button_state,
      &_status_out
      );
  _msg_out_available = true;
}

// -- Mavsdk Convenience Task Wrapper --
// to make it easy for api_mavsdk to call functions

void MavlinkTelem::sendQShotCmdConfigure(uint8_t mode, uint8_t shot_state)
{
  _t_qshot_cmd_mode = mode;
  qshot.mode = mode;
  qshot.shot_state = shot_state;
  SETTASK(TASK_ME, TASK_SENDCMD_DO_QSHOT_CONFIGFURE);
}

void MavlinkTelem::sendQShotStatus(uint8_t mode, uint8_t shot_state)
{
  _t_qshot_mode = mode;
  qshot.mode = mode;
  qshot.shot_state = shot_state;
  SETTASK(TASK_ME, TASK_SENDMSG_QSHOT_STATUS);
}

void MavlinkTelem::sendQShotButtonState(uint8_t button_state)
{
  _t_qshot_button_state = button_state;
  SETTASK(TASK_ME, TASK_SENDMSG_QSHOT_BUTTON_STATE);
}

// -- Task handlers --

bool MavlinkTelem::doTaskQShot(void)
{
  if (!_task[TASK_ME]) return false; // no task pending

  if (_task[TASK_ME] & TASK_SENDCMD_DO_QSHOT_CONFIGFURE) {
    RESETTASK(TASK_ME, TASK_SENDCMD_DO_QSHOT_CONFIGFURE);
    generateCmdDoQShotConfigure(0, 0, _t_qshot_cmd_mode, _t_qshot_cmd_shot_state); //broadcast
    return true; //do only one per loop
  }
  if (_task[TASK_ME] & TASK_SENDMSG_QSHOT_STATUS) {
    RESETTASK(TASK_ME, TASK_SENDMSG_QSHOT_STATUS);
    generateQShotStatus(_t_qshot_mode, _t_qshot_shot_state);
    return true; //do only one per loop
  }
  if (_task[TASK_ME] & TASK_SENDMSG_QSHOT_BUTTON_STATE) {
    RESETTASK(TASK_ME, TASK_SENDMSG_QSHOT_BUTTON_STATE);
    generateButtonChange(_t_qshot_button_state);
    return true; //do only one per loop
  }

  return false;
}

// -- Handle incoming MAVLink messages, which are for qshot --

void MavlinkTelem::handleMessageQShot(void)
{
  switch (_msg.msgid) {
    case FASTMAVLINK_MSG_ID_QSHOT_STATUS: {
      fmav_qshot_status_t payload;
      fmav_msg_qshot_status_decode(&payload, &_msg);
      qshot.mode = payload.mode;
      qshot.shot_state = payload.shot_state;
      break;
    }
  }
}

// -- Startup Requests --

// -- Resets --

void MavlinkTelem::_resetQShot(void)
{
  _task[TASK_ME] = 0;

  qshot.mode = MAV_QSHOT_MODE_UNDEFINED;
  qshot.shot_state = 0;
}
