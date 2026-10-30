// SPDX-License-Identifier: GPL-2.0-only
#include "board.h"
#include "calibration.h"
#include "diagnostics.h"
#include "ff.h"
#include "gimbals.h"
#include "hal/adc_driver.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"
#include "timers_driver.h"

namespace
{
mambo::HallSensor sensors[2];
mambo::FactoryCalibration factoryCalibration;
volatile MamboCalibrationStatus calibrationStatus =
    MamboCalibrationStatus::Missing;
mambo::BenchDiagnostics benchDiagnostics;
// Tracer 6.19 transfer routine 0x080c4974 selects PE8 for sensor 0 and
// PD7 for sensor 1. Keep this order aligned with calibration and DRDY.
const gpio_t selectPins[] = {GPIO_PIN(GPIOE, 8), GPIO_PIN(GPIOD, 7)};
const gpio_t readyPins[] = {GPIO_PIN(GPIOC, 5), GPIO_PIN(GPIOB, 9)};

void busInit()
{
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_SPI2);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_SPI2);
  gpio_init_af(GPIO_PIN(GPIOB, 10), GPIO_AF5, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(GPIO_PIN(GPIOC, 2), GPIO_AF5, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(GPIO_PIN(GPIOC, 3), GPIO_AF5, GPIO_PIN_SPEED_HIGH);
  // APB1=42 MHz / 16 = 2.625 MHz. Mode 3, 8-bit, software NSS, master.
  SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL |
              SPI_CR1_CPHA | SPI_CR1_BR_1 | SPI_CR1_BR_0;
  SPI2->CR1 |= SPI_CR1_SPE;
}

bool transfer(unsigned sensor, const uint8_t* command, size_t commandSize,
              uint8_t* response, size_t responseSize)
{
  // One deadline bounds the entire transaction, including BSY. DWT runs even
  // if a peripheral stalls; no unbounded loops or conversion-delay sleeps.
  const uint32_t start = DWT->CYCCNT;
  const uint32_t budget = CPU_FREQ / 10000;  // 100 us
  gpio_clear(selectPins[sensor]);
  bool ok = true;
  for (size_t i = 0; i < commandSize + responseSize; ++i) {
    while (!(SPI2->SR & SPI_SR_TXE)) {
      if (uint32_t(DWT->CYCCNT - start) >= budget) {
        ok = false;
        break;
      }
    }
    if (!ok) break;
    *reinterpret_cast<volatile uint8_t*>(&SPI2->DR) =
        i < commandSize ? command[i] : 0;
    while (!(SPI2->SR & SPI_SR_RXNE)) {
      if (uint32_t(DWT->CYCCNT - start) >= budget) {
        ok = false;
        break;
      }
    }
    if (!ok) break;
    uint8_t byte = *reinterpret_cast<volatile uint8_t*>(&SPI2->DR);
    if (i >= commandSize) response[i - commandSize] = byte;
  }
  while (ok && (SPI2->SR & SPI_SR_BSY)) {
    if (uint32_t(DWT->CYCCNT - start) >= budget) ok = false;
  }
  gpio_set(selectPins[sensor]);
  if (!ok) busInit();  // discard partial transfer/overrun before another sensor
  return ok;
}
}  // namespace

void mamboGimbalsInit()
{
  benchDiagnostics = {};
  benchDiagnostics.resetFlags = RCC->CSR & 0xfe000000u;
  // Timeout clock is deliberately independent of FreeRTOS and timer IRQs.
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  for (unsigned i = 0; i < 2; ++i) {
    gpio_set(selectPins[i]);
    gpio_init(selectPins[i], GPIO_OUT, GPIO_PIN_SPEED_HIGH);
    gpio_init(readyPins[i], GPIO_IN_PD, GPIO_PIN_SPEED_LOW);
    sensors[i].begin(timersGetUsTick());
  }
  busInit();
}

void mamboGimbalsDeinit()
{
  for (unsigned i = 0; i < 2; ++i) {
    gpio_set(selectPins[i]);
    sensors[i].begin(timersGetUsTick());
  }
  SPI2->CR1 &= ~SPI_CR1_SPE;
}

void mamboGimbalsRead()
{
  uint32_t now = timersGetUsTick();
  uint16_t axes[4] = {2048, 2048, 2048, 2048};
  for (unsigned i = 0; i < 2; ++i) {
    sensors[i].poll(i, now, gpio_read(readyPins[i]), transfer);
    uint16_t x, y;
    mambo::GimbalCorrection correction;
    // SD mounting runs outside the acquisition task. Snapshot publication and
    // reads under a short IRQ mask; no file I/O or math with IRQs masked.
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    bool loaded = calibrationStatus == MamboCalibrationStatus::Loaded;
    if (loaded) correction = factoryCalibration.gimbals[i];
    __set_PRIMASK(mask);
    bool valid =
        sensors[i].fresh(now) &&
        (loaded ? mambo::calibratedHall(sensors[i].sample(), correction, x, y)
                : mambo::hallAngles(sensors[i].sample(), x, y));
    if (valid) {
      // RF publishes its intermediate X/Y/X/Y as LH/LV/RV/RH.
      axes[i ? 3 : 0] = x;
      axes[i ? 2 : 1] = y;
    }
  }
  for (unsigned i = 0; i < 4; ++i) setAnalogValue(i, axes[i]);
  uint32_t duration = timersGetUsTick() - now;
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  for (unsigned i = 0; i < 2; ++i)
    benchDiagnostics.sensors[i] = sensors[i].sample();
  for (unsigned i = 0; i < 4; ++i) benchDiagnostics.sticks[i] = axes[i];
  ++benchDiagnostics.frames;
  benchDiagnostics.lastDurationUs = duration;
  if (duration > benchDiagnostics.maxDurationUs)
    benchDiagnostics.maxDurationUs = duration;
  benchDiagnostics.calibration = calibrationStatus;
  __set_PRIMASK(mask);
}

const mambo::HallSample& mamboGimbalSample(unsigned sensor)
{
  return sensors[sensor < 2 ? sensor : 0].sample();
}

void mamboGimbalsLoadCalibration()
{
  FIL file;
  uint8_t bytes[604];
  UINT count = 0;
  mambo::FactoryCalibration candidate;
  auto status = MamboCalibrationStatus::Missing;
  // FA_READ only. Never create, update or repair the original Tracer backup.
  auto result = f_open(&file, "/CROSSFIRE/EEPROM.bin", FA_READ);
  if (result == FR_OK) {
    result = f_read(&file, bytes, sizeof(bytes), &count);
    auto closeResult = f_close(&file);
    status = result == FR_OK && closeResult == FR_OK &&
                     count == sizeof(bytes) &&
                     mambo::decodeFactoryCalibration(bytes, count, candidate)
                 ? MamboCalibrationStatus::Loaded
                 : MamboCalibrationStatus::Invalid;
  } else if (result != FR_NO_FILE && result != FR_NO_PATH) {
    status = MamboCalibrationStatus::Invalid;
  }
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  if (status == MamboCalibrationStatus::Loaded) factoryCalibration = candidate;
  calibrationStatus = status;
  __set_PRIMASK(mask);
}

MamboCalibrationStatus mamboGimbalsCalibrationStatus()
{
  return calibrationStatus;
}

mambo::BenchDiagnostics mamboBenchDiagnostics()
{
  uint32_t mask = __get_PRIMASK();
  __disable_irq();
  auto result = benchDiagnostics;
  result.calibration = calibrationStatus;
  __set_PRIMASK(mask);
  return result;
}
