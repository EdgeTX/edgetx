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

#include "stm32_hal_ll.h"
#include "stm32_gpio.h"
#include "stm32_ws2812.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/module_port.h"
#include "hal/abnormal_reboot.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rgbleds.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/intmodule_heartbeat.h"
#include "boards/generic_stm32/analog_inputs.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "debug.h"
#include "rtc.h"

#include "timers_driver.h"
#include "dataconstants.h"
#include "trainer.h"

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif

#if !defined(BOOT)
  #include "edgetx.h"
  #if defined(PXX1)
    #include "pulses/pxx1.h"
  #endif
#endif

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
#endif

HardwareOptions hardwareOptions;

#if defined(LED_STRIP_GPIO)
extern const stm32_pulse_timer_t _led_timer;
#endif

#if !defined(BOOT)

#if defined(FUNCTION_SWITCHES)
#include "storage/storage.h"
#endif

#if defined(SIXPOS_SWITCH_INDEX)
uint8_t lastADCState = 0;
uint8_t sixPosState = 0;
bool dirty = true;
uint16_t getSixPosAnalogValue(uint16_t adcValue)
{
  uint8_t currentADCState = 0;
  if (adcValue > 3800)
    currentADCState = 6;
  else if (adcValue > 3100)
    currentADCState = 5;
  else if (adcValue > 2300)
    currentADCState = 4;
  else if (adcValue > 1500)
    currentADCState = 3;
  else if (adcValue > 1000)
    currentADCState = 2;
  else if (adcValue > 400)
    currentADCState = 1;
  if (lastADCState != currentADCState) {
    lastADCState = currentADCState;
  } else if (lastADCState != 0 && lastADCState - 1 != sixPosState) {
    sixPosState = lastADCState - 1;
    dirty = true;
  }
  if (dirty) {
    for (uint8_t i = 0; i < 6; i++) {
      if (i == sixPosState)
        ws2812_set_color(i, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      else
        ws2812_set_color(i, 0, 0, 0);
    }
    ws2812_update(&_led_timer);
  }
  return (4096/5)*(sixPosState);
}
#endif

void boardInit()
{
  LL_APB1_GRP1_EnableClock(AUDIO_RCC_APB1Periph);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

#if defined(USB_CHARGE_LED) && !defined(DEBUG)
  usbInit();
  // prime debounce state...
  usbPlugged();

  if (usbPlugged()) {
    delaysInit();
#if defined(AUDIO_MUTE_GPIO)
     // Charging can make a buzzing noise
     gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
     gpio_set(AUDIO_MUTE_GPIO);
 #endif
    while (usbPlugged()) {
      delay_ms(1000);
    }
    pwrOff();
  }
#endif

#if defined(BLUETOOTH) && !defined(PCBX9E)
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
#endif

#if defined(MANUFACTURER_RADIOMASTER) && defined(STM32F407xx)
  void board_set_bor_level();
  board_set_bor_level();
#endif

  board_trainer_init();

  // Sets 'hardwareOption.pcbrev' as well
  pwrInit();
  boardInitModulePorts();

#if defined(INTERNAL_MODULE_PXX1) && defined(PXX_FREQUENCY_HIGH)
  pxx1SetInternalBaudrate(PXX1_FAST_SERIAL_BAUDRATE);
#endif

#if defined(INTMODULE_HEARTBEAT) &&                                     \
  (defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2))
  pulsesSetModuleInitCb(_intmodule_heartbeat_init);
  pulsesSetModuleDeInitCb(_intmodule_heartbeat_deinit);
  trainerSetChangeCb(_intmodule_heartbeat_trainer_hook);
#endif
  
// #if defined(AUTOUPDATE)
//   telemetryPortInit(FRSKY_SPORT_BAUDRATE, TELEMETRY_SERIAL_WITHOUT_DMA);
//   sportSendByteLoop(0x7E);
// #endif

#if defined(STATUS_LEDS)
  ledInit();
#if defined(MANUFACTURER_RADIOMASTER) || defined(MANUFACTURER_JUMPER) || defined(RADIO_COMMANDO8)
  ledBlue();
#else
  ledGreen();
#endif
#endif

// If the radio was powered on by dual use USB, halt the boot process, let battery charge
// TODO: needs refactoring if any other manufacturer implements either of the following:
// - function switches and the radio supports charging
// - single USB for data + charge which powers on radio
#if defined(MANUFACTURER_JUMPER) && !defined(USB_CHARGE_LED)
  // This is needed to prevent radio from starting when usb is plugged to charge
  usbInit();
  // prime debounce state...
  usbPlugged();

  if (usbPlugged()) {
    delaysInit();
    __enable_irq();
    adcInit(&_adc_driver);
    getADC();
    pwrOn();  // required to get bat adc reads
    INTERNAL_MODULE_OFF();
    EXTERNAL_MODULE_OFF();
    delay_ms(2000); // let this stabilize
    while (usbPlugged()) {
      //    // Let it charge ...
      getADC();  // Warning: the value read does not include VBAT calibration
      delay_ms(20);
#if defined(FUNCTION_SWITCHES)
      // Support for FS Led to indicate battery charge level
      if (getBatteryVoltage() >= 660) setFSLedON(0);
      if (getBatteryVoltage() >= 700) setFSLedON(1);
      if (getBatteryVoltage() >= 740) setFSLedON(2);
      if (getBatteryVoltage() >= 780) setFSLedON(3);
      if (getBatteryVoltage() >= 820) setFSLedON(4);
      if (getBatteryVoltage() >= 842) setFSLedON(5);
#elif defined(STATUS_LEDS)
      // Use Status LED to indicate battery charge level instead
      if (getBatteryVoltage() <= 660) ledRed();         // low discharge
      else if (getBatteryVoltage() <= 842) ledBlue();   // charging
      else ledGreen();                                  // charging done
      delay_ms(1000);
#endif
    }
    pwrOff();
  }
#endif

  delaysInit();
  __enable_irq();

  keysInit();
  switchInit();

#if defined(ROTARY_ENCODER_NAVIGATION)
  rotaryEncoderInit();
#endif

#if defined(PWM_STICKS)
  sticksPwmDetect();
#endif

#if defined(FLYSKY_GIMBAL)
  flysky_gimbal_init();
#endif

  if (!adcInit(&_adc_driver))
    TRACE("adcInit failed");

  lcdInit(); // delaysInit() must be called before
  audioInit();
  timersInit();
  usbInit();

#if defined(LED_STRIP_GPIO)
  ws2812_init(&_led_timer, LED_STRIP_LENGTH, WS2812_GRB);
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 50);
  }
  ws2812_update(&_led_timer);
#endif

#if defined(HAPTIC)
  hapticInit();
#endif

#if defined(PXX2_PROBE)
  intmodulePxx2Probe();
#endif

#if defined(DEBUG)
  // Freeze timers & watchdog when core is halted
  DBGMCU->APB1FZ = 0x00E009FF;
  DBGMCU->APB2FZ = 0x00070003;
#endif

#if defined(PWR_BUTTON_PRESS)
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
#endif

#if defined(TOPLCD_GPIO)
  toplcdInit();
#endif

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

#if defined(JACK_DETECT_GPIO)
  initJackDetect();
#endif

  initSpeakerEnable();
  enableSpeaker();

  initHeadphoneTrainerSwitch();

#if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif

  backlightInit();

#if defined(GUI)
  lcdSetContrast(true);
#endif

#if defined(RADIO_GX12)
  gpio_init(HALL_SYNC, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

}
#endif

void boardOff()
{
#if defined(STATUS_LEDS) && !defined(BOOT)
  ledOff();
#endif

  BACKLIGHT_DISABLE();

#if defined(TOPLCD_GPIO) && !defined(BOOT)
  toplcdOff();
#endif

#if defined(PWR_BUTTON_PRESS)
  while (pwrPressed()) {
    WDG_RESET();
  }
#endif

#if defined(MANUFACTURER_RADIOMASTER) && defined(STM32F407xx)
  lcdInit(); 
#endif

  lcdOff();
  SysTick->CTRL = 0; // turn off systick
  pwrOff();

  // disable interrupts
  __disable_irq();

  while (1) {
    WDG_RESET();
#if defined(PWR_BUTTON_PRESS)
    // X9E/X7 needs watchdog reset because CPU is still running while
    // the power key is held pressed by the user.
    // The power key should be released by now, but we must make sure
    if (!pwrPressed()) {
      // Put the CPU into sleep to reduce the consumption,
      // it might help with the RTC reset issue
      PWR->CR |= PWR_CR_CWUF;
      /* Select STANDBY mode */
      PWR->CR |= PWR_CR_PDDS;
      /* Set SLEEPDEEP bit of Cortex System Control Register */
      SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
      /* Request Wait For Event */
      __WFE();
    }
#endif
  }

  // this function must not return!
}

#if defined(AUDIO_SPEAKER_ENABLE_GPIO)
void initSpeakerEnable()
{
  gpio_init(AUDIO_SPEAKER_ENABLE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

void enableSpeaker()
{
  gpio_set(AUDIO_SPEAKER_ENABLE_GPIO);
}

void disableSpeaker()
{
  gpio_clear(AUDIO_SPEAKER_ENABLE_GPIO);
}
#endif

#if defined(HEADPHONE_TRAINER_SWITCH_GPIO)
void initHeadphoneTrainerSwitch()
{
  gpio_init(HEADPHONE_TRAINER_SWITCH_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

void enableHeadphone()
{
  gpio_clear(HEADPHONE_TRAINER_SWITCH_GPIO);
}

void enableTrainer()
{
  gpio_set(HEADPHONE_TRAINER_SWITCH_GPIO);
}
#endif

#if defined(JACK_DETECT_GPIO)
void initJackDetect(void)
{
  gpio_init(JACK_DETECT_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
}

bool isJackPlugged()
{
  // debounce
  static bool debounced_state = 0;
  static bool last_state = 0;

  if (gpio_read(JACK_DETECT_GPIO)) {
    if (!last_state) {
      debounced_state = false;
    }
    last_state = false;
  }
  else {
    if (last_state) {
      debounced_state = true;
    }
    last_state = true;
  }
  return debounced_state;
}
#endif
