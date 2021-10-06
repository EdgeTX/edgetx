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
#include "tabsgroup.h"
#include "480_bitmaps.h"
#include "theme_manager.h"

const ZoneOption OPTIONS_THEME_DEFAULT[] = {
  { STR_BACKGROUND_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_PRIMARY2) },
  { STR_MAIN_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_WARNING) },
  { nullptr, ZoneOption::Bool }
};

class Theme480: public OpenTxTheme
{
  public:
    Theme480():
      OpenTxTheme("EdgeTX", OPTIONS_THEME_DEFAULT)
    {
      loadColors();
    }

    void loadColors() const
    {
      TRACE("Load EdgeTX theme colors");

      lcdColorTable[DEFAULT_COLOR_INDEX] = RGB(18, 94, 153);

      lcdColorTable[COLOR_THEME_PRIMARY1_INDEX] = RGB(0, 0, 0);
      lcdColorTable[COLOR_THEME_PRIMARY2_INDEX] = RGB(255, 255, 255);
      lcdColorTable[COLOR_THEME_PRIMARY3_INDEX] = RGB(12, 63, 102);
      lcdColorTable[COLOR_THEME_SECONDARY1_INDEX] = RGB(18, 94, 153);
      lcdColorTable[COLOR_THEME_SECONDARY2_INDEX] = RGB(182, 224, 242);
      lcdColorTable[COLOR_THEME_SECONDARY3_INDEX] = RGB(228, 238, 242);
      lcdColorTable[COLOR_THEME_FOCUS_INDEX] = RGB(20, 161, 229);
      lcdColorTable[COLOR_THEME_EDIT_INDEX] = RGB(0, 153, 9);
      lcdColorTable[COLOR_THEME_ACTIVE_INDEX] = RGB(255, 222, 0);
      lcdColorTable[COLOR_THEME_WARNING_INDEX] = RGB(224, 0, 0);
      lcdColorTable[COLOR_THEME_DISABLED_INDEX] = RGB(140, 140, 140);
      lcdColorTable[CUSTOM_COLOR_INDEX] = RGB(170, 85, 0);
    }

    void loadMenuIcon(uint8_t index, const uint8_t * lbm, bool reload) const
    {
      BitmapBuffer * mask;
      
      if (reload) {
        mask = BitmapBuffer::load8bitMask(lbm);
        if (mask) {
          delete iconMask[index];
          iconMask[index] = mask;

          delete menuIconNormal[index];
          menuIconNormal[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());

          delete menuIconSelected[index];
          menuIconSelected[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
        }
      } else
        mask = iconMask[index];
      
      if (menuIconNormal[index]) {
        menuIconNormal[index]->clear(COLOR_THEME_SECONDARY1);
        menuIconNormal[index]->drawMask(0, 0, mask, COLOR_THEME_PRIMARY2);
      }
  
      if (menuIconSelected[index]) {
        menuIconSelected[index]->clear(COLOR_THEME_FOCUS);
        menuIconSelected[index]->drawMask(0, 0, mask, COLOR_THEME_PRIMARY2);
      }
    }

    void loadIcons(bool reload) const
    {
#if defined(LOG_TELEMETRY) || !defined(WATCHDOG)
      loadMenuIcon(ICON_OPENTX, mask_opentx_testmode, reload);
#else
      loadMenuIcon(ICON_OPENTX, mask_edgetx, reload);
#endif
#if defined(HARDWARE_TOUCH) //TODO: get rid of, and use a real hitbox instead...
      loadMenuIcon(ICON_NEXT, mask_next, reload);
      loadMenuIcon(ICON_BACK, mask_back, reload);
#endif
      loadMenuIcon(ICON_RADIO, mask_menu_radio, reload);
      loadMenuIcon(ICON_RADIO_SETUP, mask_radio_setup, reload);
      loadMenuIcon(ICON_RADIO_SD_MANAGER, mask_radio_sd_browser, reload);
      loadMenuIcon(ICON_RADIO_TOOLS, mask_radio_tools, reload);
      //loadMenuIcon(ICON_RADIO_SPECTRUM_ANALYSER, mask_radio_spectrum_analyser, reload);
      loadMenuIcon(ICON_RADIO_GLOBAL_FUNCTIONS, mask_radio_global_functions, reload);
      loadMenuIcon(ICON_RADIO_TRAINER, mask_radio_trainer, reload);
      loadMenuIcon(ICON_RADIO_HARDWARE, mask_radio_hardware, reload);
      loadMenuIcon(ICON_RADIO_CALIBRATION, mask_radio_calibration, reload);
      loadMenuIcon(ICON_RADIO_EDIT_THEME, mask_radio_edit_theme, reload);
      loadMenuIcon(ICON_RADIO_VERSION, mask_radio_version, reload);
      loadMenuIcon(ICON_MODEL, mask_menu_model, reload);
      loadMenuIcon(ICON_MODEL_SETUP, mask_model_setup, reload);
      loadMenuIcon(ICON_MODEL_HELI, mask_model_heli, reload);
      loadMenuIcon(ICON_MODEL_FLIGHT_MODES, mask_model_flight_modes, reload);
      loadMenuIcon(ICON_MODEL_INPUTS, mask_model_inputs, reload);
      loadMenuIcon(ICON_MODEL_MIXER, mask_model_mixer, reload);
      loadMenuIcon(ICON_MODEL_NOTES, mask_menu_notes, reload);
      loadMenuIcon(ICON_MODEL_OUTPUTS, mask_model_outputs, reload);
      loadMenuIcon(ICON_MODEL_CURVES, mask_model_curves, reload);
      loadMenuIcon(ICON_MODEL_GVARS, mask_model_gvars, reload);
      loadMenuIcon(ICON_MODEL_LOGICAL_SWITCHES, mask_model_logical_switches, reload);
      loadMenuIcon(ICON_MODEL_SPECIAL_FUNCTIONS, mask_model_special_functions, reload);
      loadMenuIcon(ICON_MODEL_LUA_SCRIPTS, mask_model_lua_scripts, reload);
      loadMenuIcon(ICON_MODEL_TELEMETRY, mask_model_telemetry, reload);
      loadMenuIcon(ICON_MODEL_SELECT, mask_menu_model_select, reload);
      loadMenuIcon(ICON_MODEL_SELECT_CATEGORY, mask_model_select_category, reload);
      loadMenuIcon(ICON_STATS, mask_menu_stats, reload);
      loadMenuIcon(ICON_STATS_THROTTLE_GRAPH, mask_stats_throttle_graph, reload);
      loadMenuIcon(ICON_STATS_TIMERS, mask_stats_timers, reload);
      loadMenuIcon(ICON_STATS_ANALOGS, mask_stats_analogs, reload);
      loadMenuIcon(ICON_STATS_DEBUG, mask_stats_debug, reload);
      loadMenuIcon(ICON_THEME, mask_menu_theme, reload);
      loadMenuIcon(ICON_THEME_SETUP, mask_theme_setup, reload);
      loadMenuIcon(ICON_THEME_VIEW1, mask_theme_view1, reload);
      loadMenuIcon(ICON_THEME_VIEW2, mask_theme_view2, reload);
      loadMenuIcon(ICON_THEME_VIEW3, mask_theme_view3, reload);
      loadMenuIcon(ICON_THEME_VIEW4, mask_theme_view4, reload);
      loadMenuIcon(ICON_THEME_VIEW5, mask_theme_view5, reload);
      loadMenuIcon(ICON_THEME_ADD_VIEW, mask_theme_add_view, reload);
      loadMenuIcon(ICON_MONITOR, mask_monitor, reload);
      loadMenuIcon(ICON_MONITOR_CHANNELS1, mask_monitor_channels1, reload);
      loadMenuIcon(ICON_MONITOR_CHANNELS2, mask_monitor_channels2, reload);
      loadMenuIcon(ICON_MONITOR_CHANNELS3, mask_monitor_channels3, reload);
      loadMenuIcon(ICON_MONITOR_CHANNELS4, mask_monitor_channels4, reload);
      loadMenuIcon(ICON_MONITOR_LOGICAL_SWITCHES, mask_monitor_logsw, reload);

      BitmapBuffer * background = BitmapBuffer::load8bitMask(mask_currentmenu_bg);
      BitmapBuffer * shadow = BitmapBuffer::load8bitMask(mask_currentmenu_shadow);
      BitmapBuffer * dot = BitmapBuffer::load8bitMask(mask_currentmenu_dot);

      if (!currentMenuBackground) {
        currentMenuBackground = new BitmapBuffer(BMP_RGB565, 36, 53);
      }

      if (currentMenuBackground) {

        currentMenuBackground->drawSolidFilledRect(
            0, 0, currentMenuBackground->width(), MENU_HEADER_HEIGHT,
            COLOR_THEME_SECONDARY1);

        currentMenuBackground->drawSolidFilledRect(
            0, MENU_HEADER_HEIGHT, currentMenuBackground->width(),
            MENU_TITLE_TOP - MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY3);

        currentMenuBackground->drawSolidFilledRect(
            0, MENU_TITLE_TOP, currentMenuBackground->width(),
            currentMenuBackground->height() - MENU_TITLE_TOP, COLOR_THEME_SECONDARY1);

        currentMenuBackground->drawMask(0, 0, background,
                                        COLOR_THEME_FOCUS);

        currentMenuBackground->drawMask(0, 0, shadow, COLOR_THEME_PRIMARY1);

        currentMenuBackground->drawMask(10, 39, dot, COLOR_THEME_PRIMARY2);
      }

      delete topleftBitmap;
      topleftBitmap = BitmapBuffer::load8bitMaskOnBackground(
          mask_topleft, COLOR_THEME_FOCUS, COLOR_THEME_SECONDARY1);

      delete background;
      delete shadow;
      delete dot;
    }

    void loadThemeBitmaps() const
    {
      // Calibration screen
      delete calibStick;
      calibStick =
          BitmapBuffer::loadRamBitmap(stick_pointer, sizeof(stick_pointer));

      delete calibStickBackground;
      calibStickBackground = BitmapBuffer::loadRamBitmap(
          stick_background, sizeof(stick_background));

      delete calibTrackpBackground;
      calibTrackpBackground = BitmapBuffer::loadRamBitmap(
          trackp_background, sizeof(trackp_background));

      // Model Selection screen

      // Unused:
      //
      // delete modelselIconBitmap;
      // modelselIconBitmap = BitmapBuffer::loadMaskOnBackground("modelsel/mask_iconback.png", COLOR_THEME_SECONDARY1, COLOR_THEME_SECONDARY3);
      // if (modelselIconBitmap) {
      //   BitmapBuffer * bitmap = BitmapBuffer::loadBitmap(getFilePath("modelsel/icon_default.png"));
      //   modelselIconBitmap->drawBitmap(20, 8, bitmap);
      //   delete bitmap;
      // }

      delete modelselSdFreeBitmap;
      modelselSdFreeBitmap = BitmapBuffer::load8bitMask(mask_sdfree);

      delete modelselModelQtyBitmap;
      modelselModelQtyBitmap = BitmapBuffer::load8bitMask(mask_modelqty);

      delete modelselModelNameBitmap;
      modelselModelNameBitmap = BitmapBuffer::load8bitMask(mask_modelname);

      delete modelselModelMoveBackground;
      modelselModelMoveBackground = BitmapBuffer::load8bitMask(mask_moveback);

      delete modelselModelMoveIcon;
      modelselModelMoveIcon = BitmapBuffer::load8bitMask(mask_moveico);

      //TODO: should be loaded from LUA, not here!!!
      delete modelselWizardBackground;
      modelselWizardBackground = BitmapBuffer::loadBitmap(getFilePath("wizard/background.png"));

      // Channels monitor screen
      delete chanMonLockedBitmap;
      chanMonLockedBitmap = BitmapBuffer::load8bitMask(mask_monitor_lockch);

      delete chanMonInvertedBitmap;
      chanMonInvertedBitmap = BitmapBuffer::load8bitMask(mask_monitor_inver);

      // Mixer setup screen
      delete mixerSetupMixerBitmap;
      mixerSetupMixerBitmap = BitmapBuffer::load8bitMask(mask_sbar_mixer);

      delete mixerSetupToBitmap;
      mixerSetupToBitmap = BitmapBuffer::load8bitMask(mask_sbar_to);

      delete mixerSetupOutputBitmap;
      mixerSetupOutputBitmap = BitmapBuffer::load8bitMask(mask_sbar_output);

      delete mixerSetupAddBitmap;
      mixerSetupAddBitmap = BitmapBuffer::load8bitMask(mask_mplex_add);

      delete mixerSetupMultiBitmap;
      mixerSetupMultiBitmap = BitmapBuffer::load8bitMask(mask_mplex_multi);

      delete mixerSetupReplaceBitmap;
      mixerSetupReplaceBitmap = BitmapBuffer::load8bitMask(mask_mplex_replace);

      delete mixerSetupLabelIcon;
      mixerSetupLabelIcon = BitmapBuffer::load8bitMask(mask_textline_label);

      delete mixerSetupCurveIcon;
      mixerSetupCurveIcon = BitmapBuffer::load8bitMask(mask_textline_curve);

      delete mixerSetupSwitchIcon;
      mixerSetupSwitchIcon = BitmapBuffer::load8bitMask(mask_textline_switch);

      delete mixerSetupFlightmodeIcon;
      mixerSetupFlightmodeIcon = BitmapBuffer::load8bitMask(mask_textline_fm);

      delete mixerSetupSlowIcon;
      mixerSetupSlowIcon = BitmapBuffer::load8bitMask(mask_textline_slow);

      delete mixerSetupDelayIcon;
      mixerSetupDelayIcon = BitmapBuffer::load8bitMask(mask_textline_delay);

      delete mixerSetupDelaySlowIcon;
      mixerSetupDelaySlowIcon = BitmapBuffer::load8bitMask(mask_textline_delayslow);
    }

    void load() const override
    {
      loadColors();
      ThemePersistance::instance()->loadDefaultTheme();
      OpenTxTheme::load();
      if (!backgroundBitmap) {
        backgroundBitmap = BitmapBuffer::loadBitmap(getFilePath("background.png"));
      }
      update();
    }

    void update(bool reload = true) const override
    {
      loadIcons(reload);
      if (reload)
        loadThemeBitmaps();
    }

    void drawBackground(BitmapBuffer * dc) const override
    {
      if (backgroundBitmap) {
        dc->drawBitmap(0 - dc->getOffsetX(), 0 - dc->getOffsetY(), backgroundBitmap);
      }
      else {
        dc->drawSolidFilledRect(0 - dc->getOffsetX(), 0 - dc->getOffsetY(),
                                LCD_W, LCD_H, COLOR_THEME_SECONDARY3);
      }
    }

    void drawTopLeftBitmap(BitmapBuffer * dc) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      }
    }

    void drawPageHeaderBackground(BitmapBuffer *dc, uint8_t icon,
                                  const char *title) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        uint16_t width = topleftBitmap->width();
        dc->drawSolidFilledRect(width, 0, LCD_W - width, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
      }
      else {
        dc->drawSolidFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, COLOR_THEME_SECONDARY1);
      }

      if (icon == ICON_OPENTX)
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      else
        dc->drawBitmap(5, 7, menuIconSelected[icon]);

      dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W,
                              MENU_TITLE_TOP - MENU_HEADER_HEIGHT,
                              COLOR_THEME_SECONDARY3);  // the white separation line

      dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT,
                              COLOR_THEME_SECONDARY1);  // the title line background
      if (title) {
        dc->drawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP + 3, title, COLOR_THEME_PRIMARY2);
      }

      drawMenuDatetime(dc);
    }

    const BitmapBuffer * getIconMask(uint8_t index) const override
    {
      return iconMask[index];
    }

    const BitmapBuffer * getIcon(uint8_t index, IconState state) const override
    {
      return state == STATE_DEFAULT ? menuIconNormal[index] : menuIconSelected[index];
    }

    void drawPageHeader(BitmapBuffer *dc, std::vector<PageTab *> &tabs,
                        uint8_t currentIndex) const override
    {
      for (unsigned index = 0; index < tabs.size(); index++) {
        if (index != currentIndex) {
          dc->drawBitmap(index * MENU_HEADER_BUTTON_WIDTH + 2, 7,
                         menuIconNormal[tabs[index]->getIcon()]);
        }
      }
      dc->drawBitmap(currentIndex * MENU_HEADER_BUTTON_WIDTH, 0,
                     currentMenuBackground);
      dc->drawBitmap(currentIndex * MENU_HEADER_BUTTON_WIDTH + 2, 7,
                     menuIconSelected[tabs[currentIndex]->getIcon()]);
    }

    void drawMenuDatetime(BitmapBuffer * dc) const
    {
      //dc->drawSolidVerticalLine(DATETIME_SEPARATOR_X, 7, 31, COLOR_THEME_PRIMARY2);
      const TimerOptions timerOptions = {.options = SHOW_TIME};
      struct gtm t;
      gettime(&t);
      char str[10];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW)
      sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
      const char * const STR_MONTHS[] = TR_MONTHS;
      sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE1, str, FONT(XS)|COLOR_THEME_PRIMARY2|CENTERED);
      getTimerString(str, getValue(MIXSRC_TX_TIME), timerOptions);
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE2, str, FONT(XS)|COLOR_THEME_PRIMARY2|CENTERED);
    }

    void drawProgressBar(BitmapBuffer *dc, coord_t x, coord_t y, coord_t w,
                         coord_t h, int value, int total) const override
    {
      dc->drawSolidRect(x, y, w, h, 1, COLOR_THEME_SECONDARY1);
      if (value > 0) {
        int width = (w * value) / total;
        dc->drawSolidFilledRect(x + 2, y + 2, width - 4, h - 4, COLOR_THEME_FOCUS);
      }
    }

  protected:
    static const BitmapBuffer * backgroundBitmap;
    static BitmapBuffer * topleftBitmap;
    static BitmapBuffer * menuIconNormal[MENUS_ICONS_COUNT];
    static BitmapBuffer * menuIconSelected[MENUS_ICONS_COUNT];
    static BitmapBuffer * iconMask[MENUS_ICONS_COUNT];
    static BitmapBuffer * currentMenuBackground;
};

const BitmapBuffer * Theme480::backgroundBitmap = nullptr;
BitmapBuffer * Theme480::topleftBitmap = nullptr;
BitmapBuffer * Theme480::iconMask[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::menuIconNormal[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::menuIconSelected[MENUS_ICONS_COUNT] = { nullptr };
BitmapBuffer * Theme480::currentMenuBackground = nullptr;

Theme480 Theme480;

#if LCD_W == 480  || LCD_H == 480
OpenTxTheme * defaultTheme = &Theme480;
Theme * theme = &Theme480;
#endif
