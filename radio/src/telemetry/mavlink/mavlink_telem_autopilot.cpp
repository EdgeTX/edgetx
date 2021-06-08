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

constexpr float FPI = 3.141592653589793f;
constexpr float FDEGTORAD = FPI/180.0f;
constexpr float FRADTODEG = 180.0f/FPI;

#define INCU8(x)    if ((x) < UINT8_MAX) { (x)++; }

// -- Rate defines --

#define DATA_STREAM_RAW_SENSORS_RATE      2 // not used

// MAVLINK_MSG_ID_GPS_RAW_INT
// MAVLINK_MSG_ID_GPS2_RAW
// MAVLINK_MSG_ID_SYS_STATUS
// MSG_NAV_CONTROLLER_OUTPUT
// MSG_CURRENT_WAYPOINT // MISSION_CURRENT
#define DATA_STREAM_EXTENDED_STATUS_RATE  2

// MAVLINK_MSG_ID_GLOBAL_POSITION_INT
#define DATA_STREAM_POSITION_RATE         4  // not used

// MAVLINK_MSG_ID_ATTITUDE
#define DATA_STREAM_EXTRA1_RATE           4  // not used

// MAVLINK_MSG_ID_VFR_HUD
#define DATA_STREAM_EXTRA2_RATE           2

// MAVLINK_MSG_ID_BATTERY_STATUS (only if batt monitor configured)
// MAVLINK_MSG_ID_BATTERY2 (only if batt2 monitor configured)
// MAVLINK_MSG_ID_EKF_STATUS_REPORT
#define DATA_STREAM_EXTRA3_RATE           2

// MSG_SERVO_OUTPUT_RAW
// MSG_RC_CHANNELS
// MSG_RC_CHANNELS_RAW (only sent on a mavlink1 connection)
#define DATA_STREAM_RC_CHANNELS_RATE      1

// call at high rates of 10 Hz, or 5 Hz
// MAVLINK_MSG_ID_ATTITUDE
#define MSG_ID_ATTITUDE_RATE              100000 // 100 ms = 10 Hz

// call at high rates of 10 Hz, or 5 Hz
// MAVLINK_MSG_ID_GLOBAL_POSITION_INT
#define MSG_ID_GLOBAL_POSITION_INT_RATE   200000 // 200 ms = 5 Hz

// -- Generate MAVLink messages --
// these should never be called directly, should only by called by the task handler

//ArduPilot:
//  base_mode must have MAV_MODE_FLAG_CUSTOM_MODE_ENABLED bit set,
//  custom_mode then determines the mode it will switch to
//  usage of this cmd is thus very likely very flightstack dependent!!
void MavlinkTelem::generateCmdDoSetMode(uint8_t tsystem, uint8_t tcomponent, MAV_MODE base_mode, uint32_t custom_mode)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_DO_SET_MODE, base_mode, custom_mode);
}

//ArduPilot: supports a param3 which is not in the specs, ignores param1,4,5,6
// param3 = horizontal navigation by pilot acceptable
void MavlinkTelem::generateCmdNavTakeoff(uint8_t tsystem, uint8_t tcomponent, float alt_m, bool hor_nav_by_pilot)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_NAV_TAKEOFF, 0,0, (hor_nav_by_pilot) ? 1.0f : 0.0f, 0,0,0, alt_m);
}

// speed type 0=Airspeed, 1=Ground Speed, 2=Climb Speed, 3=Descent Speed
//ArduPilot: ignores param3 =  Throttle and param4 = Relative
void MavlinkTelem::generateCmdDoChangeSpeed(uint8_t tsystem, uint8_t tcomponent, float speed_mps, uint16_t speed_type, bool relative)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_DO_CHANGE_SPEED, speed_type, speed_mps, -1, (relative) ? 1.0f : 0.0f);
}

//ArduPilot: current = 2 or 3
// current = 2 is a flag to tell this is a "guided mode" waypoint and not for the mission
// current = 3 is a flag to tell this is a alt change
void MavlinkTelem::generateMissionItemInt(uint8_t tsystem, uint8_t tcomponent,
    uint8_t frame, uint16_t cmd, uint8_t current, int32_t lat, int32_t lon, float alt_m)
{
  fmav_msg_mission_item_int_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      0, frame, cmd, current, 0, 0.0f, 0.0f, 0.0f, 0.0f, lat, lon, alt_m, MAV_MISSION_TYPE_MISSION,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateSetPositionTargetGlobalInt(uint8_t tsystem, uint8_t tcomponent,
    uint8_t frame, uint16_t type_mask,
    int32_t lat, int32_t lon, float alt, float vx, float vy, float vz, float yaw_rad, float yaw_rad_rate)
{
  fmav_msg_set_position_target_global_int_pack(
      &_msg_out, _my_sysid, _my_compid,
      time_boot_ms(),
      tsystem, tcomponent,
      frame, type_mask,
      lat, lon, alt, vx, vy, vz, 0.0f, 0.0f, 0.0f, yaw_rad, yaw_rad_rate, // alt in m, v in m/s, yaw in rad
      &_status_out
      );
  _msg_out_available = true;
}

// yaw must be in range 0..360
void MavlinkTelem::generateCmdConditionYaw(uint8_t tsystem, uint8_t tcomponent, float yaw_deg, float yaw_deg_rate, int8_t dir, bool rel)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_CONDITION_YAW, yaw_deg, yaw_deg_rate, (dir>0)?1.0f:-1.0f, (rel)?1.0f:0.0f);
}

void MavlinkTelem::generateRcChannelsOverride(uint8_t sysid, uint8_t tsystem, uint8_t tcomponent, uint16_t* chan_raw)
{
  fmav_msg_rc_channels_override_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      chan_raw[0], chan_raw[1], chan_raw[2], chan_raw[3], chan_raw[4], chan_raw[5], chan_raw[6], chan_raw[7],
      chan_raw[8], chan_raw[9], chan_raw[10], chan_raw[11], chan_raw[12], chan_raw[13], chan_raw[14], chan_raw[15],
      chan_raw[16], chan_raw[17],
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateMissionRequestList(uint8_t tsystem, uint8_t tcomponent, uint8_t mission_type)
{
  fmav_msg_mission_request_list_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      mission_type,
      &_status_out
      );
  _msg_out_available = true;
}

void MavlinkTelem::generateMissionRequestInt(uint8_t tsystem, uint8_t tcomponent, uint16_t seq, uint8_t mission_type)
{
  fmav_msg_mission_request_int_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      seq, mission_type,
      &_status_out
      );
  _msg_out_available = true;
}

// -- Mavsdk Convenience Task Wrapper --
// to make it easy for api_mavsdk to call functions

void MavlinkTelem::apSetFlightMode(uint32_t ap_flight_mode)
{
  _tcsm_base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
  _tcsm_custom_mode = ap_flight_mode;
  SETTASK(TASK_AUTOPILOT, TASK_SENDCMD_DO_SET_MODE);
}

void MavlinkTelem::apSetGroundSpeed(float speed)
{
  _tccs_speed_mps = speed;
  _tccs_speed_type = 1;
  SETTASK(TASK_AUTOPILOT, TASK_SENDCMD_DO_CHANGE_SPEED);
}

void MavlinkTelem::apSimpleGotoPosAlt(int32_t lat, int32_t lon, float alt)
{
//  _tmii_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT; //Ardupilot doesn't seem to take this
  _tmii_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
  _tmii_cmd = MAV_CMD_NAV_WAYPOINT;
  _tmii_current = 2;
  _tmii_lat = lat; 
  _tmii_lon = lon; 
  _tmii_alt_m = alt;
  SETTASK(TASK_AUTOPILOT, TASK_SENDMSG_MISSION_ITEM_INT);
}

//alt and yaw can be NAN if they should be ignored
// this function is not very useful as it really moves very slowly
void MavlinkTelem::apGotoPosAltYawDeg(int32_t lat, int32_t lon, float alt, float yaw)
{
  _t_coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
  _t_type_mask = 
      POSITION_TARGET_TYPEMASK_VX_IGNORE | POSITION_TARGET_TYPEMASK_VY_IGNORE | POSITION_TARGET_TYPEMASK_VZ_IGNORE |
      POSITION_TARGET_TYPEMASK_AX_IGNORE | POSITION_TARGET_TYPEMASK_AY_IGNORE | POSITION_TARGET_TYPEMASK_AZ_IGNORE |
      POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE;
  if (isnan(alt)) { 
    _t_type_mask |= POSITION_TARGET_TYPEMASK_Z_IGNORE; 
    alt = 1.0f; 
  }
  if (isnan(yaw)) { 
    _t_type_mask |= POSITION_TARGET_TYPEMASK_YAW_IGNORE; 
    yaw = 0.0f; 
  }
  _t_lat = lat; 
  _t_lon = lon;
  _t_alt = alt; // m
  _t_vx = _t_vy = _t_vz = 0.0f;
  _t_yaw_rad = yaw * FDEGTORAD; // rad
  _t_yaw_rad_rate = 0.0f;
  SETTASK(TASK_AUTOPILOT, TASK_SENDMSG_SET_POSITION_TARGET_GLOBAL_INT);
}

void MavlinkTelem::apGotoPosAltVel(int32_t lat, int32_t lon, float alt, float vx, float vy, float vz)
{
  _t_coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
  _t_type_mask = 
      POSITION_TARGET_TYPEMASK_AX_IGNORE | POSITION_TARGET_TYPEMASK_AY_IGNORE | POSITION_TARGET_TYPEMASK_AZ_IGNORE |
      POSITION_TARGET_TYPEMASK_YAW_IGNORE |
      POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE;
  _t_lat = lat; 
  _t_lon = lon;
  _t_alt = alt; // m
  _t_vx = vx; // m/s 
  _t_vy = vy; 
  _t_vz = vz;
  _t_yaw_rad = _t_yaw_rad_rate = 0.0f; // rad
  SETTASK(TASK_AUTOPILOT, TASK_SENDMSG_SET_POSITION_TARGET_GLOBAL_INT);
}

//note, we can enter negative yaw here, sign determines direction
void MavlinkTelem::apSetYawDeg(float yaw, bool relative)
{
  if (relative) {
    _tccy_relative = 1.0f;
    if (yaw < 0.0f) { 
      _tccy_dir = -1.0f; 
      yaw = -yaw; 
    } 
    else { 
      _tccy_dir = 1.0f; 
    }
  } 
  else {
    _tccy_relative = 0.0f;
    _tccy_dir = 0.0f;
  }
  float res = fmodf(yaw, 360.0f);
  if (res < 0.0f) res += 360.0f;
  _tccy_yaw_deg = res;  // is in deg, must be in range [0..360]
  SETTASK(TASK_AUTOPILOT, TASK_SENDCMD_CONDITION_YAW);
}

// -- Task handlers --

bool MavlinkTelem::doTaskAutopilot(void)
{
  if (!_task[TASK_AUTOPILOT]) return false; // no task pending

  if (!autopilot.compid) { 
    _task[TASK_AUTOPILOT] = _task[TASK_AP] = 0; 
    return false; 
  }

  // we give RC_CHANNELS_OVERRIDE highest priority
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_RC_CHANNELS_OVERRIDE) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_RC_CHANNELS_OVERRIDE);
    if (autopilottype == MAV_AUTOPILOT_ARDUPILOTMEGA) {
      // RC_CHHANELS_OVERRIDE requires the "correct" sysid, so try to work it out
      // somewhat dirty, but that's how ArduPilot works, it only allows one GCS, and its sys id needs to be set by hand
      // RC_CHHANELS_OVERRIDE is also considered as kind of a heartbeat, e.g. with respect to gcs failsafe
      if (param.SYSID_MYGCS >= 0)
        generateRcChannelsOverride(param.SYSID_MYGCS, _sysid, autopilot.compid, _tovr_chan_raw);
    } 
    else {
      //TODO: check what other flight stacks expect
      generateRcChannelsOverride(_my_sysid, _sysid, autopilot.compid, _tovr_chan_raw);
    }
    return true; //do only one per loop
  }

  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_DO_CHANGE_SPEED) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_DO_CHANGE_SPEED);
    generateCmdDoChangeSpeed(_sysid, autopilot.compid, _tccs_speed_mps, _tccs_speed_type, true);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_MISSION_ITEM_INT) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_MISSION_ITEM_INT);
    generateMissionItemInt(_sysid, autopilot.compid, _tmii_frame, _tmii_cmd, _tmii_current,
        _tmii_lat, _tmii_lon, _tmii_alt_m);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_SET_POSITION_TARGET_GLOBAL_INT) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_SET_POSITION_TARGET_GLOBAL_INT);
    generateSetPositionTargetGlobalInt(_sysid, autopilot.compid, _t_coordinate_frame, _t_type_mask,
        _t_lat, _t_lon, _t_alt, _t_vx, _t_vy, _t_vz, _t_yaw_rad, _t_yaw_rad_rate);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_CONDITION_YAW) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_CONDITION_YAW);
    generateCmdConditionYaw(_sysid, autopilot.compid, _tccy_yaw_deg, 0.0f, _tccy_dir, _tccy_relative);
    return true; //do only one per loop
  }

  return false;
}

bool MavlinkTelem::doTaskAutopilotLowPriority(void)
{
  if (!_task[TASK_AUTOPILOT] && !_task[TASK_AP]) return false; // no task pending

  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_DO_SET_MODE) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_DO_SET_MODE);
    generateCmdDoSetMode(_sysid, autopilot.compid, (MAV_MODE)_tcsm_base_mode, _tcsm_custom_mode);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_NAV_TAKEOFF) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_NAV_TAKEOFF);
    generateCmdNavTakeoff(_sysid, autopilot.compid, _tcnt_alt_m, 1);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_PARAM_REQUEST_LIST) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_PARAM_REQUEST_LIST);
    generateParamRequestList(_sysid, autopilot.compid);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_MISSION_REQUEST_LIST) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_MISSION_REQUEST_LIST);
    generateMissionRequestList(_sysid, autopilot.compid, MAV_MISSION_TYPE_MISSION);
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDMSG_MISSION_REQUEST_INT) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDMSG_MISSION_REQUEST_INT);
    generateMissionRequestInt(_sysid, autopilot.compid, _tmri_seq, _tmri_missiontype);
    return true; //do only one per loop
  }

  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_RAW_SENSORS) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_RAW_SENSORS);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_RAW_SENSORS, DATA_STREAM_RAW_SENSORS_RATE, 1); // 2 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_EXTENDED_STATUS) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_EXTENDED_STATUS);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_EXTENDED_STATUS, DATA_STREAM_EXTENDED_STATUS_RATE, 1); // 2 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_POSITION) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_POSITION);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_POSITION, DATA_STREAM_POSITION_RATE, 1); // do faster, 4 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_EXTRA1) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_EXTRA1);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_EXTRA1, DATA_STREAM_EXTRA1_RATE, 1); // do faster, 4 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_EXTRA2) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_EXTRA2);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_EXTRA2, DATA_STREAM_EXTRA2_RATE, 1); // 2 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_EXTRA3) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_EXTRA3);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_EXTRA3, DATA_STREAM_EXTRA3_RATE, 1); // 2 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDREQUESTDATASTREAM_RC_CHANNELS) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDREQUESTDATASTREAM_RC_CHANNELS);
    generateRequestDataStream(_sysid, autopilot.compid, MAV_DATA_STREAM_RC_CHANNELS, DATA_STREAM_RC_CHANNELS_RATE, 1); // 1 Hz
    return true; //do only one per loop
  }

  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_REQUEST_ATTITUDE) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_REQUEST_ATTITUDE);
    generateCmdSetMessageInterval(_sysid, autopilot.compid, FASTMAVLINK_MSG_ID_ATTITUDE, MSG_ID_ATTITUDE_RATE, 1); // 100 ms = 10 Hz
    return true; //do only one per loop
  }
  if (_task[TASK_AUTOPILOT] & TASK_SENDCMD_REQUEST_GLOBAL_POSITION_INT) {
    RESETTASK(TASK_AUTOPILOT,TASK_SENDCMD_REQUEST_GLOBAL_POSITION_INT);
    generateCmdSetMessageInterval(_sysid, autopilot.compid, FASTMAVLINK_MSG_ID_GLOBAL_POSITION_INT, MSG_ID_GLOBAL_POSITION_INT_RATE, 1); // 200 ms = 5 Hz
    return true; //do only one per loop
  }

  if (_task[TASK_AP] & TASK_AP_ARM) { //MAV_CMD_COMPONENT_ARM_DISARM
    RESETTASK(TASK_AP, TASK_AP_ARM);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_COMPONENT_ARM_DISARM, 1.0f);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_DISARM) { //MAV_CMD_COMPONENT_ARM_DISARM
    RESETTASK(TASK_AP, TASK_AP_DISARM);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_COMPONENT_ARM_DISARM, 0.0f);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_COPTER_TAKEOFF) { //MAV_CMD_NAV_TAKEOFF
    RESETTASK(TASK_AP, TASK_AP_COPTER_TAKEOFF);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_NAV_TAKEOFF, 0,0, 0.0f, 0,0,0, _tact_takeoff_alt_m); //must_navigate = true
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_LAND) { //MAV_CMD_NAV_LAND
    RESETTASK(TASK_AP, TASK_AP_LAND);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_NAV_LAND);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_COPTER_FLYCLICK) {
    RESETTASK(TASK_AP, TASK_AP_COPTER_FLYCLICK);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_SOLO_BTN_FLY_CLICK);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_COPTER_FLYHOLD) {
    RESETTASK(TASK_AP, TASK_AP_COPTER_FLYHOLD);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_SOLO_BTN_FLY_HOLD, _tacf_takeoff_alt_m);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_COPTER_FLYPAUSE) {
    RESETTASK(TASK_AP, TASK_AP_COPTER_FLYPAUSE);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_SOLO_BTN_PAUSE_CLICK, 0.0f); //shoot = no
    return true; //do only one per loop
  }

  if (_task[TASK_AP] & TASK_AP_REQUESTBANNER) { //MAV_CMD_DO_SEND_BANNER
    RESETTASK(TASK_AP, TASK_AP_REQUESTBANNER);
    _generateCmdLong(_sysid, autopilot.compid, MAV_CMD_DO_SEND_BANNER);
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_BATT_CAPACITY) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_BATT_CAPACITY);
    generateParamRequestRead(_sysid, autopilot.compid, "BATT_CAPACITY");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_BATT2_CAPACITY) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_BATT2_CAPACITY);
    generateParamRequestRead(_sysid, autopilot.compid, "BATT2_CAPACITY");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_WPNAV_SPEED) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_SPEED);
    generateParamRequestRead(_sysid, autopilot.compid, "WPNAV_SPEED");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_WPNAV_ACCEL) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_ACCEL);
    generateParamRequestRead(_sysid, autopilot.compid, "WPNAV_ACCEL");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_WPNAV_ACCEL_Z) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_ACCEL_Z);
    generateParamRequestRead(_sysid, autopilot.compid, "WPNAV_ACCEL_Z");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_SYSID_MYGCS) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_SYSID_MYGCS);
    generateParamRequestRead(_sysid, autopilot.compid, "SYSID_MYGCS");
    return true; //do only one per loop
  }
  if (_task[TASK_AP] & TASK_AP_REQUESTPARAM_ARMING_CHECK) {
    RESETTASK(TASK_AP, TASK_AP_REQUESTPARAM_ARMING_CHECK);
    generateParamRequestRead(_sysid, autopilot.compid, "ARMING_CHECK");
    return true; //do only one per loop
  }

  return false;
}

// -- Handle incoming MAVLink messages, which are for the Autopilot --

void MavlinkTelem::handleMessageAutopilot(void)
{
  autopilot.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT; //we accept any msg from the autopilot to indicate it is alive

  switch (_msg.msgid) {
    case FASTMAVLINK_MSG_ID_HEARTBEAT: {
      fmav_heartbeat_t payload;
      fmav_msg_heartbeat_decode(&payload, &_msg);
      flightmode = payload.custom_mode;
      autopilot.system_status = payload.system_status;
      autopilot.is_armed = (payload.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ? true : false;
      autopilot.is_standby = (payload.system_status <= MAV_STATE_STANDBY) ? true : false;
      autopilot.is_critical = (payload.system_status >= MAV_STATE_CRITICAL) ? true : false;
      INCU8(autopilot.updated);
      //autopilot.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT;
      break;
    }

    case FASTMAVLINK_MSG_ID_SYS_STATUS: {
      fmav_sys_status_t payload;
      fmav_msg_sys_status_decode(&payload, &_msg);
      sysstatus.sensors_present = payload.onboard_control_sensors_present;
      sysstatus.sensors_enabled = payload.onboard_control_sensors_enabled;
      sysstatus.sensors_health = payload.onboard_control_sensors_health;
      sysstatus.received = true;
      INCU8(sysstatus.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_ATTITUDE: {
      fmav_attitude_t payload;
      fmav_msg_attitude_decode(&payload, &_msg);
      att.roll_rad = payload.roll;
      att.pitch_rad = payload.pitch;
      att.yaw_rad = payload.yaw;
      INCU8(att.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDCMD_REQUEST_ATTITUDE);
      autopilot.requests_waiting_mask &=~ AUTOPILOT_REQUESTWAITING_ATTITUDE;
      break;
    }

    case FASTMAVLINK_MSG_ID_GPS_RAW_INT: {
      fmav_gps_raw_int_t payload;
      fmav_msg_gps_raw_int_decode(&payload, &_msg);
      gps1.fix = payload.fix_type;
      gps1.sat = payload.satellites_visible;
      gps1.hdop = payload.eph;
      gps1.vdop = payload.epv;
      gps1.lat = payload.lat;
      gps1.lon = payload.lon;
      gps1.alt_mm = payload.alt;
      gps1.vel_cmps = payload.vel;
      gps1.cog_cdeg = payload.cog;
      INCU8(gps1.updated);
      gps_instancemask |= 0x01;
      clear_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTENDED_STATUS);
      autopilot.requests_waiting_mask &=~ AUTOPILOT_REQUESTWAITING_GPS_RAW_INT;
      telemetrySetValue(GPS_ALT_FIRST_ID, 0, 10, (int32_t)(payload.alt), UNIT_METERS, 3);
      if (payload.vel != UINT16_MAX) {
        telemetrySetValue(GPS_SPEED_FIRST_ID, 0, 11, (int32_t)(payload.vel), UNIT_METERS, 2);
      }
      break;
    }

    case FASTMAVLINK_MSG_ID_GPS2_RAW: {
      fmav_gps2_raw_t payload;
      fmav_msg_gps2_raw_decode(&payload, &_msg);
      gps2.fix = payload.fix_type;
      gps2.sat = payload.satellites_visible;
      gps2.hdop = payload.eph;
      gps2.vdop = payload.epv;
      gps2.lat = payload.lat;
      gps2.lon = payload.lon;
      gps2.alt_mm = payload.alt;
      gps2.vel_cmps = payload.vel;
      gps2.cog_cdeg = payload.cog;
      INCU8(gps2.updated);
      gps_instancemask |= 0x02;
      break;
    }

    case FASTMAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
      fmav_global_position_int_t payload;
      fmav_msg_global_position_int_decode(&payload, &_msg);
      gposition.lat = payload.lat;
      gposition.lon = payload.lon;
      gposition.alt_mm = payload.alt;
      gposition.relative_alt_mm = payload.relative_alt;
      gposition.vx_cmps = payload.vx;
      gposition.vy_cmps = payload.vy;
      gposition.vz_cmps = payload.vz;
      gposition.hdg_cdeg = payload.hdg;
      INCU8(gposition.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDCMD_REQUEST_GLOBAL_POSITION_INT);
      autopilot.requests_waiting_mask &=~ AUTOPILOT_REQUESTWAITING_GLOBAL_POSITION_INT;
      break;
    }

    case FASTMAVLINK_MSG_ID_VFR_HUD: {
      fmav_vfr_hud_t payload;
      fmav_msg_vfr_hud_decode(&payload, &_msg);
      vfr.airspd_mps = payload.airspeed;
      vfr.groundspd_mps = payload.groundspeed;
      vfr.alt_m = payload.alt;
      vfr.climbrate_mps = payload.climb;
      vfr.heading_deg = payload.heading;
      vfr.thro_pct = payload.throttle;
      INCU8(vfr.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTRA2);
      autopilot.requests_waiting_mask &=~ AUTOPILOT_REQUESTWAITING_VFR_HUD;
      telemetrySetValue(ALT_FIRST_ID, 0, 13, (int32_t)(payload.alt * 100.0f), UNIT_METERS, 2);
      telemetrySetValue(VARIO_FIRST_ID, 0, 14, (int32_t)(payload.climb * 100.0f), UNIT_METERS_PER_SECOND, 2);
      telemetrySetValue(AIR_SPEED_FIRST_ID, 0, 15, (int32_t)(payload.airspeed * 100.0f), UNIT_METERS_PER_SECOND, 2);
      telemetrySetValue(GPS_COURS_FIRST_ID, 0, 16, (int32_t)payload.heading * 10, UNIT_DEGREE, 1);
      break;
    }

    /* let's use BATTERY_STATUS, is nearly the same thing
    case FASTMAVLINK_MSG_ID_SYS_STATUS: {
      fmav_sys_status_t payload;
      fmav_msg_sys_status_decode(&payload, &_msg);
      // voltage_battery  uint16_t  mV
      // current_battery  int16_t cA
      // battery_remaining  int8_t  %
      break;
    }*/

    case FASTMAVLINK_MSG_ID_BATTERY_STATUS: {
      fmav_battery_status_t payload;
      fmav_msg_battery_status_decode(&payload, &_msg);
      int32_t voltage = 0;
      int8_t cellcount = 0;
      bool validcellcount = true;
      for (uint8_t i=0; i<10; i++) {
        if (payload.voltages[i] != UINT16_MAX) {
          voltage += payload.voltages[i]; //uint16_t mV, UINT16_MAX if not known
          if (payload.voltages[i] == UINT16_MAX-1) validcellcount = false;
          cellcount++;
        }
      }
      for (uint8_t i=0; i<4; i++) { //we assume this never is relevant if validcellcount = false
        if (payload.voltages_ext[i] != 0) {
          voltage += payload.voltages_ext[i]; //uint16_t mV, 0 if not known
          cellcount++;
        }
      }
      if (!validcellcount) cellcount = -1;
      if (payload.id == 0) {
        bat1.charge_consumed_mAh = payload.current_consumed; // mAh, -1 if not known
        bat1.energy_consumed_hJ = payload.energy_consumed; // 0.1 kJ, -1 if not known
        bat1.temperature_cC = payload.temperature; // centi-degrees C°, INT16_MAX if not known
        bat1.voltage_mV = voltage; // mV
        bat1.current_cA = payload.current_battery; // 10*mA, -1 if not known
        bat1.remaining_pct = payload.battery_remaining; //(0%: 0, 100%: 100), -1 if not knwon
        bat1.time_remaining = payload.time_remaining; // 0 if not knwon
        bat1.charge_state = payload.charge_state; // 0 if not knwon
        bat1.fault_bitmask = payload.fault_bitmask;
        bat1.cellcount = cellcount;
        INCU8(bat1.updated);
      }
      if (payload.id == 1) {
        bat2.charge_consumed_mAh = payload.current_consumed; // mAh, -1 if not known
        bat2.energy_consumed_hJ = payload.energy_consumed; // 0.1 kJ, -1 if not known
        bat2.temperature_cC = payload.temperature; // centi-degrees C°, INT16_MAX if not known
        bat2.voltage_mV = voltage; // mV
        bat2.current_cA = payload.current_battery; // 10*mA, -1 if not known
        bat2.remaining_pct = payload.battery_remaining; //(0%: 0, 100%: 100), -1 if not knwon
        bat2.time_remaining = payload.time_remaining; // 0 if not knwon
        bat2.charge_state = payload.charge_state; // 0 if not knwon
        bat2.fault_bitmask = payload.fault_bitmask;
        bat2.cellcount = cellcount;
        INCU8(bat2.updated);
      }
      if (payload.id < 8) bat_instancemask |= (1 << payload.id);
      telemetrySetValue(BATT_ID, 0, 17, voltage/100, UNIT_VOLTS, 1);
      telemetrySetValue(VFAS_FIRST_ID, 0, 18, voltage/10, UNIT_VOLTS, 2);
      telemetrySetValue(CURR_FIRST_ID, 0, 19, payload.current_battery/10, UNIT_AMPS, 1);
      break;
    }

    case FASTMAVLINK_MSG_ID_STATUSTEXT: {
      fmav_statustext_t payload;
      fmav_msg_statustext_decode(&payload, &_msg);
      statustext.fifo.push(payload);
      INCU8(statustext.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_EKF_STATUS_REPORT: {
      fmav_ekf_status_report_t payload;
      fmav_msg_ekf_status_report_decode(&payload, &_msg);
      //we don't really need the other fields
      ekf.flags = payload.flags;
      INCU8(ekf.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTRA3);
      autopilot.requests_waiting_mask &=~ AUTOPILOT_REQUESTWAITING_EKF_STATUS_REPORT;
      break;
    }

    case FASTMAVLINK_MSG_ID_RC_CHANNELS_RAW: { //#35
      fmav_rc_channels_raw_t payload;
      fmav_msg_rc_channels_raw_decode(&payload, &_msg);
      radio.rssi35 = payload.rssi;
      radio.is_receiving35 = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT;
      if (!radio.is_receiving && !radio.is_receiving65) {
        telemetrySetRssiValue(radio.rssi35);
      }
      break;
    }

    case FASTMAVLINK_MSG_ID_RC_CHANNELS: { //#65
      fmav_rc_channels_t payload;
      fmav_msg_rc_channels_decode(&payload, &_msg);
      radio.rssi65 = payload.rssi;
      radio.is_receiving65 = MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT;
      clear_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_RC_CHANNELS);
      if (!radio.is_receiving) {
        telemetrySetRssiValue(radio.rssi65);
      }
      break;
    }

    case FASTMAVLINK_MSG_ID_RANGEFINDER: {
      fmav_rangefinder_t payload;
      fmav_msg_rangefinder_decode(&payload, &_msg);
      //we don't really need the other fields
      rangefinder.distance = payload.distance;
      INCU8(rangefinder.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT: {
      fmav_nav_controller_output_t payload;
      fmav_msg_nav_controller_output_decode(&payload, &_msg);
      //we don't really need the other fields
      navControllerOutput.nav_bearing = payload.nav_bearing;
      navControllerOutput.target_bearing = payload.target_bearing;
      navControllerOutput.wp_dist = payload.wp_dist;
      INCU8(navControllerOutput.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_MISSION_COUNT: {
      fmav_mission_count_t payload;
      fmav_msg_mission_count_decode(&payload, &_msg);
      if (payload.mission_type != MAV_MISSION_TYPE_MISSION) break; //not a MISSION item
      mission.count = payload.count;
      INCU8(mission.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDMSG_MISSION_REQUEST_LIST);
      break;
    }

    case FASTMAVLINK_MSG_ID_MISSION_CURRENT: {
      fmav_mission_current_t payload;
      fmav_msg_mission_current_decode(&payload, &_msg);
      bool has_changed = (mission.seq_current != payload.seq);
      mission.seq_current = payload.seq;
      INCU8(mission.updated);
      // if the current has changed, then trigger a new MISSION_REQUEST_INT
      if (has_changed) requestMissionRequestInt(mission.seq_current);
      break;
    }

    case FASTMAVLINK_MSG_ID_MISSION_ITEM_INT: {
      fmav_mission_item_int_t payload;
      fmav_msg_mission_item_int_decode(&payload, &_msg);
      if (payload.mission_type != MAV_MISSION_TYPE_MISSION) break; //not a MISSION item
      if (payload.frame == MAV_FRAME_MISSION) break; //not a coordinate frame, indicates a mission command
      missionItem.seq = payload.seq;
      missionItem.frame = payload.frame;
      missionItem.command = payload.command;
      missionItem.x = payload.x;
      missionItem.y = payload.y;
      missionItem.z = payload.z;
      INCU8(missionItem.updated);
      clear_request(TASK_AUTOPILOT, TASK_SENDMSG_MISSION_REQUEST_INT);
      break;
    }

    case FASTMAVLINK_MSG_ID_PARAM_VALUE: {
      fmav_param_value_t payload;
      fmav_msg_param_value_decode(&payload, &_msg);
      if (!strncmp(payload.param_id,"BATT_CAPACITY",16)) {
        param.BATT_CAPACITY = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_BATT_CAPACITY);
      }
      if (!strncmp(payload.param_id,"BATT2_CAPACITY",16)) {
        param.BATT2_CAPACITY = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_BATT2_CAPACITY);
      }
      if (!strncmp(payload.param_id,"WPNAV_SPEED",16)) {
        param.WPNAV_SPEED = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_SPEED);
      }
      if (!strncmp(payload.param_id,"WPNAV_ACCEL",16)) {
        param.WPNAV_ACCEL = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_ACCEL);
      }
      if (!strncmp(payload.param_id,"WPNAV_ACCEL_Z",16)) {
        param.WPNAV_ACCEL_Z = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_WPNAV_ACCEL_Z);
      }
      if (!strncmp(payload.param_id,"SYSID_MYGCS",16)) {
        param.SYSID_MYGCS = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_SYSID_MYGCS);
      }
      if (!strncmp(payload.param_id,"ARMING_CHECK",16)) {
        param.ARMING_CHECK = payload.param_value;
        clear_request(TASK_AP, TASK_AP_REQUESTPARAM_ARMING_CHECK);
      }
      break;
    }
  }
}

// -- Resets --

void MavlinkTelem::_resetAutopilot(void)
{
  _task[TASK_AUTOPILOT] = 0;
  _task[TASK_AP] = 0;

  autopilot.compid = 0;
  autopilot.is_receiving = 0;

  autopilot.requests_triggered = 0;
  autopilot.requests_waiting_mask = AUTOPILOT_REQUESTWAITING_ALL;
  autopilot.is_initialized = false;

  autopilot.system_status = MAV_STATE_UNINIT;
  autopilot.custom_mode = 0;
  autopilot.is_armed = false;
  autopilot.is_standby = true;
  autopilot.is_critical = false;
  autopilot.prearm_ok = false;
  autopilot.updated = 0;

  sysstatus.sensors_present = 0;
  sysstatus.sensors_enabled = 0;
  sysstatus.sensors_health = 0;
  sysstatus.received = false;
  sysstatus.updated = 0;

  att.roll_rad = 0.0f;
  att.pitch_rad = 0.0f;
  att.yaw_rad = 0.0f;
  att.updated = 0;

  gps1.fix = GPS_FIX_TYPE_NO_GPS;
  gps1.sat = UINT8_MAX;
  gps1.hdop = UINT16_MAX;
  gps1.vdop = UINT16_MAX;
  gps1.lat = 0;
  gps1.lon = 0;
  gps1.alt_mm = 0;
  gps1.vel_cmps = UINT16_MAX;
  gps1.cog_cdeg = UINT16_MAX;
  gps1.updated = 0;

  gps2.fix = GPS_FIX_TYPE_NO_GPS;
  gps2.sat = UINT8_MAX;
  gps2.hdop = UINT16_MAX;
  gps2.vdop = UINT16_MAX;
  gps2.lat = 0;
  gps2.lon = 0;
  gps2.alt_mm = 0;
  gps2.vel_cmps = UINT16_MAX;
  gps2.cog_cdeg = UINT16_MAX;
  gps2.updated = 0;

  gps_instancemask = 0;

  gposition.lat = 0;
  gposition.lon = 0;
  gposition.alt_mm = 0;
  gposition.relative_alt_mm = 0;
  gposition.vx_cmps = 0;
  gposition.vy_cmps = 0;
  gposition.vz_cmps = 0;
  gposition.hdg_cdeg = UINT16_MAX;
  gposition.updated = 0;

  vfr.airspd_mps = 0.0f;
  vfr.groundspd_mps = 0.0f;
  vfr.alt_m = 0.0f;
  vfr.climbrate_mps = 0.0f;
  vfr.heading_deg = 0;
  vfr.thro_pct = 0;
  vfr.updated = 0;

  bat1.charge_consumed_mAh = -1;
  bat1.energy_consumed_hJ = -1;
  bat1.temperature_cC = INT16_MAX;
  bat1.voltage_mV = 0;
  bat1.current_cA = -1;
  bat1.remaining_pct = -1;
  bat1.time_remaining = 0;
  bat1.charge_state = 0;
  bat1.fault_bitmask = 0;
  bat1.cellcount = -1;
  bat1.updated = 0;

  bat2.charge_consumed_mAh = -1;
  bat2.energy_consumed_hJ = -1;
  bat2.temperature_cC = INT16_MAX;
  bat2.voltage_mV = 0;
  bat2.current_cA = -1;
  bat2.remaining_pct = -1;
  bat2.time_remaining = 0;
  bat2.charge_state = 0;
  bat2.fault_bitmask = 0;
  bat2.cellcount = -1;
  bat2.updated = 0;

  bat_instancemask = 0;

  statustext.fifo.clear();
  statustext.updated =  0;

  ekf.flags = 0;
  ekf.updated = 0;

  rangefinder.distance = 0.0f;
  rangefinder.updated = 0;

  navControllerOutput.nav_bearing = 0; // Current desired heading in degrees
  navControllerOutput.target_bearing = 0; // Bearing to current MISSION/target in degrees
  navControllerOutput.wp_dist = 0; // Distance to active MISSION in meters
  navControllerOutput.updated = 0;

  mission.count = 0;
  mission.seq_current = UINT16_MAX; //to enforce a request at first incoming MISSION_CURRENT
  mission.updated = 0;

  missionItem.seq = 0; // Waypoint ID (sequence number). Starts at zero. Increases monotonically for each waypoint, no gaps in the sequence (0,1,2,3,4).
  missionItem.frame = 0; // The coordinate system of the waypoint.
  missionItem.command = 0; // The scheduled action for the waypnt.
  missionItem.x = 0; // PARAM5 / local: x position in meters * 1e4, global: latitude in degrees * 10^7
  missionItem.y = 0; // PARAM6 / y position: local: x position in meters * 1e4, global: longitude in degrees *10^7
  missionItem.z = 0.0f; // PARAM7 / z position: global: altitude in meters (relative or absolute, depending on frame.
  missionItem.updated = 0;

  param.number = -1;
  param.BATT_CAPACITY = -1;
  param.BATT2_CAPACITY = -1;
  param.WPNAV_SPEED = NAN;
  param.WPNAV_ACCEL = NAN;
  param.WPNAV_ACCEL_Z = NAN;
  param.SYSID_MYGCS = -1;
  param.ARMING_CHECK = -1;
}

// -- Miscellaneous --

// ArduPilot starts with sending heartbeat every 1 sec with FE, TimeSync every 5 sec with FE
// we then need to request the data stream
// TODO: we can also request them individually now

void MavlinkTelem::setAutopilotStartupRequests(void)
{
  if (autopilottype == MAV_AUTOPILOT_ARDUPILOTMEGA) {
    // 2 Hz sufficient
    // MAVLINK_MSG_ID_GPS_RAW_INT
    // MAVLINK_MSG_ID_GPS2_RAW
    // MAVLINK_MSG_ID_SYS_STATUS
    // MSG_NAV_CONTROLLER_OUTPUT
    // MSG_CURRENT_WAYPOINT // MISSION_CURRENT
    // cleared by MAVLINK_MSG_ID_GPS_RAW_INT (is send even when both GPS are 0, so we can use it to clear)
    set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTENDED_STATUS, 100, 200-4);

    // MAVLINK_MSG_ID_GLOBAL_POSITION_INT
    // cleared by MAVLINK_MSG_ID_GLOBAL_POSITION_INT
    //set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_POSITION, 100, 198);

    // MAVLINK_MSG_ID_ATTITUDE
    // cleared by MAVLINK_MSG_ID_ATTITUDE
    //set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTRA1, 100, 211);

    // 2 Hz sufficient
    // MAVLINK_MSG_ID_VFR_HUD
    // cleared by MAVLINK_MSG_ID_VFR_HUD
    set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTRA2, 100, 200+20);

    // 2 Hz sufficient
    // MAVLINK_MSG_ID_BATTERY_STATUS (only if batt monitor configured)
    // MAVLINK_MSG_ID_BATTERY2 (only if batt2 monitor configured)
    // MAVLINK_MSG_ID_EKF_STATUS_REPORT
    // cleared by MAVLINK_MSG_ID_EKF_STATUS_REPORT
    set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_EXTRA3, 100, 200+3);

    // 1 Hz sufficient
    // MSG_SERVO_OUTPUT_RAW
    // MSG_RC_CHANNELS
    // MSG_RC_CHANNELS_RAW (only sent on a mavlink1 connection)
    // cleared by MAVLINK_MSG_ID_RC_CHANNELS
    set_request(TASK_AUTOPILOT, TASK_SENDREQUESTDATASTREAM_RC_CHANNELS, 100, 200-8);

    // call at high rates of 10 Hz, or 5 Hz
    // MAVLINK_MSG_ID_ATTITUDE
    // cleared by MAVLINK_MSG_ID_ATTITUDE
    set_request(TASK_AUTOPILOT, TASK_SENDCMD_REQUEST_ATTITUDE, 100, 200+5);

    // call at high rates of 10 Hz, or 5 Hz
    // MAVLINK_MSG_ID_GLOBAL_POSITION_INT
    // cleared by MAVLINK_MSG_ID_GLOBAL_POSITION_INT
    set_request(TASK_AUTOPILOT, TASK_SENDCMD_REQUEST_GLOBAL_POSITION_INT, 100, 200+7);

    set_request(TASK_AUTOPILOT, TASK_SENDMSG_MISSION_REQUEST_LIST, 10, 341);

    set_request(TASK_AP, TASK_AP_REQUESTPARAM_BATT_CAPACITY, 10, 200+25);
    set_request(TASK_AP, TASK_AP_REQUESTPARAM_BATT2_CAPACITY, 10, 200+28);
    set_request(TASK_AP, TASK_AP_REQUESTPARAM_SYSID_MYGCS, 10, 123); //we request it more frequently to get it sooner
    set_request(TASK_AP, TASK_AP_REQUESTPARAM_ARMING_CHECK, 10, 200+33);

    push_task(TASK_AP, TASK_AP_REQUESTBANNER);
  }

  // other autopilots
  // TODO
}





/*
ArduPilot Copter Streams:
updated 29.01.2021

static const ap_message STREAM_RAW_SENSORS_msgs[] = {
    MSG_RAW_IMU,
    MSG_SCALED_IMU2,
    MSG_SCALED_IMU3,
    MSG_SCALED_PRESSURE,
    MSG_SCALED_PRESSURE2,
    MSG_SCALED_PRESSURE3,
    MSG_SENSOR_OFFSETS
};
static const ap_message STREAM_EXTENDED_STATUS_msgs[] = {
    MSG_SYS_STATUS,
    MSG_POWER_STATUS,
    MSG_MEMINFO,
    MSG_CURRENT_WAYPOINT, // MISSION_CURRENT
    MSG_GPS_RAW,
    MSG_GPS_RTK,
    MSG_GPS2_RAW,
    MSG_GPS2_RTK,
    MSG_NAV_CONTROLLER_OUTPUT,
    MSG_FENCE_STATUS,
    MSG_POSITION_TARGET_GLOBAL_INT,
};
static const ap_message STREAM_POSITION_msgs[] = {
    MSG_LOCATION,
    MSG_LOCAL_POSITION
};
static const ap_message STREAM_RC_CHANNELS_msgs[] = {
    MSG_SERVO_OUTPUT_RAW,
    MSG_RC_CHANNELS,
    MSG_RC_CHANNELS_RAW, // only sent on a mavlink1 connection
};
static const ap_message STREAM_EXTRA1_msgs[] = {
    MSG_ATTITUDE,
    MSG_SIMSTATE,
    MSG_AHRS2,
    MSG_PID_TUNING // Up to four PID_TUNING messages are sent, depending on GCS_PID_MASK parameter
};
static const ap_message STREAM_EXTRA2_msgs[] = {
    MSG_VFR_HUD
};
static const ap_message STREAM_EXTRA3_msgs[] = {
    MSG_AHRS,
    MSG_HWSTATUS,
    MSG_SYSTEM_TIME,
    MSG_RANGEFINDER,       -> ROTATION_PITCH_270
    MSG_DISTANCE_SENSOR,   -> send any and all distance_sensor messages. This starts by sending any distance sensors not used by a Proximity sensor, then sends the proximity sensor ones.

#if AP_TERRAIN_AVAILABLE && AC_TERRAIN
    MSG_TERRAIN,
#endif
    MSG_BATTERY2,
    MSG_BATTERY_STATUS,
    MSG_MOUNT_STATUS,
    MSG_OPTICAL_FLOW,
    MSG_GIMBAL_REPORT,
    MSG_MAG_CAL_REPORT,
    MSG_MAG_CAL_PROGRESS,
    MSG_EKF_STATUS_REPORT,
    MSG_VIBRATION,
    MSG_RPM,
    MSG_ESC_TELEMETRY,
    MSG_GENERATOR_STATUS,
    MSG_WINCH_STATUS,
};
static const ap_message STREAM_PARAMS_msgs[] = {
    MSG_NEXT_PARAM
};
static const ap_message STREAM_ADSB_msgs[] = {
    MSG_ADSB_VEHICLE
};

const struct GCS_MAVLINK::stream_entries GCS_MAVLINK::all_stream_entries[] = {
    MAV_STREAM_ENTRY(STREAM_RAW_SENSORS),
    MAV_STREAM_ENTRY(STREAM_EXTENDED_STATUS),
    MAV_STREAM_ENTRY(STREAM_POSITION),
    MAV_STREAM_ENTRY(STREAM_RC_CHANNELS),
    MAV_STREAM_ENTRY(STREAM_EXTRA1),
    MAV_STREAM_ENTRY(STREAM_EXTRA2),
    MAV_STREAM_ENTRY(STREAM_EXTRA3),
    MAV_STREAM_ENTRY(STREAM_ADSB),
    MAV_STREAM_ENTRY(STREAM_PARAMS),
    MAV_STREAM_TERMINATOR // must have this at end of stream_entries
};
*/

/*
AP battery vs sys_status

=> BATTERY sends the very same data as SYS_STATUS, but SYS_STATUS has initialized() and health() tests in addition
=> not clear which is better
let's decide to use BATTERY

BATTERY:
  AP_BattMonitor::cells fake_cells;
  sends total voltage splited up into cells of 65534

  float current, consumed_mah, consumed_wh;
  if (battery.current_amps(current, instance)) {
     current *= 100;
  }
  else {
    current = -1;
  }
  if (!battery.consumed_mah(consumed_mah, instance)) {
    consumed_mah = -1;
  }
  if (battery.consumed_wh(consumed_wh, instance)) {
    consumed_wh *= 36;
  }
  else {
    consumed_wh = -1;
  }

  mavlink_msg_battery_status_send(chan,
                  instance, // id
                  MAV_BATTERY_FUNCTION_UNKNOWN, // function
                  MAV_BATTERY_TYPE_UNKNOWN, // type
                  got_temperature ? ((int16_t) (temp * 100)) : INT16_MAX, // temperature. INT16_MAX if unknown
                  battery.has_cell_voltages(instance) ? battery.get_cell_voltages(instance).cells : fake_cells.cells, // cell voltages
                  current,    // current in centiampere
                  consumed_mah, // total consumed current in milliampere.hour
                  consumed_wh,  // consumed energy in hJ (hecto-Joules)
                  battery.capacity_remaining_pct(instance),
                  0, // time remaining, seconds (not provided)
                  MAV_BATTERY_CHARGE_STATE_UNDEFINED);

SYS_STATUS:
  if (!gcs().vehicle_initialised()) {
    return;
  }

  const AP_BattMonitor &battery = AP::battery();
  float battery_current;
  int8_t battery_remaining;

  if (battery.healthy() && battery.current_amps(battery_current)) {
    battery_remaining = battery.capacity_remaining_pct();
    battery_current *= 100;
  }
  else {
    battery_current = -1;
    battery_remaining = -1;
  }

  ...

  mavlink_msg_sys_status_send(
    chan,
    control_sensors_present,
    control_sensors_enabled,
    control_sensors_health,
    static_cast<uint16_t>(AP::scheduler().load_average() * 1000),
    battery.voltage() * 1000,  // mV
    battery_current,    // in 10mA units
    battery_remaining,    // in %
    0,  // comm drops %,
    0,  // comm drops in pkts,
    errors1,
    errors2,
    0,  // errors3
    errors4); // errors4
*/

/*
DATA_ID == 0x5006 then -- ROLLPITCH
DATA_ID == 0x5005 then -- VELANDYAW
DATA_ID == 0x5001 then -- AP STATUS
DATA_ID == 0x5002 then -- GPS STATUS
DATA_ID == 0x5003 then -- BATT
DATA_ID == 0x5008 then -- BATT2
DATA_ID == 0x5004 then -- HOME
DATA_ID == 0x5000 then -- MESSAGES
DATA_ID == 0x5007 then -- PARAMS
DATA_ID == 0x5009 then -- WAYPOINTS @1Hz
DATA_ID == 0x50F1 then -- RC CHANNELS
DATA_ID == 0x50F2 then -- VFR

AP
DATA_ID == 0x5000 then -- MESSAGES
DATA_ID == 0x5006 then -- ROLLPITCH
x0800   GPS LAT LON
DATA_ID == 0x5005 then -- VELANDYAW
DATA_ID == 0x5001 then -- AP STATUS
DATA_ID == 0x5002 then -- GPS STATUS
DATA_ID == 0x5004 then -- HOME
DATA_ID == 0x5008 then -- BATT2
DATA_ID == 0x5003 then -- BATT
DATA_ID == 0x5007 then -- PARAMS
 */
/*
  int32_t data = 12345 + _data;
  _data++;
#if defined(LUA)
  if (luaInputTelemetryFifo && luaInputTelemetryFifo->hasSpace(sizeof(SportTelemetryPacket))) {
    SportTelemetryPacket luaPacket;
    luaPacket.physicalId = 0; //sensor_id, isn't used
    luaPacket.primId = 0x10; //frame_id, only 0x10 is evaluated
    luaPacket.dataId = 0x5003; //data_id:
    luaPacket.value = data; //value
    for (uint8_t i=0; i<sizeof(SportTelemetryPacket); i++) {
    luaInputTelemetryFifo->push(luaPacket.raw[i]);
    }
  }
#endif
 */


/*
DroneKit
is_armable(self):
  return self.mode != 'INITIALISING' and (self.gps_0.fix_type is not None and self.gps_0.fix_type > 1) and self._ekf_predposhorizabs
ekf_ok(self):
  # use same check that ArduCopter::system.pde::position_ok() is using
  if self.armed: return self._ekf_poshorizabs and not self._ekf_constposmode
  else: return self._ekf_poshorizabs or self._ekf_predposhorizabs
groundspeed(self, speed):
  command_long_encode(0, 0, mavutil.mavlink.MAV_CMD_DO_CHANGE_SPEED, 0, 1, speed, -1, 0, 0, 0, 0)
airspeed(self, speed):
  command_long_encode(0, 0, mavutil.mavlink.MAV_CMD_DO_CHANGE_SPEED,  0, 0, speed, -1, 0, 0, 0, 0)
simple_takeoff(self, alt=None):
  command_long_send(0, 0, mavutil.mavlink.MAV_CMD_NAV_TAKEOFF, 0, 0, 0, 0, 0, 0, altitude)
simple_goto(self, location, airspeed=None, groundspeed=None):
  frame = mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT
  mission_item_send(0, 0, 0, frame, mavutil.mavlink.MAV_CMD_NAV_WAYPOINT, 2, 0, 0, 0, 0, 0, location.lat, location.lon, alt)

gimbal
rotate(self, pitch, roll, yaw):
  mount_configure_encode( 0, 1, mavutil.mavlink.MAV_MOUNT_MODE_MAVLINK_TARGETING, 1, 1, 1)
  mount_control_encode(0, 1, pitch * 100, roll * 100, yaw * 100, 0)
target_location(self, roi):
  mount_configure_encode(0, 1, mavutil.mavlink.MAV_MOUNT_MODE_GPS_POINT, 1, 1, 1)
  command_long_encode(0, 1, mavutil.mavlink.MAV_CMD_DO_SET_ROI, 0, 0, 0, 0, 0, roi.lat, roi.lon, alt)
release(self):
  mount_configure_encode(0, 1, mavutil.mavlink.MAV_MOUNT_MODE_RC_TARGETING, 1, 1, 1)


solo shots
class VectorPathHandler(PathHandler):
def move(self, channels):
def travel(self):
  set_position_target_global_int_encode(
      0, 0, 1, mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT,
      0b0000110111000000,
      int(loc.lat * 10000000), int(loc.lon * 10000000), loc.alt,
      velVector.x, velVector.y, velVector.z,
      0, 0, 0,
      0, 0)

 initStreamRates(self):
    STREAM_RATES = {
      mavutil.mavlink.MAV_DATA_STREAM_EXTENDED_STATUS: 2,
      mavutil.mavlink.MAV_DATA_STREAM_EXTRA1: 10,
      mavutil.mavlink.MAV_DATA_STREAM_EXTRA2: 10,
      mavutil.mavlink.MAV_DATA_STREAM_EXTRA3: 2,
      mavutil.mavlink.MAV_DATA_STREAM_POSITION: 10,
      mavutil.mavlink.MAV_DATA_STREAM_RAW_SENSORS: 2,
      mavutil.mavlink.MAV_DATA_STREAM_RAW_CONTROLLER: 3,
      mavutil.mavlink.MAV_DATA_STREAM_RC_CHANNELS: 5,
    }

class TwoPointPathHandler(PathHandler):
def MoveTowardsEndpt( self, channels ):
  simple_goto(target)
  command_long_encode(0, 1, mavutil.mavlink.MAV_CMD_DO_CHANGE_SPEED, 0, 1, abs(self.currentSpeed), -1, 0.0, 0.0, 0.0, 0.0)

class MultipointShot():
def handleRCs(self, channels):
  set_position_target_global_int_encode(
      0, 0, 1, mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT,
      0b0000110111000000,
      int(self.commandPos.lat * 10000000), int(self.commandPos.lon * 10000000),
      self.commandPos.alt,
      self.commandVel.x, self.commandVel.y, self.commandVel.z,
      0, 0, 0,
      0, 0)
def enterRecordMode(self):
    mode = VehicleMode("LOITER")
def enterPlayMode(self):
    mode = VehicleMode("GUIDED")
    mount_configure_encode(0, 1, mavutil.mavlink.MAV_MOUNT_MODE_MAVLINK_TARGETING, 1, 1, 1)
def handleAttach(self, attach):
    simple_goto(self.commandPos)
    command_long_encode(0, 1, mavutil.mavlink.MAV_CMD_DO_CHANGE_SPEED, 0, 1, ATTACH_SPEED, -1, 0.0, 0.0, 0.0, 0.0)


 */







/*
  MIXSRC_FIRST_CH,
  MIXSRC_CH1 = MIXSRC_FIRST_CH,       LUA_EXPORT_MULTIPLE("ch", "Channel CH%d", MAX_OUTPUT_CHANNELS)
  MIXSRC_CH2,
  MIXSRC_CH3,
  MIXSRC_CH4,
  MIXSRC_CH5,
  MIXSRC_CH6,
  MIXSRC_CH7,
  MIXSRC_CH8,
  MIXSRC_CH9,
  MIXSRC_CH10,
  MIXSRC_CH11,
  MIXSRC_CH12,
  MIXSRC_CH13,
  MIXSRC_CH14,
  MIXSRC_CH15,
  MIXSRC_CH16,
  MIXSRC_LAST_CH = MIXSRC_CH1+MAX_OUTPUT_CHANNELS-1,

MAX_OUTPUT_CHANNELS = 32

extern int16_t      channelOutputs[MAX_OUTPUT_CHANNELS];


#define PPM_CENTER           1500
#if defined(PPM_CENTER_ADJUSTABLE)
  #define PPM_CH_CENTER(ch)      (PPM_CENTER + limitAddress(ch)->ppmCenter)
#else
  #define PPM_CH_CENTER(ch)      (PPM_CENTER)
#endif


  int16_t PPM_range = g_model.extendedLimits ? (512*LIMIT_EXT_PERCENT/100) * 2 : 512 * 2; // range of 0.7 .. 1.7msec
  uint8_t firstCh = channelsStart;
  uint8_t lastCh = min<uint8_t>(MAX_OUTPUT_CHANNELS, firstCh + 8 + channelsCount);
  for (uint32_t i=firstCh; i<lastCh; i++) {
  int16_t v = limit((int16_t)-PPM_range, channelOutputs[i], (int16_t)PPM_range) + 2*PPM_CH_CENTER(i);

  int16_t PPM_range = g_model.extendedLimits ? 640*2 : 512*2;
  int firstCh = g_model.trainerData.channelsStart;
  int lastCh = firstCh + 8;
  for (int channel=0; channel<lastCh; channel+=2, cur+=3) {
  uint16_t channelValue1 = PPM_CH_CENTER(channel) + limit((int16_t)-PPM_range, channelOutputs[channel], (int16_t)PPM_range) / 2;
  uint16_t channelValue2 = PPM_CH_CENTER(channel+1) + limit((int16_t)-PPM_range, channelOutputs[channel+1], (int16_t)PPM_range) / 2;

  for (int i=0; i<DSM2_CHANS; i++) {
  int channel = g_model.moduleData[EXTERNAL_MODULE].channelsStart + i;
  int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;

  for (int i = 0; i < MULTI_CHANS; i++) {
  int channel = g_model.moduleData[moduleIdx].channelsStart + i;
  int value = channelOutputs[channel] + 2 * PPM_CH_CENTER(channel) - 2 * PPM_CENTER;

    if (i < sendUpperChannels) {
      int channel = 8 + g_model.moduleData[moduleIdx].channelsStart + i;
      int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
      pulseValue = limit(2049, (value * 512 / 682) + 3072, 4094);
    }
    else if (i < sentModulePXXChannels(moduleIdx)) {
      int channel = g_model.moduleData[moduleIdx].channelsStart + i;
      int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
      pulseValue = limit(1, (value * 512 / 682) + 1024, 2046);
    }

  uint8_t channel = g_model.moduleData[module].channelsStart;
  uint8_t count = sentModuleChannels(module);
  for (int8_t i = 0; i < count; i++, channel++) {
  int value = channelOutputs[channel] + 2*PPM_CH_CENTER(channel) - 2*PPM_CENTER;
  pulseValue = limit(1, (value * 512 / 682) + 1024, 2046);

  for (int i=0; i<SBUS_NORMAL_CHANS; i++) {
  int value = getChannelValue(EXTERNAL_MODULE, i);


inline int getChannelValue(uint8_t port, int channel)
{
  int ch = g_model.moduleData[port].channelsStart + channel;
  // We will ignore 17 and 18th if that brings us over the limit
  if (ch > 31)
  return 0;
  return channelOutputs[ch] + 2 * PPM_CH_CENTER(ch) - 2*PPM_CENTER;
}

extern uint8_t g_moduleIdx;



  int i = 0;

  for (int ch=1; ch<=MAX_OUTPUT_CHANNELS; ch++) {
  MixData * md;
  coord_t y = MENU_CONTENT_TOP + (cur-menuVerticalOffset)*FH;
  if (i<MAX_MIXERS && (md=mixAddress(i))->srcRaw && md->destCh+1 == ch) {
    if (cur-menuVerticalOffset >= 0 && cur-menuVerticalOffset < NUM_BODY_LINES) {
    putsChn(MENUS_MARGIN_LEFT, y, ch, 0); // show CHx
    }


*/

