#include "csd203_driver.h"

#include "board.h"
#include "debug.h"
#include "delays_driver.h"
#include "hal/i2c_driver.h"
#include "stm32_i2c_driver.h"
#include "timers_driver.h"

namespace {

constexpr tmr10ms_t CSD203_RETRY_INTERVAL = 100;

constexpr uint8_t CSD203_REG_CONFIGURATION = 0x00;
constexpr uint8_t CSD203_REG_BUS_VOLTAGE = 0x02;
constexpr uint8_t CSD203_REG_CURRENT = 0x04;
constexpr uint8_t CSD203_REG_CALIBRATION = 0x05;
constexpr uint8_t CSD203_REG_MANUFACTURER_ID = 0xFE;
constexpr uint8_t CSD203_I2C_ADDR = 0x44;

constexpr uint16_t CSD203_MANUFACTURER_ID = 0x4153;
constexpr uint16_t CSD203_CONFIG_VALUE =
  ((1u << 15) | (2u << 9) | (4u << 5) | (4u << 2) | 7u);
constexpr uint16_t CSD203_CALIBRATION_VALUE = 512;

bool g_csd203Ready = false;
tmr10ms_t g_lastProbeTick = 0;
bool g_i2cBusReady = false;
uint16_t g_i2cRecoverCount = 0;

bool csd203RecoverI2cBus(const char *reason)
{
  TRACE("V15 CSD203: I2C_Bus_1 recover (%s)", reason);

  if (i2c_deinit(I2C_Bus_1) < 0) {
    TRACE("V15 CSD203: I2C_Bus_1 deinit failed");
  }

  if (i2c_init(I2C_Bus_1) < 0) {
    g_i2cBusReady = false;
    TRACE("V15 CSD203: I2C_Bus_1 recover failed");
    return false;
  }
  g_i2cBusReady = true;
  g_csd203Ready = false;
  ++g_i2cRecoverCount;
  TRACE("V15 CSD203: I2C_Bus_1 recover ok (%u)", g_i2cRecoverCount);
  return true;
}

bool csd203WriteRegister16Raw(uint8_t address, uint8_t reg, uint16_t value)
{
  uint8_t frame[3] = {reg, static_cast<uint8_t>(value >> 8),
                      static_cast<uint8_t>(value & 0xFF)};
  return (stm32_i2c_master_tx(I2C_Bus_1, address, frame, sizeof(frame), 100) ==
          0);
}

bool csd203ReadRegister16Raw(uint8_t address, uint8_t reg, uint16_t &value)
{
  uint8_t rx[2] = {0};
  if (stm32_i2c_read(I2C_Bus_1, address, reg, 1, rx, sizeof(rx), 100) < 0) {
    return false;
  }

  value = (static_cast<uint16_t>(rx[0]) << 8) | rx[1];
  return true;
}

bool csd203WriteRegister16(uint8_t address, uint8_t reg, uint16_t value,
                           bool allowRecover = true)
{
  if (csd203WriteRegister16Raw(address, reg, value)) {
    return true;
  }

  if (!allowRecover) {
    return false;
  }

  TRACE("V15 CSD203: write reg 0x%02X failed at 0x%02X", reg, address);
  if (!csd203RecoverI2cBus("write")) {
    return false;
  }

  return csd203WriteRegister16Raw(address, reg, value);
}

bool csd203ReadRegister16(uint8_t address, uint8_t reg, uint16_t &value,
                          bool allowRecover = true)
{
  if (csd203ReadRegister16Raw(address, reg, value)) {
    return true;
  }

  if (!allowRecover) {
    return false;
  }

  TRACE("V15 CSD203: read reg 0x%02X failed at 0x%02X", reg, address);
  if (!csd203RecoverI2cBus("read")) {
    return false;
  }

  return csd203ReadRegister16Raw(address, reg, value);
}

bool csd203Configure(uint8_t address)
{
  if (!csd203WriteRegister16(address, CSD203_REG_CONFIGURATION,
                             CSD203_CONFIG_VALUE, false)) {
    return false;
  }

  if (!csd203WriteRegister16(address, CSD203_REG_CALIBRATION,
                             CSD203_CALIBRATION_VALUE, false)) {
    return false;
  }

  delay_us(200);

  uint16_t manufacturer = 0;
  return csd203ReadRegister16(address, CSD203_REG_MANUFACTURER_ID,
                              manufacturer, false) &&
         manufacturer == CSD203_MANUFACTURER_ID;
}

}  // namespace

void csd203DriverInit(void)
{
  g_csd203Ready = false;
  g_lastProbeTick = 0;
  g_i2cBusReady = false;

  if (i2c_init(I2C_Bus_1) >= 0) {
    g_i2cBusReady = true;
    TRACE("V15 CSD203: I2C_Bus_1 init ok");
  } else {
    TRACE("V15 CSD203: I2C_Bus_1 init failed");
  }
}

void csd203DriverInvalidate(void)
{
  g_csd203Ready = false;
}

bool csd203DriverEnsureReady(void)
{
  if (g_csd203Ready) {
    return true;
  }

  tmr10ms_t now = get_tmr10ms();
  if (g_lastProbeTick != 0 &&
      (tmr10ms_t)(now - g_lastProbeTick) < CSD203_RETRY_INTERVAL) {
    return false;
  }
  g_lastProbeTick = now;

  if (!g_i2cBusReady) {
    if (i2c_init(I2C_Bus_1) < 0) {
      TRACE("V15 CSD203: I2C_Bus_1 init failed");
      return false;
    }
    g_i2cBusReady = true;
    TRACE("V15 CSD203: I2C_Bus_1 re-init ok");
  }

  if (csd203Configure(CSD203_I2C_ADDR)) {
    g_csd203Ready = true;
    TRACE("V15 CSD203: sensor ready at fixed addr 0x%02X", CSD203_I2C_ADDR);
    return true;
  }

  return false;
}

bool csd203DriverReadVoltage(uint16_t &voltageMv)
{
  uint16_t rawBus = 0;
  if (!csd203ReadRegister16(CSD203_I2C_ADDR, CSD203_REG_BUS_VOLTAGE, rawBus)) {
    g_csd203Ready = false;
    return false;
  }

  voltageMv = static_cast<uint16_t>((static_cast<uint32_t>(rawBus) * 125u) / 100u);
  return true;
}

bool csd203DriverReadCurrent(int16_t &currentMa)
{
  uint16_t rawCurrent = 0;
  if (!csd203ReadRegister16(CSD203_I2C_ADDR, CSD203_REG_CURRENT, rawCurrent)) {
    g_csd203Ready = false;
    return false;
  }

  currentMa = static_cast<int16_t>(rawCurrent);
  return true;
}
