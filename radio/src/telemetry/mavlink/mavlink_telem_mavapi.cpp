/*
* (c) www.olliw.eu, OlliW, OlliW42
*/

#include "opentx.h"

// -- Receive stuff --

// we probably need to differentiate not only by msgid, but also by sysid-compid
// if two components send the same message at (too) high rate considering only msgid leads to message loss

uint8_t MavlinkTelem::_mavapiMsgInFindOrAdd(uint32_t msgid)
{
  for (uint8_t i = 0; i < MAVMSGLIST_MAX; i++) {
    if (!_mavapi_rx_list[i]) continue;
    if (_mavapi_rx_list[i]->msgid == msgid) { return i; }
  }
  for (uint8_t i = 0; i < MAVMSGLIST_MAX; i++) {
    if (_mavapi_rx_list[i]) continue;
    // free spot, so add it
    _mavapi_rx_list[i] = (MavMsg*)malloc(sizeof(MavMsg));
    if (!_mavapi_rx_list[i]) return UINT8_MAX; // grrrr
    _mavapi_rx_list[i]->msgid = msgid;
    _mavapi_rx_list[i]->payload_ptr = NULL;
    _mavapi_rx_list[i]->updated = false;
    return i;
  }
  return UINT8_MAX;
}

void MavlinkTelem::mavapiHandleMessage(fmav_message_t* msg)
{
  if (!_mavapi_rx_enabled) return;

  uint8_t i = _mavapiMsgInFindOrAdd(msg->msgid);
  if (i == UINT8_MAX) return;

  if (!_mavapi_rx_list[i]->payload_ptr) _mavapi_rx_list[i]->payload_ptr = malloc(msg->payload_max_len);
  if (!_mavapi_rx_list[i]->payload_ptr) return; // grrrr

  _mavapi_rx_list[i]->sysid = msg->sysid;
  _mavapi_rx_list[i]->compid = msg->compid;
  _mavapi_rx_list[i]->target_sysid = msg->target_sysid;
  _mavapi_rx_list[i]->target_compid = msg->target_compid;
  memcpy(_mavapi_rx_list[i]->payload_ptr, msg->payload, msg->payload_max_len);
  _mavapi_rx_list[i]->updated = true;
  _mavapi_rx_list[i]->timestamp = time10us();
}

void MavlinkTelem::mavapiMsgInEnable(bool flag)
{
  _mavapi_rx_enabled = flag;
}

uint8_t MavlinkTelem::mavapiMsgInCount(void)
{
  if (!_mavapi_rx_enabled) return 0;

  uint8_t cnt = 0;
  for (uint8_t i = 0; i < MAVMSGLIST_MAX; i++) if(_mavapi_rx_list[i]) cnt++;
  return cnt;
}

MavlinkTelem::MavMsg* MavlinkTelem::mavapiMsgInGet(uint32_t msgid)
{
  if (!_mavapi_rx_enabled) return NULL;

  uint8_t i_found = UINT8_MAX;
  for (uint8_t i = 0; i < MAVMSGLIST_MAX; i++) {
    if (!_mavapi_rx_list[i]) continue;
    if (!_mavapi_rx_list[i]->payload_ptr) continue; // it must have been received completely
    if (_mavapi_rx_list[i]->msgid == msgid) { i_found = i; }
  }
  if (i_found == UINT8_MAX) return NULL;
  return _mavapi_rx_list[i_found];
}

MavlinkTelem::MavMsg* MavlinkTelem::mavapiMsgInGetLast(void)
{
  if (!_mavapi_rx_enabled) return NULL;

  uint32_t t_max = 0;
  uint8_t i_found = UINT8_MAX;
  for (uint8_t i = 0; i < MAVMSGLIST_MAX; i++) {
    if (!_mavapi_rx_list[i]) continue;
    if (!_mavapi_rx_list[i]->payload_ptr) continue; // it must have been received completely
    if (_mavapi_rx_list[i]->timestamp > t_max) { t_max = _mavapi_rx_list[i]->timestamp; i_found = i; }
  }
  if (i_found == UINT8_MAX) return NULL;
  return _mavapi_rx_list[i_found];
}

// -- Send stuff --

void MavlinkTelem::mavapiMsgOutEnable(bool flag)
{
  _mavapi_tx_enabled = flag;

  if (_mavapi_tx_enabled && _mavapiMsgOutFifo == NULL) {
    _mavapiMsgOutFifo = (fmav_message_t*)malloc(sizeof(fmav_message_t) * MAVOUTFIFO_MAX);
    if (!_mavapiMsgOutFifo) _mavapi_tx_enabled = false; // grrrr
  }
}


// returns the pointer into which we should write, without advancing write index, probe()-like
fmav_message_t* MavlinkTelem::mavapiMsgOutPtr(void)
{
  if (!_mavapi_tx_enabled) return NULL;

  uint32_t wi_next = (_wi + 1) & (MAVOUTFIFO_MAX - 1);
  if (wi_next == _ri) return NULL; // blocking push, push not allowed if full
  return &(_mavapiMsgOutFifo[_wi]);
}

// advances write index, and sets task, push()-like
void MavlinkTelem::mavapiMsgOutSet(void)
{
  if (!_mavapi_tx_enabled) return;

  _wi = (_wi + 1) & (MAVOUTFIFO_MAX - 1);
  SETTASK(TASK_ME, TASK_SENDMSG_MAVLINK_API);
}

// generate from msg at read index, and advance read index, pop()-like
void MavlinkTelem::mavapiGenerateMessage(void)
{
  if (!_mavapi_tx_enabled) return;

  if (_wi == _ri) return; // empty
  fmav_message_t* msgptr = &(_mavapiMsgOutFifo[_ri]);
  _ri = (_ri + 1) & (MAVOUTFIFO_MAX - 1);
  memcpy(&_msg_out, msgptr, sizeof(fmav_message_t));
  fmav_finalize_msg(&_msg_out, &_status_out);
  _msg_out_available = true;
}



