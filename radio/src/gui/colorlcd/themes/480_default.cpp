/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

const ZoneOption OPTIONS_THEME_DEFAULT[] = {
  { STR_BACKGROUND_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(WHITE) },
  { STR_MAIN_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(RED) },
  { nullptr, ZoneOption::Bool }
};

class Theme480: public OpenTxTheme
{
  public:
    Theme480():
      OpenTxTheme("480_default", OPTIONS_THEME_DEFAULT)
    {
      loadColors();
    }

    void loadColors() const
    {
      TRACE("Load Theme480 theme colors");
      lcdColorTable[ALARM_COLOR_INDEX] = RED;
      lcdColorTable[BARGRAPH1_COLOR_INDEX] = RED;
      lcdColorTable[BARGRAPH2_COLOR_INDEX] = RGB(167, 167, 167);
      lcdColorTable[BARGRAPH_BGCOLOR_INDEX] = RGB(222, 222, 222);
      lcdColorTable[BATTERY_CHARGE_COLOR_INDEX] = GREEN;
      lcdColorTable[CHECKBOX_COLOR_INDEX] = RED;
      lcdColorTable[CURVE_COLOR_INDEX] = RED;
      lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = RED;
      lcdColorTable[DEFAULT_BGCOLOR_INDEX] = WHITE;
      lcdColorTable[DEFAULT_COLOR_INDEX] = BLACK;
      lcdColorTable[DISABLE_COLOR_INDEX] = LIGHTGREY;
      lcdColorTable[FOCUS_BGCOLOR_INDEX] = RED;
      lcdColorTable[FOCUS_COLOR_INDEX] = WHITE;
      lcdColorTable[HEADER_COLOR_INDEX] = DARKRED;
      lcdColorTable[HEADER_CURRENT_BGCOLOR_INDEX] = RED;
      lcdColorTable[HEADER_ICON_BGCOLOR_INDEX] = RED;
      lcdColorTable[HIGHLIGHT_COLOR_INDEX] = YELLOW;
      lcdColorTable[LINE_COLOR_INDEX] = GREY;
      lcdColorTable[MAINVIEW_GRAPHICS_COLOR_INDEX] = RED;
      lcdColorTable[MAINVIEW_PANES_COLOR_INDEX] = WHITE;

      // this is what the status bar uses...
      //lcdColorTable[MENU_BGCOLOR_INDEX] = DARKRED;

      // Menu font color
      lcdColorTable[MENU_BGCOLOR_INDEX] = DARKRED;//DARKGREY;
      lcdColorTable[MENU_COLOR_INDEX] = WHITE;
      // Selected item in menu
      lcdColorTable[MENU_HIGHLIGHT_BGCOLOR_INDEX] = BLACK;
      lcdColorTable[MENU_HIGHLIGHT_COLOR_INDEX] = WHITE;

      lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] = RGB(GET_RED(RED)>>1, GET_GREEN(RED)>>1, GET_BLUE(RED)>>1);
      lcdColorTable[OVERLAY_COLOR_INDEX] = BLACK;
      lcdColorTable[SCROLLBAR_COLOR_INDEX] = RED;
      lcdColorTable[TEXT_DISABLE_COLOR_INDEX] = GREY;
      lcdColorTable[TEXT_STATUSBAR_COLOR_INDEX] = WHITE;
      lcdColorTable[TITLE_BGCOLOR_INDEX] = RED;
      lcdColorTable[TRIM_BGCOLOR_INDEX] = RED;
      lcdColorTable[TRIM_SHADOW_COLOR_INDEX] = BLACK;
    }

    void loadMenuIcon(uint8_t index, const char * filename, uint32_t color=MENU_COLOR) const
    {
      TRACE("loadMenuIcon %s", getFilePath(filename));
      BitmapBuffer * mask = BitmapBuffer::loadMask(getFilePath(filename));
      if (mask) {
        delete iconMask[index];
        iconMask[index] = mask;

        delete menuIconNormal[index];
        menuIconNormal[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
        if (menuIconNormal[index]) {
          menuIconNormal[index]->clear(MENU_BGCOLOR);
          menuIconNormal[index]->drawMask(0, 0, mask, color);
        }

        delete menuIconSelected[index];
        menuIconSelected[index] = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
        if (menuIconSelected[index]) {
          menuIconSelected[index]->clear(HEADER_CURRENT_BGCOLOR);
          menuIconSelected[index]->drawMask(0, 0, mask, color);
        }
      }
    }

    void loadIcons() const
    {
#if defined(LOG_TELEMETRY) || !defined(WATCHDOG)
      loadMenuIcon(ICON_OPENTX, "mask_opentx_testmode.png", DEFAULT_COLOR);
#else
      loadMenuIcon(ICON_OPENTX, "mask_opentx.png");
#endif
#if defined(HARDWARE_TOUCH)
      loadMenuIcon(ICON_NEXT, "mask_next.png");
      loadMenuIcon(ICON_BACK, "mask_back.png");
#endif
      loadMenuIcon(ICON_RADIO, "mask_menu_radio.png");
      loadMenuIcon(ICON_RADIO_SETUP, "mask_radio_setup.png");
      loadMenuIcon(ICON_RADIO_SD_MANAGER, "mask_radio_sd_browser.png");
      loadMenuIcon(ICON_RADIO_TOOLS, "mask_radio_tools.png");
      loadMenuIcon(ICON_RADIO_SPECTRUM_ANALYSER, "/mask_radio_spectrum_analyser.png");
      loadMenuIcon(ICON_RADIO_GLOBAL_FUNCTIONS, "mask_radio_global_functions.png");
      loadMenuIcon(ICON_RADIO_TRAINER, "mask_radio_trainer.png");
      loadMenuIcon(ICON_RADIO_HARDWARE, "mask_radio_hardware.png");
      loadMenuIcon(ICON_RADIO_CALIBRATION, "mask_radio_calibration.png");
      loadMenuIcon(ICON_RADIO_VERSION, "mask_radio_version.png");
      loadMenuIcon(ICON_MODEL, "mask_menu_model.png");
      loadMenuIcon(ICON_MODEL_SETUP, "mask_model_setup.png");
      loadMenuIcon(ICON_MODEL_HELI, "mask_model_heli.png");
      loadMenuIcon(ICON_MODEL_FLIGHT_MODES, "mask_model_flight_modes.png");
      loadMenuIcon(ICON_MODEL_INPUTS, "mask_model_inputs.png");
      loadMenuIcon(ICON_MODEL_MIXER, "mask_model_mixer.png");
      loadMenuIcon(ICON_MODEL_OUTPUTS, "mask_model_outputs.png");
      loadMenuIcon(ICON_MODEL_CURVES, "mask_model_curves.png");
      loadMenuIcon(ICON_MODEL_GVARS, "mask_model_gvars.png");
      loadMenuIcon(ICON_MODEL_LOGICAL_SWITCHES, "mask_model_logical_switches.png");
      loadMenuIcon(ICON_MODEL_SPECIAL_FUNCTIONS, "mask_model_special_functions.png");
      loadMenuIcon(ICON_MODEL_LUA_SCRIPTS, "mask_model_lua_scripts.png");
      loadMenuIcon(ICON_MODEL_TELEMETRY, "mask_model_telemetry.png");
      loadMenuIcon(ICON_STATS, "mask_menu_stats.png");
      loadMenuIcon(ICON_STATS_THROTTLE_GRAPH, "mask_stats_throttle_graph.png");
      loadMenuIcon(ICON_STATS_TIMERS, "mask_stats_timers.png");
      loadMenuIcon(ICON_STATS_ANALOGS, "mask_stats_analogs.png");
      loadMenuIcon(ICON_STATS_DEBUG, "mask_stats_debug.png");
      loadMenuIcon(ICON_THEME, "mask_menu_theme.png");
      loadMenuIcon(ICON_THEME_SETUP, "mask_theme_setup.png");
      loadMenuIcon(ICON_THEME_VIEW1, "mask_theme_view1.png");
      loadMenuIcon(ICON_THEME_VIEW2, "mask_theme_view2.png");
      loadMenuIcon(ICON_THEME_VIEW3, "mask_theme_view3.png");
      loadMenuIcon(ICON_THEME_VIEW4, "mask_theme_view4.png");
      loadMenuIcon(ICON_THEME_VIEW5, "mask_theme_view5.png");
      loadMenuIcon(ICON_THEME_ADD_VIEW, "mask_theme_add_view.png");
      loadMenuIcon(ICON_MONITOR, "mask_monitor.png");
      loadMenuIcon(ICON_MONITOR_CHANNELS1, "mask_monitor_channels1.png");
      loadMenuIcon(ICON_MONITOR_CHANNELS2, "mask_monitor_channels2.png");
      loadMenuIcon(ICON_MONITOR_CHANNELS3, "mask_monitor_channels3.png");
      loadMenuIcon(ICON_MONITOR_CHANNELS4, "mask_monitor_channels4.png");
      loadMenuIcon(ICON_MONITOR_LOGICAL_SWITCHES, "mask_monitor_logsw.png");

      BitmapBuffer * background = BitmapBuffer::loadMask(getFilePath("mask_currentmenu_bg.png"));
      BitmapBuffer * shadow = BitmapBuffer::loadMask(getFilePath("mask_currentmenu_shadow.png"));
      BitmapBuffer * dot = BitmapBuffer::loadMask(getFilePath("mask_currentmenu_dot.png"));

      if (!currentMenuBackground) {
        currentMenuBackground = new BitmapBuffer(BMP_RGB565, 36, 53);
      }

      if (currentMenuBackground) {
        currentMenuBackground->drawSolidFilledRect(0, 0, currentMenuBackground->width(), MENU_HEADER_HEIGHT, MENU_BGCOLOR);
        currentMenuBackground->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, currentMenuBackground->width(), MENU_TITLE_TOP - MENU_HEADER_HEIGHT, DEFAULT_BGCOLOR);
        currentMenuBackground->drawSolidFilledRect(0, MENU_TITLE_TOP, currentMenuBackground->width(), currentMenuBackground->height() - MENU_TITLE_TOP, TITLE_BGCOLOR);
        currentMenuBackground->drawMask(0, 0, background, HEADER_CURRENT_BGCOLOR);
        currentMenuBackground->drawMask(0, 0, shadow, TRIM_SHADOW_COLOR);
        currentMenuBackground->drawMask(10, 39, dot, MENU_COLOR);
      }

      delete topleftBitmap;
      topleftBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("topleft.png"), TITLE_BGCOLOR, HEADER_COLOR);

      delete background;
      delete shadow;
      delete dot;
    }

    void loadThemeBitmaps() const
    {
      // Calibration screen
      delete calibStick;
      calibStick = BitmapBuffer::loadBitmap(getFilePath("stick_pointer.png"));

      delete calibStickBackground;
      calibStickBackground = BitmapBuffer::loadBitmap(getFilePath("stick_background.png"));

      delete calibTrackpBackground;
      calibTrackpBackground = BitmapBuffer::loadBitmap(getFilePath("trackp_background.png"));

      delete calibRadioPict;
#if defined(PCBX10)
      if(STICKS_PWM_ENABLED()) {
        calibRadioPict = BitmapBuffer::loadBitmap(getFilePath("X10S.bmp"));
      }
      else {
        calibRadioPict = BitmapBuffer::loadBitmap(getFilePath("X10.bmp"));
      }
#else
      calibRadioPict = BitmapBuffer::loadBitmap(getFilePath("horus.bmp"));
#endif

      // Model Selection screen
      delete modelselIconBitmap;
      modelselIconBitmap = BitmapBuffer::loadMaskOnBackground("modelsel/mask_iconback.png", TITLE_BGCOLOR, DEFAULT_BGCOLOR);
      if (modelselIconBitmap) {
        BitmapBuffer * bitmap = BitmapBuffer::loadBitmap(getFilePath("modelsel/icon_default.png"));
        modelselIconBitmap->drawBitmap(20, 8, bitmap);
        delete bitmap;
      }

      delete modelselSdFreeBitmap;
      modelselSdFreeBitmap = BitmapBuffer::loadMask(getFilePath("modelsel/mask_sdfree.png"));

      delete modelselModelQtyBitmap;
      modelselModelQtyBitmap = BitmapBuffer::loadMask(getFilePath("modelsel/mask_modelqty.png"));

      delete modelselModelNameBitmap;
      modelselModelNameBitmap = BitmapBuffer::loadMask(getFilePath("modelsel/mask_modelname.png"));

      delete modelselModelMoveBackground;
      modelselModelMoveBackground = BitmapBuffer::loadMask(getFilePath("modelsel/mask_moveback.png"));

      delete modelselModelMoveIcon;
      modelselModelMoveIcon = BitmapBuffer::loadMask(getFilePath("modelsel/mask_moveico.png"));

      delete modelselWizardBackground;
      modelselWizardBackground = BitmapBuffer::loadBitmap(getFilePath("wizard/background.png"));

      // Channels monitor screen
      delete chanMonLockedBitmap;
      chanMonLockedBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("mask_monitor_lockch.png"), DEFAULT_COLOR, DEFAULT_BGCOLOR);

      delete chanMonInvertedBitmap;
      chanMonInvertedBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("mask_monitor_inver.png"), DEFAULT_COLOR, DEFAULT_BGCOLOR);

      // Mixer setup screen
      delete mixerSetupMixerBitmap;
      mixerSetupMixerBitmap = BitmapBuffer::loadMaskOnBackground("mask_sbar_mixer.png", MENU_COLOR, MENU_BGCOLOR);

      delete mixerSetupToBitmap;
      mixerSetupToBitmap = BitmapBuffer::loadMaskOnBackground("mask_sbar_to.png", MENU_COLOR, MENU_BGCOLOR);

      delete mixerSetupOutputBitmap;
      mixerSetupOutputBitmap = BitmapBuffer::loadMaskOnBackground("mask_sbar_output.png", MENU_COLOR, MENU_BGCOLOR);

      delete mixerSetupAddBitmap;
      mixerSetupAddBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("mask_mplex_add.png"), DEFAULT_COLOR, DEFAULT_BGCOLOR);

      delete mixerSetupMultiBitmap;
      mixerSetupMultiBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("mask_mplex_multi.png"), DEFAULT_COLOR, DEFAULT_BGCOLOR);

      delete mixerSetupReplaceBitmap;
      mixerSetupReplaceBitmap = BitmapBuffer::loadMaskOnBackground(getFilePath("mask_mplex_replace.png"), DEFAULT_COLOR, DEFAULT_BGCOLOR);

      delete mixerSetupLabelIcon;
      mixerSetupLabelIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_label.png"));

      delete mixerSetupCurveIcon;
      mixerSetupCurveIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_curve.png"));

      delete mixerSetupSwitchIcon;
      mixerSetupSwitchIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_switch.png"));

      delete mixerSetupFlightmodeIcon;
      mixerSetupFlightmodeIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_fm.png"));

//      delete mixerSetupSlowIcon;
//      mixerSetupSlowIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_slow.png"));
//
//      delete mixerSetupDelayIcon;
//      mixerSetupDelayIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_delay.png"));
//
//      delete mixerSetupDelaySlowIcon;
//      mixerSetupDelaySlowIcon = BitmapBuffer::loadMask(getFilePath("mask_textline_delayslow.png"));
    }

    void load() const override
    {
      loadColors();
      OpenTxTheme::load();
      if (!backgroundBitmap) {
        backgroundBitmap = BitmapBuffer::loadBitmap(getFilePath("background.png"));
      }
      update();
    }

    void update() const override
    {
      TRACE("TODO THEME::UPDATE()");
#if 0
      uint32_t color = g_eeGeneral.themeData.options[1].value.unsignedValue;
      uint32_t bg_color = UNEXPECTED_SHUTDOWN() ? WHITE : g_eeGeneral.themeData.options[0].value.unsignedValue;

      lcdColorTable[DEFAULT_BGCOLOR_INDEX] = bg_color;
      lcdColorTable[FOCUS_BGCOLOR_INDEX] = color;
      lcdColorTable[CHECKBOX_COLOR_INDEX] = color;
      lcdColorTable[SCROLLBAR_COLOR_INDEX] = color;
      lcdColorTable[CURVE_COLOR_INDEX] = color;
      lcdColorTable[CURVE_CURSOR_COLOR_INDEX] = color;
      lcdColorTable[TITLE_BGCOLOR_INDEX] = color;
      lcdColorTable[MENU_TITLE_DISABLE_COLOR_INDEX] =
          RGB(GET_RED(color)>>1, GET_GREEN(color)>>1, GET_BLUE(color)>>1);
      lcdColorTable[TRIM_BGCOLOR_INDEX] = color;
      lcdColorTable[MAINVIEW_GRAPHICS_COLOR_INDEX] = color;
      #define DARKER(x)     ((x * 70) / 100)
      lcdColorTable[MENU_BGCOLOR_INDEX] = RGB(DARKER(GET_RED(color)), DARKER(GET_GREEN(color)), DARKER(GET_BLUE(color)));
      lcdColorTable[HEADER_ICON_BGCOLOR_INDEX] = color;
      lcdColorTable[HEADER_CURRENT_BGCOLOR_INDEX] = color;
#endif
      loadIcons();
      loadThemeBitmaps();
    }

    void drawBackground(BitmapBuffer * dc) const override
    {
      if (backgroundBitmap) {
        dc->drawBitmap(0 - dc->getOffsetX(), 0 - dc->getOffsetY(), backgroundBitmap);
      }
      else {
        lcdSetColor(g_eeGeneral.themeData.options[0].value.unsignedValue);
        dc->drawSolidFilledRect(0, 0, LCD_W, LCD_H, CUSTOM_COLOR);
      }
    }

    void drawTopLeftBitmap(BitmapBuffer * dc) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      }
    }

    void drawMenuBackground(BitmapBuffer * dc, uint8_t icon, const char * title) const override
    {
      if (topleftBitmap) {
        dc->drawBitmap(0, 0, topleftBitmap);
        uint16_t width = topleftBitmap->width();
        dc->drawSolidFilledRect(width, 0, LCD_W - width, MENU_HEADER_HEIGHT, HEADER_COLOR);
      }
      else {
        dc->drawSolidFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, HEADER_COLOR);
      }

      if (icon == ICON_OPENTX)
        dc->drawBitmap(4, 10, menuIconSelected[ICON_OPENTX]);
      else
        dc->drawBitmap(5, 7, menuIconSelected[icon]);

      dc->drawSolidFilledRect(0, MENU_HEADER_HEIGHT, LCD_W, MENU_TITLE_TOP - MENU_HEADER_HEIGHT, DEFAULT_BGCOLOR); // the white separation line
      dc->drawSolidFilledRect(0, MENU_TITLE_TOP, LCD_W, MENU_TITLE_HEIGHT, TITLE_BGCOLOR); // the title line background
      if (title) {
        dc->drawText(MENUS_MARGIN_LEFT, MENU_TITLE_TOP + 3, title, MENU_COLOR);
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

    void drawMenuHeader(BitmapBuffer * dc, std::vector<PageTab *> & tabs, uint8_t currentIndex) const override
    {
      for (unsigned index = 0; index < tabs.size(); index++) {
        if (index != currentIndex) {
          dc->drawBitmap(index*MENU_HEADER_BUTTON_WIDTH + 2, 7, menuIconNormal[tabs[index]->getIcon()]);
        }
      }
      dc->drawBitmap(currentIndex*MENU_HEADER_BUTTON_WIDTH, 0, currentMenuBackground);
      dc->drawBitmap(currentIndex*MENU_HEADER_BUTTON_WIDTH + 2, 7, menuIconSelected[tabs[currentIndex]->getIcon()]);
    }

    void drawMenuDatetime(BitmapBuffer * dc) const
    {
      dc->drawSolidVerticalLine(DATETIME_SEPARATOR_X, 7, 31, FOCUS_COLOR);

      struct gtm t;
      gettime(&t);
      char str[10];
#if defined(TRANSLATIONS_CN)
      sprintf(str, "%d" TR_MONTH "%d", t.tm_mon + 1, t.tm_mday);
#else
      const char * const STR_MONTHS[] = TR_MONTHS;
      sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE1, str, FONT(XS)|FOCUS_COLOR|CENTERED);
      getTimerString(str, getValue(MIXSRC_TX_TIME));
      dc->drawText(DATETIME_MIDDLE, DATETIME_LINE2, str, FONT(XS)|FOCUS_COLOR|CENTERED);
    }

    void drawProgressBar(BitmapBuffer * dc, coord_t x, coord_t y, coord_t w, coord_t h, int value) const override
    {
      dc->drawSolidRect(x, y, w, h, 1, DEFAULT_COLOR);
      if (value > 0) {
        int width = (w * value) / 100;
        dc->drawSolidFilledRect(x + 2, y + 2, width - 4, h - 4, CHECKBOX_COLOR);
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

#if LCD_W == 480
OpenTxTheme * defaultTheme = &Theme480;
Theme * theme = &Theme480;
#endif
