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

#include "dsmp.h"
#include "hal/module_port.h"
#include "mixer_scheduler.h"

#include "edgetx.h"
#include "telemetry/spektrum.h"

#define DSMP_SEND_X_PLUS    0

#define DSMP_BITRATE        115200

#define MAX_REG_CHANNELS    12
#define MAX_CHANNELS        20

// DSMP Flags
#define DSMP_FLAGS_DSMX 0x01
#define DSMP_FLAGS_DSM2 0x00
#define DSMP_FLAGS_11mS 0x02
#define DSMP_FLAGS_22mS 0x00
#define DSMP_FLAGS_2048 0x04    // 11 bits
#define DSMP_FLAGS_1024 0x00    // 10 bits
#define DSMP_FLAGS_FUTABA 0x10  // Translate to AETR -> TAER
// #define DSMP_FLAGS_DEVO   0x20
#define DSMP_FLAGS_AUTO 0x40
#define DSMP_FLAGS_BIND 0x80

#define DSMP_FLAGS_TXMODE (DSMP_FLAGS_DSMX | DSMP_FLAGS_11mS | DSMP_FLAGS_2048)

// DSMP Power Settings
#define DSMP_POWER_NORMAL   7
#define DSMP_POWER_RANGE    4

// Same Encoding as MultiModule  (What channel is each of the functions)
//                           R    | T    | E    | A
#define DSMP_CH_AETR       ((3<<6)|(2<<4)|(1<<2)|0)
#define DSMP_CH_TAER       ((3<<6)|(0<<4)|(2<<2)|1)

#define DSMP_NO_CHANNEL     0xFF

// MAP AETR->TAER:     
static uint8_t AETR_TAER_MAP[] = {2, 0, 1};  

static DSMPModuleStatus dsmpStatus = DSMPModuleStatus();

// Power cycle of the DSMP module
// The DSMP module code has the channel data initialized to 0 and initial number of bits as 10b (ch resolution 1024)
// The Setup message pass the last BIND configuration to the Module via the setup package (for example, 2048 it was bound to DSMX).
// Scenario 1: Setup package processed by DSMP before Ch data
// When the Setup package is received, will send the existing CH values (0), so servos will move to the side until valid
// Ch data is received.
// Scenario 2: Channel packed process by DSMP before Setup package
// In this case, it will assume that the Ch data is encoded in 10bits instead of 2048 (if DSMX), then the servos move to 
// to extreme too.
// 
// Both scenarios are not right, so initially send more frequently the Setup package (usualy every 2.2 sec), but will
// be sent every 2 messages (66ms). There is still jump, but for an instant.
// The only way to properly fix it is at the DSMP firmware level.. (fixed on DSMP V2).


#define FAST_INITIAL_SETUP_COUNT  50  // 3*22*50=3.3 sec of sending fast status after module activation

// Using 22.06ms instead of 22ms for V1 module.. This is becuase the module try to send 4 RF
// packets to the RX with with spacing 4/7/4/7 (22ms cycle) just after receiving
// the message from the TX (perfect world). But the timing is not perfect on the
// DSMP module, is really 4.025/7.033/4.025/6.x and the last will be cut too short
// to 6.88 by the new TX message (needs to be at least 6.92, up to 7.05). That
// can create problems in some RXs (Servos not moving smoth). Compensating for
// that little extra delay to make the last closer to 7ms spacing. 

#define SCHEDULER_PERIOD_V1       22060
#define SCHEDULER_PERIOD_V2       11000

static uint8_t pass;
static uint8_t pass0_counter;
static uint8_t pass0_period;

static uint16_t fastSetup_count;  

#if DSMP_SEND_X_PLUS
static uint8_t x_plus_ch = 0;
#endif

DSMPModuleStatus& getDSMPStatus(uint8_t module)
{ 
    return dsmpStatus; 
}

static void* dsmpInit(uint8_t module)
{
    TRACE("[DSMP] dsmpInit()");
    pass0_counter = 2;  // Send Setup every 2 ch data packages
    pass = 1;           // Start Sending channels
    pass0_period = 100; // Send pass0 every 100 messages
    fastSetup_count = FAST_INITIAL_SETUP_COUNT;
    return (void*)dsmInit(module, DSMP_BITRATE, SCHEDULER_PERIOD_V1, true);
}


static inline void sendByte(uint8_t*& p_buf, uint8_t b)
{
    *p_buf++ = b;
}

#if defined(LUA)
static uint8_t isFPDataReady()
{
    // Check for Forward Programming data
    // Multi_Buffer[0..2]=="DSM" -> Lua script is running
    // Multi_Buffer[3]==0x70 + len -> TX to RX data ready to be sent
    // Multi_Buffer[4..9]=6 bytes of TX to RX data
    return (Multi_Buffer && (Multi_Buffer[3] & 0xF8) == 0x70 &&
            memcmp(Multi_Buffer, "DSM", 3) == 0);
}

static void sendFPLemonDSMP(uint8_t*& p_buf)
{
    [[maybe_unused]] auto forwardProgLen = Multi_Buffer[3] & 0x0F;
    TRACE("LemonDSMP: DSMP FwdProg Send data len = [%d]", forwardProgLen);
    // Send Forward Prog Data (Includes Len + 6 bytes).. 9 bytes totat
    sendByte(p_buf, 0xAA);
    sendByte(p_buf, 3);  // Pass=3

    for (uint8_t i = 0; i < 7; i++) {
        sendByte(p_buf, Multi_Buffer[3 + i]);
    }

    Multi_Buffer[3] = 0x00;  // Data sent, clear LUA to send more
}
#endif

static void updateModuleStatus(uint8_t flags)
{
    // Update Status
    dsmpStatus.lastUpdate = get_tmr10ms();
    dsmpStatus.flags = flags;
    dsmpStatus.ch_order = (flags & DSMP_FLAGS_FUTABA) ? DSMP_CH_AETR : DSMP_CH_TAER;
}

static uint16_t getDSMPChannelValue(uint8_t module, uint8_t flags, uint8_t channel)
{ 
    uint8_t txChannel = channel;
    // Map from AETR->TAER ???
    if ((flags & DSMP_FLAGS_FUTABA) && (channel < 3)) { 
        txChannel = AETR_TAER_MAP[channel];
    }

    int value = channelOutputs[txChannel] + 2 * PPM_CH_CENTER(txChannel) - 2 * PPM_CENTER;
    uint16_t pulse;
    
    if (flags & DSMP_FLAGS_2048) {  // Use 11-bit ?
        // Scale to 349/512=0.681, MultiModule is about 0.667
        pulse = limit(0, ((value * 349) >> 9) + 1024, 2047) | (channel << 11);
    } else {
        pulse = limit(0, ((value * 13) >> 5) + 512, 1023) | (channel << 10);
    }

    return pulse;
}

static void setupPulsesLemonDSMP(uint8_t module, uint8_t*& p_buf)
{
    const auto modelId = g_model.header.modelId[module];
  /* const*/ auto& md = g_model.moduleData[module];

    //uint8_t start_channel = md.channelsStart;
    auto channels = md.getChannelsCount();
    auto flags = md.dsmp.flags;
    auto module_mode = getModuleMode(module);
    auto version     = dsmpStatus.version[0];

    if (md.dsmp.enableAETR) { // Move GUI settings to flags
        flags |= DSMP_FLAGS_FUTABA;  
    }

    if (pass0_counter == pass0_period/2) {
        // update status String.. about each second
        updateModuleStatus(flags);
    }

#if defined(LUA)
    if ((version> 1) && isFPDataReady()) {  // Sent any forward prog data??
        sendFPLemonDSMP(p_buf);
        Multi_Buffer[3] = 0x00;  // Data sent, clear LUA to send more
        return;
    }
#endif

    // Send channel data
    sendByte(p_buf, 0xAA);
    sendByte(p_buf, pass);

    // Setup packet
    if (pass == 0) {
        flags &= DSMP_FLAGS_TXMODE; // Only the TXMODE part

        if (module_mode == MODULE_MODE_BIND) {
            flags = DSMP_FLAGS_BIND | DSMP_FLAGS_AUTO;
            channels = min<uint8_t>(channels, MAX_REG_CHANNELS);
            md.dsmp.flags = DSMP_FLAGS_DSMX | DSMP_FLAGS_11mS | DSMP_FLAGS_2048;
            storageDirty(EE_MODEL);
        }
        sendByte(p_buf, flags);

        uint8_t pwr = DSMP_POWER_NORMAL;
        if (module_mode == MODULE_MODE_RANGECHECK) {
            pwr = DSMP_POWER_RANGE;
        }
        sendByte(p_buf, pwr);
        sendByte(p_buf, channels );
        sendByte(p_buf, modelId); // V1.0 ignores it, V2.0 use it

        pass = 1; // move to send Ch data
        return; 
    } else {
#if DSMP_SEND_X_PLUS
        uint8_t xPlusEnabled = (version > 1) && (channels >= MAX_REG_CHANNELS) && (flags & DSMP_FLAGS_DSMX);
#endif
        //  Channel DATA messages
        uint8_t current_channel = 0;
        if (pass == 2) {
            current_channel += 7;
        }

        // Send channels
        for (int i=0; i<7; i++) {
            uint16_t pulse = 0xFFFF;  // NO-DATA
            uint8_t ch_to_send = DSMP_NO_CHANNEL;

            if (current_channel < min<uint8_t>(channels,MAX_REG_CHANNELS)) {
              ch_to_send = current_channel;
            }

#if DSMP_SEND_X_PLUS
            else if (xPlusEnabled && (current_channel >= MAX_REG_CHANNELS)) {
              // X-plus channel multiplexing (Ch13-20)
              ch_to_send = MAX_REG_CHANNELS + x_plus_ch;
              x_plus_ch = (x_plus_ch + 1) % 8;  // 8 X-plus channels
            }
#endif

            if (ch_to_send != DSMP_NO_CHANNEL) {
              pulse = getDSMPChannelValue(module, flags, ch_to_send);
            }

            sendByte(p_buf, pulse >> 8);
            sendByte(p_buf, pulse & 0xFF);
            current_channel++;
        }  // For
    }

    if (++pass > 2) pass = 1;
    if (channels < 8) pass = 1;

    if (module_mode == MODULE_MODE_BIND) { // Force setup packet in Bind mode.
        pass = 0;
    } else if (--pass0_counter == 0) {
        pass = 0;
        updateModuleStatus(flags);
        if ((version == 1) && (fastSetup_count > 0)) {
            fastSetup_count--;
            pass0_counter = 2;  // Keep sendint setup every 2 ch messages
        } else {
          pass0_counter = pass0_period;  // restar counter
        }
  }
}

static void dsmpSendPulses(void* ctx, uint8_t* buffer, int16_t* channels, uint8_t nChannels)
{
    auto mod_st = (etx_module_state_t*)ctx;
    auto module = modulePortGetModule(mod_st);
    auto drv = modulePortGetSerialDrv(mod_st->tx);
    auto drvCtx = modulePortGetCtx(mod_st->tx);

    auto p_data = buffer;
    setupPulsesLemonDSMP(module, p_data);
    drv->sendBuffer(drvCtx, buffer, p_data - buffer);
}

/* The DSMP Bind Return Message contains:
Byte
0       Flags: 
            Protocol:      Maxk 0x01   0=DSM2, 1=DSMX
            RefreshCycle:  Mask 0x02   0=22ms, 1=11ms
            Resolution:    Mask 0x04   0=1024, 1=2048    
            Bind:          Mask 0x80   1=Enter Bind Mode  (Output only)
            Bind_Auto:     Mask 0x40   1=Use AutoBind     (Output only)
1       ?? Always 0x00;   ?? RX type maybe
2       Number of Channels  (Up to 12) 
3       TX Mode: 
            0x01 - DSM2 1024/22ms,
            0x02 - DSM2 1024/22ms >7 channels,
            0x11 - DSM2 2048/11ms
            0x12 - DSM2 2048/11ms >7 channels,
            0xa2 - DSMX 22ms > 7 channels,
            0xb2 - DSMX 11ms
 */

static void processDSMPBindPacket(uint8_t module, uint8_t* packet) 
{
    uint8_t flags    = packet[0];
    uint8_t rxType   = packet[1];
    uint8_t channels = packet[2];
    uint8_t txMode   = packet[3];

    // save flags, only the TXMode Part
    g_model.moduleData[module].dsmp.flags = flags & DSMP_FLAGS_TXMODE;

    // save number of channels
    if (channels > MAX_REG_CHANNELS) {
      channels = MAX_REG_CHANNELS;
    }
    g_model.moduleData[module].channelsCount = channels - 8;
    if (dsmpStatus.version[0] == 1) { // V1.0 always use modelId=0
        g_model.header.modelId[module] = 0;  
    }

    TRACE("[DSMP] DSMP bind packet: flags:0x%X / Ch: %i / TxMode =0x%X", flags & 0x3F, channels, txMode);

    storageDirty(EE_MODEL);

    /* Finally stop binding as the rx just told us that it is bound */
    if (getModuleMode(module) == MODULE_MODE_BIND) {
        setModuleMode(module, MODULE_MODE_NORMAL);
    }

    // Convert DSMP bind to Spektrum compatible bind and continue with common Spektrum.cpp bind processing
    uint8_t newPacket[10] = {0, 0, 0, 0, rxType, channels, txMode, 0, 0, 0};
    processDSMBindPacket(newPacket);

    // FARZU:.. The real problem was using module[moduleID].counter.. is actually used by the restart mechanism
    restartModuleAsync(module, 10);  // ~100ms
}


static void processDSMPManufacturerData(uint8_t module, uint8_t* packet) 
{
    // Format M,M,M,M, V, V    where M is 4 byte manufacturer data, and V is 2 byte
    dsmpStatus.version[0] = packet[4];  // Major
    dsmpStatus.version[1] = packet[5];  // Minor

    TRACE("LemonDSMP: Ver [%d.%d]", dsmpStatus.version[0], dsmpStatus.version[1]);

    if (dsmpStatus.version[0] > 1 && mixerSchedulerGetPeriod(module) != SCHEDULER_PERIOD_V2) {
      // V2 suppors 11ms 
      mixerSchedulerSetPeriod(module, SCHEDULER_PERIOD_V2);
      pass0_period = 200; // extend period
    }
}

static void dsmpTelemetryData(uint8_t module, uint8_t data,
                              uint8_t* rxBuffer, uint8_t& rxBufferCount)
{
    dsmpStatus.lastUpdate = get_tmr10ms();

    if (rxBuffer[1] == 0x80 && rxBufferCount >= DSM_BIND_PACKET_LENGTH) {
        processDSMPBindPacket(module, rxBuffer + 2);  // Skip 0xAA 0x80
        rxBufferCount = 0;
        return;
    }

    if (rxBuffer[1] == 0xFF && rxBufferCount >= SPEKTRUM_TELEMETRY_LENGTH) {
        processDSMPManufacturerData(module, rxBuffer + 2); // Skip 0xAA 0xFF
        rxBufferCount = 0;
        return;
    }

    if (rxBufferCount >= SPEKTRUM_TELEMETRY_LENGTH) {
    // Debug print content of Telemetry to console
#if 0
        debugPrintf("[DSMP] Packet 0x%02X rssi 0x%02X: ic2 0x%02x, %02x: ",
                    rxBuffer[0], rxBuffer[1], rxBuffer[2], rxBuffer[3]);
        for (int i=4; i<SPEKTRUM_TELEMETRY_LENGTH; i+=4) {
            debugPrintf("%02X%02X %02X%02X  ", rxBuffer[i], rxBuffer[i + 1],
                        rxBuffer[i + 2], rxBuffer[i + 3]);
        }
        debugPrintf(CRLF);
#endif

        processSpektrumPacket(rxBuffer);
        rxBufferCount = 0;
    }
}


static void dsmpDebugData(uint8_t module, uint8_t data, uint8_t* rxBuffer,
                           uint8_t& rxBufferCount)
{
    // Format is: 0xAB, <len> <type> <data>
    if (rxBufferCount < 2) return;  // Keep buiding

    auto expectedLen = rxBuffer[1];

    // Overflow???
    if (expectedLen + 2 > TELEMETRY_RX_PACKET_SIZE) {
        expectedLen = TELEMETRY_RX_PACKET_SIZE - 2;
    }

    if (rxBufferCount >= expectedLen + 2) {
        // msg Completed
        auto dataType = rxBuffer[2];

        if (dataType == 1) {  // Debug Msg
            TRACE_NOCRLF("[DSMP] Debug: ");
            for (auto i = 0; i < expectedLen - 1; i++) {
                TRACE_NOCRLF("%c", rxBuffer[3 + i]);
            }
            TRACE(CRLF);
        }

        rxBufferCount = 0;  // Build new message
    }
}


static void dsmpProcessData(void* ctx, uint8_t data, uint8_t* buffer,
                            uint8_t* len)
{
    auto mod_st = (etx_module_state_t*)ctx;
    auto module = modulePortGetModule(mod_st);

    uint8_t& rxBufferCount = *len;

    if (rxBufferCount == 0 && (data != 0xAA && data != 0xAB)) {
        TRACE("[DSMP] invalid start byte 0x%02X", data);
        return;
    }

    if (rxBufferCount < TELEMETRY_RX_PACKET_SIZE) {

        TRACE("[DSMP] Data 0x%02X, len = %d", data, rxBufferCount);
        buffer[rxBufferCount++] = data;  // Keep building message
    } else {
        TRACE("[DSMP] array size %d error", rxBufferCount);
        rxBufferCount = 0;
    }

    if (buffer[0] == 0xAA) {  // Telemetry data or Bind
        dsmpTelemetryData(module, data, buffer, *len);
    } else if (buffer[0] == 0xAB) {  // Status
        dsmpDebugData(module, data, buffer, *len);
    }
}

void DSMPModuleStatus::getStatusString(char* statusText) const
{
    if (!isValid()) {
        strcpy(statusText, STR_MODULE_NO_TELEMETRY);
        return;
    }

    const auto& md = g_model.moduleData[EXTERNAL_MODULE];
    [[maybe_unused]] auto channels = md.getChannelsCount();

    char* tmp = statusText;

    // Version
    *tmp = 0;
    tmp = strAppend(tmp, "v", 1);
    tmp = strAppendUnsigned(tmp, dsmpStatus.version[0]);
    tmp = strAppend(tmp, ".", 1);
    tmp = strAppendUnsigned(tmp, dsmpStatus.version[1]);

    char b[] = "?   ";
    if (ch_order != 0xFF) { // Same encoding as MultiModule
        uint8_t temp = ch_order;
        b[temp & 0x03] = 'A';
        temp >>= 2;
        b[temp & 0x03] = 'E';
        temp >>= 2;
        b[temp & 0x03] = 'T';
        temp >>= 2;
        b[temp & 0x03] = 'R';
    }

    tmp = strAppend(tmp, " ", 1);
    tmp = strAppend(tmp, b, strlen(b));

    const char* mode;

    mode = (flags & DSMP_FLAGS_DSMX) ? " X" : " 2"; // DSMX or DSM2
    tmp = strAppend(tmp, mode, strlen(mode));

    mode = (flags & DSMP_FLAGS_11mS) ? "_2F" : "_1F"; // 1 or 2 Frames (11ms/22ms)
    tmp = strAppend(tmp, mode, strlen(mode));

#if 0
    // Good for Debugging, but not much for regular users
    
    //mode = (flags & DSMP_FLAGS_2048) ? " 2048" : " 1024";
    //tmp = strAppend(tmp, mode, strlen(mode));

    tmp = strAppend(tmp, "  S:", 4);
    uint16_t servoRefresh = (mixerSchedulerGetPeriod(EXTERNAL_MODULE) / 1000); 
    if (channels > 7) servoRefresh *= 2;
    tmp = strAppendUnsigned(tmp, servoRefresh);
    tmp = strAppend(tmp, "ms", 2);

    servoRefresh = mixerSchedulerGetPeriod(EXTERNAL_MODULE);
    tmp = strAppend(tmp, "  TX:", 5);
    tmp = strAppendUnsigned(tmp, servoRefresh);
    tmp = strAppend(tmp, "us", 2);
#endif

}

const etx_proto_driver_t DSMPDriver = {
    .protocol = PROTOCOL_CHANNELS_DSMP,
    .init = dsmpInit,
    .deinit = dsmDeInit,
    .sendPulses = dsmpSendPulses,
    .processData = dsmpProcessData,
    .processFrame = nullptr,
    .onConfigChange = nullptr,
    .txCompleted = modulePortSerialTxCompleted,
};
