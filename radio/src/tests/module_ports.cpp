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

#include "gtests.h"
#include "hal/module_driver.h"
#include "hal/module_port.h"
#include "hal/serial_driver.h"
#include "pulses/modules_constants.h"
#include "pulses/pulses.h"
#include "translations.h"

TEST(ports, softserialFallback)
{
  modulePortInit();

  const etx_serial_init serialCfg = {
    .baudrate = 57600,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_RX,
    .polarity = ETX_Pol_Inverted,
  };

  bool has_softserial = modulePortFind(EXTERNAL_MODULE, ETX_MOD_TYPE_SERIAL,
                                       ETX_MOD_PORT_SPORT_INV, ETX_Pol_Inverted,
                                       ETX_Dir_RX);
  if (has_softserial) {
    auto mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT,
                                       &serialCfg, false);
    EXPECT_TRUE(mod_st == nullptr);

    mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT,
                                  &serialCfg, true);
    EXPECT_TRUE(mod_st != nullptr);
    if (!mod_st) return;

    modulePortDeInit(mod_st);
  }
}

#if defined(HARDWARE_EXTERNAL_MODULE)
TEST(ports, isPortUsed)
{
  modulePortInit();

  const etx_serial_init serialCfg = {
    .baudrate = 57600,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
  };
  
  auto mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT,
                                     &serialCfg, false);
  EXPECT_TRUE(mod_st != nullptr);
  EXPECT_TRUE(mod_st && mod_st->rx.port != nullptr);

  auto module = modulePortGetModuleForPort(ETX_MOD_PORT_SPORT);
  EXPECT_EQ(EXTERNAL_MODULE, module);
  
  if (mod_st) modulePortDeInit(mod_st);
  EXPECT_FALSE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));
}
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(HARDWARE_EXTERNAL_MODULE)
#include "pulses/pxx1.h"

static void _setModuleDrv(uint8_t module, const etx_proto_driver_t* drv, void* ctx)
{
  auto mod_drv = pulsesGetModuleDriver(module);
  mod_drv->drv = drv;
  mod_drv->ctx = ctx;
}

static void _deinitModuleDrv(uint8_t module)
{
  auto mod_drv = pulsesGetModuleDriver(module);
  auto drv = mod_drv->drv;
  auto ctx = mod_drv->ctx;
  
  drv->deinit(ctx);
  memset(mod_drv, 0, sizeof(module_pulse_driver));
}

static void _sendPulses(uint8_t module, uint8_t* buffer)
{
  auto mod_drv = pulsesGetModuleDriver(module);
  auto drv = mod_drv->drv;
  auto ctx = mod_drv->ctx;

  int16_t* channels = &channelOutputs[0];
  drv->sendPulses(ctx, buffer, channels, 16);
}

TEST(ports, deactivateRX_pxx1)
{
  modulePortInit();
  g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_R9M_PXX1;

  void* ext_ctx = Pxx1Driver.init(EXTERNAL_MODULE);
  EXPECT_TRUE(ext_ctx != nullptr);
  EXPECT_TRUE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));

  if (!ext_ctx) return;
  _setModuleDrv(EXTERNAL_MODULE, &Pxx1Driver, ext_ctx);

  void* int_ctx = Pxx1Driver.init(INTERNAL_MODULE);
  EXPECT_TRUE(int_ctx != nullptr);
  EXPECT_EQ(INTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  if (!int_ctx) return;
  _setModuleDrv(INTERNAL_MODULE, &Pxx1Driver, int_ctx);
  
  _deinitModuleDrv(INTERNAL_MODULE);
  EXPECT_EQ(EXTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  _deinitModuleDrv(EXTERNAL_MODULE);
  EXPECT_FALSE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));
}

#include "pulses/multi.h"
#include "telemetry/multi.h"

TEST(ports, deactivateRX_multi)
{
  modulePortInit();
  g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_MULTIMODULE;

  void* ext_ctx = MultiDriver.init(EXTERNAL_MODULE);
  EXPECT_TRUE(ext_ctx != nullptr);
  EXPECT_TRUE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));

  if (!ext_ctx) return;
  _setModuleDrv(EXTERNAL_MODULE, &MultiDriver, ext_ctx);

  uint8_t buffer[64];
  _sendPulses(EXTERNAL_MODULE, buffer);
  EXPECT_FALSE(buffer[0x1A] & 2);

  auto& mpm_status = getMultiModuleStatus(EXTERNAL_MODULE);
  mpm_status.invalidate();
  mpm_status.getStatusString((char*)buffer);
  EXPECT_STREQ(STR_MODULE_NO_TELEMETRY, (char*)buffer);
  
  void* int_ctx = Pxx1Driver.init(INTERNAL_MODULE);
  EXPECT_TRUE(int_ctx != nullptr);
  EXPECT_EQ(INTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  if (!int_ctx) return;
  _setModuleDrv(INTERNAL_MODULE, &Pxx1Driver, int_ctx);

  _sendPulses(EXTERNAL_MODULE, buffer);
  EXPECT_TRUE(buffer[0x1A] & 2);

  mpm_status.getStatusString((char*)buffer);
  EXPECT_STREQ(STR_DISABLE_INTERNAL, (char*)buffer);

  _deinitModuleDrv(INTERNAL_MODULE);
  EXPECT_EQ(EXTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  _sendPulses(EXTERNAL_MODULE, buffer);
  EXPECT_FALSE(buffer[0x1A] & 2);

  _deinitModuleDrv(EXTERNAL_MODULE);
  EXPECT_FALSE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));
}

TEST(ports, boot_pxx1_multi)
{
  modulePortInit();
  g_model.moduleData[EXTERNAL_MODULE].type = MODULE_TYPE_MULTIMODULE;

  // Init PXX1 internal module first
  void* int_ctx = Pxx1Driver.init(INTERNAL_MODULE);
  EXPECT_TRUE(int_ctx != nullptr);

  if (!int_ctx) return;
  _setModuleDrv(INTERNAL_MODULE, &Pxx1Driver, int_ctx);

  EXPECT_TRUE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));
  EXPECT_EQ(INTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  // Init MPM external module second
  void* ext_ctx = MultiDriver.init(EXTERNAL_MODULE);
  EXPECT_TRUE(ext_ctx != nullptr);

  if (!ext_ctx) return;
  _setModuleDrv(EXTERNAL_MODULE, &MultiDriver, ext_ctx);

  // Verify internal module still owns S.PORT
  EXPECT_EQ(INTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));
  EXPECT_FALSE(modulePortIsPortUsedByModule(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT));

  // Verify MPM sends "disable telemetry" bit
  uint8_t buffer[64];
  _sendPulses(EXTERNAL_MODULE, buffer);
  EXPECT_TRUE(buffer[0x1A] & 2);

  // Verify MPM status
  auto& mpm_status = getMultiModuleStatus(EXTERNAL_MODULE);
  mpm_status.invalidate();
  mpm_status.getStatusString((char*)buffer);
  EXPECT_STREQ(STR_DISABLE_INTERNAL, (char*)buffer);

  // Disable internal module
  _deinitModuleDrv(INTERNAL_MODULE);

  // Verify external module now owns S.PORT
  EXPECT_EQ(EXTERNAL_MODULE, modulePortGetModuleForPort(ETX_MOD_PORT_SPORT));

  // Verify MPM does not send "disable telemetry" bit
  _sendPulses(EXTERNAL_MODULE, buffer);
  EXPECT_FALSE(buffer[0x1A] & 2);

  // Verify MPM status
  mpm_status.getStatusString((char*)buffer);
  EXPECT_STREQ(STR_MODULE_NO_TELEMETRY, (char*)buffer);

  // disable MPM
  _deinitModuleDrv(EXTERNAL_MODULE);
  EXPECT_FALSE(modulePortIsPortUsed(ETX_MOD_PORT_SPORT));
}

TEST(ports, isTelemAllowedOnBind)
{
  modulePortInit();

  void* int_ctx = Pxx1Driver.init(INTERNAL_MODULE);
  EXPECT_TRUE(int_ctx != nullptr);
  if (!int_ctx) return;
  _setModuleDrv(INTERNAL_MODULE, &Pxx1Driver, int_ctx);

  // Telem always has priority on internal
  EXPECT_TRUE(isTelemAllowedOnBind(INTERNAL_MODULE));

  // When internal uses SPORT, you cannot bind FRSKY with telem on external module
  EXPECT_FALSE(isTelemAllowedOnBind(EXTERNAL_MODULE));

  // but you can when internal is disabled
  _deinitModuleDrv(INTERNAL_MODULE);
  EXPECT_TRUE(isTelemAllowedOnBind(EXTERNAL_MODULE));
}
#elif !defined(PCBFLYSKY)  //defined(INTERNAL_MODULE_PXX1) && defined(HARDWARE_EXTERNAL_MODULE)
TEST(ports, isTelemAllowedOnBind)
{
  modulePortInit();

  const etx_serial_init serialCfg = {
    .baudrate = 921000,
    .encoding = ETX_Encoding_8N1,
    .direction = ETX_Dir_TX_RX,
    .polarity = ETX_Pol_Normal,
  };

  auto mod_st = modulePortInitSerial(INTERNAL_MODULE, ETX_MOD_PORT_UART, &serialCfg, false);
  EXPECT_TRUE(mod_st != nullptr);

  // Since internal module doesn't use SPORT, you should be able to bind FrSky with telem
  EXPECT_TRUE(isTelemAllowedOnBind(EXTERNAL_MODULE));
}
#endif
