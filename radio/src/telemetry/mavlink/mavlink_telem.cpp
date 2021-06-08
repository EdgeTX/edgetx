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
#include "telemetry/mavlink/mavlink_telem.h"

MAVLINK_RAM_SECTION MavlinkTelem mavlinkTelem;

// -- TASK handlers --
// tasks can be set directly with SETTASK()
// some tasks don't need immediate execution, or need reliable request
// this is what these handlers are for
// they push the task to a fifo, and also allow to set number of retries and retry rates

void MavlinkTelem::push_task(uint8_t idx, uint32_t task)
{
  struct Task t = {.task = task, .idx = idx};
  _taskFifo.push(t);
}

void MavlinkTelem::pop_and_set_task(void)
{
  struct Task t;
  if (_taskFifo.pop(t)) SETTASK(t.idx, t.task);
}

// -- REQUEST handlers --

void MavlinkTelem::set_request(uint8_t idx, uint32_t task, uint8_t retry, tmr10ms_t rate)
{
  push_task(idx, task);

  _request_is_waiting[idx] |= task;

  if (retry == 0) return; // well, if there would be another pending we would not kill it

  int8_t empty_i = -1;

  // first check if request is already pending, at the same time find free slot, to avoid having to loop twice
  for (uint16_t i = 0; i < REQUESTLIST_MAX; i++) {
    //TODO: should we modify the retry & rate of the pending task?
    if ((_requestList[i].idx == idx) && (_requestList[i].task == task)) return; // already pending, we can get out of here
    if ((empty_i < 0) && !_requestList[i].task) empty_i = i; // free slot
  }

  // if not already pending, add it
  if (empty_i < 0) return; // no free slot

  _requestList[empty_i].task = task;
  _requestList[empty_i].idx = idx;
  _requestList[empty_i].retry = retry;
  _requestList[empty_i].tlast = get_tmr10ms();
  _requestList[empty_i].trate = rate;
}

void MavlinkTelem::clear_request(uint8_t idx, uint32_t task)
{
  for (uint16_t i = 0; i < REQUESTLIST_MAX; i++) {
    if ((_requestList[i].idx == idx) && (_requestList[i].task == task)) {
      _requestList[i].task = 0;
      _request_is_waiting[idx] &=~ task;
    }
  }
}

// what happens if a clear never comes?
// well, this is what retry = UINT8_MAX says, right

void MavlinkTelem::do_requests(void)
{
  tmr10ms_t tnow = get_tmr10ms();

  for (uint16_t i = 0; i < TASKIDX_MAX; i++) _request_is_waiting[i] = 0;

  for (uint16_t i = 0; i < REQUESTLIST_MAX; i++) {
    if (!_requestList[i].task) continue;

    _request_is_waiting[_requestList[i].idx] |= _requestList[i].task;

    if ((tnow - _requestList[i].tlast) >= _requestList[i].trate) {
      push_task(_requestList[i].idx, _requestList[i].task);
      _requestList[i].tlast = get_tmr10ms();
      if (_requestList[i].retry < UINT8_MAX) {
        if (_requestList[i].retry) _requestList[i].retry--;
        if (!_requestList[i].retry) _requestList[i].task = 0; // clear request
      }
    }
  }

  if ((tnow - _taskFifo_tlast) > 6) { // 60 ms decimation
    _taskFifo_tlast = tnow;
    // change this, so that it skips tasks with 0, this would allow an easy means to clear tasks also in the Fifo
    if (!_taskFifo.isEmpty()) pop_and_set_task();
  }
}

// -- Generate MAVLink messages --
// these should never be called directly, should only by called by the task handler

void MavlinkTelem::_generateCmdLong(
    uint8_t tsystem, uint8_t tcomponent, uint16_t cmd,
    float p1, float p2, float p3, float p4, float p5, float p6, float p7)
{
  fmav_msg_command_long_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent, cmd, 0, p1, p2, p3, p4, p5, p6, p7,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateHeartbeat(uint8_t base_mode, uint32_t custom_mode, uint8_t system_status)
{
  fmav_msg_heartbeat_pack(
      &_msg_out, _my_sysid, _my_compid,
      MAV_TYPE_GCS, MAV_AUTOPILOT_INVALID, base_mode, custom_mode, system_status,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateParamRequestList(uint8_t tsystem, uint8_t tcomponent)
{
  fmav_msg_param_request_list_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateParamRequestRead(uint8_t tsystem, uint8_t tcomponent, const char* param_name)
{
char param_id[16];

  strncpy(param_id, param_name, 16);
  fmav_msg_param_request_read_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent, param_id, -1,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateRequestDataStream(
    uint8_t tsystem, uint8_t tcomponent, uint8_t data_stream, uint16_t rate_hz, uint8_t startstop)
{
  fmav_msg_request_data_stream_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent, data_stream, rate_hz, startstop,
      &_status_out
      );
  _msg_out_available = true;
}

// ArduPilot: ignores param7
void MavlinkTelem::generateCmdSetMessageInterval(uint8_t tsystem, uint8_t tcomponent, uint8_t msgid, int32_t period_us, uint8_t startstop)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_SET_MESSAGE_INTERVAL, msgid, (startstop) ? period_us : -1.0f);
}

// -- Main message handler for incoming MAVLink messages --

void MavlinkTelem::handleMessage(void)
{
  if (_msg.sysid == 0) return; //this can't be anything meaningful

  // autodetect sys id, and handle autopilot connecting
  if (!isSystemIdValid() || (autopilot.compid == 0)) {
    if (_msg.msgid == FASTMAVLINK_MSG_ID_HEARTBEAT) {
      fmav_heartbeat_t payload;
      fmav_msg_heartbeat_decode(&payload, &_msg);
      if ((_msg.compid == MAV_COMP_ID_AUTOPILOT1) || (payload.autopilot != MAV_AUTOPILOT_INVALID)) {
        _sysid = _msg.sysid;
        autopilottype = payload.autopilot;
        vehicletype = payload.type;
        _resetAutopilot();
        autopilot.compid = _msg.compid;
        autopilot.requests_triggered = 1; // we need to postpone and schedule them
      }
    }
    if (!isSystemIdValid()) return;
  }

  msg_rx_count++;
  _msg_rx_persec_cnt++;
  _bytes_rx_persec_cnt += fmav_msg_frame_len(&_msg);

  // discoverers
  // somewhat inefficient, lots of heartbeat decodes, we probably want a separate heartbeat handler

  if ((camera.compid == 0) && (_msg.msgid == FASTMAVLINK_MSG_ID_HEARTBEAT)) {
    fmav_heartbeat_t payload;
    fmav_msg_heartbeat_decode(&payload, &_msg);
    if ( (payload.autopilot == MAV_AUTOPILOT_INVALID) &&
       ( (payload.type == MAV_TYPE_CAMERA) ||
         ((_msg.compid >= MAV_COMP_ID_CAMERA) && (_msg.compid <= MAV_COMP_ID_CAMERA6)) ) ) {
      _resetCamera();
      camera.compid = _msg.compid;
      camera.requests_triggered = 1; //we schedule them
    }
  }

  if ((gimbal.compid == 0) && (_msg.msgid == FASTMAVLINK_MSG_ID_HEARTBEAT)) {
    fmav_heartbeat_t payload;
    fmav_msg_heartbeat_decode(&payload, &_msg);
    if ( (payload.autopilot == MAV_AUTOPILOT_INVALID) &&
       ( (payload.type == MAV_TYPE_GIMBAL) ||
         ((_msg.compid == MAV_COMP_ID_GIMBAL) ||
         ((_msg.compid >= MAV_COMP_ID_GIMBAL2) && (_msg.compid <= MAV_COMP_ID_GIMBAL6))) ) ) {
      _resetGimbalAndGimbalClient();
      gimbal.compid = _msg.compid;
      gimbal.is_initialized = true; //no startup requests, so true
    }
  }

  if ((gimbalmanager.compid == 0) && (gimbal.compid > 0) && (_msg.msgid == FASTMAVLINK_MSG_ID_STORM32_GIMBAL_MANAGER_STATUS)) {
    fmav_storm32_gimbal_manager_status_t payload;
    fmav_msg_storm32_gimbal_manager_status_decode(&payload, &_msg);
    if (payload.gimbal_id == gimbal.compid) { //this is the gimbal's gimbal manager
      _resetGimbalClient();
      gimbalmanager.compid = _msg.compid;
      gimbalmanagerOut.device_flags = payload.device_flags;
      gimbalmanagerOut.manager_flags = payload.manager_flags;
      gimbalmanager.requests_triggered = 1; //we schedule them
    }
  }

  // reset receiving timeout, but ignore RADIO_STATUS
  if (_msg.msgid != FASTMAVLINK_MSG_ID_RADIO_STATUS) {
    _is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT;
  }

  // MAVLINK API
  mavapiHandleMessage(&_msg);

  // MAVSDK
  // also try to convert the MAVLink messages to FrSky sensors

  // RADIO_STATUS is somewhat tricky, this may need doing it better if there are more sources of it
  // SiK comes as vehicle 51, comp 68!
  // it must NOT be rated as _is_recieving!
  if (_msg.msgid == FASTMAVLINK_MSG_ID_RADIO_STATUS) {
    fmav_radio_status_t payload;
    fmav_msg_radio_status_decode(&payload, &_msg);
    radio.rssi = payload.rssi;
    radio.remrssi = payload.remrssi;
    radio.noise = payload.noise;
    radio.remnoise = payload.remnoise;
    radio.is_receiving = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT;
    telemetrySetRssiValue(radio.rssi);
    return;
  }

  //we handle all qshot wherever they come from
  handleMessageQShot();

  if (_msg.sysid != _sysid) return; //this is not from our system

  // handle messages coming from autopilot
  if (autopilot.compid && (_msg.compid == autopilot.compid)) {
    handleMessageAutopilot();
  }
  if (camera.compid && (_msg.compid == camera.compid)) {
    handleMessageCamera();
  }
  if (gimbal.compid && (_msg.compid == gimbal.compid)) {
    handleMessageGimbal();
  }
  if (gimbalmanager.compid && (_msg.compid == gimbalmanager.compid)) {
    handleMessageGimbalClient();
  }
}

// -- Main task handler --

void MavlinkTelem::doTask(void)
{
  tmr10ms_t tnow = get_tmr10ms();

  bool tick_1Hz = false;

  if ((tnow - _my_heartbeat_tlast) > 100) { //1 sec
    _my_heartbeat_tlast = tnow;
    SETTASK(TASK_ME, TASK_SENDMYHEARTBEAT);

    msg_rx_persec = _msg_rx_persec_cnt;
    bytes_rx_persec = _bytes_rx_persec_cnt;
    _msg_rx_persec_cnt = 0;
    _bytes_rx_persec_cnt = 0;

    msg_tx_persec = _msg_tx_persec_cnt;
    bytes_tx_persec = _bytes_tx_persec_cnt;
    _msg_tx_persec_cnt = 0;
    _bytes_tx_persec_cnt = 0;

    tick_1Hz = true;
  }

  if (!isSystemIdValid()) return;

  // trigger startup requests

  // we need to wait until at least one heartbeat was send out before requesting data streams
  if (autopilot.compid && autopilot.requests_triggered) {
    if (tick_1Hz) autopilot.requests_triggered++;
    if (autopilot.requests_triggered > 3) { // wait for 3 heartbeats
      autopilot.requests_triggered = 0;
      setAutopilotStartupRequests();
    }
  }

  // we wait until at least one heartbeat was send out, and autopilot requests have been done
  if (camera.compid && camera.requests_triggered && !autopilot.requests_triggered) {
    if (tick_1Hz) camera.requests_triggered++;
    if (camera.requests_triggered > 1) { // wait for the next heartbeat
      camera.requests_triggered = 0;
      setCameraStartupRequests();
    }
  }

  // we wait until at least one heartbeat was send out, and autopilot requests have been done
  if (gimbal.compid && gimbal.requests_triggered && !autopilot.requests_triggered) {
    if (tick_1Hz) gimbal.requests_triggered++;
    if (gimbal.requests_triggered > 1) { // wait for the next heartbeat
      gimbal.requests_triggered = 0;
      setGimbalStartupRequests();
    }
  }
  if (gimbalmanager.compid && gimbalmanager.requests_triggered && !autopilot.requests_triggered) {
    if (tick_1Hz) gimbalmanager.requests_triggered++;
    if (gimbalmanager.requests_triggered > 1) { // wait for the next heartbeat
      gimbalmanager.requests_triggered = 0;
      setGimbalClientStartupRequests();
    }
  }

  if (!autopilot.is_initialized) autopilot.is_initialized = (autopilot.requests_waiting_mask == 0); 
  
  if (!camera.is_initialized) camera.is_initialized = (camera.requests_waiting_mask == 0); 
  
  if (!gimbal.is_initialized) gimbal.is_initialized = (gimbal.requests_waiting_mask == 0); 
  
  if (!gimbalmanager.is_initialized) gimbalmanager.is_initialized = (gimbalmanager.requests_waiting_mask == 0); 
  
  // handle pending requests
  do_requests();

  // do rc override
  // ArduPilot has a DAMED BUG!!!
  // per MAVLink spec 0 and UNIT16_MAX should not be considered for channels >= 8, but it doesn't do it for 0
  // but we can hope that it handles 0 for the higher channels
  if (g_model.mavlinkRcOverride && param.SYSID_MYGCS >= 0) {
    if ((tnow - _rcoverride_tlast) >= 5) { //50 ms
      _rcoverride_tlast = tnow;
      for (uint8_t i = 0; i < 8; i++) {
        /* would this be the right way to figure out which output is actually active ??
        MixData * md;
        if (i < MAX_MIXERS && (md=mixAddress(i))->srcRaw && md->destCh == i) {
          int value = channelOutputs[i] + 2 * PPM_CH_CENTER(i) - 2 * PPM_CENTER;
          _tovr_chan_raw[i] = value;
        }
        else {
          _tovr_chan_raw[i] = UINT16_MAX;
        }*/
        // the first four channels may not be ordered like with transmitter!!
        int value = channelOutputs[i]/2 + PPM_CH_CENTER(i);
        _tovr_chan_raw[i] = value;
      }
      for (uint8_t i = 8; i < 18; i++) { 
        _tovr_chan_raw[i] = 0; 
      }
      SETTASK(TASK_AUTOPILOT, TASK_SENDMSG_RC_CHANNELS_OVERRIDE);
    }
  }

  // handle pending tasks
  // do only one task and hence one msg_out per loop
  if (!_msg_out_available && TASK_IS_PENDING()) {
    //other TASKS
    if (doTaskAutopilot()) return;
    if (doTaskGimbalAndGimbalClient()) return;
    if (doTaskCamera()) return;

    //TASK_ME
    if (_task[TASK_ME] & TASK_SENDMYHEARTBEAT) {
      RESETTASK(TASK_ME, TASK_SENDMYHEARTBEAT);
      uint8_t base_mode = MAV_MODE_PREFLIGHT | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED;
      uint8_t system_status = MAV_STATE_UNINIT | MAV_STATE_ACTIVE;
      uint32_t custom_mode = 0;
      generateHeartbeat(base_mode, custom_mode, system_status);
      return; // do only one per loop
    }
    if (_task[TASK_ME] & TASK_SENDMSG_MAVLINK_API) {
      RESETTASK(TASK_ME, TASK_SENDMSG_MAVLINK_API);
      mavapiGenerateMessage();
      return; // do only one per loop
    }

    //other TASKS low priority
    if (doTaskAutopilotLowPriority()) return;
    if (doTaskCameraLowPriority()) return;
    if (doTaskQShot()) return;
  }
}

// -- Wakeup call from OpenTx --
// this is the main entry point

// ourself = link 0
// serial1 = link 1
// serial2 = link 2
// usb     = link 3

void MavlinkTelem::wakeup()
{
  // track configuration changes
  bool aux1_enabled = (g_eeGeneral.auxSerialMode == UART_MODE_MAVLINK);
  bool aux2_enabled = (g_eeGeneral.aux2SerialMode == UART_MODE_MAVLINK);
#if defined(TELEMETRY_MAVLINK_USB_SERIAL)
  bool usb_enabled = (getSelectedUsbMode() == USB_SERIAL_MODE);
#else
  bool usb_enabled = false;
#endif
  bool external_enabled = (g_eeGeneral.mavlinkExternal == 1) && !s_pulses_paused;

  if ((_aux1_enabled != aux1_enabled) || (_aux2_enabled != aux2_enabled) ||
      (_aux1_baudrate != g_eeGeneral.mavlinkBaudrate) || (_aux2_baudrate != g_eeGeneral.mavlinkBaudrate2) ||
      (_external_enabled != external_enabled)) {
    _aux1_enabled = aux1_enabled;
    _aux2_enabled = aux2_enabled;
    _aux1_baudrate = g_eeGeneral.mavlinkBaudrate;
    _aux2_baudrate = g_eeGeneral.mavlinkBaudrate2;
    _external_enabled = external_enabled;
    mavlinkTelemExternal_init(external_enabled);
    map_serials();
    _reset();
  }

  if (_usb_enabled != usb_enabled) {
    _usb_enabled = usb_enabled;
    fmav_router_clearout_link(3);
  }

  if (external_enabled) mavlinkTelemExternal_wakeup();

  // skip out if not one of the serial1, serial2 is enabled
  if (!serial1_enabled && !serial1_enabled) return;

  // look for incoming messages on all channels
  // only do one at a time
  #define INCc(x,p)  {x++; if(x >= p) x = 0;}

  INCc(_scheduled_serial, 3);
  uint8_t currently_scheduled_serial = _scheduled_serial;

  uint32_t available = 0;
  switch (currently_scheduled_serial) {
    case 0: available = mavlinkTelem1Available(); break;
    case 1: available = mavlinkTelem2Available(); break;
    case 2: available = mavlinkTelem3Available(); break;
  }
  if (available > 128) available = 128; // 128 = 22 ms @ 57600 bps

  uint8_t c;
  fmav_result_t result;

  // read serial1
  if (currently_scheduled_serial == 0) {
    for (uint32_t i = 0; i < available; i++) {
      if (!mavlinkTelem1Getc(&c)) break;
      if (fmav_parse_and_check_to_frame_buf(&result, _buf1, &_status1, c)) {
        fmav_router_handle_message(1, &result);
        if (fmav_router_send_to_link(1)) {} // WE DO NOT REFLECT, SO THIS MUST NEVER HAPPEN !!
        if (fmav_router_send_to_link(2)) { mavlinkTelem2PutBuf(_buf1, result.frame_len); }
        if (fmav_router_send_to_link(3)) { mavlinkTelem3PutBuf(_buf1, result.frame_len); }
        if (result.res == FASTMAVLINK_PARSE_RESULT_OK && fmav_router_send_to_link(0)) {
          fmav_frame_buf_to_msg(&_msg, &result, _buf1);
          handleMessage(); // checks _msg, and puts any result into a task queue
        }
      }
    }
  }

  // read serial2
  if (currently_scheduled_serial == 1) {
    for (uint32_t i = 0; i < available; i++) {
      if (!mavlinkTelem2Getc(&c)) break;
      if (fmav_parse_and_check_to_frame_buf(&result, _buf2, &_status2, c)) {
        fmav_router_handle_message(2, &result);
        if (fmav_router_send_to_link(1)) { mavlinkTelem1PutBuf(_buf2, result.frame_len); }
        if (fmav_router_send_to_link(2)) {} // WE DO NOT REFLECT, SO THIS MUST NEVER HAPPEN !!
        if (fmav_router_send_to_link(3)) { mavlinkTelem3PutBuf(_buf2, result.frame_len); }
        if (result.res == FASTMAVLINK_PARSE_RESULT_OK && fmav_router_send_to_link(0)) {
          fmav_frame_buf_to_msg(&_msg, &result, _buf2);
          handleMessage(); // checks _msg, and puts any result into a task queue
        }
      }
    }
  }

  // read serial3 = usb
  if (currently_scheduled_serial == 2) {
    for (uint32_t i = 0; i < available; i++) {
      if (!mavlinkTelem3Getc(&c)) break;
      if (fmav_parse_and_check_to_frame_buf(&result, _buf3, &_status3, c)) {
        fmav_router_handle_message(3, &result);
        if (fmav_router_send_to_link(1)) { mavlinkTelem1PutBuf(_buf3, result.frame_len); }
        if (fmav_router_send_to_link(2)) { mavlinkTelem2PutBuf(_buf3, result.frame_len); }
        if (fmav_router_send_to_link(3)) {} // WE DO NOT REFLECT, SO THIS MUST NEVER HAPPEN !!
        if (result.res == FASTMAVLINK_PARSE_RESULT_OK && fmav_router_send_to_link(0)) {
          fmav_frame_buf_to_msg(&_msg, &result, _buf3);
          handleMessage(); // checks _msg, and puts any result into a task queue
        }
      }
    }
  }

  // do tasks
  doTask(); // checks task queue _msg, and puts one result into _msg_out

  // send out pending message
  if (_msg_out_available) {
    fmav_router_handle_message_by_msg(0, &_msg_out);
    if (fmav_router_send_to_link(1) || fmav_router_send_to_link(2) || fmav_router_send_to_link(3)) {
      uint16_t count = fmav_msg_to_frame_buf(_buf_out, &_msg_out);
      // check that message can be send to all enabled serials
      if ((!serial1_enabled || mavlinkTelem1HasSpace(count)) &&
          (!serial2_enabled || mavlinkTelem2HasSpace(count)) &&
          (!_usb_enabled || mavlinkTelem3HasSpace(count))) {
        if (serial1_enabled && fmav_router_send_to_link(1)) mavlinkTelem1PutBuf(_buf_out, count);
        if (serial2_enabled && fmav_router_send_to_link(2)) mavlinkTelem2PutBuf(_buf_out, count);
        if (_usb_enabled && fmav_router_send_to_link(3)) mavlinkTelem3PutBuf(_buf_out, count);
        _msg_out_available = false;
        msg_tx_count++;
        _msg_tx_persec_cnt++;
        _bytes_tx_persec_cnt += count;
      }
    } else {
      _msg_out_available = false; // message is targeted at unknown component
    }
  }
}

// -- 10 ms tick --

void MavlinkTelem::tick10ms()
{
  #define check(x,y) if(x){ (x)--; if(!(x)){ (y); }}

  check(_is_receiving, _reset());

  check(radio.is_receiving, _resetRadio());
  check(radio.is_receiving65, _resetRadio65());
  check(radio.is_receiving35, _resetRadio35());

  check(autopilot.is_receiving, _resetAutopilot());
  check(gimbal.is_receiving, _resetGimbalAndGimbalClient());
  check(gimbalmanager.is_receiving, _resetGimbalClient());
  check(camera.is_receiving, _resetCamera());

  // keep 10us timer updated
  time10us();
}

// -- Resets --

void MavlinkTelem::_resetRadio(void)
{
  radio.is_receiving = 0;

  radio.rssi = UINT8_MAX;
  radio.remrssi = UINT8_MAX;
  radio.noise = 0;
  radio.remnoise = 0;

  telemetryResetRssiValue();
}

void MavlinkTelem::_resetRadio65(void)
{
  radio.is_receiving65 = 0;
  radio.rssi65 = UINT8_MAX;

  telemetryResetRssiValue();
}

void MavlinkTelem::_resetRadio35(void)
{
  radio.is_receiving35 = 0;
  radio.rssi35 = UINT8_MAX;

  telemetryResetRssiValue();
}

void MavlinkTelem::_reset(void)
{
#if defined(CLI) || defined(DEBUG)
#define UART_MODE_NONE_OR_DEBUG UART_MODE_DEBUG
#else
#define UART_MODE_NONE_OR_DEBUG UART_MODE_NONE
#endif
#if !defined(AUX_SERIAL)
  if (g_eeGeneral.auxSerialMode == UART_MODE_MAVLINK) g_eeGeneral.auxSerialMode = UART_MODE_NONE_OR_DEBUG;
#endif
#if !defined(AUX2_SERIAL)
  if (g_eeGeneral.aux2SerialMode == UART_MODE_MAVLINK) g_eeGeneral.aux2SerialMode = UART_MODE_NONE_OR_DEBUG;
#endif
  if (g_eeGeneral.mavlinkExternal > 1) g_eeGeneral.mavlinkExternal = 0;

  _my_sysid = MAVLINK_TELEM_MY_SYSID;
  _my_compid = MAVLINK_TELEM_MY_COMPID;

  _sysid = 0;
  autopilottype = MAV_AUTOPILOT_GENERIC; //TODO: shouldn't these be in _resetAutopilot() ??
  vehicletype = MAV_TYPE_GENERIC;
  flightmode = 0;

  for (uint16_t i = 0; i < TASKIDX_MAX; i++) _task[i] = 0;
  _taskFifo.clear();
  _taskFifo_tlast = 0;
  for (uint16_t i = 0; i < REQUESTLIST_MAX; i++) _requestList[i].task = 0;

  _resetRadio();
  _resetRadio65();
  _resetRadio35();
  radio.rssi_scaled = 0;
  radio.rssi_voice_disabled = false;

  _resetAutopilot();
  _resetGimbalAndGimbalClient();
  _resetCamera();

  _resetQShot();

  fmav_status_reset(&_status1);
  fmav_status_reset(&_status2);
  fmav_status_reset(&_status3);
  fmav_status_reset(&_status_out);
  fmav_router_reset();
  fmav_router_add_ourself(MAVLINK_TELEM_MY_SYSID, MAVLINK_TELEM_MY_COMPID);

  msg_rx_count = 0;
  msg_rx_persec = 0;
  bytes_rx_persec = 0;
  _msg_rx_persec_cnt = 0;
  _bytes_rx_persec_cnt = 0;

  msg_tx_count = 0;
  msg_tx_persec = 0;
  bytes_tx_persec = 0;
  _msg_tx_persec_cnt = 0;
  _bytes_tx_persec_cnt = 0;

  mavapiInit();
}


void MavlinkTelem::_init(void)
{
  fmav_router_init();
  fmav_router_set_link_properties_all(
      FASTMAVLINK_ROUTER_LINK_PROPERTY_FLAG_ALWAYS_SEND_HEARTBEAT |
      FASTMAVLINK_ROUTER_LINK_PROPERTY_FLAG_DISCOVER_BY_HEARTBEAT
      );
  _reset();
}
