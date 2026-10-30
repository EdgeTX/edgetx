// SPDX-License-Identifier: GPL-2.0-only
#include "external_rf.h"

#include "edgetx.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"

namespace
{
bool shutdownInhibited = false;
MamboExternalDiagnostics diagnostics;
}  // namespace

void mamboExternalTxObserved(uint8_t type)
{
  ++diagnostics.txFrames;
  diagnostics.lastTxType = type;
}
void mamboExternalRxObserved(uint8_t, bool error)
{
  ++diagnostics.rxBytes;
  if (error) ++diagnostics.uartErrors;
}
void mamboExternalIdleObserved() { ++diagnostics.idleEvents; }
void mamboExternalParseObserved() { ++diagnostics.parseEvents; }
void mamboExternalFrameObserved(uint8_t type)
{
  ++diagnostics.validFrames;
  diagnostics.lastRxType = type;
}
void mamboExternalCrcErrorObserved() { ++diagnostics.crcErrors; }
void mamboExternalHeaderErrorObserved() { ++diagnostics.headerErrors; }

MamboExternalDiagnostics mamboExternalDiagnostics()
{
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  auto result = diagnostics;
  result.powered = gpio_read(EXTMODULE_PWR_GPIO);
  result.pins = (gpio_read(TELEMETRY_TX_REV_GPIO) ? 1 : 0) |
                (gpio_read(TELEMETRY_RX_REV_GPIO) ? 2 : 0) |
                (gpio_read(TELEMETRY_DIR_GPIO) ? 4 : 0);
  __set_PRIMASK(mask);
  return result;
}

void mamboExternalModulePower(bool enable)
{
  // Keep the bay off for Off and unsupported persisted/imported protocols.
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  bool allowed =
      enable && !shutdownInhibited &&
      g_model.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_CROSSFIRE;
  gpio_write(EXTMODULE_PWR_GPIO, allowed);
  __set_PRIMASK(mask);
}

void mamboExternalModuleShutdown()
{
  // Nonblocking emergency inhibition: no task locks, waits or protocol I/O.
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  shutdownInhibited = true;
  gpio_clear(EXTMODULE_PWR_GPIO);
  __set_PRIMASK(mask);
}
