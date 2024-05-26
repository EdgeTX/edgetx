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

#if !defined(SIMU)
#include "stm32_ws2812.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#endif

#include "edgetx.h"
#include "io/frsky_firmware_update.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "hal/storage.h"
#include "hal/watchdog_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/usb_driver.h"

#include "timers_driver.h"

#include "switches.h"
#include "inactivity_timer.h"
#include "input_mapping.h"
#include "trainer.h"

#include "tasks.h"
#include "tasks/mixer_task.h"

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
#endif

#if defined(LIBOPENUI)
  #include "libopenui.h"
  #include "radio_calibration.h"
  #include "view_main.h"
  #include "view_text.h"
  #include "theme_manager.h"
  #include "switch_warn_dialog.h"
  #include "startup_shutdown.h"

  #include "LvglWrapper.h"
#endif

#if defined(CROSSFIRE)
#include "telemetry/crossfire.h"
#endif

#if defined(CSD203_SENSOR)
  #include "csd203_sensor.h"
#endif

#if !defined(SIMU)
#include <malloc.h>
#endif

RadioData  g_eeGeneral;
ModelData  g_model;

Clipboard clipboard;

GlobalData globalData;

uint32_t maxMixerDuration; // microseconds

constexpr uint8_t HEART_TIMER_10MS = 0x01;
uint8_t heartbeat;

#if defined(OVERRIDE_CHANNEL_FUNCTION)
safetych_t safetyCh[MAX_OUTPUT_CHANNELS];
#endif

// __DMA for the MSC_BOT_Data member
union ReusableBuffer reusableBuffer __DMA;

#if !defined(SIMU)
uint8_t* MSC_BOT_Data = reusableBuffer.MSC_BOT_Data;
#endif

#if defined(DEBUG_LATENCY)
uint8_t latencyToggleSwitch = 0;
#endif

volatile uint8_t rtc_count = 0;

#if defined(DEBUG_LATENCY)
void toggleLatencySwitch()
{
  latencyToggleSwitch ^= 1;

#if defined(PCBHORUS)
  if (latencyToggleSwitch)
    gpio_clear(EXTMODULE_TX_GPIO);
  else
    gpio_set(EXTMODULE_TX_GPIO);
#else
  if (latencyToggleSwitch)
    sportUpdatePowerOn();
  else
    sportUpdatePowerOff();
#endif
}
#endif

void checkValidMCU(void)
{
#if !defined(SIMU) && !defined(BOOT)
  // Checks the radio MCU type matches intended firmware type
  uint32_t idcode = DBGMCU->IDCODE & 0xFFF;

#if defined(RADIO_TLITE)
  #define TARGET_IDCODE_SECONDARY   0x413
  // Tlite ELRS have a CKS F4 run as an F2 (F4 firmware won't run on those)
#endif

#if defined(STM32F205xx)
  #define TARGET_IDCODE   0x411
#elif defined(STM32F407xx)
  #define TARGET_IDCODE   0x413
#elif defined(STM32F429xx)
  #define TARGET_IDCODE   0x419
#elif defined(STM32F413xx)
  #define TARGET_IDCODE   0x463
#elif defined(STM32H750xx) || defined(STM32H747xx)
  #define TARGET_IDCODE   0x450
#elif defined(STM32H7RS)
  #define TARGET_IDCODE   0x485
#else
  // Ensure new radio get registered :)
  #warning "Target MCU code undefined"
  #define TARGET_IDCODE   0x0
#endif

#if defined(TARGET_IDCODE_SECONDARY)
  if(idcode != TARGET_IDCODE && idcode != TARGET_IDCODE_SECONDARY) {
    runFatalErrorScreen("Wrong MCU");
  }
#else
  if(idcode != TARGET_IDCODE) {
    runFatalErrorScreen("Wrong MCU");
  }
#endif
#endif
}

void timer_10ms()
{
  DEBUG_TIMER_START(debugTimerPer10ms);
  DEBUG_TIMER_SAMPLE(debugTimerPer10msPeriod);

  g_tmr10ms++;

#if defined(GUI)
  if (lightOffCounter) lightOffCounter--;
  if (flashCounter) flashCounter--;
#if !defined(LIBOPENUI)
  if (noHighlightCounter) noHighlightCounter--;
#endif
#endif

  if (trimsCheckTimer) trimsCheckTimer--;
  trainerDecTimer();

  if (trimsDisplayTimer)
    trimsDisplayTimer--;
  else
    trimsDisplayMask = 0;

#if defined(DEBUG_LATENCY_END_TO_END)
  static tmr10ms_t lastLatencyToggle = 0;
  if (g_tmr10ms - lastLatencyToggle == 10) {
    lastLatencyToggle = g_tmr10ms;
    toggleLatencySwitch();
  }
#endif

#if defined(RTCLOCK)
  /* Update global Date/Time every 100 per10ms cycles */
  if (++g_ms100 == 100) {
    g_rtcTime++;   // inc global unix timestamp one second
    g_ms100 = 0;
  }
#endif

  if (keysPollingCycle()) {
    inactivityTimerReset(ActivitySource::Keys);
  }

#if defined(FUNCTION_SWITCHES)
  evalFunctionSwitches();
#endif

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(LIBOPENUI)
  if (rotaryEncoderPollingCycle()) {
    inactivityTimerReset(ActivitySource::Keys);
  }
#endif

#if defined(CSD203_SENSOR) && !defined(SIMU)
  readCSD203();
#endif

  telemetryInterrupt10ms();

  // These moved here from evalFlightModeMixes() to improve beep trigger reliability.
#if !defined(AUDIO)
  if (mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) AUDIO_MIX_WARNING(1);
  if (mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) AUDIO_MIX_WARNING(2);
  if (mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) AUDIO_MIX_WARNING(3);
#endif

  outputTelemetryBuffer.per10ms();

  heartbeat |= HEART_TIMER_10MS;

  DEBUG_TIMER_STOP(debugTimerPer10ms);
}

#if !defined(SIMU)
// Handle 10ms timer asynchronously
#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

static void _timer_10ms_cb(void *pvParameter1, uint32_t ulParameter2)
{
  (void)pvParameter1;
  (void)ulParameter2;
  timer_10ms();
}

void per10ms()
{
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTimerPendFunctionCallFromISR(_timer_10ms_cb, nullptr, 0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

#else // !defined(SIMU)
void per10ms() { timer_10ms(); }
#endif


FlightModeData *flightModeAddress(uint8_t idx)
{
  return &g_model.flightModeData[idx];
}

ExpoData *expoAddress(uint8_t idx )
{
  return &g_model.expoData[idx];
}

LimitData *limitAddress(uint8_t idx)
{
  return &g_model.limitData[idx];
}

USBJoystickChData *usbJChAddress(uint8_t idx)
{
  return &g_model.usbJoystickCh[idx];
}


void memswap(void * a, void * b, uint8_t size)
{
  uint8_t * x = (uint8_t *)a;
  uint8_t * y = (uint8_t *)b;
  uint8_t temp ;

  while (size--) {
    temp = *x;
    *x++ = *y;
    *y++ = temp;
  }
}

#if defined(PXX2)
void setDefaultOwnerId()
{
  uint8_t ch;
  for (uint8_t i = 0; i < PXX2_LEN_REGISTRATION_ID; i++) {
    ch = ((uint8_t *)cpu_uid)[4+i]&0x7f;
    if(ch<0x20 || ch==0x7f) ch='-';
    g_eeGeneral.ownerRegistrationID[PXX2_LEN_REGISTRATION_ID-1-i] = ch;
  }
}
#endif

void generalDefault()
{
  memclear(&g_eeGeneral, sizeof(g_eeGeneral));

#if defined(COLORLCD)
  g_eeGeneral.blOffBright = 20;
#endif

#if defined(LCD_CONTRAST_DEFAULT)
  g_eeGeneral.contrast = LCD_CONTRAST_DEFAULT;
#endif

#if defined(LCD_BRIGHTNESS_DEFAULT)
  g_eeGeneral.backlightBright = LCD_BRIGHTNESS_DEFAULT;
#endif

#if defined(DEFAULT_INTERNAL_MODULE)
    g_eeGeneral.internalModule = DEFAULT_INTERNAL_MODULE;
    if (g_eeGeneral.internalModule == MODULE_TYPE_CROSSFIRE)
      g_eeGeneral.internalModuleBaudrate = min(1, (int)CROSSFIRE_MAX_INTERNAL_BAUDRATE);  // 921k if possible
#endif

  adcCalibDefaults();

  g_eeGeneral.potsConfig = adcGetDefaultPotsConfig();
  g_eeGeneral.switchConfig = switchGetDefaultConfig();

#if defined(STICK_DEAD_ZONE)
  g_eeGeneral.stickDeadZone = DEFAULT_STICK_DEADZONE;
#endif

  // vBatWarn is voltage in 100mV, vBatMin is in 100mV but with -9V offset,
  // vBatMax has a -12V offset
  g_eeGeneral.vBatWarn = BATTERY_WARN;
  if (BATTERY_MIN != 90)
    g_eeGeneral.vBatMin = BATTERY_MIN - 90;
  if (BATTERY_MAX != 120)
    g_eeGeneral.vBatMax = BATTERY_MAX - 120;

#if defined(SURFACE_RADIO)
  g_eeGeneral.stickMode = 0;
  g_eeGeneral.templateSetup = 0;
#elif defined(DEFAULT_MODE)
  g_eeGeneral.stickMode = DEFAULT_MODE - 1;
  g_eeGeneral.templateSetup = DEFAULT_TEMPLATE_SETUP;
#endif


  g_eeGeneral.backlightMode = e_backlight_mode_all;
  g_eeGeneral.lightAutoOff = 2;
  g_eeGeneral.inactivityTimer = 10;

  g_eeGeneral.ttsLanguage[0] = 'e';
  g_eeGeneral.ttsLanguage[1] = 'n';
  g_eeGeneral.wavVolume = 2;
  g_eeGeneral.backgroundVolume = 1;

  auto controls = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = 0; i < controls; ++i) {
    g_eeGeneral.trainer.mix[i].mode = 2;
    g_eeGeneral.trainer.mix[i].srcChn = inputMappingChannelOrder(i);
    g_eeGeneral.trainer.mix[i].studWeight = 100;
  }

#if defined(PCBX9E)
  const int8_t defaultName[] = { 20, -1, -18, -1, -14, -9, -19 };
  memcpy(g_eeGeneral.bluetoothName, defaultName, sizeof(defaultName));
#endif

#if defined(STORAGE_MODELSLIST)
  strcpy(g_eeGeneral.currModelFilename, DEFAULT_MODEL_FILENAME);
#endif

#if defined(PXX2)
  setDefaultOwnerId();
#endif

#if defined(RADIOMASTER_RTF_RELEASE)
  // Those settings are for headless radio
  g_eeGeneral.USBMode = USB_JOYSTICK_MODE;
  g_eeGeneral.disableRtcWarning = 1;
  g_eeGeneral.splashMode = 3; // Disable splash
  g_eeGeneral.pwrOnSpeed = 1; // 1 second
#endif

#if defined(IFLIGHT_RELEASE)
  g_eeGeneral.splashMode = 3;
  g_eeGeneral.pwrOnSpeed = 2;
  g_eeGeneral.pwrOffSpeed = 2;
#endif

#if defined(MANUFACTURER_RADIOMASTER)
  g_eeGeneral.audioMuteEnable = 1;
#endif

  // disable Custom Script
  g_eeGeneral.modelCustomScriptsDisabled = true;

#if defined(USE_HATS_AS_KEYS)
  g_eeGeneral.hatsMode = HATSMODE_SWITCHABLE;
#endif

  g_eeGeneral.chkSum = 0xFFFF;
}

uint16_t evalChkSum()
{
  uint16_t sum = 0;
  auto main_calib_bytes = adcGetMaxInputs(ADC_INPUT_MAIN) * sizeof(CalibData);

  const uint8_t *calibValues = (const uint8_t *)&g_eeGeneral.calib[0];
  for (unsigned i = 0; i < main_calib_bytes; i++) {
    sum += calibValues[i];
  }
  return sum;
}

bool isInputRecursive(int index)
{
  ExpoData * line = expoAddress(0);
  for (int i=0; i<MAX_EXPOS; i++, line++) {
    if (line->chn > index)
      break;
    else if (line->chn < index)
      continue;
    else if (line->srcRaw >= MIXSRC_FIRST_LOGICAL_SWITCH)
      return true;
  }
  return false;
}

#if defined(AUTOSOURCE)
constexpr int MULTIPOS_STEP_SIZE = (2 * RESX) / XPOTS_MULTIPOS_COUNT;

int8_t getMovedSource(uint8_t min)
{
  int8_t result = 0;
  static tmr10ms_t s_move_last_time = 0;

  static int16_t inputsStates[MAX_INPUTS];
  if (min <= MIXSRC_FIRST_INPUT) {
    for (uint8_t i = 0; i < MAX_INPUTS; i++) {
      if (abs(anas[i] - inputsStates[i]) > MULTIPOS_STEP_SIZE) {
        if (!isInputRecursive(i)) {
          result = MIXSRC_FIRST_INPUT + i;
          break;
        }
      }
    }
  }

  static int16_t sourcesStates[MAX_ANALOG_INPUTS];
  if (result == 0) {
    for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
      if (abs(calibratedAnalogs[i] - sourcesStates[i]) > MULTIPOS_STEP_SIZE) {
        auto offset = adcGetInputOffset(ADC_INPUT_FLEX);
        if (i >= offset) {
          result = MIXSRC_FIRST_POT + i - offset;
          break;
        }
        result = MIXSRC_FIRST_STICK + inputMappingConvertMode(i);
        break;
      }
    }
  }

  bool recent = ((tmr10ms_t)(get_tmr10ms() - s_move_last_time) > 10);
  if (recent) {
    result = 0;
  }

  if (result || recent) {
    memcpy(inputsStates, anas, sizeof(inputsStates));
    memcpy(sourcesStates, calibratedAnalogs, sizeof(sourcesStates));
  }

  s_move_last_time = get_tmr10ms();
  return result;
}
#endif

#if defined(FLIGHT_MODES)
uint8_t getFlightMode()
{
  for (uint8_t i=1; i<MAX_FLIGHT_MODES; i++) {
    FlightModeData *phase = &g_model.flightModeData[i];
    if (phase->swtch && getSwitch(phase->swtch)) {
      return i;
    }
  }
  return 0;
}
#endif

trim_t getRawTrimValue(uint8_t phase, uint8_t idx)
{
  FlightModeData * p = flightModeAddress(phase);
  return p->trim[idx];
}

int getTrimValue(uint8_t phase, uint8_t idx)
{
  int result = 0;
  for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
    trim_t v = getRawTrimValue(phase, idx);
    if (v.mode == TRIM_MODE_NONE || v.mode == TRIM_MODE_3POS) {
      return result;
    }
    else {
      unsigned int p = v.mode >> 1;
      if (p == phase || phase == 0) {
        return result + v.value;
      }
      else {
        phase = p;
        if (v.mode % 2 != 0) {
          result += v.value;
        }
      }
    }
  }
  return 0;
}

bool setTrimValue(uint8_t phase, uint8_t idx, int trim)
{
  for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
    trim_t & v = flightModeAddress(phase)->trim[idx];
    if (v.mode == TRIM_MODE_NONE || v.mode == TRIM_MODE_3POS)
      return false;
    unsigned int p = v.mode >> 1;
    if (p == phase || phase == 0) {
      v.value = trim;
      break;
    }
    else if (v.mode % 2 == 0) {
      phase = p;
    }
    else {
      v.value = limit<int>(TRIM_EXTENDED_MIN, trim - getTrimValue(p, idx), TRIM_EXTENDED_MAX);
      break;
    }
  }
  storageDirty(EE_MODEL);
  return true;
}

getvalue_t convert16bitsTelemValue(source_t channel, ls_telemetry_value_t value)
{
  return value;
}

ls_telemetry_value_t maxTelemValue(source_t channel)
{
  return 30000;
}

void checkBacklight()
{
  static uint8_t tmr10ms ;

  uint8_t x = g_blinkTmr10ms;
  if (tmr10ms != x) {
    tmr10ms = x;
    if (inactivityCheckInputs()) {
      inactivityTimerReset(ActivitySource::MainControls);
    }

    if (requiredBacklightBright == BACKLIGHT_FORCED_ON) {
      currentBacklightBright = g_eeGeneral.getBrightness();
      BACKLIGHT_ENABLE();
    } else {
      bool backlightOn = ((g_eeGeneral.backlightMode == e_backlight_mode_on) ||
                          (g_eeGeneral.backlightMode != e_backlight_mode_off &&
                           lightOffCounter) ||
                          (g_eeGeneral.backlightMode == e_backlight_mode_off &&
                           isFunctionActive(FUNCTION_BACKLIGHT)));

      if (flashCounter) {
        backlightOn = !backlightOn;
      }
      if (backlightOn) {
        currentBacklightBright = requiredBacklightBright;
        BACKLIGHT_ENABLE();
      } else {
        BACKLIGHT_DISABLE();
      }
    }
  }
}

void resetBacklightTimeout()
{
  uint16_t autoOff = g_eeGeneral.lightAutoOff;
#if defined(COLORLCD)
  // prevent the timeout from being 0 seconds on color lcd radios
  autoOff = std::max<uint16_t>(1, autoOff);
#endif
  lightOffCounter = (autoOff*250) << 1;
}


#if defined(MULTIMODULE)
void checkMultiLowPower()
{
  bool low_power_warning = false;
  for (uint8_t i = 0; i < MAX_MODULES; i++) {
    if (isModuleMultimodule(i) &&
        g_model.moduleData[i].multi.lowPowerMode) {
      low_power_warning = true;
    }
  }

  if (low_power_warning) {
    ALERT("MULTI", STR_WARN_MULTI_LOWPOWER, AU_ERROR);
  }
}
#endif

static void checkRTCBattery()
{
  if (!mixerTaskRunning()) getADC();
  if (getRTCBatteryVoltage() < 200) {
    ALERT(STR_BATTERY, STR_WARN_RTC_BATTERY_LOW, AU_ERROR);
  }
}

void checkSDfreeStorage() {
  if(sdIsFull()) {
    ALERT(STR_SD_CARD, STR_SDCARD_FULL, AU_ERROR);
  }
}

#if defined(PCBFRSKY) || defined(PCBFLYSKY)
static void checkFailsafe()
{
  for (int i=0; i<NUM_MODULES; i++) {
#if defined(MULTIMODULE)
    // use delayed check for MPM
    if (isModuleMultimodule(i)) break;
#endif
    if (isModuleFailsafeAvailable(i)) {
      ModuleData & moduleData = g_model.moduleData[i];
      if (moduleData.failsafeMode == FAILSAFE_NOT_SET) {
        ALERT(STR_FAILSAFEWARN, STR_NO_FAILSAFE, AU_ERROR);
        break;
      }
    }
  }
}
#else
#define checkFailsafe()
#endif

#if defined(GUI)
void checkAll(bool isBootCheck)
{
  checkSDfreeStorage();
  
  // we don't check the throttle stick if the radio is not calibrated
  if (g_eeGeneral.chkSum == evalChkSum()) {
    checkThrottleStick();
  }

  checkSwitches();
  checkFailsafe();

  if (isBootCheck && !g_eeGeneral.disableRtcWarning) {
    // only done once at board start
    enableVBatBridge();
    checkRTCBattery();
  }
  disableVBatBridge();

  if (g_model.displayChecklist && modelHasNotes()) {
    cancelSplash();
#if defined(COLORLCD)
    readChecklist();
#else
    readModelNotes();
#endif
  }

#if defined(MULTIMODULE)
  checkMultiLowPower();
#endif

#if defined(COLORLCD)
  if (!waitKeysReleased()) {
    auto dlg = new FullScreenDialog(WARNING_TYPE_ALERT, STR_KEYSTUCK);
    LED_ERROR_BEGIN();
    AUDIO_ERROR_MESSAGE(AU_ERROR);

    tmr10ms_t tgtime = get_tmr10ms() + 500;
    uint32_t keys = readKeys();

    std::string strKeys;
    for (int i = 0; i < (int)MAX_KEYS; i++) {
      if (keys & (1 << i)) {
        strKeys += std::string(keysGetLabel(EnumKeys(i)));
      }
    }

    dlg->setMessage(strKeys.c_str());
    dlg->setCloseCondition([tgtime]() {
      if (tgtime >= get_tmr10ms() && keyDown()) {
        return false;
      } else {
        return true;
      }
    });
    dlg->runForever();
    LED_ERROR_END();
  }
#else
  if (!waitKeysReleased()) {
    showMessageBox(STR_KEYSTUCK);
    tmr10ms_t tgtime = get_tmr10ms() + 500;
    while (tgtime != get_tmr10ms()) {
      RTOS_WAIT_MS(1);
      WDG_RESET();
    }
  }
#endif

#if defined(EXTERNAL_ANTENNA) && defined(INTERNAL_MODULE_PXX1)
  checkExternalAntenna();
#endif

  START_SILENCE_PERIOD();
}
#endif // GUI

bool isThrottleWarningAlertNeeded()
{
  if (g_model.disableThrottleWarning) {
    return false;
  }

  uint8_t thr_src = throttleSource2Source(g_model.thrTraceSrc);

  // in case an output channel is choosen as throttle source
  // we assume the throttle stick is the input (no computed channels yet)
  if (thr_src >= MIXSRC_FIRST_CH) {
    thr_src = throttleSource2Source(0);
  }

  if (!mixerTaskRunning()) getADC();
  evalInputs(e_perout_mode_notrainer); // let do evalInputs do the job

  int16_t v = getValue(thr_src);

  // TODO: this looks fishy....
  if (g_model.thrTraceSrc && g_model.throttleReversed) {
    v = -v;
  }

  if (g_model.enableCustomThrottleWarning) {
    int16_t idleValue = (int32_t)RESX *
                        (int32_t)g_model.customThrottleWarningPosition /
                        (int32_t)100;
    return abs(v - idleValue) > THRCHK_DEADBAND;
  } else {
#if defined(SURFACE_RADIO) // surface radio, stick centered
    return v > THRCHK_DEADBAND;
#else
    return v > THRCHK_DEADBAND - RESX;
#endif
  }
}

#if defined(COLORLCD)
void checkThrottleStick()
{
  char throttleNotIdle[strlen(STR_THROTTLE_NOT_IDLE) + 9];
  if (isThrottleWarningAlertNeeded()) {
    if (g_model.enableCustomThrottleWarning) {
    sprintf(throttleNotIdle, "%s (%d%%)", STR_THROTTLE_NOT_IDLE, g_model.customThrottleWarningPosition);
    }
    else {
      strcpy(throttleNotIdle, STR_THROTTLE_NOT_IDLE);
    }
    LED_ERROR_BEGIN();
    auto dialog = new ThrottleWarnDialog(throttleNotIdle);
    dialog->runForever();
  }
  LED_ERROR_END();
}
#else
void checkThrottleStick()
{
  char throttleNotIdle[strlen(STR_THROTTLE_NOT_IDLE) + 9];
  if (!isThrottleWarningAlertNeeded()) {
    return;
  }
  if (g_model.enableCustomThrottleWarning) {
    sprintf(throttleNotIdle, "%s (%d%%)", STR_THROTTLE_NOT_IDLE, g_model.customThrottleWarningPosition);
  }
  else {
    strcpy(throttleNotIdle, STR_THROTTLE_NOT_IDLE);
  }
  // first - display warning; also deletes inputs if any have been before
  LED_ERROR_BEGIN();
  RAISE_ALERT(TR_THROTTLE_UPPERCASE, throttleNotIdle, STR_PRESS_ANY_KEY_TO_SKIP, AU_THROTTLE_ALERT);

#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  while (!keyDown()) {
    if (!isThrottleWarningAlertNeeded()) {
      return;
    }

#if defined(PWR_BUTTON_PRESS)
    uint32_t power = pwrCheck();
    if (power == e_power_off) {
      drawSleepBitmap();
      boardOff();
      break;
    }
    else if (power == e_power_press) {
      refresh = true;
    }
    else if (power == e_power_on && refresh) {
      RAISE_ALERT(TR_THROTTLE_UPPERCASE, throttleNotIdle, STR_PRESS_ANY_KEY_TO_SKIP, AU_NONE);
      refresh = false;
    }
#else
    if (pwrCheck() == e_power_off) {
      break;
    }
#endif

    checkBacklight();

    WDG_RESET();

    RTOS_WAIT_MS(10);
  }

  LED_ERROR_END();
}
#endif

void checkAlarm() // added by Gohst
{
  if (g_eeGeneral.disableAlarmWarning) {
    return;
  }

  if (IS_SOUND_OFF()) {
    ALERT(STR_ALARMSWARN, STR_ALARMSDISABLED, AU_ERROR);
  }
}

void alert(const char * title, const char * msg , uint8_t sound)
{
  LED_ERROR_BEGIN();

  TRACE("ALERT %s: %s", title, msg);

  RAISE_ALERT(title, msg, STR_PRESSANYKEY, sound);

#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  while (true) {
    RTOS_WAIT_MS(10);

    if (getEvent())  // wait for key release
      break;

    checkBacklight();

    WDG_RESET();

    const uint32_t pwr_check = pwrCheck();
    if (pwr_check == e_power_off) {
      drawSleepBitmap();
      boardOff();
      return;   // only happens in SIMU, required for proper shutdown
    }
#if defined(PWR_BUTTON_PRESS)
    else if (pwr_check == e_power_press) {
      refresh = true;
    }
    else if (pwr_check == e_power_on && refresh) {
      RAISE_ALERT(title, msg, STR_PRESSANYKEY, AU_NONE);
      refresh = false;
    }
#endif
  }

  LED_ERROR_END();
}

#if defined(GVARS)
#if MAX_TRIMS == 8
  int8_t trimGvar[MAX_TRIMS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
#elif MAX_TRIMS == 6
  int8_t trimGvar[MAX_TRIMS] = { -1, -1, -1, -1, -1, -1 };
#elif MAX_TRIMS == 4
  int8_t trimGvar[MAX_TRIMS] = { -1, -1, -1, -1 };
#elif MAX_TRIMS == 2
  int8_t trimGvar[MAX_TRIMS] = { -1, -1 };
#endif
#endif

void checkTrims()
{
  event_t event = getTrimEvent();
  if (event && !IS_KEY_BREAK(event)) {
    int8_t k = EVT_KEY_MASK(event);
    uint8_t idx = inputMappingConvertMode(uint8_t(k / 2));
    uint8_t phase;
    int before;
    bool thro;
    trim_t tmode = getRawTrimValue(mixerCurrentFlightMode, idx);

    trimsDisplayTimer = 200; // 2 seconds
    trimsDisplayMask |= (1<<idx);

#if defined(GVARS)
    if (TRIM_REUSED(idx)) {
      phase = getGVarFlightMode(mixerCurrentFlightMode, trimGvar[idx]);
      before = GVAR_VALUE(trimGvar[idx], phase);
      thro = false;
    }
    else {
      phase = getTrimFlightMode(mixerCurrentFlightMode, idx);
      before = getTrimValue(phase, idx);
      thro = (idx == (g_model.getThrottleStickTrimSource() - MIXSRC_FIRST_TRIM) && g_model.thrTrim);
    }
#else
    phase = getTrimFlightMode(mixerCurrentFlightMode, idx);
    before = getTrimValue(phase, idx);
    thro = (idx==inputMappingConvertMode(inputMappingGetThrottle()) && g_model.thrTrim);
#endif
    int8_t trimInc = g_model.trimInc + 1;
    int16_t v = (trimInc==-1) ? min(32, abs(before)/4+1) : (1 << trimInc); // TODO flash saving if (trimInc < 0)
    if (thro) v = 4; // if throttle trim and trim throttle then step=4
#if defined(GVARS)
    if (TRIM_REUSED(idx)) v = tmode.mode == TRIM_MODE_3POS ? RESX : 1;
#endif
    int16_t after = (k&1) ? before + v : before - v;   // positive = k&1
    bool beepTrim = true;

    if (!thro && before!=0 && tmode.mode != TRIM_MODE_3POS &&
        ((!(after < 0) == (before < 0)) || after==0)) { //forcing a stop at centered trim when changing sides
      after = 0;
      AUDIO_TRIM_MIDDLE();
      pauseTrimEvents(event);
    }

#if defined(GVARS)
    if (TRIM_REUSED(idx)) {
      int8_t gvar = trimGvar[idx];
      int16_t vmin = GVAR_MIN + g_model.gvars[gvar].min;
      int16_t vmax = GVAR_MAX - g_model.gvars[gvar].max;
      if (after < vmin) {
        after = vmin;
        beepTrim = false;
        AUDIO_TRIM_MIN();
        killTrimEvents(event);
      }
      else if (after > vmax) {
        after = vmax;
        beepTrim = false;
        AUDIO_TRIM_MAX();
        killTrimEvents(event);
      }

      setGVarValue(gvar, after, mixerCurrentFlightMode);
    }
    else
#endif
    {
      // Determine Max and Min trim values based on Extended Trim setting
      int16_t tMax = g_model.extendedTrims ? TRIM_EXTENDED_MAX : TRIM_MAX;
      int16_t tMin = g_model.extendedTrims ? TRIM_EXTENDED_MIN : TRIM_MIN;

      // Play warning whe going past limits and remove any buffered trim moves
      if (before >= tMin && after <= tMin) {
        beepTrim = false;
        AUDIO_TRIM_MIN();
        killTrimEvents(event);
      }
      else if (before <= tMax && after >= tMax) {
        beepTrim = false;
        AUDIO_TRIM_MAX();
        killTrimEvents(event);
      }

      // If the new value is outside the limit, set it to the limit. This could have
      // been done while playing the warning above but this way it catches any other
      // scenarios
      if (after < tMin) {
        after = tMin;
      }
      else if (after > tMax) {
        after = tMax;
      }

      if (!setTrimValue(phase, idx, after)) {
        // we don't play a beep, so we exit now the function
        return;
      }
    }

    if (beepTrim) {
      AUDIO_TRIM_PRESS(after);
    }
  }
}

uint8_t g_vbat100mV = 0;
uint16_t lightOffCounter;
uint8_t flashCounter = 0;

uint16_t sessionTimer;
uint16_t s_timeCumThr;    // THR in 1/16 sec
uint16_t s_timeCum16ThrP; // THR% in 1/16 sec

uint8_t trimsCheckTimer = 0;
uint8_t trimsDisplayTimer = 0;
uint8_t trimsDisplayMask = 0;

void flightReset(uint8_t check)
{
  // we don't reset the whole audio here (the hello.wav would be cut, if a prompt is queued before FlightReset, it should be played)
  // TODO check if the vario / background music are stopped correctly if switching to a model which doesn't have these functions enabled

  if (!IS_MANUAL_RESET_TIMER(0)) {
    timerReset(0);
  }

#if TIMERS > 1
  if (!IS_MANUAL_RESET_TIMER(1)) {
    timerReset(1);
  }
#endif

#if TIMERS > 2
  if (!IS_MANUAL_RESET_TIMER(2)) {
    timerReset(2);
  }
#endif

  telemetryReset();

  s_mixer_first_run_done = false;

  START_SILENCE_PERIOD();

  RESET_THR_TRACE();

  logicalSwitchesReset();

  if (check) {
    checkAll();
  }
}

void edgeTxClose(uint8_t shutdown)
{
  TRACE("edgeTxClose");

  watchdogSuspend(2000/*20s*/);

  if (shutdown) {
    pulsesStop();
    AUDIO_BYE();
    // TODO needed? telemetryEnd();
#if defined(HAPTIC)
    hapticOff();
#endif
  }

#if defined(LUA)
  luaClose(&lsScripts);
#endif

  logsClose();

  storageFlushCurrentModel();

  if (sessionTimer > 0) {
    g_eeGeneral.globalTimer += sessionTimer;
    sessionTimer = 0;
    storageDirty(EE_GENERAL);
  }

  storageCheck(true);

  while (IS_PLAYING(ID_PLAY_PROMPT_BASE + AU_BYE)) {
    RTOS_WAIT_MS(10);
  }

  RTOS_WAIT_MS(100);

#if defined(COLORLCD)
  cancelShutdownAnimation();  // To prevent simulator crash
  MainWindow::instance()->shutdown();
#if defined(LUA)
  luaUnregisterWidgets();
  luaClose(&lsWidgets);
  lsWidgets = 0;
#endif
#endif

  sdDone();
}

void edgeTxResume()
{
  TRACE("edgeTxResume");

  sdMount();
#if defined(COLORLCD) && defined(LUA)
  // reload widgets
  luaInitThemesAndWidgets();
#endif

  storageReadAll();

#if defined(COLORLCD)
  //TODO: needs to go into storageReadAll()
  TRACE("reloading theme");
  ThemePersistance::instance()->loadDefaultTheme();
#endif

  referenceSystemAudioFiles();
}

#define INSTANT_TRIM_MARGIN 10 /* around 1% */

void instantTrim()
{
  int16_t anas_0[MAX_INPUTS];
  evalInputs(e_perout_mode_notrainer | e_perout_mode_nosticks);
  memcpy(anas_0, anas, sizeof(anas_0));

  evalInputs(e_perout_mode_notrainer);

  auto controls = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (uint8_t st = 0; st < controls; st++) {
    uint8_t stick = inputMappingConvertMode(st);
    if (stick != inputMappingGetThrottle()) { // don't instant trim the throttle stick
      bool addTrim = false;
      int16_t delta = 0;
      uint8_t trimFlightMode = getTrimFlightMode(mixerCurrentFlightMode, stick);
      for (uint8_t i = 0; i < MAX_EXPOS; i++) {
        ExpoData * expo = expoAddress(i);
        if (!EXPO_VALID(expo))
          break; // end of list
        if (stick == expo->srcRaw - MIXSRC_FIRST_STICK) {
          if (expo->trimSource < 0) {
            // only default trims will be taken into account
            addTrim = false;
            break;
          }
          auto newDelta = anas[expo->chn] - anas_0[expo->chn];
          if (addTrim && delta != newDelta) {
            // avoid 2 different delta values
            addTrim = false;
            break;
          }
          addTrim = true;
          delta = newDelta;
        }
      }
      if (addTrim && abs(delta) >= INSTANT_TRIM_MARGIN) {
        int16_t trim = limit<int16_t>(
            TRIM_EXTENDED_MIN, (delta + trims[stick]) / 2, TRIM_EXTENDED_MAX);
        setTrimValue(trimFlightMode, stick, trim);
      }
    }
  }

  storageDirty(EE_MODEL);
  AUDIO_WARNING2();
}

void copySticksToOffset(uint8_t ch)
{
  mixerTaskStop();
  int32_t zero = (int32_t)channelOutputs[ch];

  evalFlightModeMixes(e_perout_mode_nosticks+e_perout_mode_notrainer, 0);
  int32_t val = chans[ch];
  LimitData *ld = limitAddress(ch);
  limit_min_max_t lim = LIMIT_MIN(ld);
  if (val < 0) {
    val = -val;
    lim = LIMIT_MIN(ld);
  }
  zero = (zero*256000 - val*lim) / (1024*256-val);
  ld->offset = (ld->revert ? -zero : zero);

  mixerTaskStart();
  storageDirty(EE_MODEL);
}

void copyTrimsToOffset(uint8_t ch)
{
  int16_t zero;

  mixerTaskStop();

  evalFlightModeMixes(e_perout_mode_noinput, 0); // do output loop - zero input sticks and trims
  zero = applyLimits(ch, chans[ch]);

  evalFlightModeMixes(e_perout_mode_noinput-e_perout_mode_notrims, 0); // do output loop - only trims

  int16_t output = applyLimits(ch, chans[ch]) - zero;
  int16_t v = g_model.limitData[ch].offset;
  if (g_model.limitData[ch].revert)
    output = -output;
  v += (output * 125) / 128;
  g_model.limitData[ch].offset = limit((int16_t)-1000, (int16_t)v, (int16_t)1000); // make sure the offset doesn't go haywire

  mixerTaskStart();
  storageDirty(EE_MODEL);
}

void copyMinMaxToOutputs(uint8_t ch)
{
  LimitData *ld = limitAddress(ch);
  int16_t min = ld->min;
  int16_t max = ld->max;
  int16_t center = ld->ppmCenter;

  mixerTaskStop();

  for (uint8_t chan = 0; chan < MAX_OUTPUT_CHANNELS; chan++) {
    ld = limitAddress(chan);
    ld->min = min;
    ld->max = max;
    ld->ppmCenter = center;
  }

  mixerTaskStart();
  storageDirty(EE_MODEL);
}

#if defined(PWR_BUTTON_PRESS) || defined(STARTUP_ANIMATION)
uint32_t pwrDelayTime(int delay)
{
  static uint8_t vals[] = { 0, 5, 10, 20, 30 };
  return vals[pwrDelayFromYaml(delay)] * 10;
}
#endif

#if defined(STARTUP_ANIMATION)

inline uint32_t PWR_PRESS_DURATION_MIN()
{
  return pwrDelayTime(g_eeGeneral.pwrOnSpeed);
}

constexpr uint32_t PWR_PRESS_DURATION_MAX = 500; // 5s

void runStartupAnimation()
{
  tmr10ms_t start = get_tmr10ms();
  tmr10ms_t duration = 0;
  bool isPowerOn = false;

  while (pwrPressed()) {
    duration = get_tmr10ms() - start;
    if (duration < PWR_PRESS_DURATION_MIN()) {
      drawStartupAnimation(duration, PWR_PRESS_DURATION_MIN());
    }
    else if (duration >= PWR_PRESS_DURATION_MAX) {
      drawSleepBitmap();
      backlightDisable();
    }
    else if (!isPowerOn) {
      isPowerOn = true;
      pwrOn();
#if defined(HAPTIC)
      if (!g_eeGeneral.disablePwrOnOffHaptic &&
          (g_eeGeneral.hapticMode != e_mode_quiet))
        haptic.play(15, 3, PLAY_NOW);
#endif
    }
  }

  if (duration <= PWR_PRESS_DURATION_MIN() || duration >= PWR_PRESS_DURATION_MAX) {
    boardOff();
  }
}
#endif

void moveTrimsToOffsets() // copy state of 3 primary to subtrim
{
  int16_t zeros[MAX_OUTPUT_CHANNELS];

  mixerTaskStop();

  evalFlightModeMixes(e_perout_mode_noinput, 0); // do output loop - zero input sticks and trims

  for (uint8_t i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
    zeros[i] = applyLimits(i, chans[i]);
  }

  evalFlightModeMixes(e_perout_mode_noinput-e_perout_mode_notrims, 0); // do output loop - only trims

  for (uint8_t i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
    int16_t diff = applyLimits(i, chans[i]) - zeros[i];
    int16_t v = g_model.limitData[i].offset;
    if (g_model.limitData[i].revert)
      diff = -diff;
    v += (diff * 125) / 128;

    g_model.limitData[i].offset = limit((int16_t) -1000, (int16_t) v, (int16_t) 1000); // make sure the offset doesn't go haywire
  }

  // reset all trims, except throttle (if throttle trim)
  for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {
    auto thrStick = g_model.getThrottleStickTrimSource() - MIXSRC_FIRST_TRIM;
    if (i != thrStick || !g_model.thrTrim) {
      int16_t original_trim = getTrimValue(mixerCurrentFlightMode, i);
      for (uint8_t fm=0; fm<MAX_FLIGHT_MODES; fm++) {
        trim_t trim = getRawTrimValue(fm, i);
        if (trim.mode / 2 == fm)
          setTrimValue(fm, i, trim.value - original_trim);
      }
    }
  }

  mixerTaskStart();

  storageDirty(EE_MODEL);
  AUDIO_WARNING2();
}

// Overridden by simulator startup
uint8_t startOptions = 0;

void edgeTxInit()
{
  TRACE("edgeTxInit");

  // Show splash screen (color LCD)
  if (!(startOptions & OPENTX_START_NO_SPLASH))
    startSplash();

#if defined(LIBOPENUI)
  initLvglTheme();
  // create ViewMain
  ViewMain::instance();
#elif defined(GUI)
  // TODO add a function for this (duplicated)
  menuHandlers[0] = menuMainView;
  menuHandlers[1] = menuModelSelect;
#endif

#if defined(GUI) && !defined(COLORLCD)
  lcdRefreshWait();
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();
#endif

  // Load radio.yml so radio settings can be used
#if defined(RTC_BACKUP_RAM)
  // Skip loading if EM startup and radio has RTC backup data
  if (!UNEXPECTED_SHUTDOWN())
    storageReadRadioSettings(false);
#else
  // No RTC backup - try and load even for EM startup
  storageReadRadioSettings(false);
#endif

#if defined(GUI) && !defined(COLORLCD)
  lcdSetContrast();
#endif

  BACKLIGHT_ENABLE(); // we start the backlight during the startup animation

#if defined(STARTUP_ANIMATION)
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
  else {
    runStartupAnimation();
  }
#else // defined(STARTUP_ANIMATION)
  pwrOn();
#if defined(HAPTIC)
  if (!g_eeGeneral.disablePwrOnOffHaptic &&
      (g_eeGeneral.hapticMode != e_mode_quiet))
    haptic.play(15, 3, PLAY_NOW);
#endif
#endif

  // SDCARD related stuff, only enable if normal boot
  if (!UNEXPECTED_SHUTDOWN()) {

    if (!sdMounted())
      sdInit();

#if !defined(COLORLCD)
    if (!sdMounted()) {
      g_eeGeneral.pwrOffSpeed = 2;
      runFatalErrorScreen(STR_NO_SDCARD);
    }
#endif // !defined(COLORLCD)

#if defined(AUTOUPDATE)
    sportStopSendByteLoop();
    if (f_stat(AUTOUPDATE_FILENAME, nullptr) == FR_OK) {
      FrSkyFirmwareInformation information;
      if (readFrSkyFirmwareInformation(AUTOUPDATE_FILENAME, information) == nullptr) {
#if defined(BLUETOOTH)
        if (information.productFamily == FIRMWARE_FAMILY_BLUETOOTH_CHIP) {
          if (bluetooth.flashFirmware(AUTOUPDATE_FILENAME) == nullptr)
            f_unlink(AUTOUPDATE_FILENAME);
        }
#endif // defined(BLUETOOTH)
      }
    }
#endif // defined(AUTOUPDATE)

    logsInit();
  }

#if defined(COLORLCD) && defined(LUA)
  if (!UNEXPECTED_SHUTDOWN()) {
    // lua widget state must be prepared before the call to storageReadAll()
    luaInitThemesAndWidgets();
  }
#endif

  // handling of storage for radios
#if defined(RTC_BACKUP_RAM) && !defined(SIMU)
  if (UNEXPECTED_SHUTDOWN()) {
    // SDCARD not available, try to restore last model from RAM
    TRACE("rambackupRestore");
    rambackupRestore();
    logicalSwitchesInit(true);
  }
  else {
    storageReadAll();
  }
#else
  storageReadAll();
#endif

  initSerialPorts();

#if defined(AUDIO)
  currentSpeakerVolume = requiredSpeakerVolume =
      g_eeGeneral.speakerVolume + VOLUME_LEVEL_DEF;
#if !defined(SOFTWARE_VOLUME)
  setScaledVolume(currentSpeakerVolume);
#endif
#endif

  currentBacklightBright = requiredBacklightBright = g_eeGeneral.getBrightness();


  referenceSystemAudioFiles();
  audioQueue.start();
  BACKLIGHT_ENABLE();

#if defined(COLORLCD)
  ThemePersistance::instance()->loadDefaultTheme();
  if (g_eeGeneral.backlightMode == e_backlight_mode_off) {
    // no backlight mode off on color lcd radios
    g_eeGeneral.backlightMode = e_backlight_mode_keys;
  }
  if (g_eeGeneral.backlightBright > BACKLIGHT_LEVEL_MAX - BACKLIGHT_LEVEL_MIN)
    g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - BACKLIGHT_LEVEL_MIN;
  if (g_eeGeneral.lightAutoOff < 1)
    g_eeGeneral.lightAutoOff = 1;
#endif

  if (g_eeGeneral.backlightMode != e_backlight_mode_off) {
    // on Tx start turn the light on
    resetBacklightTimeout();
  }

  if (!UNEXPECTED_SHUTDOWN()) {

    uint8_t calibration_needed = !(startOptions & OPENTX_START_NO_CALIBRATION) && (g_eeGeneral.chkSum != evalChkSum());

#if defined(GUI)
    if (!calibration_needed && !(startOptions & OPENTX_START_NO_SPLASH)) {
      if (!g_eeGeneral.dontPlayHello) AUDIO_HELLO();

      waitSplash();
    }
#endif // defined(GUI)

#if defined(BLUETOOTH_PROBE)
    extern volatile uint8_t btChipPresent;
    auto oldBtMode = g_eeGeneral.bluetoothMode;
    g_eeGeneral.bluetoothMode = BLUETOOTH_TELEMETRY;
#endif

#if defined(DEBUG_TRACE_BUFFER)
    trace_event(trace_start, 0x12345678);
#endif

#if defined(TEST_BUILD_WARNING)
    ALERT(STR_TEST_WARNING, TR_TEST_NOTSAFE, AU_ERROR);
#endif

#if defined(FUNCTION_SWITCHES)
    if (!UNEXPECTED_SHUTDOWN()) {
      setFSStartupPosition();
    }
#endif

#if defined(GUI)
    if (calibration_needed) {
      cancelSplash();
#if defined(LIBOPENUI)
      startCalibration();
#else
      chainMenu(menuFirstCalib);
#endif // defined(LIBOPENUI)
    }
    else if (!(startOptions & OPENTX_START_NO_CHECKS)) {
      checkAlarm();
      checkAll(true);
      PLAY_MODEL_NAME();
    }
#endif // defined(GUI)

#if defined(BLUETOOTH_PROBE)
    if (bluetooth.localAddr[0] != '\0')
      btChipPresent = 1;
    g_eeGeneral.bluetoothMode = oldBtMode;
#endif
  }

#if defined(GUI) && !defined(COLORLCD) && !defined(STARTUP_ANIMATION)
  lcdSetContrast();
#endif

  resetBacklightTimeout();

#if defined(LED_STRIP_GPIO) && !defined(SIMU)
  rgbLedStart();
#endif

  pulsesStart();
  WDG_ENABLE(WDG_DURATION);
}

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

#if defined(SIMU)
void simuMain()
#else
int main()
#endif
{
#if defined(SEMIHOSTING)
  initialise_monitor_handles();
#endif


#if !defined(SIMU)
  /* Ensure all priority bits are assigned as preemption priority bits. */
  NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );
#endif

  // G: The WDT remains active after a WDT reset -- at maximum clock speed. So it's
  // important to disable it before commencing with system initialisation (or
  // we could put a bunch more WDG_RESET()s in. But I don't like that approach
  // during boot up.)
#if defined(LCD_CONTRAST_DEFAULT)
  g_eeGeneral.contrast = LCD_CONTRAST_DEFAULT;
#endif

  boardInit();

  modulePortInit();
  pulsesInit();

#if !defined(DISABLE_MCUCHECK)
  checkValidMCU();
#endif

#if defined(PCBHORUS)
  if (!IS_FIRMWARE_COMPATIBLE_WITH_BOARD()) {
    runFatalErrorScreen(STR_WRONG_PCBREV);
  }
#endif

#if defined(COLORLCD)
  // SD_CARD_PRESENT() does not work properly on most
  // B&W targets, so that we need to delay the detection
  // until the SD card is mounted (requires RTOS scheduler running)
  if (!SD_CARD_PRESENT() && !UNEXPECTED_SHUTDOWN()) {
    runFatalErrorScreen(STR_NO_SDCARD);
  }
#endif
  
  tasksStart();
}

#if defined(PWR_BUTTON_PRESS)
int pwrDelayFromYaml(int delay)
{
  static int8_t vals[] = { 1, 4, 3, 2, 0 };
  return vals[delay + 2];
}

int pwrDelayToYaml(int delay)
{
  static int8_t vals[] = { 2, -2, 1, 0, -1 };
  return vals[delay];
}
#endif

#if !defined(SIMU)
#if defined(PWR_BUTTON_PRESS)

inline uint32_t PWR_PRESS_SHUTDOWN_DELAY()
{
  // Instant off when both power button are pressed
  if (pwrForcePressed())
    return 0;

  return pwrDelayTime(g_eeGeneral.pwrOffSpeed);
}

uint32_t pwr_press_time = 0;

uint32_t pwrPressedDuration()
{
  if (pwr_press_time == 0)
    return 0;

  return get_tmr10ms() - pwr_press_time;
}

#if defined(COLORLCD)
inline tmr10ms_t getTicks() { return g_tmr10ms; }
#endif

bool pwrOffDueToInactivity()
{
  uint8_t inactivityLimit = g_eeGeneral.pwrOffIfInactive;
  static tmr10ms_t lastConnectedTime = 0;

  tmr10ms_t currentTime = get_tmr10ms();

  if (TELEMETRY_STREAMING() ||
      (usbPlugged() && getSelectedUsbMode() != USB_UNSELECTED_MODE) ||
      isTrainerValid())
    lastConnectedTime = currentTime;

  bool inactivityShutdown =
      inactivityLimit && inactivity.counter > 60u * inactivityLimit &&
      (currentTime - lastConnectedTime) / 100u > 60u * inactivityLimit;

  return inactivityShutdown;
}

uint32_t pwrCheck()
{
  const char * message = nullptr;

  enum PwrCheckState {
    PWR_CHECK_ON,
    PWR_CHECK_OFF,
    PWR_CHECK_PAUSED,
  };

  static uint8_t pwr_check_state = PWR_CHECK_ON;

  bool inactivityShutdown = pwrOffDueToInactivity();
  
  if (pwr_check_state == PWR_CHECK_OFF) {
    return e_power_off;
  }
  else if (pwrPressed() || inactivityShutdown) {
    if (!inactivityShutdown)
      inactivityTimerReset(ActivitySource::Keys);

    if (TELEMETRY_STREAMING()) {
      message = STR_MODEL_STILL_POWERED;
    }
    if (pwr_check_state == PWR_CHECK_PAUSED) {
      // nothing
    }
    else if (pwr_press_time == 0) {
      pwr_press_time = get_tmr10ms();
      if (message && !g_eeGeneral.disableRssiPoweroffAlarm) {
        audioEvent(AU_MODEL_STILL_POWERED);
      }
    }
    else {
      if (g_eeGeneral.backlightMode != e_backlight_mode_off) {
        BACKLIGHT_ENABLE();
      }
      if (get_tmr10ms() - pwr_press_time > PWR_PRESS_SHUTDOWN_DELAY()) {
#if defined(COLORLCD)
        bool usbConfirmed = !usbPlugged() || getSelectedUsbMode() == USB_UNSELECTED_MODE;
        bool modelConnectedConfirmed = !TELEMETRY_STREAMING() || g_eeGeneral.disableRssiPoweroffAlarm;
        bool trainerConfirmed = !isTrainerConnected();
#endif
#if defined(SHUTDOWN_CONFIRMATION)
        while (1)
#else
        while (
            (usbPlugged() && getSelectedUsbMode() != USB_UNSELECTED_MODE) ||
            (TELEMETRY_STREAMING() && !g_eeGeneral.disableRssiPoweroffAlarm) ||
            (isTrainerConnected() && !g_eeGeneral.disableTrainerPoweroffAlarm))
#endif
        {

#if !defined(COLORLCD)

          lcdRefreshWait();
          lcdClear();

          POPUP_CONFIRMATION(STR_MODEL_SHUTDOWN, nullptr);

          const char* msg = STR_MODEL_STILL_POWERED;
          uint8_t msg_len = sizeof(TR_MODEL_STILL_POWERED);
          if (usbPlugged() && getSelectedUsbMode() != USB_UNSELECTED_MODE) {
            msg = STR_USB_STILL_CONNECTED;
            msg_len = sizeof(TR_USB_STILL_CONNECTED);
          }
          else if (isTrainerConnected() && !g_eeGeneral.disableTrainerPoweroffAlarm) {
            msg = STR_TRAINER_STILL_CONNECTED;
            msg_len = sizeof(TR_TRAINER_STILL_CONNECTED);
          }
          event_t evt = getEvent();
          SET_WARNING_INFO(msg, msg_len, 0);
          DISPLAY_WARNING(evt);
          LED_ERROR_BEGIN();

          WDG_RESET();
          lcdRefresh();

          if (warningResult) {
            pwr_check_state = PWR_CHECK_OFF;
            return e_power_off;
          }
          else if (!warningText) {
            // shutdown has been cancelled
            pwr_check_state = PWR_CHECK_PAUSED;
            LED_ERROR_END();
            return e_power_on;
          }
#else  // COLORLCD

          cancelShutdownAnimation();

          const char* message = nullptr;
          std::function<bool(void)> closeCondition = nullptr;
          if (!usbConfirmed) {
            message = STR_USB_STILL_CONNECTED;
            closeCondition = [](){
              return !usbPlugged();
            };
          }
          else if (!modelConnectedConfirmed) {
            message = STR_MODEL_STILL_POWERED;
            closeCondition = []() {
              tmr10ms_t startTime = getTicks();
              while (!TELEMETRY_STREAMING()) {
                if (getTicks() - startTime > TELEMETRY_CHECK_DELAY10ms) break;
              }
              return !TELEMETRY_STREAMING() || g_eeGeneral.disableRssiPoweroffAlarm;
            };
          }
          else if (!trainerConfirmed && !g_eeGeneral.disableTrainerPoweroffAlarm) {
            message = STR_TRAINER_STILL_CONNECTED;
            closeCondition = [](){
              return !isTrainerConnected();
            };
          }

          // TODO: abort dialog condition (here, RSSI lost / USB connected)
          if (confirmationDialog(STR_MODEL_SHUTDOWN, message, false, closeCondition)) {
            if( message == STR_USB_STILL_CONNECTED)
              usbConfirmed=true;
            if( message == STR_MODEL_STILL_POWERED)
              modelConnectedConfirmed = true;

            if(!usbConfirmed || !modelConnectedConfirmed)
              continue;

            pwr_check_state = PWR_CHECK_OFF;
            return e_power_off;
          } else {
            // shutdown has been cancelled
            pwr_check_state = PWR_CHECK_PAUSED;
            LED_ERROR_END();
            return e_power_on;
          }

#endif // COLORLCD
        }

#if defined(HAPTIC)
        if (!g_eeGeneral.disablePwrOnOffHaptic &&
            (g_eeGeneral.hapticMode != e_mode_quiet))
          haptic.play(15, 3, PLAY_NOW);
#endif
        pwr_check_state = PWR_CHECK_OFF;
        return e_power_off;
      }
      else {
        drawShutdownAnimation(pwrPressedDuration(), PWR_PRESS_SHUTDOWN_DELAY(), message);
        return e_power_press;
      }
    }
  }
  else {
#if defined(COLORLCD)
    cancelShutdownAnimation();
#endif
    pwr_check_state = PWR_CHECK_ON;
    pwr_press_time = 0;
  }

  return e_power_on;
}
#else
uint32_t pwrCheck()
{
#if defined(SOFT_PWR_CTRL)
  if (pwrPressed()) {
    return e_power_on;
  }
#endif

  if (usbPlugged()) {
    return e_power_usb;
  }

  if (!g_eeGeneral.disableRssiPoweroffAlarm) {
    if (TELEMETRY_STREAMING()) {
      RAISE_ALERT(STR_MODEL, STR_MODEL_STILL_POWERED, STR_PRESS_ENTER_TO_CONFIRM, AU_MODEL_STILL_POWERED);
      while (TELEMETRY_STREAMING()) {
        resetForcePowerOffRequest();
        RTOS_WAIT_MS(20);
        if (pwrPressed()) {
          return e_power_on;
        }
        else if (readKeys() == (1 << KEY_ENTER)) {
          return e_power_off;
        }
      }
    }
  }

  return e_power_off;
}
#endif  // defined(PWR_BUTTON_PRESS)
#endif  // !defined(SIMU)

uint32_t availableMemory()
{
#if defined(SIMU)
  return 1000;
#else
  extern unsigned char *heap;
  extern int _heap_end;

  struct mallinfo info = mallinfo();

  return ((uint32_t)((unsigned char *)&_heap_end - heap)) + info.fordblks;
#endif
}

#define FEATURE_ENABLED(f) (g_model.f == OVERRIDE_GLOBAL && g_eeGeneral.f == 0) || (g_model.f == OVERRIDE_ON)

// Radio menu tab state
#if defined(COLORLCD)
bool radioThemesEnabled() {
  return FEATURE_ENABLED(radioThemesDisabled);
}
#endif
bool radioGFEnabled() {
  return FEATURE_ENABLED(radioGFDisabled);
}
bool radioTrainerEnabled() {
  return FEATURE_ENABLED(radioTrainerDisabled);
}

// Model menu tab state
bool modelHeliEnabled() {
  return FEATURE_ENABLED(modelHeliDisabled);
}
bool modelFMEnabled() {
  return FEATURE_ENABLED(modelFMDisabled);
}
bool modelCurvesEnabled() {
  return FEATURE_ENABLED(modelCurvesDisabled);
}
bool modelGVEnabled() {
  return FEATURE_ENABLED(modelGVDisabled);
}
bool modelLSEnabled() {
  return FEATURE_ENABLED(modelLSDisabled);
}
bool modelSFEnabled() {
  return FEATURE_ENABLED(modelSFDisabled);
}
bool modelCustomScriptsEnabled() {
  return FEATURE_ENABLED(modelCustomScriptsDisabled);
}
bool modelTelemetryEnabled() {
  return FEATURE_ENABLED(modelTelemetryDisabled);
}

void getMixSrcRange(const int source, int16_t & valMin, int16_t & valMax, LcdFlags * flags)
{
  int asrc = abs(source);

  if (asrc >= MIXSRC_FIRST_TRIM && asrc <= MIXSRC_LAST_TRIM) {
    valMax = g_model.extendedTrims ? TRIM_EXTENDED_MAX : TRIM_MAX;
    valMin = -valMax;
  }
#if defined(LUA_INPUTS)
  else if (asrc >= MIXSRC_FIRST_LUA && asrc <= MIXSRC_LAST_LUA) {
    valMax = 30000;
    valMin = -valMax;
  }
#endif
  else if (asrc < MIXSRC_FIRST_CH) {
    valMax = 100;
    valMin = -valMax;
  }
  else if (asrc <= MIXSRC_LAST_CH) {
    valMax = g_model.extendedLimits ? LIMIT_EXT_PERCENT : 100;
    valMin = -valMax;
  }
#if defined(GVARS)
  else if (asrc >= MIXSRC_FIRST_GVAR && asrc <= MIXSRC_LAST_GVAR) {
    valMax = min<int>(CFN_GVAR_CST_MAX, MODEL_GVAR_MAX(asrc-MIXSRC_FIRST_GVAR));
    valMin = max<int>(CFN_GVAR_CST_MIN, MODEL_GVAR_MIN(asrc-MIXSRC_FIRST_GVAR));
    if (flags && g_model.gvars[asrc-MIXSRC_FIRST_GVAR].prec)
      *flags |= PREC1;
  }
#endif
  else if (asrc == MIXSRC_TX_VOLTAGE) {
    valMax =  255;
    valMin = 0;
    if (flags)
      *flags |= PREC1;
  }
  else if (asrc == MIXSRC_TX_TIME) {
    valMax =  23 * 60 + 59;
    valMin = 0;
  }
  else if (asrc >= MIXSRC_FIRST_TIMER && asrc <= MIXSRC_LAST_TIMER) {
    valMax =  9 * 60 * 60 - 1;
    valMin = -valMax;
    if (flags)
      *flags |= TIMEHOUR;
  }
  else {
    valMax = 30000;
    valMin = -valMax;
  }
}
