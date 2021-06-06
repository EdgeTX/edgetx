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

#include "model_outputs.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

class OutputEditWindow : public Page {
  public:
    explicit OutputEditWindow(uint8_t channel) :
      Page(ICON_MODEL_OUTPUTS),
      channel(channel)
    {
      buildBody(&body);
      buildHeader(&header);
    }

  protected:
    uint8_t channel;

    void buildHeader(Window * window)
    {
      new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_MENULIMITS, 0, MENU_COLOR);
      new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, getSourceString(MIXSRC_CH1 + channel), 0, MENU_COLOR);
    }

    void buildBody(FormWindow * window)
    {
      FormGridLayout grid;
      grid.spacer(8);

      int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : 1000);

      LimitData * output = limitAddress(channel);

      // Name
      new StaticText(window, grid.getLabelSlot(), STR_NAME);
      new RadioTextEdit(window, grid.getFieldSlot(), output->name, sizeof(output->name));
      grid.nextLine();

      // Offset
      new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_SUBTRIM);
      new NumberEdit(window, grid.getFieldSlot(), -1000, +1000, GET_SET_DEFAULT(output->offset), 0, PREC1);
      grid.nextLine();

      // Min
      new StaticText(window, grid.getLabelSlot(), TR_MIN);
      new NumberEdit(window, grid.getFieldSlot(), -limit, 0,
                     GET_VALUE(output->min - 1000),
                     SET_VALUE(output->min, newValue + 1000),
                     0, PREC1);
      grid.nextLine();

      // Max
      new StaticText(window, grid.getLabelSlot(), TR_MAX);
      new NumberEdit(window, grid.getFieldSlot(), 0, +limit,
                     GET_VALUE(output->max + 1000),
                     SET_VALUE(output->max, newValue - 1000),
                     0, PREC1);
      grid.nextLine();

      // Direction
      new StaticText(window, grid.getLabelSlot(), STR_INVERTED);
      new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(output->revert));
      grid.nextLine();

      // Curve
      new StaticText(window, grid.getLabelSlot(), TR_CURVE);
      auto edit = new NumberEdit(window, grid.getFieldSlot(), -MAX_CURVES, +MAX_CURVES, GET_SET_DEFAULT(output->curve));
      edit->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
        dc->drawText(2, 2, getCurveString(value));
      });
      grid.nextLine();

      // PPM center
      new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_PPMCENTER);
      new NumberEdit(window, grid.getFieldSlot(), PPM_CENTER - PPM_CENTER_MAX, PPM_CENTER + PPM_CENTER_MAX,
                     GET_VALUE(output->ppmCenter + PPM_CENTER),
                     SET_VALUE(output->ppmCenter, newValue - PPM_CENTER));
      grid.nextLine();

      // Subtrims mode
      new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_SUBTRIMMODE);
      new Choice(window, grid.getFieldSlot(), STR_SUBTRIMMODES, 0, 1, GET_SET_DEFAULT(output->symetrical));
      grid.nextLine();

      window->setInnerHeight(grid.getWindowHeight());
    }
};

class OutputLineButton : public Button {
  public:
    OutputLineButton(FormGroup * parent, const rect_t &rect, LimitData * output) :
      Button(parent, rect),
      output(output)
    {
      if (output->revert || output->curve || output->name[0]) {
        setHeight(height() + PAGE_LINE_HEIGHT + FIELD_PADDING_TOP);
      }
    }

    void paint(BitmapBuffer * dc) override
    {
      // first line
      dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, output->min - 1000, PREC1);
      dc->drawNumber(68, FIELD_PADDING_TOP, output->max + 1000, PREC1);
      dc->drawNumber(132, FIELD_PADDING_TOP, output->offset, PREC1);
      dc->drawNumber(226, FIELD_PADDING_TOP, PPM_CENTER + output->ppmCenter, RIGHT);
      dc->drawText(228, FIELD_PADDING_TOP, output->symetrical ? "=" : "\210");

      // second line
      if (output->revert) {
        dc->drawTextAtIndex(4, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, STR_MMMINV, output->revert);
      }
      if (output->curve) {
        dc->drawMask(68, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, mixerSetupCurveIcon, DEFAULT_COLOR);
        dc->drawText(88, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, getCurveString(output->curve));
      }
      if (output->name[0]) {
        dc->drawMask(146, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, mixerSetupLabelIcon, DEFAULT_COLOR);
        dc->drawSizedText(166, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, output->name, sizeof(output->name));
      }

      // bounding rect
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, hasFocus() ? CHECKBOX_COLOR : DISABLE_COLOR);
    }

  protected:
    LimitData * output;
};

ModelOutputsPage::ModelOutputsPage() :
  PageTab(STR_MENULIMITS, ICON_MODEL_OUTPUTS)
{
}

void ModelOutputsPage::rebuild(FormWindow * window, int8_t focusChannel)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusChannel);
  window->setScrollPositionY(scrollPosition);
}

void ModelOutputsPage::build(FormWindow * window, int8_t focusChannel)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);

  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
    LimitData * output = limitAddress(ch);

    // Channel label
    new StaticText(window, grid.getLabelSlot(), getSourceString(MIXSRC_CH1 + ch), BUTTON_BACKGROUND, CENTERED);

    // Channel settings
    Button * button = new OutputLineButton(window, grid.getFieldSlot(), output);
    button->setPressHandler([=]() -> uint8_t {
      Menu * menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() {
        editOutput(window, ch);
      });
      menu->addLine(STR_RESET, [=]() {
        output->min = 0;
        output->max = 0;
        output->offset = 0;
        output->ppmCenter = 0;
        output->revert = false;
        output->curve = 0;
        output->symetrical = 0;
        storageDirty(EE_MODEL);
        rebuild(window, ch);
      });
      menu->addLine(STR_COPY_STICKS_TO_OFS, [=]() {
        copySticksToOffset(ch);
        storageDirty(EE_MODEL);
        button->invalidate();
      });
      menu->addLine(STR_COPY_TRIMS_TO_OFS, [=]() {
        copyTrimsToOffset(ch);
        storageDirty(EE_MODEL);
        button->invalidate();
      });
      return 0;
    });

    if (focusChannel == ch) {
      button->setFocus(SET_FOCUS_DEFAULT);
    }

    grid.spacer(button->height() + 5);
  }

  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}

void ModelOutputsPage::editOutput(FormWindow * window, uint8_t channel)
{
  Window::clearFocus();
  Window * editWindow = new OutputEditWindow(channel);
  editWindow->setCloseHandler([=]() {
    rebuild(window, channel);
  });
}
