/*
 * (c) www.olliw.eu, OlliW, OlliW42
 */

#include "opentx.h"

constexpr float FPI = 3.141592653589793f;
constexpr float FDEGTORAD = FPI/180.0f;
constexpr float FRADTODEG = 180.0f/FPI;

#define INCU8(x)    if ((x) < UINT8_MAX) { (x)++; }

// -- Generate MAVLink messages --
// these should never be called directly, should only by called by the task handler

void MavlinkTelem::generateCmdRequestCameraInformation(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_REQUEST_CAMERA_INFORMATION, 1, 0,0,0,0,0,0);
}

void MavlinkTelem::generateCmdRequestCameraSettings(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_REQUEST_CAMERA_SETTINGS, 1, 0,0,0,0,0,0);
}

void MavlinkTelem::generateCmdRequestStorageInformation(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_REQUEST_STORAGE_INFORMATION, 0, 1, 0,0,0,0,0);
}

void MavlinkTelem::generateCmdRequestCameraCapturesStatus(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS, 1, 0,0,0,0,0,0);
}

void MavlinkTelem::generateCmdSetCameraMode(uint8_t tsystem, uint8_t tcomponent, uint8_t mode)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_SET_CAMERA_MODE, 0, mode, 0,0,0,0,0);
}

void MavlinkTelem::generateCmdImageStartCapture(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_IMAGE_START_CAPTURE, 0, 0, 1, 0, 0,0,0);
}

void MavlinkTelem::generateCmdVideoStartCapture(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_VIDEO_START_CAPTURE, 0, 0.2f, 0,0,0,0,0);
}

void MavlinkTelem::generateCmdVideoStopCapture(uint8_t tsystem, uint8_t tcomponent)
{
  _generateCmdLong(tsystem, tcomponent, MAV_CMD_VIDEO_STOP_CAPTURE, 0,0,0,0,0,0,0);
}

// -- Mavsdk Convenience Task Wrapper --
// to make it easy for api_mavsdk to call functions
// are short here and defined in mavlink_telem.h

// -- Task handlers --

bool MavlinkTelem::doTaskCamera(void)
{
  if (!_task[TASK_CAMERA]) return false; // no task pending

  if (!camera.compid) { _task[TASK_CAMERA] = 0; return false; }

  if (_task[TASK_CAMERA] & TASK_SENDCMD_IMAGE_START_CAPTURE) {
    RESETTASK(TASK_CAMERA, TASK_SENDCMD_IMAGE_START_CAPTURE);
    generateCmdImageStartCapture(_sysid, camera.compid);
    set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS, 2);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDCMD_VIDEO_START_CAPTURE) {
    RESETTASK(TASK_CAMERA, TASK_SENDCMD_VIDEO_START_CAPTURE);
    generateCmdVideoStartCapture(_sysid, camera.compid);
    set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS, 2);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDCMD_VIDEO_STOP_CAPTURE) {
    RESETTASK(TASK_CAMERA, TASK_SENDCMD_VIDEO_STOP_CAPTURE);
    generateCmdVideoStopCapture(_sysid, camera.compid);
    set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS, 2);
    return true; //do only one per loop
  }

  return false;
}

bool MavlinkTelem::doTaskCameraLowPriority(void)
{
  if (!_task[TASK_CAMERA]) return false; // no task pending

  if (_task[TASK_CAMERA] & TASK_SENDCMD_SET_CAMERA_VIDEO_MODE) {
    RESETTASK(TASK_CAMERA, TASK_SENDCMD_SET_CAMERA_VIDEO_MODE);
    generateCmdSetCameraMode(_sysid, camera.compid, CAMERA_MODE_VIDEO);
    set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_SETTINGS, 2);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDCMD_SET_CAMERA_PHOTO_MODE) {
    RESETTASK(TASK_CAMERA, TASK_SENDCMD_SET_CAMERA_PHOTO_MODE);
    generateCmdSetCameraMode(_sysid, camera.compid, CAMERA_MODE_IMAGE);
    set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_SETTINGS, 2);
    return true; //do only one per loop
  }

  // the sequence here defines the startup sequence
  if (_task[TASK_CAMERA] & TASK_SENDREQUEST_CAMERA_INFORMATION) {
    RESETTASK(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_INFORMATION);
    generateCmdRequestCameraInformation(_sysid, camera.compid);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDREQUEST_CAMERA_SETTINGS) {
    RESETTASK(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_SETTINGS);
    generateCmdRequestCameraSettings(_sysid, camera.compid);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS) {
    RESETTASK(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS);
    generateCmdRequestCameraCapturesStatus(_sysid, camera.compid);
    return true; //do only one per loop
  }
  if (_task[TASK_CAMERA] & TASK_SENDREQUEST_STORAGE_INFORMATION) {
    RESETTASK(TASK_CAMERA, TASK_SENDREQUEST_STORAGE_INFORMATION);
    generateCmdRequestStorageInformation(_sysid, camera.compid);
    return true; //do only one per loop
  }

  return false;
}

// -- Handle incoming MAVLink messages, which are for the Camera --

void MavlinkTelem::handleMessageCamera(void)
{
  camera.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT; //we accept any msg from the camera to indicate it is alive

  switch (_msg.msgid) {
    case FASTMAVLINK_MSG_ID_HEARTBEAT: {
      fmav_heartbeat_t payload;
      fmav_msg_heartbeat_decode(&payload, &_msg);
      camera.system_status = payload.system_status;
      camera.is_armed = (payload.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ? true : false;
      camera.is_standby = (payload.system_status <= MAV_STATE_STANDBY) ? true : false;
      camera.is_critical = (payload.system_status >= MAV_STATE_CRITICAL) ? true : false;
      INCU8(camera.updated);
      //camera.is_receiving = MAVLINK_TELEM_RECEIVING_TIMEOUT;
      break;
    }

    case FASTMAVLINK_MSG_ID_CAMERA_INFORMATION: {
      fmav_camera_information_t payload;
      fmav_msg_camera_information_decode(&payload, &_msg);
      memset(cameraInfo.vendor_name, 0, 32+1);
      memcpy(cameraInfo.vendor_name, payload.vendor_name, 32);
      memset(cameraInfo.model_name, 0, 32+1);
      memcpy(cameraInfo.model_name, payload.model_name, 32);
      cameraInfo.firmware_version = payload.firmware_version;
      cameraInfo.flags = payload.flags;
      cameraInfo.has_video = (cameraInfo.flags & CAMERA_CAP_FLAGS_CAPTURE_VIDEO);
      cameraInfo.has_photo = (cameraInfo.flags & CAMERA_CAP_FLAGS_CAPTURE_IMAGE);
      cameraInfo.has_modes = (cameraInfo.flags & CAMERA_CAP_FLAGS_HAS_MODES);
      clear_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_INFORMATION);
      camera.requests_waiting_mask &=~ CAMERA_REQUESTWAITING_CAMERA_INFORMATION;
      break;
    }

    case FASTMAVLINK_MSG_ID_CAMERA_SETTINGS: {
      fmav_camera_settings_t payload;
      fmav_msg_camera_settings_decode(&payload, &_msg);
      cameraStatus.mode = (payload.mode_id == CAMERA_MODE_IMAGE) ? CAMERA_MODE_IMAGE : CAMERA_MODE_VIDEO;
      clear_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_SETTINGS);
      camera.requests_waiting_mask &=~ CAMERA_REQUESTWAITING_CAMERA_SETTINGS;
      break;
    }

    case FASTMAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS: {
      fmav_camera_capture_status_t payload;
      fmav_msg_camera_capture_status_decode(&payload, &_msg);
      cameraStatus.recording_time_ms = payload.recording_time_ms;
      cameraStatus.available_capacity_MiB = payload.available_capacity;
      cameraStatus.video_on = (payload.video_status > 0);
      cameraStatus.photo_on = (payload.image_status > 0); //0: idle, 1: capture in progress, 2: interval set but idle, 3: interval set and capture in progress
      clear_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS);
      camera.requests_waiting_mask &=~ CAMERA_REQUESTWAITING_CAMERA_CAPTURE_STATUS;
      break;
    }

    case FASTMAVLINK_MSG_ID_STORAGE_INFORMATION: {
      fmav_storage_information_t payload;
      fmav_msg_storage_information_decode(&payload, &_msg);
      if (payload.status == STORAGE_STATUS_READY) {
        cameraInfo.total_capacity_MiB = payload.total_capacity;
        cameraStatus.available_capacity_MiB = payload.available_capacity;
      }
      else {
        cameraInfo.total_capacity_MiB = NAN;
        cameraStatus.available_capacity_MiB = NAN;
      }
      clear_request(TASK_CAMERA, TASK_SENDREQUEST_STORAGE_INFORMATION);
      break;
    }

    case FASTMAVLINK_MSG_ID_BATTERY_STATUS: {
      fmav_battery_status_t payload;
      fmav_msg_battery_status_decode(&payload, &_msg);
      int32_t voltage = 0;
      bool has_voltage = false;
      for (uint8_t i=0; i<10; i++) {
        if (payload.voltages[i] != UINT16_MAX) {
          voltage += payload.voltages[i]; //uint16_t mV, UINT16_MAX if not known
          has_voltage = true;
        }
      }
      cameraStatus.battery_voltage_V = (has_voltage) ? 0.001f * (float)voltage : NAN;
      cameraStatus.battery_remaining_pct = payload.battery_remaining; // -1 if not known
      break;
    }
  }
}

// -- Startup Requests --

void MavlinkTelem::setCameraStartupRequests(void)
{
  set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_INFORMATION, 10, 200+0); //10x every ca 2sec
  set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_SETTINGS, 10, 200+5);
  set_request(TASK_CAMERA, TASK_SENDREQUEST_CAMERA_CAPTURE_STATUS, 10, 200+10);
  set_request(TASK_CAMERA, TASK_SENDREQUEST_STORAGE_INFORMATION, 10, 200+15);
}

// -- Resets --

void MavlinkTelem::_resetCamera(void)
{
  _task[TASK_CAMERA] = 0;

  camera.compid = 0;
  camera.is_receiving = 0;

  camera.requests_triggered = 0;
  camera.requests_waiting_mask = CAMERA_REQUESTWAITING_ALL;
  camera.is_initialized = false;

  camera.system_status = MAV_STATE_UNINIT;
  camera.custom_mode = 0;
  camera.is_armed = false;
  camera.is_standby = true;
  camera.is_critical = false;
  camera.prearm_ok = false;
  camera.updated = 0;

  cameraInfo.vendor_name[0] = 0;
  cameraInfo.model_name[0] = 0;
  cameraInfo.flags = 0;
  cameraInfo.has_video = false;
  cameraInfo.has_photo = false;
  cameraInfo.has_modes = false;
  cameraInfo.total_capacity_MiB = NAN;

  cameraStatus.mode = 0;
  cameraStatus.video_on = false;
  cameraStatus.photo_on = false;
  cameraStatus.available_capacity_MiB = NAN;
  cameraStatus.recording_time_ms = UINT32_MAX;
  cameraStatus.battery_voltage_V = NAN;
  cameraStatus.battery_remaining_pct = -1;
}
