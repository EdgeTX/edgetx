/*
 * (c) www.olliw.eu, OlliW, OlliW42
 */

#include "opentx.h"

constexpr float FPI = 3.141592653589793f;
constexpr float FDEGTORAD = FPI/180.0f;
constexpr float FRADTODEG = 180.0f/FPI;

#define INCU8(x)    if ((x) < UINT8_MAX) { (x)++; }

// quaternions and euler angles
// we do not use NED (roll-pitch-yaw) to convert quaternion to Euler angles and vice versa
// we use pitch-roll-yaw instead
// when the roll angle is zero, both are equivalent, this should be the majority of cases anyhow
// also, for most gimbals pitch-roll-yaw is appropriate
// the issue with NED is the gimbal lock at pitch +-90°, but pitch +-90° is a common operation point for gimbals
// the angles we store in this lib are thus pitch-roll-yaw Euler

//NED
void calc_q_from_NED_angles_deg(float* q, float roll_deg, float pitch_deg, float yaw_deg)
{
  float cr2 = cosf(roll_deg * 0.5f * FDEGTORAD);
  float sr2 = sinf(roll_deg * 0.5f * FDEGTORAD);
  float cp2 = cosf(pitch_deg * 0.5f * FDEGTORAD);
  float sp2 = sinf(pitch_deg * 0.5f * FDEGTORAD);
  float cy2 = cosf(yaw_deg * 0.5f * FDEGTORAD);
  float sy2 = sinf(yaw_deg * 0.5f * FDEGTORAD);

  q[0] = cr2*cp2*cy2 + sr2*sp2*sy2;
  q[1] = sr2*cp2*cy2 - cr2*sp2*sy2;
  q[2] = cr2*sp2*cy2 + sr2*cp2*sy2;
  q[3] = cr2*cp2*sy2 - sr2*sp2*cy2;
}

//equal for NED and G
void calc_q_from_pitchyaw_deg(float* q, float pitch_deg, float yaw_deg)
{
  float cp2 = cosf(pitch_deg * 0.5f * FDEGTORAD);
  float sp2 = sinf(pitch_deg * 0.5f * FDEGTORAD);
  float cy2 = cosf(yaw_deg * 0.5f * FDEGTORAD);
  float sy2 = sinf(yaw_deg * 0.5f * FDEGTORAD);

  q[0] = cp2*cy2;
  q[1] = -sp2*sy2;
  q[2] = sp2*cy2;
  q[3] = cp2*sy2;
}

void calc_NED_angles_deg_from_q(float* roll_deg, float* pitch_deg, float* yaw_deg, float* q)
{
  *roll_deg = atan2f( 2.0f*(q[0]*q[1] + q[2]*q[3]), 1.0f - 2.0f*(q[1]*q[1] + q[2]*q[2]) ) * FRADTODEG;

  float arg = 2.0f*(q[0]*q[2] - q[1]*q[3]);
  if (isnan(arg)) {
     *pitch_deg = 0.0f;
  } 
  else if (arg >= 1.0f) {
    *pitch_deg = 90.0f;
  } 
  else if (arg <= -1.0f) {
    *pitch_deg = -90.0f;
  } 
  else {
    *pitch_deg = asinf(arg) * FRADTODEG;
  }

  *yaw_deg = atan2f( 2.0f*(q[0]*q[3] + q[1]*q[2]), 1.0f - 2.0f*(q[2]*q[2] + q[3]*q[3]) ) * FRADTODEG;
}

void calc_G_angles_deg_from_q(float* roll_deg, float* pitch_deg, float* yaw_deg, float* q)
{
  *pitch_deg = atan2f( q[0]*q[2] - q[1]*q[3], 0.5f - q[1]*q[1] - q[2]*q[2] ) * FRADTODEG;

  float arg = 2.0f*(q[0]*q[1] + q[2]*q[3]);
  if (isnan(arg)) {
     *roll_deg = 0.0f;
  } 
  else if (arg >= 1.0f) {
    *roll_deg = 90.0f;
  } 
  else if (arg <= -1.0f) {
    *roll_deg = -90.0f;
  } 
  else {
    *roll_deg = asinf(arg) * FRADTODEG;
  }

  *yaw_deg = atan2f( q[0]*q[3] - q[1]*q[2], 0.5f - q[1]*q[1] - q[3]*q[3] ) * FRADTODEG;
}

// -- Generate MAVLink messages --
// these should never be called directly, should only be called by the task handler

void MavlinkTelem::generateCmdDoMountConfigure(uint8_t tsystem, uint8_t tcomponent, uint8_t mode)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_DO_MOUNT_CONFIGURE,
      mode, 0,0,0,0,0,0
      );
}

//ArduPilot: if a mount has no pan control, then this will also yaw the copter in guided mode overwriting _fixed_yaw !!
void MavlinkTelem::generateCmdDoMountControl(uint8_t tsystem, uint8_t tcomponent, float pitch_deg, float yaw_deg, uint8_t mode)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_DO_MOUNT_CONTROL,
      pitch_deg, 0.0, yaw_deg, 0,0,0, mode
      );
}

void MavlinkTelem::generateCmdRequestGimbalDeviceInformation(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_REQUEST_MESSAGE,
      FASTMAVLINK_MSG_ID_GIMBAL_DEVICE_INFORMATION, 0,0,0,0,0,0
      );
}

//STorM32 specific
void MavlinkTelem::generateStorm32GimbalDeviceControl(uint8_t tsystem, uint8_t tcomponent,
    float pitch_deg, float yaw_deg, uint16_t flags)
{
float q[4];

  if ((pitch_deg != NAN) && (yaw_deg != NAN)) {
    calc_q_from_pitchyaw_deg(q, pitch_deg, yaw_deg);
  } 
  else {
    q[0] = q[1] = q[2] = q[3] = NAN;
  }

  fmav_msg_storm32_gimbal_device_control_pack(
      &_msg_out, _my_sysid, _my_compid, //_sys_id and not _my_sysid !!! we mimic being part of the autopilot system
      tsystem, tcomponent,
      flags,
      q,
      NAN, NAN, NAN,
      &_status_out
      );
  _msg_out_available = true;
}

//STorM32 specific
void MavlinkTelem::generateCmdRequestStorm32GimbalManagerInformation(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_REQUEST_MESSAGE,
      FASTMAVLINK_MSG_ID_STORM32_GIMBAL_MANAGER_INFORMATION, 0,0,0,0,0,0
      );
}

//STorM32 specific
void MavlinkTelem::generateStorm32GimbalManagerControl(uint8_t tsystem, uint8_t tcomponent,
    uint8_t gimbal_id, float pitch_deg, float yaw_deg, uint16_t device_flags, uint16_t manager_flags)
{
float q[4];

  if ((pitch_deg != NAN) && (yaw_deg != NAN)) {
    calc_q_from_pitchyaw_deg(q, pitch_deg, yaw_deg);
  } 
  else {
    q[0] = q[1] = q[2] = q[3] = NAN;
  }

  fmav_msg_storm32_gimbal_manager_control_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      gimbal_id,
      MAV_STORM32_GIMBAL_MANAGER_CLIENT_GCS, //client
      device_flags, manager_flags,
      q,
      NAN, NAN, NAN,
      &_status_out
      );
  _msg_out_available = true;
}

//STorM32 specific
void MavlinkTelem::generateStorm32GimbalManagerControlPitchYaw(uint8_t tsystem, uint8_t tcomponent,
    uint8_t gimbal_id, float pitch_deg, float yaw_deg, uint16_t device_flags, uint16_t manager_flags)
{
  fmav_msg_storm32_gimbal_manager_control_pitchyaw_pack(
      &_msg_out, _my_sysid, _my_compid,
      tsystem, tcomponent,
      gimbal_id,
      MAV_STORM32_GIMBAL_MANAGER_CLIENT_GCS, //client
      device_flags, manager_flags,
      pitch_deg*FDEGTORAD, yaw_deg*FDEGTORAD,
      NAN, NAN,
      &_status_out
      );
  _msg_out_available = true;
}

//STorM32 specific
void MavlinkTelem::generateCmdStorm32DoGimbalManagerControlPitchYaw(uint8_t tsystem, uint8_t tcomponent,
    uint8_t gimbal_id, float pitch_deg, float yaw_deg, uint16_t device_flags, uint16_t manager_flags)
{
  _generateCmdLong(tsystem, tcomponent,
      MAV_CMD_STORM32_DO_GIMBAL_MANAGER_CONTROL_PITCHYAW,
      pitch_deg, yaw_deg, NAN, NAN, device_flags, manager_flags,
      (uint16_t)gimbal_id + ((uint16_t)MAV_STORM32_GIMBAL_MANAGER_CLIENT_GCS < 8)
      );
}

// -- Mavsdk Convenience Task Wrapper --
// to make it easy for api_mavsdk to call functions

void MavlinkTelem::sendGimbalTargetingMode(uint8_t mode)
{
  _t_gimbal_mode = mode;
  gimbalmanagerOut.mount_mode = mode;
  SETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_MOUNT_CONFIGURE);
}

void MavlinkTelem::sendGimbalPitchYawDeg(float pitch, float yaw)
{
  if (_storm32_gimbal_protocol_v2) {
    sendStorm32GimbalManagerPitchYawDeg(pitch, yaw);
    return;
  }
  _t_gimbal_pitch_deg = pitch;
  _t_gimbal_yaw_deg = yaw;
  _t_gimbal_mode = gimbalmanagerOut.mount_mode;
  SETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_MOUNT_CONTROL);
}

//we shouldn't use this
void MavlinkTelem::sendStorm32GimbalDevicePitchYawDeg(float pitch, float yaw)
{
  _t_storm32GD_pitch_deg = pitch;
  _t_storm32GD_yaw_deg = yaw;
  //we assume a simple 3 axis gimbal
  // our gimbal must EXACTLY have these capabilities for now, so we fake it
  _t_storm32GD_flags |= (MAV_STORM32_GIMBAL_DEVICE_FLAGS_ROLL_LOCK | MAV_STORM32_GIMBAL_DEVICE_FLAGS_PITCH_LOCK);
  _t_storm32GD_flags &=~ (MAV_STORM32_GIMBAL_DEVICE_FLAGS_YAW_LOCK | 0xFF00);
  SETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_DEVICE_CONTROL);
}

void MavlinkTelem::setStorm32GimbalClientRetract(bool enable)
{
  if (!_storm32_gimbal_protocol_v2) return;

  gimbalmanagerOut.device_flags &=~ (MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT | MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL);
  if( enable ){
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT;
  }
}

void MavlinkTelem::setStorm32GimbalClientNeutral(bool enable)
{
  if (!_storm32_gimbal_protocol_v2) return;

  gimbalmanagerOut.device_flags &=~ (MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT | MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL);
  if( enable ){
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL;
  }
}

void MavlinkTelem::setStorm32GimbalClientLock(bool roll_lock, bool pitch_lock, bool yaw_lock)
{
  if (!_storm32_gimbal_protocol_v2) return;

  gimbalmanagerOut.device_flags &=~ (MAV_STORM32_GIMBAL_DEVICE_FLAGS_ROLL_LOCK |
                                     MAV_STORM32_GIMBAL_DEVICE_FLAGS_PITCH_LOCK |
                                     MAV_STORM32_GIMBAL_DEVICE_FLAGS_YAW_LOCK );
  if( roll_lock ){
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_ROLL_LOCK;
  }
  if( pitch_lock ){
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_PITCH_LOCK;
  }
  if( yaw_lock ){
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_YAW_LOCK;
  }
}

void MavlinkTelem::setStorm32GimbalClientFlags(uint16_t manager_flags)
{
  if (!_storm32_gimbal_protocol_v2) return;

  manager_flags |= MAV_STORM32_GIMBAL_MANAGER_FLAGS_SET_SUPERVISON; //for the moment the GCS always claims supervision

  gimbalmanagerOut.manager_flags = manager_flags;
}

void MavlinkTelem::_refreshGimbalClientFlags(uint16_t* device_flags, uint16_t* manager_flags)
{
  gimbalmanagerOut.device_flags &=~ (MAV_STORM32_GIMBAL_DEVICE_FLAGS_RC_EXCLUSIVE | MAV_STORM32_GIMBAL_DEVICE_FLAGS_RC_MIXED);

  if (*manager_flags & MAV_STORM32_GIMBAL_MANAGER_FLAGS_RC_ACTIVE) {
    gimbalmanagerOut.device_flags |= MAV_STORM32_GIMBAL_DEVICE_FLAGS_RC_MIXED; //we always do it mixed!
  }
  *device_flags = gimbalmanagerOut.device_flags;

  gimbalmanagerOut.manager_flags |= MAV_STORM32_GIMBAL_MANAGER_FLAGS_SET_SUPERVISON;

  *manager_flags = gimbalmanagerOut.manager_flags;
}

void MavlinkTelem::sendStorm32GimbalManagerPitchYawDeg(float pitch, float yaw)
{
  if (!gimbalmanager.compid) return; //no gimbal manager

  _refreshGimbalClientFlags(&_t_storm32GM_gdflags, &_t_storm32GM_gmflags);

  if ((_t_storm32GM_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT) ||
      (_t_storm32GM_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL)) {
    pitch = yaw = 0.0f;
  }
  _t_storm32GM_pitch_deg = pitch;
  _t_storm32GM_yaw_deg = yaw;
  SETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_MANAGER_CONTROL_PITCHYAW);
}

void MavlinkTelem::sendStorm32GimbalManagerControlPitchYawDeg(float pitch, float yaw)
{
  if (!gimbalmanager.compid) return; //no gimbal manager

  _refreshGimbalClientFlags(&_t_storm32GM_control_gdflags, &_t_storm32GM_control_gmflags);

  if ((_t_storm32GM_control_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT) ||
      (_t_storm32GM_control_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL)) {
    pitch = yaw = 0.0f;
  }
  _t_storm32GM_control_pitch_deg = pitch;
  _t_storm32GM_control_yaw_deg = yaw;
  SETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_MANAGER_CONTROL);
}

void MavlinkTelem::sendStorm32GimbalManagerCmdPitchYawDeg(float pitch, float yaw)
{
  if (!gimbalmanager.compid) return; //no gimbal manager

  _refreshGimbalClientFlags(&_t_storm32GM_cmd_gdflags, &_t_storm32GM_cmd_gmflags);

  if ((_t_storm32GM_cmd_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT) ||
      (_t_storm32GM_cmd_gdflags & MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL)) {
    pitch = yaw = 0.0f;
  }
  _t_storm32GM_cmd_pitch_deg = pitch;
  _t_storm32GM_cmd_yaw_deg = yaw;
  SETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_GIMBAL_MANAGER_CONTROL_PITCHYAW);
}

// -- Task handlers --

bool MavlinkTelem::doTaskGimbalAndGimbalClient(void)
{
  if (!_task[TASK_GIMBAL]) return false; // no task pending

  // if there is no gimbal, then there is also no gimbal manager which needs to be served
  // so first if()-checking for gimbal, and then for gimbalmanager is ok

  if (!gimbal.compid) { // no gimbal
    _task[TASK_GIMBAL] = 0; 
    return false; 
  }

  if (_task[TASK_GIMBAL] & TASK_SENDCMD_DO_MOUNT_CONFIGURE) {
    RESETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_MOUNT_CONFIGURE);
    generateCmdDoMountConfigure(_sysid, autopilot.compid, _t_gimbal_mode);
    return true; //do only one per loop
  }
  if (_task[TASK_GIMBAL] & TASK_SENDCMD_DO_MOUNT_CONTROL) {
    RESETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_MOUNT_CONTROL);
    generateCmdDoMountControl(_sysid, autopilot.compid, _t_gimbal_pitch_deg, _t_gimbal_yaw_deg, _t_gimbal_mode);
    return true; //do only one per loop
  }

  if (_task[TASK_GIMBAL] & TASK_SENDREQUEST_GIMBAL_DEVICE_INFORMATION) {
    RESETTASK(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_DEVICE_INFORMATION);
    generateCmdRequestGimbalDeviceInformation(_sysid, gimbal.compid);
    return true; //do only one per loop
  }

  if (_task[TASK_GIMBAL] & TASK_SENDMSG_GIMBAL_DEVICE_CONTROL) {
    RESETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_DEVICE_CONTROL);
    generateStorm32GimbalDeviceControl(_sysid, gimbal.compid, _t_storm32GD_pitch_deg, _t_storm32GD_yaw_deg, _t_storm32GD_flags);
    return true; //do only one per loop
  }

  if (!gimbalmanager.compid) { // no gimbal manager
    RESETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_MANAGER_CONTROL_PITCHYAW | TASK_SENDMSG_GIMBAL_MANAGER_CONTROL |
                           TASK_SENDCMD_DO_GIMBAL_MANAGER_CONTROL_PITCHYAW | TASK_SENDREQUEST_GIMBAL_MANAGER_INFORMATION);
	  return false;
  }

  if (_task[TASK_GIMBAL] & TASK_SENDMSG_GIMBAL_MANAGER_CONTROL_PITCHYAW) {
    RESETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_MANAGER_CONTROL_PITCHYAW);
    generateStorm32GimbalManagerControlPitchYaw(_sysid, gimbalmanager.compid, gimbal.compid,
        _t_storm32GM_pitch_deg, _t_storm32GM_yaw_deg, _t_storm32GM_gdflags, _t_storm32GM_gmflags);
    return true; //do only one per loop
  }
  if (_task[TASK_GIMBAL] & TASK_SENDMSG_GIMBAL_MANAGER_CONTROL) {
    RESETTASK(TASK_GIMBAL, TASK_SENDMSG_GIMBAL_MANAGER_CONTROL);
    generateStorm32GimbalManagerControl(_sysid, gimbalmanager.compid, gimbal.compid,
        _t_storm32GM_control_pitch_deg, _t_storm32GM_control_yaw_deg, _t_storm32GM_control_gdflags, _t_storm32GM_control_gmflags);
    return true; //do only one per loop
  }
  if (_task[TASK_GIMBAL] & TASK_SENDCMD_DO_GIMBAL_MANAGER_CONTROL_PITCHYAW) {
    RESETTASK(TASK_GIMBAL, TASK_SENDCMD_DO_GIMBAL_MANAGER_CONTROL_PITCHYAW);
    generateCmdStorm32DoGimbalManagerControlPitchYaw(_sysid, gimbalmanager.compid, gimbal.compid,
        _t_storm32GM_cmd_pitch_deg, _t_storm32GM_cmd_yaw_deg, _t_storm32GM_cmd_gdflags, _t_storm32GM_cmd_gmflags);
    return true; //do only one per loop
  }
  if (_task[TASK_GIMBAL] & TASK_SENDREQUEST_GIMBAL_MANAGER_INFORMATION) {
    RESETTASK(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_MANAGER_INFORMATION);
    generateCmdRequestStorm32GimbalManagerInformation(_sysid, gimbalmanager.compid);
    return true; //do only one per loop
  }

  return false;
}

// -- Handle incoming MAVLink messages, which are for the Gimbal --

void MavlinkTelem::handleMessageGimbal(void)
{
  gimbal.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT; //we accept any msg from the gimbal to indicate it is alive

  switch (_msg.msgid) {
    case FASTMAVLINK_MSG_ID_HEARTBEAT: {
      fmav_heartbeat_t payload;
      fmav_msg_heartbeat_decode(&payload, &_msg);
      gimbal.system_status = payload.system_status;
      gimbal.custom_mode = payload.custom_mode;
      gimbal.is_armed = (payload.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ? true : false;
      gimbal.is_standby = (payload.system_status <= MAV_STATE_STANDBY) ? true : false;
      gimbal.is_critical = (payload.system_status >= MAV_STATE_CRITICAL) ? true : false;
      gimbal.prearm_ok = (payload.custom_mode & 0x80000000) ? false : true;
      INCU8(gimbal.updated);
      //gimbal.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT;
      break;
    }

    case FASTMAVLINK_MSG_ID_ATTITUDE: {
      fmav_attitude_t payload;
      fmav_msg_attitude_decode(&payload, &_msg);
      gimbalAtt.roll_deg = payload.roll * FRADTODEG;
      gimbalAtt.pitch_deg = payload.pitch * FRADTODEG;
      gimbalAtt.yaw_deg_relative = payload.yaw * FRADTODEG;
      if (gimbalAtt.yaw_deg_absolute > 180.0f) gimbalAtt.yaw_deg_absolute -= 360.0f;
      gimbalAtt.yaw_deg_absolute = gimbalAtt.yaw_deg_relative + att.yaw_rad * FRADTODEG;
      if (gimbalAtt.yaw_deg_absolute < -180.0f) gimbalAtt.yaw_deg_absolute += 360.0f;
      INCU8(gimbalAtt.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_MOUNT_STATUS: {
      fmav_mount_status_t payload;
      fmav_msg_mount_status_decode(&payload, &_msg);
      gimbalAtt.roll_deg = ((float)payload.pointing_b * 0.01f);
      gimbalAtt.pitch_deg = ((float)payload.pointing_a * 0.01f);
      gimbalAtt.yaw_deg_relative = ((float)payload.pointing_c * 0.01f);
      if (gimbalAtt.yaw_deg_absolute > 180.0f) gimbalAtt.yaw_deg_absolute -= 360.0f;
      gimbalAtt.yaw_deg_absolute = gimbalAtt.yaw_deg_relative + att.yaw_rad * FRADTODEG;
      if (gimbalAtt.yaw_deg_absolute < -180.0f) gimbalAtt.yaw_deg_absolute += 360.0f;
      INCU8(gimbalAtt.updated);
      break;
    }

    case FASTMAVLINK_MSG_ID_GIMBAL_DEVICE_INFORMATION: {
      fmav_gimbal_device_information_t payload;
      fmav_msg_gimbal_device_information_decode(&payload, &_msg);
      memset(gimbaldeviceInfo.vendor_name, 0, 32+1);
      memcpy(gimbaldeviceInfo.vendor_name, payload.vendor_name, 32);
      memset(gimbaldeviceInfo.model_name, 0, 32+1);
      memcpy(gimbaldeviceInfo.model_name, payload.model_name, 32);
      memset(gimbaldeviceInfo.custom_name, 0, 32+1);
      memcpy(gimbaldeviceInfo.custom_name, payload.custom_name, 32);
      gimbaldeviceInfo.firmware_version = payload.firmware_version;
      gimbaldeviceInfo.hardware_version = payload.hardware_version;
      //STorM32 has extra cap flags in custom area, which have to go into high word
      gimbaldeviceInfo.cap_flags = payload.cap_flags + (((uint32_t)payload.custom_cap_flags) << 16);
      INCU8(gimbaldeviceInfo.updated);
      clear_request(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_DEVICE_INFORMATION);
      gimbal.requests_waiting_mask &=~ GIMBAL_REQUESTWAITING_GIMBAL_DEVICE_INFORMATION;
      gimbalmanager.requests_waiting_mask &=~ GIMBAL_REQUESTWAITING_GIMBAL_DEVICE_INFORMATION;
      // we do not copy to the gimbal manager capability flags, we rely on the gimbal manager to provide the correct ones
      break;
    }

    case FASTMAVLINK_MSG_ID_STORM32_GIMBAL_DEVICE_STATUS: {
      fmav_storm32_gimbal_device_status_t payload;
      fmav_msg_storm32_gimbal_device_status_decode(&payload, &_msg);
      calc_G_angles_deg_from_q(&gimbalAtt.roll_deg, &gimbalAtt.pitch_deg, &gimbalAtt.yaw_deg_relative, payload.q);
      gimbalAtt.yaw_deg_absolute = gimbalAtt.yaw_deg_relative + att.yaw_rad * FRADTODEG;
      if (gimbalAtt.yaw_deg_absolute < -180.0f) gimbalAtt.yaw_deg_absolute += 360.0f;
      INCU8(gimbalAtt.updated);
      // update gimbal manager flags, do it alwayy, even if no gimbal manager has been discovered
      gimbalmanagerStatus.device_flags = payload.flags;
      break;
    }
  }
}

// -- Handle incoming MAVLink messages, which are for the Gimbal Client --

void MavlinkTelem::handleMessageGimbalClient(void)
{
  //gimbalmanager.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT; //we accept any msg from the manager to indicate it is alive
  //no: we should not do this for the manager, since this function is called for any message from the hosting component

  switch (_msg.msgid) {
    case FASTMAVLINK_MSG_ID_STORM32_GIMBAL_MANAGER_INFORMATION: {
      fmav_storm32_gimbal_manager_information_t payload;
      fmav_msg_storm32_gimbal_manager_information_decode(&payload, &_msg);
      if (payload.gimbal_id != gimbal.compid) break; //not for us
      gimbalmanagerInfo.device_cap_flags = payload.device_cap_flags;
      gimbalmanagerInfo.manager_cap_flags = payload.manager_cap_flags;
      INCU8(gimbalmanagerInfo.updated);
      clear_request(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_MANAGER_INFORMATION);
      gimbalmanager.requests_waiting_mask &=~ GIMBAL_REQUESTWAITING_GIMBAL_MANAGER_INFORMATION;
      break;
    }

    case FASTMAVLINK_MSG_ID_STORM32_GIMBAL_MANAGER_STATUS: {
      fmav_storm32_gimbal_manager_status_t payload;
      fmav_msg_storm32_gimbal_manager_status_decode(&payload, &_msg);
      if (payload.gimbal_id != gimbal.compid) break; //not for us
      gimbalmanagerStatus.supervisor = payload.supervisor;
      gimbalmanagerStatus.device_flags = payload.device_flags;
      gimbalmanagerStatus.manager_flags = payload.manager_flags;
      gimbalmanagerStatus.profile = payload.profile;
      INCU8(gimbalmanagerStatus.updated);
      gimbalmanager.is_receiving = MAVLINK_TELEM_GIMBALMANAGER_RECEIVING_TIMEOUT;
      break;
    }
  }
}

// -- Startup Requests --

void MavlinkTelem::setGimbalStartupRequests(void)
{
  set_request(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_DEVICE_INFORMATION, 10, 200-12);
}

void MavlinkTelem::setGimbalClientStartupRequests(void)
{
  set_request(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_DEVICE_INFORMATION, 10, 200-12);
  set_request(TASK_GIMBAL, TASK_SENDREQUEST_GIMBAL_MANAGER_INFORMATION, 10, 200-18);
}

// -- Resets --

void MavlinkTelem::_resetGimbalAndGimbalClient(void)
{
  _task[TASK_GIMBAL] = 0;

  gimbal.compid = 0;
  gimbal.is_receiving = 0;

  gimbal.requests_triggered = 0;
  gimbal.requests_waiting_mask = GIMBAL_REQUESTWAITING_ALL;
  gimbal.is_initialized = false;

  gimbal.system_status = MAV_STATE_UNINIT;
  gimbal.custom_mode = 0;
  gimbal.is_armed = false;
  gimbal.is_standby = true;
  gimbal.is_critical = false;
  gimbal.prearm_ok = false;
  gimbal.updated = 0;

  gimbalAtt.roll_deg = 0.0f;
  gimbalAtt.pitch_deg = 0.0f;
  gimbalAtt.yaw_deg_relative = 0.0f;
  gimbalAtt.yaw_deg_absolute = 0.0f;
  gimbalAtt.updated = 0;

  _resetGimbalClient();
}

void MavlinkTelem::_resetGimbalClient(void)
{
  //_task[TASK_GIMBAL] = 0; // only reset gimbal client tasks, but not very important

  gimbalmanager.compid = 0;
  gimbalmanager.is_receiving = 0;

  gimbalmanager.requests_triggered = 0;
  gimbalmanager.requests_waiting_mask = GIMBALCLIENT_REQUESTWAITING_ALL; //this prevents it ever becomes initialized = true
  gimbalmanager.is_initialized = false;

  gimbalmanager.updated = 0;

  gimbaldeviceInfo.vendor_name[0] = 0;
  gimbaldeviceInfo.model_name[0] = 0;
  gimbaldeviceInfo.custom_name[0] = 0;
  gimbaldeviceInfo.firmware_version = 0;
  gimbaldeviceInfo.hardware_version = 0;
  gimbaldeviceInfo.cap_flags = 0;
  gimbaldeviceInfo.updated = 0;

  gimbalmanagerInfo.device_cap_flags = 0;
  gimbalmanagerInfo.manager_cap_flags = 0;
  gimbalmanagerInfo.updated = 0;

  gimbalmanagerStatus.supervisor = 0;
  gimbalmanagerStatus.device_flags = 0;
  gimbalmanagerStatus.manager_flags = 0;
  gimbalmanagerStatus.profile = 0;
  gimbalmanagerStatus.updated = 0;

  gimbalmanagerOut.mount_mode = MAV_MOUNT_MODE_MAVLINK_TARGETING;
  gimbalmanagerOut.device_flags = MAV_STORM32_GIMBAL_DEVICE_FLAGS_ROLL_LOCK | MAV_STORM32_GIMBAL_DEVICE_FLAGS_PITCH_LOCK;
  gimbalmanagerOut.manager_flags = MAV_STORM32_GIMBAL_MANAGER_FLAGS_SET_SUPERVISON;
}
