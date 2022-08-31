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

#include "opentx.h"
#include "io/frsky_firmware_update.h"
#include "hal/adc_driver.h"
#include "aux_serial_driver.h"
#include "timers_driver.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
  // #include "shutdown_animation.h"
  #include "radio_calibration.h"
  #include "view_main.h"
  #include "view_text.h"

  #include "gui/colorlcd/LvglWrapper.h"
#endif


RadioData  g_eeGeneral;
ModelData  g_model;

#if defined(SDCARD)
Clipboard clipboard;
#endif

GlobalData globalData;

uint16_t maxMixerDuration; // step = 0.01ms
uint8_t heartbeat;

#if defined(OVERRIDE_CHANNEL_FUNCTION)
safetych_t safetyCh[MAX_OUTPUT_CHANNELS];
#endif

// __DMA for the MSC_BOT_Data member
union ReusableBuffer reusableBuffer __DMA;

uint8_t* MSC_BOT_Data = reusableBuffer.MSC_BOT_Data;

#if defined(DEBUG_LATENCY)
uint8_t latencyToggleSwitch = 0;
#endif

const uint8_t bchout_ar[]  = {
    0x1B, 0x1E, 0x27, 0x2D, 0x36, 0x39,
    0x4B, 0x4E, 0x63, 0x6C, 0x72, 0x78,
    0x87, 0x8D, 0x93, 0x9C, 0xB1, 0xB4,
    0xC6, 0xC9, 0xD2, 0xD8, 0xE1, 0xE4 };

uint8_t channelOrder(uint8_t setup, uint8_t x)
{
  return ((*(bchout_ar + setup) >> (6 - (x - 1) * 2)) & 3) + 1;
}

uint8_t channelOrder(uint8_t x)
{
  return channelOrder(g_eeGeneral.templateSetup, x);
}

/*
mode1 rud ele thr ail
mode2 rud thr ele ail
mode3 ail ele thr rud
mode4 ail thr ele rud
*/
const uint8_t modn12x3[]  = {
    0, 1, 2, 3,
    0, 2, 1, 3,
    3, 1, 2, 0,
    3, 2, 1, 0 };

volatile uint8_t rtc_count = 0;

#if defined(DEBUG_LATENCY)
void toggleLatencySwitch()
{
  latencyToggleSwitch ^= 1;

#if defined(PCBHORUS)
  if (latencyToggleSwitch)
    GPIO_ResetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  else
    GPIO_SetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
#else
  if (latencyToggleSwitch)
    sportUpdatePowerOn();
  else
    sportUpdatePowerOff();
#endif
}
#endif

static void readKeysAndTrims()
{
  uint32_t i;

  uint8_t index = 0;
  uint32_t keys_input = readKeys();
  for (i = 0; i < TRM_BASE; i++) {
    keys[index++].input(keys_input & (1 << i));
  }

  uint32_t trims_input = readTrims();
  for (i = 1; i <= 1 << (TRM_LAST-TRM_BASE); i <<= 1) {
    keys[index++].input(trims_input & i);
  }

#if defined(PWR_BUTTON_PRESS)
  if ((keys_input || trims_input || pwrPressed()) &&
      (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
#else
  if ((keys_input || trims_input) &&
      (g_eeGeneral.backlightMode & e_backlight_mode_keys)) {
#endif
    // on keypress turn the light on
    resetBacklightTimeout();
  }
}

void per10ms()
{
  g_tmr10ms++;

#if defined(GUI)
  if (lightOffCounter) lightOffCounter--;
  if (flashCounter) flashCounter--;
#if !defined(LIBOPENUI)
  if (noHighlightCounter) noHighlightCounter--;
#endif
#endif

  if (trimsCheckTimer) trimsCheckTimer--;
  if (ppmInputValidityTimer) ppmInputValidityTimer--;

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

  // TODO: to board...
  readKeysAndTrims();

#if defined(FUNCTION_SWITCHES)
  evalFunctionSwitches();
#endif

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(LIBOPENUI)
  if (IS_ROTARY_ENCODER_NAVIGATION_ENABLE()) {
    static rotenc_t rePreviousValue;
    static bool cw = false;
    rotenc_t reNewValue = (ROTARY_ENCODER_NAVIGATION_VALUE / ROTARY_ENCODER_GRANULARITY);
    rotenc_t scrollRE = reNewValue - rePreviousValue;
    if (scrollRE) {
      static uint32_t lastEvent;
      rePreviousValue = reNewValue;

      bool new_cw = (scrollRE < 0) ? false : true;
      if ((g_tmr10ms - lastEvent >= 10) || (cw == new_cw)) { // 100ms

        pushEvent(new_cw ? EVT_ROTARY_RIGHT : EVT_ROTARY_LEFT);

        // rotary encoder navigation speed (acceleration) detection/calculation
        static uint32_t delay = 2*ROTENC_DELAY_MIDSPEED;

        if (new_cw == cw) {
          // Modified moving average filter used for smoother change of speed
          delay = (((g_tmr10ms - lastEvent) << 3) + delay) >> 1;
        }
        else {
          delay = 2*ROTENC_DELAY_MIDSPEED;
        }

        if (delay < ROTENC_DELAY_HIGHSPEED)
          rotencSpeed = ROTENC_HIGHSPEED;
        else if (delay < ROTENC_DELAY_MIDSPEED)
          rotencSpeed = ROTENC_MIDSPEED;
        else
          rotencSpeed = ROTENC_LOWSPEED;
        cw = new_cw;
        lastEvent = g_tmr10ms;
      }

      if (g_eeGeneral.backlightMode & e_backlight_mode_keys) {
        resetBacklightTimeout();
      }
      inactivity.counter = 0;
    }
  }
#endif

  telemetryInterrupt10ms();

  // These moved here from evalFlightModeMixes() to improve beep trigger reliability.
#if defined(PWM_BACKLIGHT)
  if ((g_tmr10ms&0x03) == 0x00)
    backlightFade(); // increment or decrement brightness until target brightness is reached
#endif

#if !defined(AUDIO)
  if (mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) AUDIO_MIX_WARNING(1);
  if (mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) AUDIO_MIX_WARNING(2);
  if (mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) AUDIO_MIX_WARNING(3);
#endif

#if defined(SDCARD)
  sdPoll10ms();
#endif

  outputTelemetryBuffer.per10ms();

  heartbeat |= HEART_TIMER_10MS;
}

FlightModeData *flightModeAddress(uint8_t idx)
{
  return &g_model.flightModeData[idx];
}

ExpoData *expoAddress(uint8_t idx )
{
  return &g_model.expoData[idx];
}

MixData *mixAddress(uint8_t idx)
{
  return &g_model.mixData[idx];
}

LimitData *limitAddress(uint8_t idx)
{
  return &g_model.limitData[idx];
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

#if defined(PCBHORUS)
  g_eeGeneral.blOffBright = 20;
#endif

#if defined(LCD_CONTRAST_DEFAULT)
  g_eeGeneral.contrast = LCD_CONTRAST_DEFAULT;
#endif

#if defined(DEFAULT_INTERNAL_MODULE)
    g_eeGeneral.internalModule = DEFAULT_INTERNAL_MODULE;
#endif

#if defined(DEFAULT_POTS_CONFIG)
  g_eeGeneral.potsConfig = DEFAULT_POTS_CONFIG;
#endif

#if defined(DEFAULT_SWITCH_CONFIG)
  g_eeGeneral.switchConfig = DEFAULT_SWITCH_CONFIG;
#endif

#if defined(DEFAULT_SLIDERS_CONFIG)
  g_eeGeneral.slidersConfig = DEFAULT_SLIDERS_CONFIG;
#endif

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

#if defined(DEFAULT_MODE)
  g_eeGeneral.stickMode = DEFAULT_MODE - 1;
#endif

  g_eeGeneral.templateSetup = DEFAULT_TEMPLATE_SETUP;

  g_eeGeneral.backlightMode = e_backlight_mode_all;
  g_eeGeneral.lightAutoOff = 2;
  g_eeGeneral.inactivityTimer = 10;

  g_eeGeneral.ttsLanguage[0] = 'e';
  g_eeGeneral.ttsLanguage[1] = 'n';
  g_eeGeneral.wavVolume = 2;
  g_eeGeneral.backgroundVolume = 1;

  for (int i=0; i<NUM_STICKS; ++i) {
    g_eeGeneral.trainer.mix[i].mode = 2;
    g_eeGeneral.trainer.mix[i].srcChn = channelOrder(i+1) - 1;
    g_eeGeneral.trainer.mix[i].studWeight = 100;
  }

#if defined(PCBX9E)
  const int8_t defaultName[] = { 20, -1, -18, -1, -14, -9, -19 };
  memcpy(g_eeGeneral.bluetoothName, defaultName, sizeof(defaultName));
#endif

#if defined(STORAGE_MODELSLIST)
  strcpy(g_eeGeneral.currModelFilename, DEFAULT_MODEL_FILENAME);
#endif

#if defined(COLORLCD)
  strcpy(g_eeGeneral.themeName, static_cast<OpenTxTheme *>(theme)->getName());
  static_cast<OpenTxTheme *>(theme)->init();
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

  g_eeGeneral.chkSum = 0xFFFF;
}

uint16_t evalChkSum()
{
  uint16_t sum = 0;
  const int16_t * calibValues = (const int16_t *) &g_eeGeneral.calib[0];
  for (int i=0; i<12; i++)
    sum += calibValues[i];
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
    for (uint8_t i=0; i<MAX_INPUTS; i++) {
      if (abs(anas[i] - inputsStates[i]) > MULTIPOS_STEP_SIZE) {
        if (!isInputRecursive(i)) {
          result = MIXSRC_FIRST_INPUT+i;
          break;
        }
      }
    }
  }

  static int16_t sourcesStates[NUM_STICKS+NUM_POTS+NUM_SLIDERS+NUM_MOUSE_ANALOGS];
  if (result == 0) {
    for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
      if (abs(calibratedAnalogs[i] - sourcesStates[i]) > MULTIPOS_STEP_SIZE) {
        result = MIXSRC_Rud+i;
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
    if (v.mode == TRIM_MODE_NONE) {
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
    if (v.mode == TRIM_MODE_NONE)
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

#define INAC_STICKS_SHIFT   6
#define INAC_SWITCHES_SHIFT 8
bool inputsMoved()
{
  uint8_t sum = 0;
  for (uint8_t i=0; i<NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++)
    sum += anaIn(i) >> INAC_STICKS_SHIFT;
  for (uint8_t i=0; i<NUM_SWITCHES; i++)
    sum += getValue(MIXSRC_FIRST_SWITCH+i) >> INAC_SWITCHES_SHIFT;
#if defined(IMU)
  for (uint8_t i=0; i<2; i++)
    sum += getValue(MIXSRC_TILT_X+i) >> INAC_STICKS_SHIFT;
#endif

  if (abs((int8_t)(sum-inactivity.sum)) > 1) {
    inactivity.sum = sum;
    return true;
  }
  else {
    return false;
  }
}

void checkBacklight()
{
  static uint8_t tmr10ms ;

  uint8_t x = g_blinkTmr10ms;
  if (tmr10ms != x) {
    tmr10ms = x;
    if (inputsMoved()) {
      inactivity.counter = 0;
      if (g_eeGeneral.backlightMode & e_backlight_mode_sticks) {
        resetBacklightTimeout();
      }
    }

    currentBacklightBright = g_eeGeneral.backlightBright;
    if (forceBacklightOn) {
      BACKLIGHT_ENABLE();
    } else {
      bool backlightOn = (g_eeGeneral.backlightMode == e_backlight_mode_on) ||
                         (g_eeGeneral.backlightMode != e_backlight_mode_off &&
                          (lightOffCounter > 0));

      if (flashCounter) {
        backlightOn = !backlightOn;
      }

      if (backlightOn) {
        if (g_eeGeneral.backlightSrc != MIXSRC_NONE) {
          getvalue_t raw = getValue(g_eeGeneral.backlightSrc);
#if defined(COLORLCD)
          if (raw == -RESX) {
            currentBacklightBright = BACKLIGHT_LEVEL_MAX;
          } else {
            raw = ((RESX - raw) * (BACKLIGHT_LEVEL_MAX - BACKLIGHT_LEVEL_MIN)) /
                  (2 * RESX);
            currentBacklightBright = (uint8_t)raw;
          }
#else
          raw = ((RESXl - raw) * 100l) / (2*RESXl);
          currentBacklightBright = (uint8_t)raw;
#endif
        }
#if defined(COLORLCD)
        else {
          // if no physical control is used, force backlight on
          if (currentBacklightBright > BACKLIGHT_LEVEL_MAX - BACKLIGHT_LEVEL_MIN)
            currentBacklightBright = BACKLIGHT_LEVEL_MAX - BACKLIGHT_LEVEL_MIN;
        }
#endif
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

#if defined(SPLASH)
void doSplash()
{
#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  if (SPLASH_NEEDED()) {
    resetBacklightTimeout();
    drawSplash();


    getADC(); // init ADC array

    inputsMoved();

    tmr10ms_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;

    while (tgtime > get_tmr10ms()) {
      RTOS_WAIT_TICKS(1);

      getADC();

      if (getEvent() || inputsMoved())
        return;

#if defined(PWR_BUTTON_PRESS)
      uint32_t pwr_check = pwrCheck();
      if (pwr_check == e_power_off) {
        break;
      }
      else if (pwr_check == e_power_press) {
        refresh = true;
      }
      else if (pwr_check == e_power_on && refresh) {
        drawSplash();
        refresh = false;
      }
#else
      if (pwrCheck() == e_power_off) {
        return;
      }
#endif

      forceBacklightOn = true;
      checkBacklight();
    }

    forceBacklightOn = false;

#if defined(LIBOPENUI)
    MainWindow::instance()->setActiveScreen();
#endif
  }
}
#else
#define Splash()
#define doSplash()
#endif


#if defined(MULTIMODULE)
void checkMultiLowPower()
{
  if (isModuleMultimodule(EXTERNAL_MODULE) && g_model.moduleData[EXTERNAL_MODULE].multi.lowPowerMode) {
    ALERT("MULTI", STR_WARN_MULTI_LOWPOWER, AU_ERROR);
    return;
  }
#if defined(INTERNAL_MODULE_MULTI)
  if (isModuleMultimodule(INTERNAL_MODULE) && g_model.moduleData[INTERNAL_MODULE].multi.lowPowerMode) {
    ALERT("MULTI", STR_WARN_MULTI_LOWPOWER, AU_ERROR);
  }
#endif
}
#endif

static void checkRTCBattery()
{
  GET_ADC_IF_MIXER_NOT_RUNNING();
  if (getRTCBatteryVoltage() < 200) {
    ALERT(STR_BATTERY, STR_WARN_RTC_BATTERY_LOW, AU_ERROR);
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
void checkAll()
{
#if defined(EEPROM_RLC) && !defined(SDCARD_RAW) && !defined(SDCARD_YAML)
  checkLowEEPROM();
#endif

  // we don't check the throttle stick if the radio is not calibrated
  if (g_eeGeneral.chkSum == evalChkSum()) {
    checkThrottleStick();
  }

  checkSwitches();
  checkFailsafe();


  if (isVBatBridgeEnabled() && !g_eeGeneral.disableRtcWarning) {
    // only done once at board start
    checkRTCBattery();
  }
  disableVBatBridge();

  if (g_model.displayChecklist && modelHasNotes()) {
    readModelNotes();
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
    for (int i = 0; i < (int)TRM_BASE; i++) {
      if (keys & (1 << i)) {
        strKeys += std::string(STR_VKEYS[i]);
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


#if defined(EEPROM_RLC) && !defined(SDCARD_RAW) && !defined(SDCARD_YAML)
void checkLowEEPROM()
{
  if (g_eeGeneral.disableMemoryWarning) return;
  if (EeFsGetFree() < 100) {
    ALERT(STR_STORAGE_WARNING, STR_EEPROMLOWMEM, AU_ERROR);
  }
}
#endif

bool isThrottleWarningAlertNeeded()
{
  if (g_model.disableThrottleWarning) {
    return false;
  }

  // throttle channel is either the stick according stick mode (already handled in evalInputs)
  // or P1 to P3;
  // in case an output channel is choosen as throttle source (thrTraceSrc>NUM_POTS+NUM_SLIDERS) we assume the throttle stick is the input
  // no other information available at the moment, and good enough to my option (otherwise too much exceptions...)
  uint8_t thrchn = ((g_model.thrTraceSrc==0) || (g_model.thrTraceSrc>NUM_POTS+NUM_SLIDERS)) ? THR_STICK : g_model.thrTraceSrc+NUM_STICKS-1;

  GET_ADC_IF_MIXER_NOT_RUNNING();
  evalInputs(e_perout_mode_notrainer); // let do evalInputs do the job

  int16_t v = calibratedAnalogs[thrchn];
  if (g_model.thrTraceSrc && g_model.throttleReversed) { // TODO : proper review of THR source definition and handling
    v = -v;
  }

  if (g_model.enableCustomThrottleWarning) {
    int16_t idleValue = (int32_t)RESX * (int32_t)g_model.customThrottleWarningPosition / (int32_t)100;
    return abs(v - idleValue) > THRCHK_DEADBAND;
  }
  else {
    return v > THRCHK_DEADBAND - RESX;
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
    AUDIO_ERROR_MESSAGE(AU_THROTTLE_ALERT);
    auto dialog =
        new FullScreenDialog(WARNING_TYPE_ALERT, TR_THROTTLE_UPPERCASE,
                             throttleNotIdle, STR_PRESS_ANY_KEY_TO_SKIP);
    dialog->setCloseCondition([]() { return !isThrottleWarningAlertNeeded(); });
    dialog->runForever();
    LED_ERROR_END();
  }
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

    forceBacklightOn = true;
    checkBacklight();

    WDG_RESET();

    RTOS_WAIT_MS(10);
  }

  forceBacklightOn = false;
  LED_ERROR_END();
}
#endif

void checkAlarm() // added by Ghost
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

    forceBacklightOn = true;
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

  forceBacklightOn = false;
  LED_ERROR_END();
}

#if defined(GVARS)
#if NUM_TRIMS == 6
  int8_t trimGvar[NUM_TRIMS] = { -1, -1, -1, -1, -1, -1 };
#elif NUM_TRIMS == 4
  int8_t trimGvar[NUM_TRIMS] = { -1, -1, -1, -1 };
#elif NUM_TRIMS == 2
  int8_t trimGvar[NUM_TRIMS] = { -1, -1 };
#endif
#endif

void checkTrims()
{
  event_t event = getEvent(true);
  if (event && !IS_KEY_BREAK(event)) {
    int8_t k = EVT_KEY_MASK(event) - TRM_BASE;
    // LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
    uint8_t idx = CONVERT_MODE_TRIMS((uint8_t)k/2);
    uint8_t phase;
    int before;
    bool thro;

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
    thro = (idx==THR_STICK && g_model.thrTrim);
#endif
    int8_t trimInc = g_model.trimInc + 1;
    int8_t v = (trimInc==-1) ? min(32, abs(before)/4+1) : (1 << trimInc); // TODO flash saving if (trimInc < 0)
    if (thro) v = 4; // if throttle trim and trim throttle then step=4
#if defined(GVARS)
    if (TRIM_REUSED(idx)) v = 1;
#endif
    int16_t after = (k&1) ? before + v : before - v;   // positive = k&1
    bool beepTrim = true;

    if (!thro && before!=0 && ((!(after < 0) == (before < 0)) || after==0)) { //forcing a stop at centered trim when changing sides
      after = 0;
      beepTrim = true;
      AUDIO_TRIM_MIDDLE();
      pauseEvents(event);
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
        killEvents(event);
      }
      else if (after > vmax) {
        after = vmax;
        beepTrim = false;
        AUDIO_TRIM_MAX();
        killEvents(event);
      }

      SET_GVAR_VALUE(gvar, phase, after);
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
        killEvents(event);
      }
      else if (before <= tMax && after >= tMax) {
        beepTrim = false;
        AUDIO_TRIM_MAX();
        killEvents(event);
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

uint8_t  trimsCheckTimer = 0;

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

#if defined(THRTRACE)
uint8_t  s_traceBuf[MAXTRACE];
uint16_t s_traceWr;
uint8_t  s_cnt_10s;
uint16_t s_cnt_samples_thr_10s;
uint16_t s_sum_samples_thr_10s;
#endif

void evalTrims()
{
  uint8_t phase = mixerCurrentFlightMode;
  for (uint8_t i=0; i<NUM_TRIMS; i++) {
    // do trim -> throttle trim if applicable
    int16_t trim = getTrimValue(phase, i);
    if (trimsCheckTimer > 0) {
      trim = 0;
    }

    trims[i] = trim*2;
  }
}

uint8_t s_mixer_first_run_done = false;

void doMixerCalculations()
{
  static tmr10ms_t lastTMR = 0;

  tmr10ms_t tmr10ms = get_tmr10ms();

#if defined(DEBUG_LATENCY_MIXER_RF) || defined(DEBUG_LATENCY_RF_ONLY)
  static tmr10ms_t lastLatencyToggle = 0;
  if (tmr10ms - lastLatencyToggle >= 10) {
    lastLatencyToggle = tmr10ms;
    toggleLatencySwitch();
  }
#endif

  uint8_t tick10ms = (tmr10ms >= lastTMR ? tmr10ms - lastTMR : 1);
  // handle tick10ms overrun
  // correct overflow handling costs a lot of code; happens only each 11 min;
  // therefore forget the exact calculation and use only 1 instead; good compromise
  lastTMR = tmr10ms;

  DEBUG_TIMER_START(debugTimerGetAdc);
  getADC();
  DEBUG_TIMER_STOP(debugTimerGetAdc);

  DEBUG_TIMER_START(debugTimerGetSwitches);
  getSwitchesPosition(!s_mixer_first_run_done);
  DEBUG_TIMER_STOP(debugTimerGetSwitches);



  DEBUG_TIMER_START(debugTimerEvalMixes);
  evalMixes(tick10ms);
  DEBUG_TIMER_STOP(debugTimerEvalMixes);
}

void doMixerPeriodicUpdates()
{
  static tmr10ms_t lastTMR = 0;

  tmr10ms_t tmr10ms = get_tmr10ms();

  uint8_t tick10ms = (tmr10ms >= lastTMR ? tmr10ms - lastTMR : 1);
  // handle tick10ms overrun
  // correct overflow handling costs a lot of code; happens only each 11 min;
  // therefore forget the exact calculation and use only 1 instead; good compromise
  lastTMR = tmr10ms;

  DEBUG_TIMER_START(debugTimerMixes10ms);
  if (tick10ms) {
    /* Throttle trace */
    int16_t val;

    if (g_model.thrTraceSrc > NUM_POTS+NUM_SLIDERS) {
      uint8_t ch = g_model.thrTraceSrc-NUM_POTS-NUM_SLIDERS-1;
      val = channelOutputs[ch];

      LimitData * lim = limitAddress(ch);
      int16_t gModelMax = LIMIT_MAX_RESX(lim);
      int16_t gModelMin = LIMIT_MIN_RESX(lim);

      if (lim->revert)
        val = -val + gModelMax;
      else
        val = val - gModelMin;

#if defined(PPM_LIMITS_SYMETRICAL)
      if (lim->symetrical) {
        val -= calc1000toRESX(lim->offset);
      }
#endif

      gModelMax -= gModelMin; // we compare difference between Max and Mix for recaling needed; Max and Min are shifted to 0 by default
      // usually max is 1024 min is -1024 --> max-min = 2048 full range

      if (gModelMax != 0 && gModelMax != 2048)
        val = (int32_t) (val << 11) / (gModelMax); // rescaling only needed if Min, Max differs

      if (val < 0)
        val=0;  // prevent val be negative, which would corrupt throttle trace and timers; could occur if safetyswitch is smaller than limits
    }
    else {
      val = RESX + calibratedAnalogs[g_model.thrTraceSrc == 0 ? THR_STICK : g_model.thrTraceSrc+NUM_STICKS-1];
    }

    val >>= (RESX_SHIFT-6); // calibrate it (resolution increased by factor 4)

    evalTimers(val, tick10ms);

    static uint8_t  s_cnt_100ms;
    static uint8_t  s_cnt_1s;
    static uint8_t  s_cnt_samples_thr_1s;
    static uint16_t s_sum_samples_thr_1s;

    s_cnt_samples_thr_1s++;
    s_sum_samples_thr_1s+=val;

    if ((s_cnt_100ms += tick10ms) >= 10) { // 0.1sec
      s_cnt_100ms -= 10;
      s_cnt_1s += 1;

      logicalSwitchesTimerTick();
      checkTrainerSignalWarning();

      if (s_cnt_1s >= 10) { // 1sec
        s_cnt_1s -= 10;
        sessionTimer += 1;
        inactivity.counter++;
        if ((((uint8_t)inactivity.counter) & 0x07) == 0x01 && g_eeGeneral.inactivityTimer && inactivity.counter > ((uint16_t)g_eeGeneral.inactivityTimer * 60))
          AUDIO_INACTIVITY();

#if defined(AUDIO)
        if (mixWarning & 1) if ((sessionTimer&0x03)==0) AUDIO_MIX_WARNING(1);
        if (mixWarning & 2) if ((sessionTimer&0x03)==1) AUDIO_MIX_WARNING(2);
        if (mixWarning & 4) if ((sessionTimer&0x03)==2) AUDIO_MIX_WARNING(3);
#endif

        val = s_sum_samples_thr_1s / s_cnt_samples_thr_1s;
        s_timeCum16ThrP += (val>>3);  // s_timeCum16ThrP would overrun if we would store throttle value with higher accuracy; therefore stay with 16 steps
        if (val)
          s_timeCumThr += 1;
        s_sum_samples_thr_1s >>= 2;  // correct better accuracy now, because trace graph can show this information; in case thrtrace is not active, the compile should remove this

#if defined(THRTRACE)
        // throttle trace is done every 10 seconds; Tracebuffer is adjusted to screen size.
        // in case buffer runs out, it wraps around
        // resolution for y axis is only 32, therefore no higher value makes sense
        s_cnt_samples_thr_10s += s_cnt_samples_thr_1s;
        s_sum_samples_thr_10s += s_sum_samples_thr_1s;

        if (++s_cnt_10s >= 10) { // 10s
          s_cnt_10s -= 10;
          val = s_sum_samples_thr_10s / s_cnt_samples_thr_10s;
          s_sum_samples_thr_10s = 0;
          s_cnt_samples_thr_10s = 0;
          s_traceBuf[s_traceWr % MAXTRACE] = val;
          s_traceWr++;
        }
#endif

        s_cnt_samples_thr_1s = 0;
        s_sum_samples_thr_1s = 0;
      }
    }

#if defined(PXX) || defined(DSM2)
    static uint8_t countRangecheck = 0;
    for (uint8_t i = 0; i < NUM_MODULES; ++i) {
      if (isModuleBeeping(i)) {
        if (++countRangecheck >= 250) {
          countRangecheck = 0;
          AUDIO_PLAY(AU_SPECIAL_SOUND_CHEEP);
        }
      }
    }
#endif

    checkTrims();
  }

  DEBUG_TIMER_STOP(debugTimerMixes10ms);

  s_mixer_first_run_done = true;
}

#if !defined(OPENTX_START_DEFAULT_ARGS)
  #define OPENTX_START_DEFAULT_ARGS  0
#endif

void opentxStart(const uint8_t startOptions = OPENTX_START_DEFAULT_ARGS)
{
  TRACE("opentxStart(%u)", startOptions);

  uint8_t calibration_needed = !(startOptions & OPENTX_START_NO_CALIBRATION) && (g_eeGeneral.chkSum != evalChkSum());

#if defined(GUI)
  if (!calibration_needed && !(startOptions & OPENTX_START_NO_SPLASH)) {
    AUDIO_HELLO();
    doSplash();
  }
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
#if defined(LIBOPENUI)
    startCalibration();
#else
    chainMenu(menuFirstCalib);
#endif
  }
  else if (!(startOptions & OPENTX_START_NO_CHECKS)) {
    checkAlarm();
    checkAll();
    PLAY_MODEL_NAME();
  }
#endif
}

void opentxClose(uint8_t shutdown)
{
  TRACE("opentxClose");

  watchdogSuspend(2000/*20s*/);

  if (shutdown) {
    pausePulses();   // stop mixer task to disable trims processing while in shutdown
    AUDIO_BYE();
    // TODO needed? telemetryEnd();
#if defined(LUA)
    luaClose(&lsScripts);
#endif
#if defined(HAPTIC)
    hapticOff();
#endif
  }

#if defined(SDCARD)
  logsClose();
#endif

  storageFlushCurrentModel();

  if (sessionTimer > 0) {
    g_eeGeneral.globalTimer += sessionTimer;
    sessionTimer = 0;
  }


  g_eeGeneral.unexpectedShutdown = 0;
  storageDirty(EE_GENERAL);
  storageCheck(true);

  while (IS_PLAYING(ID_PLAY_PROMPT_BASE + AU_BYE)) {
    RTOS_WAIT_MS(10);
  }

  RTOS_WAIT_MS(100);

#if defined(COLORLCD)
  // clear layer stack first
  for (Window* w = Layer::back(); w; w = Layer::back()) w->deleteLater();
  MainWindow::instance()->clear();
  // this is necessary as the custom screens are not deleted
  // by using deleteCustomScreens(), but here through it's parent window
  memset(customScreens, 0, sizeof(customScreens));

  //TODO: In fact we want only to empty the trash (private method)
  MainWindow::instance()->run();
#if defined(LUA)
  luaUnregisterWidgets();
  luaClose(&lsWidgets);
  lsWidgets = 0;
#endif
#endif

#if defined(SDCARD)
  sdDone();
#endif
}

void opentxResume()
{
  TRACE("opentxResume");

  sdMount();
#if defined(COLORLCD) && defined(LUA)
  // reload widgets
  luaInitThemesAndWidgets();
#endif

  storageReadAll();

#if defined(COLORLCD)
  //TODO: needs to go into storageReadAll()
  TRACE("reloading theme");
  loadTheme();

  // Force redraw
  ViewMain::instance()->invalidate();
  TRACE("theme reloaded & ViewMain invalidated");
#endif

  // removed to avoid the double warnings (throttle, switch, etc.)
  // opentxStart(OPENTX_START_NO_SPLASH | OPENTX_START_NO_CALIBRATION | OPENTX_START_NO_CHECKS);

  referenceSystemAudioFiles();

  if (!g_eeGeneral.unexpectedShutdown) {
    g_eeGeneral.unexpectedShutdown = 1;
    storageDirty(EE_GENERAL);
  }
}

#define INSTANT_TRIM_MARGIN 10 /* around 1% */

void instantTrim()
{
  int16_t anas_0[MAX_INPUTS];
  evalInputs(e_perout_mode_notrainer | e_perout_mode_nosticks);
  memcpy(anas_0, anas, sizeof(anas_0));

  evalInputs(e_perout_mode_notrainer);

  for (uint8_t stick = 0; stick < NUM_STICKS; stick++) {
    if (stick != THR_STICK) { // don't instant trim the throttle stick
      bool addTrim = false;
      int16_t delta = 0;
      uint8_t trimFlightMode = getTrimFlightMode(mixerCurrentFlightMode, stick);
      for (uint8_t i = 0; i < MAX_EXPOS; i++) {
        ExpoData * expo = expoAddress(i);
        if (!EXPO_VALID(expo))
          break; // end of list
        if (stick == expo->srcRaw - MIXSRC_FIRST_STICK) {
          if (expo->carryTrim < 0) {
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
  pauseMixerCalculations();
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
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

void copyTrimsToOffset(uint8_t ch)
{
  int16_t zero;

  pauseMixerCalculations();

  evalFlightModeMixes(e_perout_mode_noinput, 0); // do output loop - zero input sticks and trims
  zero = applyLimits(ch, chans[ch]);

  evalFlightModeMixes(e_perout_mode_noinput-e_perout_mode_notrims, 0); // do output loop - only trims

  int16_t output = applyLimits(ch, chans[ch]) - zero;
  int16_t v = g_model.limitData[ch].offset;
  if (g_model.limitData[ch].revert)
    output = -output;
  v += (output * 125) / 128;
  g_model.limitData[ch].offset = limit((int16_t)-1000, (int16_t)v, (int16_t)1000); // make sure the offset doesn't go haywire

  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

void copyMinMaxToOutputs(uint8_t ch)
{
  LimitData *ld = limitAddress(ch);
  int16_t min = ld->min;
  int16_t max = ld->max;
  int16_t center = ld->ppmCenter;

  pauseMixerCalculations();

  for (uint8_t chan = 0; chan < MAX_OUTPUT_CHANNELS; chan++) {
    ld = limitAddress(chan);
    ld->min = min;
    ld->max = max;
    ld->ppmCenter = center;
  }

  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

#if defined(STARTUP_ANIMATION)

inline uint32_t PWR_PRESS_DURATION_MIN()
{
  return (2 - g_eeGeneral.pwrOnSpeed) * 100;
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
      haptic.play(15, 3, PLAY_NOW);
    }
  }

  if (duration < PWR_PRESS_DURATION_MIN() || duration >= PWR_PRESS_DURATION_MAX) {
    boardOff();
  }
}
#endif

void moveTrimsToOffsets() // copy state of 3 primary to subtrim
{
  int16_t zeros[MAX_OUTPUT_CHANNELS];

  pauseMixerCalculations();

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
  for (uint8_t i=0; i<NUM_TRIMS; i++) {
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

  resumeMixerCalculations();

  storageDirty(EE_MODEL);
  AUDIO_WARNING2();
}

#if defined(ROTARY_ENCODER_NAVIGATION)
uint8_t rotencSpeed = ROTENC_LOWSPEED;
#endif

void opentxInit()
{
  TRACE("opentxInit");
#if defined(LIBOPENUI)
  // create ViewMain
  ViewMain::instance();
#elif defined(GUI)
  // TODO add a function for this (duplicated)
  menuHandlers[0] = menuMainView;
  menuHandlers[1] = menuModelSelect;
#endif

#if defined(STARTUP_ANIMATION)
  lcdRefreshWait();
  lcdClear();
  lcdRefresh();
  lcdRefreshWait();

  bool radioSettingsValid = storageReadRadioSettings(false);
  (void)radioSettingsValid;
#endif

  BACKLIGHT_ENABLE(); // we start the backlight during the startup animation

#if defined(STARTUP_ANIMATION)
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
  else {
    runStartupAnimation();
  }
#else // defined(PWR_BUTTON_PRESS)
  pwrOn();
#endif

  // Radios handle UNEXPECTED_SHUTDOWN() differently:
  //  * radios with WDT and EEPROM and CPU controlled power use Reset status register
  //    and eeGeneral.unexpectedShutdown
  //  * radios with SDCARD model storage use Reset status register and special
  //    variables in RAM. They can not use eeGeneral.unexpectedShutdown
  //  * radios without CPU controlled power can only use Reset status register (if available)
  if (UNEXPECTED_SHUTDOWN()) {
    TRACE("Unexpected Shutdown detected");
    globalData.unexpectedShutdown = 1;
  }

#if defined(RTC_BACKUP_RAM)
  SET_POWER_REASON(0);
#endif

#if defined(SDCARD)
  // SDCARD related stuff, only done if not unexpectedShutdown
  if (!globalData.unexpectedShutdown) {

    if (!sdMounted())
      sdInit();

#if !defined(COLORLCD)
    if (!sdMounted()) {
      g_eeGeneral.pwrOffSpeed = 2;
      runFatalErrorScreen(STR_NO_SDCARD);
    }
#endif

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
#endif
#if defined(HARDWARE_POWER_MANAGEMENT_UNIT)
        if (information.productFamily == FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT) {
          FrskyChipFirmwareUpdate device;
          if (device.flashFirmware(AUTOUPDATE_FILENAME, false) == nullptr)
            f_unlink(AUTOUPDATE_FILENAME);
        }
#endif
      }
    }
#endif

    logsInit();
  }
#endif

#if defined(EEPROM)
  if (!radioSettingsValid)
    storageReadRadioSettings();
  storageReadCurrentModel();
#endif

#if defined(COLORLCD) && defined(LUA)
  if (!globalData.unexpectedShutdown) {
    // ??? lua widget state must be prepared before the call to storageReadAll()
    luaInitThemesAndWidgets();
  }
#endif

  // handling of storage for radios that have no EEPROM
#if !defined(EEPROM)
#if defined(RTC_BACKUP_RAM) && !defined(SIMU)
  if (globalData.unexpectedShutdown) {
    // SDCARD not available, try to restore last model from RAM
    TRACE("rambackupRestore");
    rambackupRestore();
  }
  else {
    storageReadAll();
  }
#else
  storageReadAll();
#endif
#endif  // #if !defined(EEPROM)

  initSerialPorts();

  currentSpeakerVolume = requiredSpeakerVolume = g_eeGeneral.speakerVolume + VOLUME_LEVEL_DEF;
  currentBacklightBright = g_eeGeneral.backlightBright;
#if !defined(SOFTWARE_VOLUME)
  setScaledVolume(currentSpeakerVolume);
#endif

  referenceSystemAudioFiles();
  audioQueue.start();
  BACKLIGHT_ENABLE();



#if defined(SPORT_UPDATE_PWR_GPIO)
  SPORT_UPDATE_POWER_INIT();
#endif

#if defined(COLORLCD)
  loadTheme();
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

  if (!globalData.unexpectedShutdown) {
    opentxStart();
  }

#if !defined(RTC_BACKUP_RAM)
  if (!g_eeGeneral.unexpectedShutdown) {
    g_eeGeneral.unexpectedShutdown = 1;
    storageDirty(EE_GENERAL);
  }
#endif

#if defined(GUI) && !defined(COLORLCD)
  lcdSetContrast();
#endif

  resetBacklightTimeout();

  startPulses();

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
  NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
#endif

  TRACE("reusableBuffer: modelSel=%d, moduleSetup=%d, calib=%d, sdManager=%d, hardwareAndSettings=%d, spectrumAnalyser=%d, usb=%d",
        sizeof(reusableBuffer.modelsel),
        sizeof(reusableBuffer.moduleSetup),
        sizeof(reusableBuffer.calib),
        sizeof(reusableBuffer.sdManager),
        sizeof(reusableBuffer.hardwareAndSettings),
        sizeof(reusableBuffer.spectrumAnalyser),
        sizeof(reusableBuffer.MSC_BOT_Data));

  // G: The WDT remains active after a WDT reset -- at maximum clock speed. So it's
  // important to disable it before commencing with system initialisation (or
  // we could put a bunch more WDG_RESET()s in. But I don't like that approach
  // during boot up.)
#if defined(LCD_CONTRAST_DEFAULT)
  g_eeGeneral.contrast = LCD_CONTRAST_DEFAULT;
#endif

  boardInit();

#if !defined(SIMU)
  stackPaint();
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

#if defined(EEPROM) && defined(EEPROM_RLC)
  eepromInit();
#endif
  
  tasksStart();
}

#if !defined(SIMU)
#if defined(PWR_BUTTON_PRESS)

inline uint32_t PWR_PRESS_SHUTDOWN_DELAY()
{
  // Instant off when both power button are pressed
  if (pwrForcePressed())
    return 0;

  return (2 - g_eeGeneral.pwrOffSpeed) * 100;
}

uint32_t pwr_press_time = 0;

uint32_t pwrPressedDuration()
{
  if (pwr_press_time == 0) {
    return 0;
  }
  else {
    return get_tmr10ms() - pwr_press_time;
  }
}

#if defined(COLORLCD)
inline tmr10ms_t getTicks() { return g_tmr10ms; }
#endif

uint32_t pwrCheck()
{
  const char * message = nullptr;

  enum PwrCheckState {
    PWR_CHECK_ON,
    PWR_CHECK_OFF,
    PWR_CHECK_PAUSED,
  };

  static uint8_t pwr_check_state = PWR_CHECK_ON;

  if (pwr_check_state == PWR_CHECK_OFF) {
    return e_power_off;
  }
  else if (pwrPressed()) {
    if (g_eeGeneral.backlightMode == e_backlight_mode_keys ||
        g_eeGeneral.backlightMode == e_backlight_mode_all)
      resetBacklightTimeout();

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
      inactivity.counter = 0;
      if (g_eeGeneral.backlightMode != e_backlight_mode_off) {
        BACKLIGHT_ENABLE();
      }
      if (get_tmr10ms() - pwr_press_time > PWR_PRESS_SHUTDOWN_DELAY()) {
#if defined(COLORLCD)
        bool usbConfirmed = !usbPlugged() || getSelectedUsbMode() == USB_UNSELECTED_MODE;
        bool modelConnectedConfirmed = !TELEMETRY_STREAMING() || g_eeGeneral.disableRssiPoweroffAlarm;
#endif
#if defined(SHUTDOWN_CONFIRMATION)
        while (1)
#else
        while ((usbPlugged() && getSelectedUsbMode() != USB_UNSELECTED_MODE) ||
               (TELEMETRY_STREAMING() && !g_eeGeneral.disableRssiPoweroffAlarm))
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

          event_t evt = getEvent(false);
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

        haptic.play(15, 3, PLAY_NOW);
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

#if defined(TRAINER_PWR)
  if (TRAINER_CONNECTED()) {
    return e_power_trainer;
  }
#endif

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
