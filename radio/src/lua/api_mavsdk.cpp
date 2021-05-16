/*
 *  (c) www.olliw.eu, OlliW, OlliW42
 */

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "lua_api.h"


constexpr float FPI = 3.141592653589793f;
constexpr float FDEGTORAD = FPI/180.0f;
constexpr float FRADTODEG = 180.0f/FPI;

void u8toBCDstr(uint8_t n, char* s)
{
  if (n >= 100) { 
    for (*s = '0'; n >= 100; n -= 100) (*s)++;
    s++; 
  }
  if (n >= 10) { 
    for (*s = '0'; n >= 10; n -= 10) (*s)++;
    s++; 
  }
  *s = '0' + n; 
  s++;
  *s = '\0';
}

// -- GIMBAL --

static int luaMavsdkGimbalIsReceiving(lua_State *L)
{
  bool flag = (mavlinkTelem.gimbal.is_receiving > 0);
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGimbalIsInitialized(lua_State *L)
{
  bool flag = (mavlinkTelem.gimbal.is_receiving > 0) && mavlinkTelem.gimbal.is_initialized;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGimbalGetInfo(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "compid", mavlinkTelem.gimbal.compid);

  lua_pushtablestring(L, "vendor_name", mavlinkTelem.gimbaldeviceInfo.vendor_name);
  lua_pushtablestring(L, "model_name", mavlinkTelem.gimbaldeviceInfo.model_name);
  lua_pushtablestring(L, "custom_name", mavlinkTelem.gimbaldeviceInfo.custom_name);
  char s[32], ss[20];
  s[0] = '\0';
  if (mavlinkTelem.gimbaldeviceInfo.firmware_version) {
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.firmware_version >> 0) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.firmware_version >> 8) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.firmware_version >> 16) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.firmware_version >> 24) & 0xFF, ss); strcat(s, ss);
  }
  lua_pushtablestring(L, "firmware_version", s);
  s[0] = '\0';
  if (mavlinkTelem.gimbaldeviceInfo.hardware_version) {
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.hardware_version >> 0) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.hardware_version >> 8) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.hardware_version >> 16) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.gimbaldeviceInfo.hardware_version >> 24) & 0xFF, ss); strcat(s, ss);
  }
  lua_pushtablestring(L, "hardware_version", s);
  lua_pushtableinteger(L, "capability_flags", mavlinkTelem.gimbaldeviceInfo.cap_flags);
  return 1;
}

static int luaMavsdkGimbalGetStatus(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablenumber(L, "system_status", mavlinkTelem.gimbal.system_status);
  lua_pushtablenumber(L, "custom_mode", mavlinkTelem.gimbal.custom_mode);
  lua_pushtableboolean(L, "is_armed", mavlinkTelem.gimbal.is_armed);
  lua_pushtableboolean(L, "prearm_ok", mavlinkTelem.gimbal.prearm_ok);
  return 1;
}

static int luaMavsdkGimbalGetAttRollDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gimbalAtt.roll_deg);
  return 1;
}

static int luaMavsdkGimbalGetAttPitchDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gimbalAtt.pitch_deg);
  return 1;
}

static int luaMavsdkGimbalGetAttYawDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gimbalAtt.yaw_deg_relative);
  return 1;
}

// gimbal protocol v1
// can also be used for gimbal protocol v2, if _gimbal_protocol_v2 is set accordingly

static int luaMavsdkGimbalSendNeutralMode(lua_State *L)
{
  mavlinkTelem.sendGimbalTargetingMode(MAV_MOUNT_MODE_NEUTRAL);
  return 0;
}

static int luaMavsdkGimbalSendMavlinkTargetingMode(lua_State *L)
{
  mavlinkTelem.sendGimbalTargetingMode(MAV_MOUNT_MODE_MAVLINK_TARGETING);
  return 0;
}

static int luaMavsdkGimbalSendRcTargetingMode(lua_State *L)
{
  mavlinkTelem.sendGimbalTargetingMode(MAV_MOUNT_MODE_RC_TARGETING);
  return 0;
}

static int luaMavsdkGimbalSendGpsPointMode(lua_State *L)
{
  mavlinkTelem.sendGimbalTargetingMode(MAV_MOUNT_MODE_GPS_POINT);
  return 0;
}

static int luaMavsdkGimbalSendSysIdTargetingMode(lua_State *L)
{
  mavlinkTelem.sendGimbalTargetingMode(MAV_MOUNT_MODE_SYSID_TARGET);
  return 0;
}

static int luaMavsdkGimbalSendPitchYawDeg(lua_State *L)
{
  float pitch = luaL_checknumber(L, 1);
  float yaw = luaL_checknumber(L, 2);
  mavlinkTelem.sendGimbalPitchYawDeg(pitch, yaw);
  return 0;
}

// -- GIMBAL CLIENT -- gimbal protocol v2

static int luaMavsdkIsGimbalProtocolV2(lua_State *L)
{
  bool flag = mavlinkTelem.isStorm32GimbalProtocolV2();
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkSetGimbalProtocolV2(lua_State *L)
{
  bool flag = (luaL_checknumber(L, 1) > 0);
  mavlinkTelem.setStorm32GimbalProtocolV2(flag);
  return 0;
}

static int luaMavsdkGimbalClientIsReceiving(lua_State *L)
{
  bool flag = (mavlinkTelem.gimbalmanager.is_receiving > 0);
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGimbalClientIsInitialized(lua_State *L)
{
  bool flag = (mavlinkTelem.gimbalmanager.is_receiving > 0) && mavlinkTelem.gimbalmanager.is_initialized;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGimbalClientGetInfo(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "gimbal_manager_id", mavlinkTelem.gimbalmanager.compid);
  lua_pushtableinteger(L, "gimbal_id", mavlinkTelem.gimbal.compid);
  lua_pushtableinteger(L, "device_capability_flags", mavlinkTelem.gimbalmanagerInfo.device_cap_flags);
  lua_pushtableinteger(L, "manager_capability_flags", mavlinkTelem.gimbalmanagerInfo.manager_cap_flags);
  return 1;
}

static int luaMavsdkGimbalClientGetStatus(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "supervisor", mavlinkTelem.gimbalmanagerStatus.supervisor);
  lua_pushtableinteger(L, "device_flags", mavlinkTelem.gimbalmanagerStatus.device_flags);
  lua_pushtableinteger(L, "manager_flags", mavlinkTelem.gimbalmanagerStatus.manager_flags);
  lua_pushtableinteger(L, "profile", mavlinkTelem.gimbalmanagerStatus.profile);
  return 1;
}

static int luaMavsdkGimbalClientSetRetract(lua_State *L)
{
  bool enable = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.setStorm32GimbalClientRetract(enable);
  return 0;
}

static int luaMavsdkGimbalClientSetNeutral(lua_State *L)
{
  bool enable = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.setStorm32GimbalClientNeutral(enable);
  return 0;
}

static int luaMavsdkGimbalClientSetLock(lua_State *L)
{
  bool roll_lock = (luaL_checkinteger(L, 1) > 0);
  bool pitch_lock = (luaL_checkinteger(L, 1) > 0);
  bool yaw_lock = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.setStorm32GimbalClientLock(roll_lock, pitch_lock, yaw_lock);
  return 0;
}

static int luaMavsdkGimbalClientSetFlags(lua_State *L)
{
  uint16_t manager_flags = luaL_checkinteger(L, 1);
  mavlinkTelem.setStorm32GimbalClientFlags(manager_flags);
  return 0;
}

static int luaMavsdkGimbalClientSendPitchYawDeg(lua_State *L)
{
  float pitch = luaL_checknumber(L, 1);
  float yaw = luaL_checknumber(L, 2);
  mavlinkTelem.sendStorm32GimbalManagerPitchYawDeg(pitch, yaw);
  return 0;
}

static int luaMavsdkGimbalClientSendControlPitchYawDeg(lua_State *L)
{
  float pitch = luaL_checknumber(L, 1);
  float yaw = luaL_checknumber(L, 2);
  mavlinkTelem.sendStorm32GimbalManagerControlPitchYawDeg(pitch, yaw);
  return 0;
}

static int luaMavsdkGimbalClientSendCmdPitchYawDeg(lua_State *L)
{
  float pitch = luaL_checknumber(L, 1);
  float yaw = luaL_checknumber(L, 2);
  mavlinkTelem.sendStorm32GimbalManagerCmdPitchYawDeg(pitch, yaw);
  return 0;
}

static int luaMavsdkGimbalDeviceSendPitchYawDeg(lua_State *L)
{
  float pitch = luaL_checknumber(L, 1);
  float yaw = luaL_checknumber(L, 2);
  mavlinkTelem.sendStorm32GimbalDevicePitchYawDeg(pitch, yaw);
  return 0;
}

// -- QSHOT --

static int luaMavsdkQShotSendCmdConfigure(lua_State *L)
{
  uint8_t mode = luaL_checkinteger(L, 1);
  uint8_t shot_state = luaL_checkinteger(L, 2);
  mavlinkTelem.sendQShotCmdConfigure(mode, shot_state);
  return 0;
}

static int luaMavsdkQShotSendStatus(lua_State *L)
{
  uint8_t mode = luaL_checkinteger(L, 1);
  uint8_t shot_state = luaL_checkinteger(L, 2);
  mavlinkTelem.sendQShotStatus(mode, shot_state);
  return 0;
}

static int luaMavsdkQShotGetStatus(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "mode", mavlinkTelem.qshot.mode);
  lua_pushtableinteger(L, "shot_state", mavlinkTelem.qshot.shot_state);
  return 1;
}

static int luaMavsdkQShotButtonState(lua_State *L)
{
  uint8_t button_state = luaL_checkinteger(L, 1);
  mavlinkTelem.sendQShotButtonState(button_state);
  return 0;
}

// -- CAMERA --

static int luaMavsdkCameraIsReceiving(lua_State *L)
{
  bool flag = (mavlinkTelem.camera.is_receiving > 0);
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkCameraIsInitialized(lua_State *L)
{
  bool flag = (mavlinkTelem.camera.is_receiving > 0) && mavlinkTelem.camera.is_initialized;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkCameraGetInfo(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "compid", mavlinkTelem.camera.compid);
  lua_pushtableinteger(L, "flags", mavlinkTelem.cameraInfo.flags);
  lua_pushtableboolean(L, "has_video", mavlinkTelem.cameraInfo.has_video);
  lua_pushtableboolean(L, "has_photo", mavlinkTelem.cameraInfo.has_photo);
  lua_pushtableboolean(L, "has_modes", mavlinkTelem.cameraInfo.has_modes);
  if (!isnan(mavlinkTelem.cameraInfo.total_capacity_MiB)) {
    lua_pushtablenumber(L, "total_capacity", mavlinkTelem.cameraInfo.total_capacity_MiB);
  } 
  else {
    lua_pushtablenil(L, "total_capacity");
  }
  lua_pushtablestring(L, "vendor_name", mavlinkTelem.cameraInfo.vendor_name);
  lua_pushtablestring(L, "model_name", mavlinkTelem.cameraInfo.model_name);
  char s[32], ss[20]; s[0] = '\0';
  if (mavlinkTelem.cameraInfo.firmware_version) {
    u8toBCDstr((mavlinkTelem.cameraInfo.firmware_version >> 0) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.cameraInfo.firmware_version >> 8) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.cameraInfo.firmware_version >> 16) & 0xFF, ss); strcat(s, ss); strcat(s, ".");
    u8toBCDstr((mavlinkTelem.cameraInfo.firmware_version >> 24) & 0xFF, ss); strcat(s, ss);
  }
  lua_pushtablestring(L, "firmware_version", s);
  return 1;
}

static int luaMavsdkCameraGetStatus(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "system_status", mavlinkTelem.camera.system_status);
  lua_pushtableinteger(L, "mode", mavlinkTelem.cameraStatus.mode);
  lua_pushtableboolean(L, "video_on", mavlinkTelem.cameraStatus.video_on);
  lua_pushtableboolean(L, "photo_on", mavlinkTelem.cameraStatus.photo_on);
  if (!isnan(mavlinkTelem.cameraStatus.available_capacity_MiB)) {
    lua_pushtablenumber(L, "available_capacity", mavlinkTelem.cameraStatus.available_capacity_MiB);
  } 
  else {
    lua_pushtablenil(L, "available_capacity");
  }
  if (!isnan(mavlinkTelem.cameraStatus.battery_voltage_V)) {
    lua_pushtablenumber(L, "battery_voltage", mavlinkTelem.cameraStatus.battery_voltage_V);
  } 
  else {
    lua_pushtablenil(L, "battery_voltage");
  }
  if (mavlinkTelem.cameraStatus.battery_remaining_pct >= 0) {
    lua_pushtableinteger(L, "battery_remainingpct", mavlinkTelem.cameraStatus.battery_remaining_pct);
  } 
  else {
    lua_pushtablenil(L, "battery_remainingpct");
  }
  return 1;
}

static int luaMavsdkCameraSendVideoMode(lua_State *L)
{
  mavlinkTelem.sendCameraSetVideoMode();
  return 0;
}

static int luaMavsdkCameraSendPhotoMode(lua_State *L)
{
  mavlinkTelem.sendCameraSetPhotoMode();
  return 0;
}

static int luaMavsdkCameraStartVideo(lua_State *L)
{
  mavlinkTelem.sendCameraStartVideo();
  return 0;
}

static int luaMavsdkCameraStopVideo(lua_State *L)
{
  mavlinkTelem.sendCameraStopVideo();
  return 0;
}

static int luaMavsdkCameraTakePhoto(lua_State *L)
{
  mavlinkTelem.sendCameraTakePhoto();
  return 0;
}

// -- MAVSDK GENERAL --

static int luaMavsdkMavTelemIsEnabled(lua_State *L)
{
  bool flag = (g_eeGeneral.auxSerialMode == UART_MODE_MAVLINK) || (g_eeGeneral.aux2SerialMode == UART_MODE_MAVLINK) || (g_eeGeneral.mavlinkExternal == 1);
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkMavTelemVersion(lua_State *L)
{
  lua_pushstring(L, OWVERSIONSTR);
  return 1;
}

static int luaMavsdkIsReceiving(lua_State *L)
{
  bool flag = mavlinkTelem.isReceiving();
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkIsInitialized(lua_State *L)
{
  bool flag = (mavlinkTelem.autopilot.is_receiving > 0) && mavlinkTelem.autopilot.is_initialized;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGetAutopilotType(lua_State *L)
{
  int nbr = mavlinkTelem.autopilottype;
  lua_pushnumber(L, nbr);
  return 1;
}

static int luaMavsdkGetVehicleType(lua_State *L)
{
  int nbr = mavlinkTelem.vehicletype;
  lua_pushnumber(L, nbr);
  return 1;
}

static int luaMavsdkGetFlightMode(lua_State *L)
{
  int nbr = mavlinkTelem.flightmode;
  lua_pushnumber(L, nbr);
  return 1;
}

typedef enum MAVSDK_VEHICLECLASS {
   MAVSDK_VEHICLECLASS_GENERIC = 0,
   MAVSDK_VEHICLECLASS_PLANE,
   MAVSDK_VEHICLECLASS_COPTER,
   MAVSDK_VEHICLECLASS_ROVER,
   MAVSDK_VEHICLECLASS_BOAT,
   MAVSDK_VEHICLECLASS_SUB,
   MAVSDK_VEHICLECLASS_ENUM_END
} MAVSDK_VEHICLECLASS;

static int luaMavsdkGetVehicleClass(lua_State *L)
{
int nbr;

  switch (mavlinkTelem.vehicletype) {
    case MAV_TYPE_FIXED_WING:
    case MAV_TYPE_FLAPPING_WING:
    case MAV_TYPE_VTOL_DUOROTOR:
    case MAV_TYPE_VTOL_QUADROTOR:
    case MAV_TYPE_VTOL_TILTROTOR:
    case MAV_TYPE_VTOL_RESERVED2:
    case MAV_TYPE_VTOL_RESERVED3:
    case MAV_TYPE_VTOL_RESERVED4:
    case MAV_TYPE_VTOL_RESERVED5:
    case MAV_TYPE_PARAFOIL:
      nbr = MAVSDK_VEHICLECLASS_PLANE;
      break;
    case MAV_TYPE_QUADROTOR:
    case MAV_TYPE_COAXIAL:
    case MAV_TYPE_HELICOPTER:
    case MAV_TYPE_HEXAROTOR:
    case MAV_TYPE_OCTOROTOR:
    case MAV_TYPE_TRICOPTER:
    case MAV_TYPE_DODECAROTOR:
      nbr = MAVSDK_VEHICLECLASS_COPTER;
      break;
    case MAV_TYPE_GROUND_ROVER:
      nbr = MAVSDK_VEHICLECLASS_ROVER;
      break;
    case MAV_TYPE_SURFACE_BOAT:
      nbr = MAVSDK_VEHICLECLASS_BOAT;
      break;
    case MAV_TYPE_SUBMARINE:
      nbr = MAVSDK_VEHICLECLASS_SUB;
      break;
    default:
      nbr = MAVSDK_VEHICLECLASS_GENERIC;
  }
  lua_pushnumber(L, nbr);
  return 1;
}

static int luaMavsdkGetSystemStatus(lua_State *L)
{
  int nbr = mavlinkTelem.autopilot.system_status;
  lua_pushnumber(L, nbr);
  return 1;
}

static int luaMavsdkIsArmed(lua_State *L)
{
  bool flag = mavlinkTelem.autopilot.is_armed;
  lua_pushboolean(L, flag);
  return 1;
}

// -- MAVSDK RADIO  --

static int luaMavsdkGetRadioRssi(lua_State *L)
{
  uint8_t rssi = UINT8_MAX;
  if (mavlinkTelem.radio.is_receiving) {
    rssi = mavlinkTelem.radio.rssi;
  }
  else if (mavlinkTelem.radio.is_receiving65) {
    rssi = mavlinkTelem.radio.rssi65;
  }
  else if (mavlinkTelem.radio.is_receiving35) {
    rssi = mavlinkTelem.radio.rssi35;
  }
  if (rssi != UINT8_MAX) {
    lua_pushinteger(L, rssi);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetRadioRssiScaled(lua_State *L)
{
  if (mavlinkTelem.radio.is_receiving || mavlinkTelem.radio.is_receiving65 || mavlinkTelem.radio.is_receiving35) {
    lua_pushinteger(L, mavlinkTelem.radio.rssi_scaled);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetRadioRemoteRssi(lua_State *L)
{
  if (mavlinkTelem.radio.is_receiving) {
    lua_pushinteger(L, mavlinkTelem.radio.remrssi);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetRadioNoise(lua_State *L)
{
  if (mavlinkTelem.radio.is_receiving) {
    lua_pushinteger(L, mavlinkTelem.radio.noise);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetRadioRemoteNoise(lua_State *L)
{
  if (mavlinkTelem.radio.is_receiving) {
    lua_pushinteger(L, mavlinkTelem.radio.remnoise);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

// -- MAVSDK SYSTEM STATUS --

static int luaMavsdkGetSystemStatusSensors(lua_State *L)
{
  if (mavlinkTelem.sysstatus.received) {
    lua_newtable(L);
    lua_pushtablenumber(L, "present", mavlinkTelem.sysstatus.sensors_present);
    lua_pushtablenumber(L, "enabled", mavlinkTelem.sysstatus.sensors_enabled);
    lua_pushtablenumber(L, "health", mavlinkTelem.sysstatus.sensors_health);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

// -- MAVSDK ATTITUDE --

static int luaMavsdkGetAttRollDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.att.roll_rad * FRADTODEG);
  return 1;
}

static int luaMavsdkGetAttPitchDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.att.pitch_rad * FRADTODEG);
  return 1;
}

static int luaMavsdkGetAttYawDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.att.yaw_rad * FRADTODEG);
  return 1;
}

// -- MAVSDK GPS --

static int luaMavsdkGetGps1Status(lua_State *L)
{
/* what method is better ??
  lua_createtable(L, 0, 4);
  lua_pushtablenumber(L, "fix", mavlinkTelem.gps_fix);
  lua_pushtablenumber(L, "hdop", mavlinkTelem.gps_hdop * 0.01);
  lua_pushtablenumber(L, "vdop", mavlinkTelem.gps_vdop * 0.01);
  lua_pushtablenumber(L, "sat", mavlinkTelem.gps_sat);
*/
  lua_newtable(L);
  lua_pushtablenumber(L, "fix", mavlinkTelem.gps1.fix);
  lua_pushtablenumber(L, "hdop", mavlinkTelem.gps1.hdop * 0.01f);
  lua_pushtablenumber(L, "vdop", mavlinkTelem.gps1.vdop * 0.01f);
  lua_pushtablenumber(L, "sat", mavlinkTelem.gps1.sat);
  return 1;
}

static int luaMavsdkGetGps1Fix(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps1.fix);
  return 1;
}

static int luaMavsdkGetGps1HDop(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps1.hdop * 0.01f);
  return 1;
}

static int luaMavsdkGetGps1VDop(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps1.vdop * 0.01f);
  return 1;
}

static int luaMavsdkGetGps1Sat(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps1.sat); // UINT8_MAX if not known, but we don't do a nil here
  return 1;
}

static int luaMavsdkGetGps1LatLonInt(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "lat", mavlinkTelem.gps1.lat);
  lua_pushtableinteger(L, "lon", mavlinkTelem.gps1.lon);
  return 1;
}

static int luaMavsdkGetGps1AltitudeMsl(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps1.alt_mm * 0.001f);
  return 1;
}

static int luaMavsdkGetGps1Speed(lua_State *L)
{
  if (mavlinkTelem.gps1.vel_cmps < UINT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.gps1.vel_cmps * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetGps1CourseOverGroundDeg(lua_State *L)
{
  if (mavlinkTelem.gps1.cog_cdeg < UINT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.gps1.cog_cdeg * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetGps2Status(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablenumber(L, "fix", mavlinkTelem.gps2.fix);
  lua_pushtablenumber(L, "hdop", mavlinkTelem.gps2.hdop * 0.01f);
  lua_pushtablenumber(L, "vdop", mavlinkTelem.gps2.vdop * 0.01f);
  lua_pushtablenumber(L, "sat", mavlinkTelem.gps2.sat);
  return 1;
}

static int luaMavsdkGetGps2Fix(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps2.fix);
  return 1;
}

static int luaMavsdkGetGps2HDop(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps2.hdop * 0.01f);
  return 1;
}

static int luaMavsdkGetGps2VDop(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps2.vdop * 0.01f);
  return 1;
}

static int luaMavsdkGetGps2Sat(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps2.sat);
  return 1;
}

static int luaMavsdkGetGps2LatLonInt(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "lat", mavlinkTelem.gps2.lat);
  lua_pushtableinteger(L, "lon", mavlinkTelem.gps2.lon);
  return 1;
}

static int luaMavsdkGetGps2AltitudeMsl(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gps2.alt_mm * 0.001f);
  return 1;
}

static int luaMavsdkGetGps2Speed(lua_State *L)
{
  if (mavlinkTelem.gps2.vel_cmps < UINT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.gps2.vel_cmps * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetGps2CourseOverGroundDeg(lua_State *L)
{
  if (mavlinkTelem.gps2.cog_cdeg < UINT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.gps2.cog_cdeg * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkIsGps1Available(lua_State *L)
{
  bool flag = mavlinkTelem.gps_instancemask & 0x01;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkIsGps2Available(lua_State *L)
{
  bool flag = mavlinkTelem.gps_instancemask & 0x02;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGetGpsCount(lua_State *L)
{
  uint16_t cnt = 0, mask = mavlinkTelem.gps_instancemask;
  for (uint8_t i = 0; i < 8; i++) { 
    if (mask & 0x01) cnt++; 
    mask >>= 1; 
  }
  lua_pushinteger(L, cnt);
  return 1;
}

// -- MAVSDK POSITION --

static int luaMavsdkGetPositionLatLonInt(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "lat", mavlinkTelem.gposition.lat);
  lua_pushtableinteger(L, "lon", mavlinkTelem.gposition.lon);
  return 1;
}

static int luaMavsdkGetPositionAltitudeMsl(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gposition.alt_mm * 0.001f);
  return 1;
}

static int luaMavsdkGetPositionAltitudeRelative(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gposition.relative_alt_mm * 0.001f);
  return 1;
}

static int luaMavsdkGetPositionHeadingDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.gposition.hdg_cdeg * 0.01f);
  return 1;
}

static int luaMavsdkGetPositionSpeedNed(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablenumber(L, "vx", mavlinkTelem.gposition.vx_cmps * 0.01f);
  lua_pushtablenumber(L, "vy", mavlinkTelem.gposition.vy_cmps * 0.01f);
  lua_pushtablenumber(L, "vz", mavlinkTelem.gposition.vz_cmps * 0.01f);
  return 1;
}

// -- MAVSDK VFR --

static int luaMavsdkGetVfrAirSpeed(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.vfr.airspd_mps);
  return 1;
}

static int luaMavsdkGetVfrGroundSpeed(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.vfr.groundspd_mps);
  return 1;
}

static int luaMavsdkGetVfrAltitudeMsl(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.vfr.alt_m);
  return 1;
}

static int luaMavsdkGetVfrClimbRate(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.vfr.climbrate_mps);
  return 1;
}

static int luaMavsdkGetVfrHeadingDeg(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.vfr.heading_deg);
  return 1;
}

static int luaMavsdkGetVfrThrottle(lua_State *L)
{
  lua_pushinteger(L, mavlinkTelem.vfr.thro_pct);
  return 1;
}

// -- MAVSDK BATTERY --

static int luaMavsdkGetBat1ChargeConsumed(lua_State *L)
{
  if (mavlinkTelem.bat1.charge_consumed_mAh != -1) {
    lua_pushnumber(L, mavlinkTelem.bat1.charge_consumed_mAh);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1EnergyConsumed(lua_State *L)
{
  if (mavlinkTelem.bat1.energy_consumed_hJ != -1) {
    lua_pushnumber(L, mavlinkTelem.bat1.energy_consumed_hJ * 100.0f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1Temperature(lua_State *L)
{
  if (mavlinkTelem.bat1.temperature_cC < INT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.bat1.temperature_cC * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1Voltage(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.bat1.voltage_mV * 0.001f);
  return 1;
}

static int luaMavsdkGetBat1Current(lua_State *L)
{
  if (mavlinkTelem.bat1.current_cA != -1) {
    lua_pushnumber(L, mavlinkTelem.bat1.current_cA * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1Remaining(lua_State *L)
{
  if (mavlinkTelem.bat1.remaining_pct != -1) {
    lua_pushinteger(L, mavlinkTelem.bat1.remaining_pct);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1TimeRemaining(lua_State *L)
{
  if (mavlinkTelem.bat1.time_remaining != 0) {
    lua_pushinteger(L, mavlinkTelem.bat1.time_remaining);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1ChargeState(lua_State *L)
{
  if (mavlinkTelem.bat1.charge_state != MAV_BATTERY_CHARGE_STATE_UNDEFINED) {
    lua_pushinteger(L, mavlinkTelem.bat1.charge_state);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1FaultBitMask(lua_State *L)
{
  if (mavlinkTelem.bat1.charge_state == MAV_BATTERY_CHARGE_STATE_FAILED ||
      mavlinkTelem.bat1.charge_state == MAV_BATTERY_CHARGE_STATE_UNHEALTHY) {
    lua_pushinteger(L, mavlinkTelem.bat1.fault_bitmask);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat1CellCount(lua_State *L)
{
  if (mavlinkTelem.bat1.cellcount < 0) { 
    lua_pushnil(L); 
  } 
  else { 
    lua_pushinteger(L, mavlinkTelem.bat1.cellcount); 
  }
  return 1;
}

static int luaMavsdkGetBat2ChargeConsumed(lua_State *L)
{
  if (mavlinkTelem.bat2.charge_consumed_mAh != -1) {
    lua_pushnumber(L, mavlinkTelem.bat2.charge_consumed_mAh);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2EnergyConsumed(lua_State *L)
{
  if (mavlinkTelem.bat2.energy_consumed_hJ != -1) {
    lua_pushnumber(L, mavlinkTelem.bat2.energy_consumed_hJ * 100.0f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2Temperature(lua_State *L)
{
  if (mavlinkTelem.bat2.temperature_cC < INT16_MAX) {
    lua_pushnumber(L, mavlinkTelem.bat2.temperature_cC * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2Voltage(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.bat2.voltage_mV * 0.001f);
  return 1;
}

static int luaMavsdkGetBat2Current(lua_State *L)
{
  if (mavlinkTelem.bat2.current_cA != -1) {
    lua_pushnumber(L, mavlinkTelem.bat2.current_cA * 0.01f);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2Remaining(lua_State *L)
{
  if (mavlinkTelem.bat2.remaining_pct != -1) {
    lua_pushinteger(L, mavlinkTelem.bat2.remaining_pct);
  } 
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2TimeRemaining(lua_State *L)
{
  if (mavlinkTelem.bat2.time_remaining != 0) {
    lua_pushinteger(L, mavlinkTelem.bat2.time_remaining);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2ChargeState(lua_State *L)
{
  if (mavlinkTelem.bat2.charge_state != MAV_BATTERY_CHARGE_STATE_UNDEFINED) {
    lua_pushinteger(L, mavlinkTelem.bat2.charge_state);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2FaultBitMask(lua_State *L)
{
  if (mavlinkTelem.bat2.charge_state == MAV_BATTERY_CHARGE_STATE_FAILED ||
      mavlinkTelem.bat2.charge_state == MAV_BATTERY_CHARGE_STATE_UNHEALTHY) {
    lua_pushinteger(L, mavlinkTelem.bat2.fault_bitmask);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaMavsdkGetBat2CellCount(lua_State *L)
{
  if (mavlinkTelem.bat2.cellcount < 0) { 
    lua_pushnil(L); 
  } 
  else { 
    lua_pushinteger(L, mavlinkTelem.bat2.cellcount); 
  }
  return 1;
}

static int luaMavsdkIsBat1Available(lua_State *L)
{
  bool flag = mavlinkTelem.bat_instancemask & 0x01;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkIsBat2Available(lua_State *L)
{
  bool flag = mavlinkTelem.bat_instancemask & 0x02;
  lua_pushboolean(L, flag);
  return 1;
}

static int luaMavsdkGetBatCount(lua_State *L)
{
  uint16_t cnt = 0, mask = mavlinkTelem.bat_instancemask;
  for (uint8_t i = 0; i < 8; i++) { 
    if (mask & 0x01) cnt++; 
    mask >>= 1; 
  }
  lua_pushinteger(L, cnt);
  return 1;
}

static int luaMavsdkGetBat1Capacity(lua_State *L)
{
  if (mavlinkTelem.param.BATT_CAPACITY < 0) { 
    lua_pushnil(L); 
  } 
  else { 
    lua_pushnumber(L, mavlinkTelem.param.BATT_CAPACITY); 
  }
  return 1;
}

static int luaMavsdkGetBat2Capacity(lua_State *L)
{
  if (mavlinkTelem.param.BATT2_CAPACITY < 0) { 
    lua_pushnil(L); 
  } 
  else { 
    lua_pushnumber(L, mavlinkTelem.param.BATT2_CAPACITY); 
  }
  return 1;
}

// -- MAVSDK ARDUPILOT --

static int luaMavsdkApIsFlying(lua_State *L)
{
  lua_pushboolean(L, !mavlinkTelem.autopilot.is_standby);
  return 1;
}

static int luaMavsdkApIsFailsafe(lua_State *L)
{
  lua_pushboolean(L, mavlinkTelem.autopilot.is_critical);
  return 1;
}

static int luaMavsdkApPositionOk(lua_State *L)
{
  lua_pushboolean(L, mavlinkTelem.apPositionOk());
  return 1;
}

static int luaMavsdkApSetFlightMode(lua_State *L)
{
  int32_t ap_flight_mode = luaL_checkinteger(L, 1);
  mavlinkTelem.apSetFlightMode(ap_flight_mode);
  return 0;
}

static int luaMavsdkApRequestBanner(lua_State *L)
{
  mavlinkTelem.apRequestBanner();
  return 0;
}

static int luaMavsdkApArm(lua_State *L)
{
  int32_t arm = luaL_checkinteger(L, 1);
  mavlinkTelem.apArm(arm > 0);
  return 0;
}

static int luaMavsdkApCopterTakeOff(lua_State *L)
{
  float alt = luaL_checknumber(L, 1);
  mavlinkTelem.apCopterTakeOff(alt);
  return 0;
}

static int luaMavsdkApLand(lua_State *L)
{
  mavlinkTelem.apLand();
  return 0;
}

static int luaMavsdkApSetGroundSpeed(lua_State *L)
{
  float speed = luaL_checknumber(L, 1);
  mavlinkTelem.apSetGroundSpeed(speed);
  return 0;
}

static int luaMavsdkApSimpleGotoPosIntAltRel(lua_State *L)
{
  int32_t lat = luaL_checkinteger(L, 1);
  int32_t lon = luaL_checkinteger(L, 2);
  float alt = luaL_checknumber(L, 3);
  mavlinkTelem.apSimpleGotoPosAlt(lat, lon, alt);
  return 0;
}

static int luaMavsdkApGotoPosIntAltRel(lua_State *L)
{
  int32_t lat = luaL_checkinteger(L, 1);
  int32_t lon = luaL_checkinteger(L, 2);
  float alt = luaL_checknumber(L, 3);
  mavlinkTelem.apGotoPosAltYawDeg(lat, lon, alt, NAN);
  return 0;
}

static int luaMavsdkApGotoPosIntAltRelYawDeg(lua_State *L)
{
  int32_t lat = luaL_checkinteger(L, 1);
  int32_t lon = luaL_checkinteger(L, 2);
  float alt = luaL_checknumber(L, 3);
  float yaw = luaL_checknumber(L, 4);
  mavlinkTelem.apGotoPosAltYawDeg(lat, lon, alt, yaw);
  return 0;
}

static int luaMavsdkApGotoPosIntAltRelVel(lua_State *L)
{
  int32_t lat = luaL_checkinteger(L, 1);
  int32_t lon = luaL_checkinteger(L, 2);
  float alt = luaL_checknumber(L, 3);
  float vx = luaL_checknumber(L, 4);
  float vy = luaL_checknumber(L, 5);
  float vz = luaL_checknumber(L, 6);
  mavlinkTelem.apGotoPosAltVel(lat, lon, alt, vx, vy, vz);
  return 0;
}

static int luaMavsdkApSetYawDeg(lua_State *L)
{
  float yaw = luaL_checknumber(L, 1);
  int32_t relative = luaL_optunsigned(L, 2, 0);
  mavlinkTelem.apSetYawDeg(yaw, (relative) ? true : false);
  return 0;
}

static int luaMavsdkApCopterFlyClick(lua_State *L)
{
  mavlinkTelem.apCopterFlyClick();
  return 0;
}

static int luaMavsdkApCopterFlyHold(lua_State *L)
{
  float alt = luaL_checknumber(L, 1);
  mavlinkTelem.apCopterFlyHold(alt);
  return 0;
}

static int luaMavsdkApCopterFlyPause(lua_State *L)
{
  mavlinkTelem.apCopterFlyPause();
  return 0;
}

static int luaMavsdkApGetRangefinder(lua_State *L)
{
  lua_pushnumber(L, mavlinkTelem.rangefinder.distance);
  return 1;
}

static int luaMavsdkApGetArmingCheck(lua_State *L)
{
  if (mavlinkTelem.param.ARMING_CHECK < 0) {
    lua_pushnil(L);
  }
  else {
    lua_pushnumber(L, mavlinkTelem.param.ARMING_CHECK);
  }
  return 1;
}


// -- MAVSDK STATUSTEXT --

static int luaMavsdkIsStatusTextAvailable(lua_State *L)
{
  lua_pushboolean(L, !mavlinkTelem.statustext.fifo.isEmpty());
  return 1;
}

static int luaMavsdkGetStatusText(lua_State *L)
{
  char text[FASTMAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN+1]; //mavlink string is not necessarily \0 terminated
  fmav_statustext_t payload;
  if (!mavlinkTelem.statustext.fifo.pop(payload)) { //should not happen, use isStatusTextAvailable() to check beforehand
    text[0] = '\0';
    payload.severity = MAV_SEVERITY_INFO;
    lua_pushnil(L);
    lua_pushnil(L);
  }
  else {
    memcpy(text, payload.text, FASTMAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN);
    text[FASTMAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN] = '\0';
    lua_pushinteger(L, payload.severity);
    lua_pushstring(L, text);
  }
  return 2;
}

// -- MAVSDK MISSION --

static int luaMavsdkGetNavControllerOutput(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablenumber(L, "nav_bearing", (float)mavlinkTelem.navControllerOutput.nav_bearing);
  lua_pushtablenumber(L, "target_bearing", (float)mavlinkTelem.navControllerOutput.target_bearing);
  lua_pushtablenumber(L, "wp_dist", (float)mavlinkTelem.navControllerOutput.wp_dist);
  return 1;
}

static int luaMavsdkGetMission(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "count", mavlinkTelem.mission.count);
  lua_pushtableinteger(L, "current_seq", mavlinkTelem.mission.seq_current);
  return 1;
}

static int luaMavsdkGetMissionItem(lua_State *L)
{
  lua_newtable(L);
  lua_pushtableinteger(L, "seq", mavlinkTelem.missionItem.seq);
  lua_pushtableinteger(L, "command", mavlinkTelem.missionItem.command);
  lua_pushtableinteger(L, "frame", mavlinkTelem.missionItem.frame);
  bool is_global = false;
  switch (mavlinkTelem.missionItem.frame) {
    case MAV_FRAME_GLOBAL_INT:
    case MAV_FRAME_GLOBAL_RELATIVE_ALT_INT:
    case MAV_FRAME_GLOBAL_TERRAIN_ALT_INT:
    //these should not occur, but play it safe and handle them anyway
    case MAV_FRAME_GLOBAL:
    case MAV_FRAME_GLOBAL_RELATIVE_ALT:
    case MAV_FRAME_GLOBAL_TERRAIN_ALT:
      is_global = true;
      break;
  }
  lua_pushtableboolean(L, "is_global", is_global);
  if (is_global) {
    lua_pushtableinteger(L, "lat", mavlinkTelem.missionItem.x);
    lua_pushtableinteger(L, "lon", mavlinkTelem.missionItem.y);
    lua_pushtablenumber(L, "alt", mavlinkTelem.missionItem.z);
  }
  else {
    lua_pushtablenumber(L, "x", mavlinkTelem.missionItem.x * 0.0001f);
    lua_pushtablenumber(L, "y", mavlinkTelem.missionItem.y * 0.0001f);
    lua_pushtablenumber(L, "z", mavlinkTelem.missionItem.z);
  }
  return 1;
}

// -- Fake RSSI --

static int luaMavsdkOptionIsRssiEnabled(lua_State *L)
{
  lua_pushboolean(L, g_model.mavlinkRssi);
  return 1;
}

static int luaMavsdkOptionEnableRssi(lua_State *L)
{
  bool enable = (luaL_checkinteger(L, 1) > 0);
  g_model.mavlinkRssi = (enable) ? 1 : 0;
  return 0;
}

static int luaMavsdkOptionGetRssiScale(lua_State *L)
{
  lua_pushinteger(L, g_model.mavlinkRssiScale);
  return 1;
}

static int luaMavsdkOptionSetRssiScale(lua_State *L)
{
  int32_t scale = luaL_checkinteger(L, 1);
  if (scale < 0) scale = 0;
  if (scale > 255) scale = 255;
  g_model.mavlinkRssiScale = scale;
  return 0;
}

static int luaMavsdkRadioDisableRssiVoice(lua_State *L)
{
  bool disable = (luaL_checkinteger(L, 1) > 0);
  mavlinkTelem.radio.rssi_voice_disabled = disable;
  return 0;
}


//------------------------------------------------------------
// mavsdk luaL and luaR arrays
//------------------------------------------------------------
// I believe the names can't be longer than 32 chars

const luaL_Reg mavsdkLib[] = {
  { "mavtelemIsEnabled", luaMavsdkMavTelemIsEnabled },
  { "getVersion", luaMavsdkMavTelemVersion },

  { "isReceiving", luaMavsdkIsReceiving },
  { "isInitialized", luaMavsdkIsInitialized },
  { "getAutopilotType", luaMavsdkGetAutopilotType },
  { "getVehicleType", luaMavsdkGetVehicleType },
  { "getFlightMode", luaMavsdkGetFlightMode },
  { "getVehicleClass", luaMavsdkGetVehicleClass },
  { "getSystemStatus", luaMavsdkGetSystemStatus },
  { "isArmed", luaMavsdkIsArmed },

  { "gimbalIsReceiving", luaMavsdkGimbalIsReceiving },
  { "gimbalIsInitialized", luaMavsdkGimbalIsInitialized },
  { "gimbalGetInfo", luaMavsdkGimbalGetInfo },
  { "gimbalGetStatus", luaMavsdkGimbalGetStatus },
  { "gimbalGetAttRollDeg", luaMavsdkGimbalGetAttRollDeg },
  { "gimbalGetAttPitchDeg", luaMavsdkGimbalGetAttPitchDeg },
  { "gimbalGetAttYawDeg", luaMavsdkGimbalGetAttYawDeg },
  // gimbal protocol v1
  { "gimbalSendNeutralMode", luaMavsdkGimbalSendNeutralMode },
  { "gimbalSendMavlinkTargetingMode", luaMavsdkGimbalSendMavlinkTargetingMode },
  { "gimbalSendRcTargetingMode", luaMavsdkGimbalSendRcTargetingMode },
  { "gimbalSendGpsPointMode", luaMavsdkGimbalSendGpsPointMode },
  { "gimbalSendSysIdTargetingMode", luaMavsdkGimbalSendSysIdTargetingMode },
  { "gimbalSendPitchYawDeg", luaMavsdkGimbalSendPitchYawDeg },
  // gimbal protocol v2
  { "gimbalIsProtocolV2", luaMavsdkIsGimbalProtocolV2 },
  { "gimbalSetProtocolV2", luaMavsdkSetGimbalProtocolV2 },
  { "gimbalClientIsReceiving", luaMavsdkGimbalClientIsReceiving },
  { "gimbalClientIsInitialized", luaMavsdkGimbalClientIsInitialized },
  { "gimbalClientGetInfo", luaMavsdkGimbalClientGetInfo },
  { "gimbalClientGetStatus", luaMavsdkGimbalClientGetStatus },
  { "gimbalClientSetRetract", luaMavsdkGimbalClientSetRetract },
  { "gimbalClientSetNeutral", luaMavsdkGimbalClientSetNeutral },
  { "gimbalClientSetLock", luaMavsdkGimbalClientSetLock },
  { "gimbalClientSetFlags", luaMavsdkGimbalClientSetFlags },
  { "gimbalClientSendPitchYawDeg", luaMavsdkGimbalClientSendPitchYawDeg },
  //for testing only
  { "gimbalClientSendControlPitchYawDeg", luaMavsdkGimbalClientSendControlPitchYawDeg },
  { "gimbalClientSendCmdPitchYawDeg", luaMavsdkGimbalClientSendCmdPitchYawDeg },
  { "gimbalDeviceSendPitchYawDeg", luaMavsdkGimbalDeviceSendPitchYawDeg },

  { "qshotSendCmdConfigure", luaMavsdkQShotSendCmdConfigure },
  { "qshotSendStatus", luaMavsdkQShotSendStatus },
  { "qshotGetStatus", luaMavsdkQShotGetStatus },
  { "qshotButtonState", luaMavsdkQShotButtonState },

  { "cameraIsReceiving", luaMavsdkCameraIsReceiving },
  { "cameraIsInitialized", luaMavsdkCameraIsInitialized },
  { "cameraGetInfo", luaMavsdkCameraGetInfo },
  { "cameraGetStatus", luaMavsdkCameraGetStatus },
  { "cameraSendVideoMode", luaMavsdkCameraSendVideoMode },
  { "cameraSendPhotoMode", luaMavsdkCameraSendPhotoMode },
  { "cameraStartVideo", luaMavsdkCameraStartVideo },
  { "cameraStopVideo", luaMavsdkCameraStopVideo },
  { "cameraTakePhoto", luaMavsdkCameraTakePhoto },

  { "getRadioRssi", luaMavsdkGetRadioRssi },
  { "getRadioRemoteRssi", luaMavsdkGetRadioRemoteRssi },
  { "getRadioNoise", luaMavsdkGetRadioNoise },
  { "getRadioRemoteNoise", luaMavsdkGetRadioRemoteNoise },
  { "getRadioRssiScaled", luaMavsdkGetRadioRssiScaled },

  { "getSystemStatusSensors", luaMavsdkGetSystemStatusSensors },

  { "getAttRollDeg", luaMavsdkGetAttRollDeg },
  { "getAttPitchDeg", luaMavsdkGetAttPitchDeg },
  { "getAttYawDeg", luaMavsdkGetAttYawDeg },

  { "getGpsStatus", luaMavsdkGetGps1Status },
  { "getGpsFix", luaMavsdkGetGps1Fix },
  { "getGpsHDop", luaMavsdkGetGps1HDop },
  { "getGpsVDop", luaMavsdkGetGps1VDop },
  { "getGpsSat", luaMavsdkGetGps1Sat },
  { "getGpsLatLonInt", luaMavsdkGetGps1LatLonInt },
  { "getGpsAltitudeMsl", luaMavsdkGetGps1AltitudeMsl },
  { "getGpsSpeed", luaMavsdkGetGps1Speed },
  { "getGpsCourseOverGroundDeg", luaMavsdkGetGps1CourseOverGroundDeg },

  { "getGps2Status", luaMavsdkGetGps2Status },
  { "getGps2Fix", luaMavsdkGetGps2Fix },
  { "getGps2HDop", luaMavsdkGetGps2HDop },
  { "getGps2VDop", luaMavsdkGetGps2VDop },
  { "getGps2Sat", luaMavsdkGetGps2Sat },
  { "getGps2LatLonInt", luaMavsdkGetGps2LatLonInt },
  { "getGps2AltitudeMsl", luaMavsdkGetGps2AltitudeMsl },
  { "getGps2Speed", luaMavsdkGetGps2Speed },
  { "getGps2CourseOverGroundDeg", luaMavsdkGetGps2CourseOverGroundDeg },

  { "isGpsAvailable", luaMavsdkIsGps1Available },
  { "isGps2Available", luaMavsdkIsGps2Available },
  { "getGpsCount", luaMavsdkGetGpsCount },

  { "getPositionLatLonInt", luaMavsdkGetPositionLatLonInt },
  { "getPositionAltitudeMsl", luaMavsdkGetPositionAltitudeMsl },
  { "getPositionAltitudeRelative", luaMavsdkGetPositionAltitudeRelative },
  { "getPositionHeadingDeg", luaMavsdkGetPositionHeadingDeg },
  { "getPositionSpeedNed", luaMavsdkGetPositionSpeedNed },

  { "getVfrAirSpeed", luaMavsdkGetVfrAirSpeed },
  { "getVfrGroundSpeed", luaMavsdkGetVfrGroundSpeed },
  { "getVfrAltitudeMsl", luaMavsdkGetVfrAltitudeMsl },
  { "getVfrClimbRate", luaMavsdkGetVfrClimbRate },
  { "getVfrHeadingDeg", luaMavsdkGetVfrHeadingDeg },
  { "getVfrThrottle", luaMavsdkGetVfrThrottle },

  { "getBatChargeConsumed", luaMavsdkGetBat1ChargeConsumed },
  { "getBatEnergyConsumed", luaMavsdkGetBat1EnergyConsumed },
  { "getBatTemperature", luaMavsdkGetBat1Temperature },
  { "getBatVoltage", luaMavsdkGetBat1Voltage },
  { "getBatCurrent", luaMavsdkGetBat1Current },
  { "getBatRemaining", luaMavsdkGetBat1Remaining },
  { "getBatTimeRemaining", luaMavsdkGetBat1TimeRemaining },
  { "getBatChargeState", luaMavsdkGetBat1ChargeState },
  { "getBatFaultBitMask", luaMavsdkGetBat1FaultBitMask },
  { "getBatCellCount", luaMavsdkGetBat1CellCount },

  { "getBat2ChargeConsumed", luaMavsdkGetBat2ChargeConsumed },
  { "getBat2EnergyConsumed", luaMavsdkGetBat2EnergyConsumed },
  { "getBat2Temperature", luaMavsdkGetBat2Temperature },
  { "getBat2Voltage", luaMavsdkGetBat2Voltage },
  { "getBat2Current", luaMavsdkGetBat2Current },
  { "getBat2Remaining", luaMavsdkGetBat2Remaining },
  { "getBat2TimeRemaining", luaMavsdkGetBat2TimeRemaining },
  { "getBat2ChargeState", luaMavsdkGetBat2ChargeState },
  { "getBat2FaultBitMask", luaMavsdkGetBat2FaultBitMask },
  { "getBat2CellCount", luaMavsdkGetBat2CellCount },

  { "isBatAvailable", luaMavsdkIsBat1Available },
  { "isBat2Available", luaMavsdkIsBat2Available },
  { "getBatCount", luaMavsdkGetBatCount },

  { "getBatCapacity", luaMavsdkGetBat1Capacity },
  { "getBat2Capacity", luaMavsdkGetBat2Capacity },

  { "isStatusTextAvailable", luaMavsdkIsStatusTextAvailable },
  { "getStatusText", luaMavsdkGetStatusText },

  { "getNavController", luaMavsdkGetNavControllerOutput },
  { "getMission", luaMavsdkGetMission },
  { "getMissionItem", luaMavsdkGetMissionItem },

  { "apIsFlying", luaMavsdkApIsFlying },
  { "apIsFailsafe", luaMavsdkApIsFailsafe },
  { "apPositionOk", luaMavsdkApPositionOk },
  { "apSetFlightMode", luaMavsdkApSetFlightMode },
  { "apRequestBanner", luaMavsdkApRequestBanner },
  { "apArm", luaMavsdkApArm },
  { "apCopterTakeOff", luaMavsdkApCopterTakeOff },
  { "apLand", luaMavsdkApLand },
  { "apSetGroundSpeed", luaMavsdkApSetGroundSpeed },
  { "apSimpleGotoPosIntAltRel", luaMavsdkApSimpleGotoPosIntAltRel },
  { "apGotoPosIntAltRel", luaMavsdkApGotoPosIntAltRel },
  { "apGotoPosIntAltRelYawDeg", luaMavsdkApGotoPosIntAltRelYawDeg },
  { "apGotoPosIntAltRelVel", luaMavsdkApGotoPosIntAltRelVel },
  { "apSetYawDeg", luaMavsdkApSetYawDeg },
  { "apCopterFlyClick", luaMavsdkApCopterFlyClick },
  { "apCopterFlyHold", luaMavsdkApCopterFlyHold },
  { "apCopterFlyPause", luaMavsdkApCopterFlyPause },
  { "apGetRangefinder", luaMavsdkApGetRangefinder },
  { "apGetArmingCheck", luaMavsdkApGetArmingCheck },

  { "optionIsRssiEnabled", luaMavsdkOptionIsRssiEnabled },
  { "optionEnableRssi", luaMavsdkOptionEnableRssi },
  { "optionGetRssiScale", luaMavsdkOptionGetRssiScale },
  { "optionSetRssiScale", luaMavsdkOptionSetRssiScale },
  { "radioDisableRssiVoice", luaMavsdkRadioDisableRssiVoice },

  { NULL, NULL }  /* sentinel */
};

const luaR_value_entry mavsdkConstants[] = {
  { "VEHICLECLASS_GENERIC", MAVSDK_VEHICLECLASS_GENERIC },
  { "VEHICLECLASS_PLANE", MAVSDK_VEHICLECLASS_PLANE },
  { "VEHICLECLASS_COPTER", MAVSDK_VEHICLECLASS_COPTER },
  { "VEHICLECLASS_ROVER", MAVSDK_VEHICLECLASS_ROVER },
  { "VEHICLECLASS_BOAT", MAVSDK_VEHICLECLASS_BOAT },
  { "VEHICLECLASS_SUB", MAVSDK_VEHICLECLASS_SUB },

  { "GDFLAGS_RETRACT", MAV_STORM32_GIMBAL_DEVICE_FLAGS_RETRACT },
  { "GDFLAGS_NEUTRAL", MAV_STORM32_GIMBAL_DEVICE_FLAGS_NEUTRAL },
  { "GDFLAGS_ROLL_LOCK", MAV_STORM32_GIMBAL_DEVICE_FLAGS_ROLL_LOCK },
  { "GDFLAGS_PITCH_LOCK", MAV_STORM32_GIMBAL_DEVICE_FLAGS_PITCH_LOCK },
  { "GDFLAGS_YAW_LOCK", MAV_STORM32_GIMBAL_DEVICE_FLAGS_YAW_LOCK },
  { "GDFLAGS_CAN_ACCEPT_YAW_ABSOLUTE", MAV_STORM32_GIMBAL_DEVICE_FLAGS_CAN_ACCEPT_YAW_ABSOLUTE },
  { "GDFLAGS_YAW_ABSOLUTE", MAV_STORM32_GIMBAL_DEVICE_FLAGS_YAW_ABSOLUTE },
  { "GDFLAGS_RC_EXCLUSIVE", MAV_STORM32_GIMBAL_DEVICE_FLAGS_RC_EXCLUSIVE },
  { "GDFLAGS_RC_MIXED", MAV_STORM32_GIMBAL_DEVICE_FLAGS_RC_MIXED },

  { "GMFLAGS_NONE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_NONE },
  { "GMFLAGS_RC_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_RC_ACTIVE },
  { "GMFLAGS_ONBOARD_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_ONBOARD_ACTIVE },
  { "GMFLAGS_AUTOPILOT_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_AUTOPILOT_ACTIVE },
  { "GMFLAGS_GCS_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_GCS_ACTIVE },
  { "GMFLAGS_CAMERA_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_CAMERA_ACTIVE },
  { "GMFLAGS_GCS2_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_GCS2_ACTIVE },
  { "GMFLAGS_CAMERA2_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_CAMERA2_ACTIVE },
  { "GMFLAGS_CLIENT_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_CUSTOM_ACTIVE },
  { "GMFLAGS_CLIENT2_ACTIVE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_CLIENT_CUSTOM2_ACTIVE },
  { "GMFLAGS_SET_SUPERVISON", MAV_STORM32_GIMBAL_MANAGER_FLAGS_SET_SUPERVISON },
  { "GMFLAGS_SET_RELEASE", MAV_STORM32_GIMBAL_MANAGER_FLAGS_SET_RELEASE },

  { "GMCLIENT_ONBOARD", MAV_STORM32_GIMBAL_MANAGER_CLIENT_ONBOARD },
  { "GMCLIENT_AUTOPILOT", MAV_STORM32_GIMBAL_MANAGER_CLIENT_AUTOPILOT },
  { "GMCLIENT_GCS", MAV_STORM32_GIMBAL_MANAGER_CLIENT_GCS },
  { "GMCLIENT_CAMERA", MAV_STORM32_GIMBAL_MANAGER_CLIENT_CAMERA },
  { "GMCLIENT_GCS2", MAV_STORM32_GIMBAL_MANAGER_CLIENT_GCS2 },
  { "GMCLIENT_CAMERA2", MAV_STORM32_GIMBAL_MANAGER_CLIENT_CAMERA2 },
  { "GMCLIENT_CUSTOM", MAV_STORM32_GIMBAL_MANAGER_CLIENT_CUSTOM },
  { "GMCLIENT_CUSTOM2", MAV_STORM32_GIMBAL_MANAGER_CLIENT_CUSTOM2 },

  { "QSHOT_MODE_UNDEFINED", MAV_QSHOT_MODE_UNDEFINED },
  { "QSHOT_MODE_DEFAULT", MAV_QSHOT_MODE_DEFAULT },
  { "QSHOT_MODE_RETRACT", MAV_QSHOT_MODE_GIMBAL_RETRACT },
  { "QSHOT_MODE_NEUTRAL", MAV_QSHOT_MODE_GIMBAL_NEUTRAL },
  { "QSHOT_MODE_RC_CONTROL", MAV_QSHOT_MODE_GIMBAL_RC_CONTROL },
  { "QSHOT_MODE_POI", MAV_QSHOT_MODE_POI_TARGETING },
  { "QSHOT_MODE_SYSID", MAV_QSHOT_MODE_SYSID_TARGETING },
  { "QSHOT_MODE_CABLECAM", MAV_QSHOT_MODE_CABLECAM_2POINT },

  { nullptr, 0 }  /* sentinel */
};

