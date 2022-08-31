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
#include "hal/adc_driver.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
  #include "gui/colorlcd/LvglWrapper.h"
#endif

#if defined(CLI)
  #include "cli.h"
#endif

uint8_t currentSpeakerVolume = 255;
uint8_t requiredSpeakerVolume = 255;
uint8_t currentBacklightBright = 0;
bool forceBacklightOn = 0;
uint8_t mainRequestFlags = 0;

static bool _usbDisabled = false;

#if defined(LIBOPENUI)
static Menu* _usbMenu = nullptr;

void closeUsbMenu()
{
  if (_usbMenu && !usbPlugged()) {

    // USB has been unplugged meanwhile
    _usbMenu->deleteLater();
  }
}

void openUsbMenu()
{
  if (_usbMenu || _usbDisabled) return;
  
  _usbMenu = new Menu(MainWindow::instance());

  _usbMenu->setCloseHandler([]() {
    _usbMenu = nullptr;
  });

  _usbMenu->setCancelHandler([]() {
    if (usbPlugged() && (getSelectedUsbMode() == USB_UNSELECTED_MODE)) {
      TRACE("disable USB");
      _usbDisabled = true;
    }
  });

  _usbMenu->setTitle("USB");
  _usbMenu->addLine(STR_USB_JOYSTICK, [] {
    TRACE("USB set joystick");
    setSelectedUsbMode(USB_JOYSTICK_MODE);
  });
  _usbMenu->addLine(STR_USB_MASS_STORAGE, [] {
    TRACE("USB mass storage");
    setSelectedUsbMode(USB_MASS_STORAGE_MODE);
  });
#if defined(USB_SERIAL)
  _usbMenu->addLine(STR_USB_SERIAL, [] {
    TRACE("USB serial");
    setSelectedUsbMode(USB_SERIAL_MODE);
  });
#endif
}

#else

void onUSBConnectMenu(const char *result)
{
  if (result == STR_USB_MASS_STORAGE) {
    setSelectedUsbMode(USB_MASS_STORAGE_MODE);
  }
  else if (result == STR_USB_JOYSTICK) {
    setSelectedUsbMode(USB_JOYSTICK_MODE);
  }
#if defined(USB_SERIAL)
  else if (result == STR_USB_SERIAL) {
    setSelectedUsbMode(USB_SERIAL_MODE);
  }
#endif
  else if (result == STR_EXIT) {
    _usbDisabled = true;
  }
}

void openUsbMenu()
{
  if (popupMenuHandler != onUSBConnectMenu) {
    POPUP_MENU_ADD_ITEM(STR_USB_JOYSTICK);
    POPUP_MENU_ADD_ITEM(STR_USB_MASS_STORAGE);
#if defined(USB_SERIAL)
    POPUP_MENU_ADD_ITEM(STR_USB_SERIAL);
#endif
    POPUP_MENU_TITLE(STR_SELECT_MODE);
    POPUP_MENU_START(onUSBConnectMenu);
  }
}

void closeUsbMenu()
{
}

#endif

void handleUsbConnection()
{
#if defined(STM32) && !defined(SIMU)

  static bool _pluggedUsb = false;

  if (_pluggedUsb && !usbPlugged()) {
    TRACE("USB unplugged");
    closeUsbMenu();
    _pluggedUsb = false;
  }
  else if (!_pluggedUsb && usbPlugged()) {
    TRACE("USB plugged");
    _pluggedUsb = true;
    _usbDisabled = false;
  }
  
  if (!_usbDisabled && !usbStarted() && usbPlugged()) {

    if (getSelectedUsbMode() == USB_UNSELECTED_MODE) {
      if (g_eeGeneral.USBMode == USB_UNSELECTED_MODE) {
        openUsbMenu();
      }
      else {
        setSelectedUsbMode(g_eeGeneral.USBMode);
      }
    }

    // Mode might have been selected in previous block
    // so re-evaluate the condition
    if (getSelectedUsbMode() != USB_UNSELECTED_MODE) {

      if (getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
        opentxClose(false);
      }
#if defined(USB_SERIAL)
      else if (getSelectedUsbMode() == USB_SERIAL_MODE) {
        serialInit(SP_VCP, serialGetMode(SP_VCP));
      }
#endif

      usbStart();
      TRACE("USB started");
    }
  }

  if (usbStarted() && !usbPlugged()) {
    usbStop();
    TRACE("USB stopped");
    if (getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
      opentxResume();
      pushEvent(EVT_ENTRY);
    } else if (getSelectedUsbMode() == USB_SERIAL_MODE) {
      serialStop(SP_VCP);
    }
    TRACE("reset selected USB mode");
    setSelectedUsbMode(USB_UNSELECTED_MODE);
  }
#endif // defined(STM32) && !defined(SIMU)
}

#if defined(JACK_DETECT_GPIO) && !defined(SIMU)
bool isJackPlugged()
{
  // debounce
  static bool debounced_state = 0;
  static bool last_state = 0;

  if (GPIO_ReadInputDataBit(JACK_DETECT_GPIO, JACK_DETECT_GPIO_PIN)) {
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

#if defined(PCBXLITES)
uint8_t jackState = SPEAKER_ACTIVE;

const char STR_JACK_HEADPHONE[] = "Headphone";
const char STR_JACK_TRAINER[] = "Trainer";

void onJackConnectMenu(const char * result)
{
  if (result == STR_JACK_HEADPHONE) {
    jackState = HEADPHONE_ACTIVE;
    disableSpeaker();
    enableHeadphone();
  }
  else if (result == STR_JACK_TRAINER) {
    jackState = TRAINER_ACTIVE;
    enableTrainer();
  }
}

void handleJackConnection()
{
  if (jackState == SPEAKER_ACTIVE && isJackPlugged()) {
    if (g_eeGeneral.jackMode == JACK_HEADPHONE_MODE) {
      jackState = HEADPHONE_ACTIVE;
      disableSpeaker();
      enableHeadphone();
    }
    else if (g_eeGeneral.jackMode == JACK_TRAINER_MODE) {
      jackState = TRAINER_ACTIVE;
      enableTrainer();
    }
    else if (popupMenuItemsCount == 0) {
      POPUP_MENU_ADD_ITEM(STR_JACK_HEADPHONE);
      POPUP_MENU_ADD_ITEM(STR_JACK_TRAINER);
      POPUP_MENU_START(onJackConnectMenu);
    }
  }
  else if (jackState == SPEAKER_ACTIVE && !isJackPlugged() && popupMenuItemsCount > 0 && popupMenuHandler == onJackConnectMenu) {
    popupMenuItemsCount = 0;
  }
  else if (jackState != SPEAKER_ACTIVE && !isJackPlugged()) {
    jackState = SPEAKER_ACTIVE;
    enableSpeaker();
  }
}
#endif

void checkSpeakerVolume()
{
  if (currentSpeakerVolume != requiredSpeakerVolume) {
    currentSpeakerVolume = requiredSpeakerVolume;
#if !defined(SOFTWARE_VOLUME)
    setScaledVolume(currentSpeakerVolume);
#endif
  }
}

#if defined(EEPROM)
void checkEeprom()
{
  if (eepromIsWriting())
    eepromWriteProcess();
  else if (TIME_TO_WRITE())
    storageCheck(false);
}
#else
void checkEeprom()
{
#if defined(RTC_BACKUP_RAM) && !defined(SIMU)
  if (TIME_TO_BACKUP_RAM()) {
    if (!globalData.unexpectedShutdown) {
      rambackupWrite();
    }
    rambackupDirtyMsk = 0;
  }
#endif
  if (TIME_TO_WRITE()) {
    storageCheck(false);
  }
}
#endif

#define BAT_AVG_SAMPLES       8

void checkBatteryAlarms()
{
  // TRACE("checkBatteryAlarms()");
  if (IS_TXBATT_WARNING()) {
    AUDIO_TX_BATTERY_LOW();
    // TRACE("checkBatteryAlarms(): battery low");
  }
}

void checkBattery()
{
  static uint32_t batSum;
  static uint8_t sampleCount;
  // filter battery voltage by averaging it
  if (g_vbat100mV == 0) {
    g_vbat100mV = (getBatteryVoltage() + 5) / 10;
    batSum = 0;
    sampleCount = 0;
  }
  else {
    batSum += getBatteryVoltage();
    // TRACE("checkBattery(): sampled = %d", getBatteryVoltage());
    if (++sampleCount >= BAT_AVG_SAMPLES) {
      g_vbat100mV = (batSum + BAT_AVG_SAMPLES * 5 ) / (BAT_AVG_SAMPLES * 10);
      batSum = 0;
      sampleCount = 0;
      // TRACE("checkBattery(): g_vbat100mV = %d", g_vbat100mV);
    }
  }
}

void periodicTick_1s()
{
  checkBattery();
}

void periodicTick_10s()
{
  checkBatteryAlarms();
#if defined(LUA)
  checkLuaMemoryUsage();
#endif
}

void periodicTick()
{
  static uint8_t count10s;
  static uint32_t lastTime;
  if ( (get_tmr10ms() - lastTime) >= 100 ) {
    lastTime += 100;
    periodicTick_1s();
    if (++count10s >= 10) {
      count10s = 0;
      periodicTick_10s();
    }
  }
}

#if defined(GUI) && defined(COLORLCD)
void guiMain(event_t evt)
{

#if defined(LUA)
  uint32_t t0 = get_tmr10ms();
  static uint32_t lastLuaTime = 0;
  uint16_t interval = (lastLuaTime == 0 ? 0 : (t0 - lastLuaTime));
  lastLuaTime = t0;
  if (interval > maxLuaInterval) {
    maxLuaInterval = interval;
  }

  DEBUG_TIMER_START(debugTimerLua);
  luaTask(0, false);
  DEBUG_TIMER_STOP(debugTimerLua);

  t0 = get_tmr10ms() - t0;
  if (t0 > maxLuaDuration) {
    maxLuaDuration = t0;
  }
#endif

  LvglWrapper::instance()->run();
  MainWindow::instance()->run();

  bool screenshotRequested = (mainRequestFlags & (1u << REQUEST_SCREENSHOT));
  if (screenshotRequested) {
    writeScreenshot();
    mainRequestFlags &= ~(1u << REQUEST_SCREENSHOT);
  }
}
#elif defined(GUI)

bool handleGui(event_t event) {
  bool refreshNeeded;
#if defined(LUA)
  refreshNeeded = luaTask(event, true);
  if (menuHandlers[menuLevel] == menuViewTelemetry && TELEMETRY_SCREEN_TYPE(s_frsky_view) == TELEMETRY_SCREEN_TYPE_SCRIPT) {
      menuHandlers[menuLevel](event);
  }
  else if (scriptInternalData[0].reference != SCRIPT_STANDALONE)
#endif
// No foreground Lua script is running - clear the screen show normal menu
  {
    lcdClear();
    menuHandlers[menuLevel](event);
    drawStatusLine();
    refreshNeeded = true;
  }
  return refreshNeeded;
}

void guiMain(event_t evt)
{
  bool refreshNeeded = menuEvent || warningText || (popupMenuItemsCount > 0);
#if defined(LUA)
  // TODO better lua stopwatch
  uint32_t t0 = get_tmr10ms();
  static uint32_t lastLuaTime = 0;
  uint16_t interval = (lastLuaTime == 0 ? 0 : (t0 - lastLuaTime));
  lastLuaTime = t0;
  if (interval > maxLuaInterval) {
    maxLuaInterval = interval;
  }

  // run Lua scripts that don't use LCD (to use CPU time while LCD DMA is running)
  luaTask(0, false);

  t0 = get_tmr10ms() - t0;
  if (t0 > maxLuaDuration) {
    maxLuaDuration = t0;
  }
#endif //#if defined(LUA)

  // wait for LCD DMA to finish before continuing, because code from this point
  // is allowed to change the contents of LCD buffer
  //
  // WARNING: make sure no code above this line does any change to the LCD display buffer!
  //
  lcdRefreshWait();

  if (menuEvent) {
    // we have a popupMenuActive entry or exit event
    menuVerticalPosition = (menuEvent == EVT_ENTRY_UP) ? menuVerticalPositions[menuLevel] : 0;
    menuHorizontalPosition = 0;
    evt = menuEvent;
    menuEvent = 0;
  }

  if (isEventCaughtByPopup()) {
    refreshNeeded |= handleGui(0);
  }
  else {
    refreshNeeded |= handleGui(evt);
    evt = 0;
  }

  if (warningText) {
    // show warning on top of the normal menus
    DISPLAY_WARNING(evt);
  }
  else if (popupMenuItemsCount > 0) {
    // show popup menu on top of the normal menus
    const char * result = runPopupMenu(evt);
    if (result) {
      TRACE("popupMenuHandler(%s)", result);
      auto handler = popupMenuHandler;
      if (result != STR_UPDATE_LIST)
        CLEAR_POPUP();
      handler(result);
    }
  }

  if (refreshNeeded) lcdRefresh();
  
  if (mainRequestFlags & (1u << REQUEST_SCREENSHOT)) {
    writeScreenshot();
    mainRequestFlags &= ~(1u << REQUEST_SCREENSHOT);
  }
}
#endif

#if !defined(SIMU)
  void initLoggingTimer();
#endif

void perMain()
{
  DEBUG_TIMER_START(debugTimerPerMain1);

  checkSpeakerVolume();

  if (!usbPlugged() || (getSelectedUsbMode() == USB_UNSELECTED_MODE)) {
    checkEeprom();
    
    #if !defined(SIMU)     // use FreeRTOS software timer if radio firmware
      initLoggingTimer();  // initialize software timer for logging
    #else
      logsWrite();         // call logsWrite the old way for simu
    #endif
  }

  handleUsbConnection();

#if defined(PCBXLITES)
  handleJackConnection();
#endif

  checkTrainerSettings();
  periodicTick();
  DEBUG_TIMER_STOP(debugTimerPerMain1);

  if (mainRequestFlags & (1u << REQUEST_FLIGHT_RESET)) {
    TRACE("Executing requested Flight Reset");
    flightReset();
    mainRequestFlags &= ~(1u << REQUEST_FLIGHT_RESET);
  }

  checkBacklight();

#if !defined(LIBOPENUI)
  event_t evt = getEvent(false);
#endif

#if defined(RTC_BACKUP_RAM)
  if (globalData.unexpectedShutdown) {
    drawFatalErrorScreen(STR_EMERGENCY_MODE);
    return;
  }
#endif

  if ((!usbPlugged() || (getSelectedUsbMode() == USB_UNSELECTED_MODE))
      && SD_CARD_PRESENT() && !sdMounted()) {
    sdMount();
  }

#if !defined(EEPROM)
  // In case the SD card is removed during the session
  if ((!usbPlugged() || (getSelectedUsbMode() == USB_UNSELECTED_MODE))
      && !SD_CARD_PRESENT() && !globalData.unexpectedShutdown) {

    // TODO: implement for b/w
#if defined(COLORLCD)
    drawFatalErrorScreen(STR_NO_SDCARD);
    return;
#endif
  }
#endif

  if (usbPlugged() && getSelectedUsbMode() == USB_MASS_STORAGE_MODE) {
#if defined(LIBOPENUI)
    // draw some image showing USB
    lcdInitDirectDrawing();
    OpenTxTheme::instance()->drawUsbPluggedScreen(lcd);
    lcdRefresh();
#else
    // disable access to menus
    lcdClear();
    menuMainView(0);
    lcdRefresh();
#endif
    return;
  }

#if defined(MULTIMODULE)
  checkFailsafeMulti();
#endif
  
#if defined(KEYS_GPIO_REG_BIND) && defined(BIND_KEY)
  bindButtonHandler(evt);
#endif

#if defined(GUI)
  DEBUG_TIMER_START(debugTimerGuiMain);
#if defined(LIBOPENUI)
  guiMain(0);
#else
  guiMain(evt);
#endif
  DEBUG_TIMER_STOP(debugTimerGuiMain);
#endif

#if defined(PCBX9E) && !defined(SIMU)
  toplcdRefreshStart();
  setTopFirstTimer(getValue(MIXSRC_FIRST_TIMER+g_model.toplcdTimer));
  setTopSecondTimer(g_eeGeneral.globalTimer + sessionTimer);
  setTopRssi(TELEMETRY_RSSI());
  setTopBatteryValue(g_vbat100mV);
  setTopBatteryState(GET_TXBATT_BARS(10), IS_TXBATT_WARNING());
  toplcdRefreshEnd();
#endif

#if defined(INTERNAL_GPS)
  gpsWakeup();
#endif
}
