#include "edgetx.h"
#include "batsenser.h"

#if defined(MODULE_BATTERY_SENSOR)

#include "csd203_driver.h"

#include "board.h"
#include "debug.h"
#include "myeeprom.h"
#include "delays_driver.h"
#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "timers_driver.h"
#include "servo_tester.h"
#include "pwm_measure.h"
#include "ppm_measure.h"
#include "sbus_measure.h"
#include "crsf_measure.h"
#include "sumd_measure.h"
#include "dji_rs_measure.h"
#include "mavlink_measure.h"
#include "logic_measure.h"
#include "dshot_tester.h"
#include "auto_measure.h"

namespace {

constexpr uint8_t BATSENSER_CELLS_MAX = 6;
constexpr uint8_t BATSENSER_SYS_CH = 6;
constexpr uint16_t BATSENSER_PRESENT_THRESHOLD = 1200;
constexpr uint16_t BATSENSER_VOLTAGE_COMPENSATION = 40;
constexpr uint16_t SYSSENSER_VOLTAGE_COMPENSATION = 90;

uint16_t g_cellVoltage[BATSENSER_CELLS_MAX] = {0};
uint16_t g_systemVoltage = 0;
int16_t g_systemCurrentMa = 0;

uint8_t g_detectedCells = 0;
uint8_t g_nextMuxChannel = 0;
tmr10ms_t g_lastScanTick = 0;

bool g_popupArmed = true;
bool g_popupPending = false;

bool g_systemVoltagestep = false;

void setBatterySensorPath(bool enable)
{
#if defined(CHIP_FUN_GPIO)
  // CHIP_FUN: 0 -> VBAT sensor path, 1 -> UART path
  gpio_write(CHIP_FUN_GPIO, enable ? 0 : 1);
#else
  (void)enable;
#endif
}

void selectMuxChannel(uint8_t channel)
{  // HC138 MUX channel select is active high, 3 bits on CS1-CS3
  gpio_write(CHIP_CS1_GPIO, (channel & 0x01) ? 1 : 0);
  gpio_write(CHIP_CS2_GPIO, (channel & 0x02) ? 1 : 0);
  gpio_write(CHIP_CS3_GPIO, (channel & 0x04) ? 1 : 0);
}

uint8_t detectCells()
{
  uint8_t cells = 0;
  for (uint8_t i = 0; i < BATSENSER_CELLS_MAX; ++i) {
    if (g_cellVoltage[i] > BATSENSER_PRESENT_THRESHOLD) {
      ++cells;
    } else {
      break;
    }
  }
  return cells;
}

}  // namespace

static void batterySensorReset(bool armPopup)
{
  g_systemVoltage = 0;
  g_systemCurrentMa = 0;
  g_detectedCells = 0;
  g_nextMuxChannel = 0;
  g_lastScanTick = get_tmr10ms();
  g_popupArmed = armPopup;
  g_popupPending = false;
  g_systemVoltagestep = false;

  for (uint8_t i = 0; i < BATSENSER_CELLS_MAX; ++i) {
    g_cellVoltage[i] = 0;
  }

  csd203DriverInit();

  setBatterySensorPath(true);
  selectMuxChannel(0);
}

void v15BatterySensorInit(void)
{
  batterySensorReset(true);
}

void v15BatterySensorResumeAfterExtPort(void)
{
  // Auto Measure (and other EXT tools) stop/start children often. Child Stop()
  // must not reclaim the EXT battery mux while another EXT session still owns
  // CHIP_FUN / CS pins.
  if (v15ServoTesterIsActive() || v15PwmMeasureIsActive() ||
      v15PpmMeasureIsActive() || v15SbusMeasureIsActive() ||
      v15CrsfMeasureIsActive() || v15DshotTesterIsActive() ||
      v15SumdMeasureIsActive() ||
      v15DjiRsMeasureIsActive() ||
      v15MavlinkMeasureIsActive() || v15LogicMeasureIsActive() || v15AutoMeasureIsActive()) {
    return;
  }

  // Keep last EXT sense readings until fresh samples arrive (charge / 6S UI).
  // TX main battery alarm uses ADC and is unaffected.
  const uint16_t keepSys = g_systemVoltage;
  const int16_t keepCur = g_systemCurrentMa;
  // EXT +5V decays slowly; do not arm 6S popup until cell1 has gone absent.
  batterySensorReset(false);
  g_systemVoltage = keepSys;
  g_systemCurrentMa = keepCur;
}

uint16_t Getbatsenservol(void) //1-6 for cells, 7 for system voltage
{
  uint16_t voltageMv = 0;
  if (!csd203DriverEnsureReady() || !csd203DriverReadVoltage(voltageMv)) {
    csd203DriverInvalidate();
    return 0;
  }
  return voltageMv;
}
int16_t Getbatsensercurrent(void) //system current in mA
{
  int16_t currentMa = 0;
  if (!csd203DriverEnsureReady() || !csd203DriverReadCurrent(currentMa)) {
    csd203DriverInvalidate();
    return 0;
  }

  return currentMa;
}

void v15BatterySensorTask(void)
{
  if (v15ServoTesterIsActive() || v15PwmMeasureIsActive() ||
      v15PpmMeasureIsActive() || v15SbusMeasureIsActive() ||
      v15CrsfMeasureIsActive() || v15DshotTesterIsActive() ||
      v15SumdMeasureIsActive() ||
      v15DjiRsMeasureIsActive() ||
      v15MavlinkMeasureIsActive() || v15LogicMeasureIsActive() || v15AutoMeasureIsActive()) {
    return;
  }

  tmr10ms_t now = get_tmr10ms();
  if ((tmr10ms_t)(now - g_lastScanTick) < 1) {
    return;
  }
  g_lastScanTick = now;

  if (/*g_model.Bat6sInterfaceFunction*/ 0 != BAT6S_INTERFACE_FUNCTION_BATTERY_6S) {
    g_nextMuxChannel = BATSENSER_SYS_CH;  //only read system voltage 
    setBatterySensorPath(false);
  } else {
    setBatterySensorPath(true);
  }
  if (g_systemVoltagestep==false)  {
    g_systemVoltagestep=true;
    selectMuxChannel(g_nextMuxChannel);//read system voltage to trigger voltage update in next loop, as voltage read is slower than current read  

    const int16_t current = Getbatsensercurrent(); //read current to trigger voltage update in next loop, as current read is faster than voltage read
    g_systemCurrentMa = current;
    //TRACE("current=%d", current);
    return;
  }
  g_systemVoltagestep=false;
  
  const uint8_t currentChannel = g_nextMuxChannel;
  const uint16_t voltage = Getbatsenservol();
  
  //TRACE("Voltage=%d %d",g_nextMuxChannel, voltage);

  if (currentChannel < BATSENSER_CELLS_MAX) { //BATSENSER_CELLS_MAX = 6
    g_cellVoltage[currentChannel] = voltage;
    //TRACE("cellVoltage[%d]=%d", currentChannel, g_cellVoltage[currentChannel]);
    if (voltage > BATSENSER_PRESENT_THRESHOLD &&
        currentChannel < (BATSENSER_CELLS_MAX - 1)) { //BATSENSER_PRESENT_THRESHOLD = 330
      g_nextMuxChannel = currentChannel + 1;
    } else {
      g_nextMuxChannel = BATSENSER_SYS_CH;  //BATSENSER_SYS_CH = 6
    }
  } else {
    g_systemVoltage = voltage + SYSSENSER_VOLTAGE_COMPENSATION;
    g_nextMuxChannel = 0;
   // TRACE("g_systemVoltage=%d", g_systemVoltage);
  }

  g_detectedCells = detectCells();

  if (!v15BatteryCell1Present()) {
    g_popupArmed = true;
    g_popupPending = false;
  } else if (g_popupArmed) {
    g_popupPending = true;
    g_popupArmed = false;
  }
}

uint16_t v15BatteryCellVoltage(uint8_t cell_num)
{
  if (cell_num < 1 || cell_num > BATSENSER_CELLS_MAX) {
    return 0;
  }

  uint16_t voltageMv = g_cellVoltage[cell_num - 1];
  if (cell_num>1)
  {
    uint16_t voltageMv1 = g_cellVoltage[cell_num - 2];
    
    if(voltageMv > voltageMv1) {
      voltageMv = voltageMv - voltageMv1;
    }
  }
  if (voltageMv > BATSENSER_PRESENT_THRESHOLD)
    voltageMv += BATSENSER_VOLTAGE_COMPENSATION;
  
  return voltageMv; 
}

// TX main battery (low-voltage alarm / bars) uses ADC via weak
// getBatteryVoltage() in battery_voltage.cpp â€?do not override it here.
// g_systemVoltage is EXT CSD203 pack/system sense for charge / 6S UI only.

uint16_t v15BatterySystemVoltage(void)
{
  return g_systemVoltage;
}

int16_t v15BatterySystemCurrent(void)
{
  return g_systemCurrentMa;
}

uint8_t v15BatteryDetectedCells(void)
{
  return g_detectedCells;
}

bool v15BatteryCell1Present(void)
{
  return g_cellVoltage[0] > BATSENSER_PRESENT_THRESHOLD;
}

bool v15BatteryUiTriggerPending(void)
{
  return g_popupPending;
}

void v15BatteryUiAcknowledgeTrigger(void)
{
  g_popupPending = false;
}

#else

// Stub implementations when MODULE_BATTERY_SENSOR is disabled
void v15BatterySensorInit(void) {}
void v15BatterySensorResumeAfterExtPort(void) {}
void v15BatterySensorTask(void) {}
uint16_t Getbatsenservol(void) { return 0; }
int16_t Getbatsensercurrent(void) { return 0; }
uint16_t v15BatteryCellVoltage(uint8_t cell_num) { return 0; }
uint16_t v15BatterySystemVoltage(void) { return 0; }
int16_t v15BatterySystemCurrent(void) { return 0; }
uint8_t v15BatteryDetectedCells(void) { return 0; }
bool v15BatteryCell1Present(void) { return false; }
bool v15BatteryUiTriggerPending(void) { return false; }
void v15BatteryUiAcknowledgeTrigger(void) {}

#endif  // MODULE_BATTERY_SENSOR
