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

#include "model_easymode.h"

#include "button.h"
#include "choice.h"
#include "static.h"
#include "form.h"
#include "edgetx.h"
#include "easymode.h"
#include "easymode_convert.h"
#include "getset_helpers.h"
#include "numberedit.h"
#include "etx_lv_theme.h"
#include "easymode_images.h"
#include "easymode_diagram.h"
#include "quick_menu.h"
#include "sourcechoice.h"
#include "toggleswitch.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

// --- String arrays for Choice widgets ---

static const char* const modelTypeValues[] = {
  "---",
  STR_EASYMODE_AIRPLANE,
  STR_EASYMODE_HELICOPTER,
  STR_EASYMODE_GLIDER,
  STR_EASYMODE_MULTIROTOR,
  STR_EASYMODE_CAR,
  STR_EASYMODE_BOAT,
  nullptr
};

static const char* const wingTypeValues[] = {
  STR_EASYMODE_WING_SINGLE_AIL,
  STR_EASYMODE_WING_DUAL_AIL,
  STR_EASYMODE_WING_FLAPERON,
  STR_EASYMODE_WING_1A1F,
  STR_EASYMODE_WING_2A1F,
  STR_EASYMODE_WING_2A2F,
  STR_EASYMODE_WING_2A4F,
  STR_EASYMODE_WING_ELEVON,
  STR_EASYMODE_WING_DELTA,
  nullptr
};

static const char* const tailTypeValues[] = {
  STR_EASYMODE_TAIL_NORMAL,
  STR_EASYMODE_TAIL_VTAIL,
  STR_EASYMODE_TAIL_TAILLESS,
  STR_EASYMODE_TAIL_DUAL_ELE,
  STR_EASYMODE_TAIL_AILEVATOR,
  nullptr
};

static const char* const motorTypeValues[] = {
  STR_EASYMODE_MOTOR_NONE,
  STR_EASYMODE_MOTOR_ELECTRIC,
  STR_EASYMODE_MOTOR_NITRO,
  nullptr
};

// --- Helper predicates ---

static bool needsWingType(EasyModelType type)
{
  return type == EASYMODE_AIRPLANE || type == EASYMODE_GLIDER;
}

static bool needsTailType(EasyModelType type)
{
  return type == EASYMODE_AIRPLANE || type == EASYMODE_GLIDER ||
         type == EASYMODE_HELICOPTER;
}

static bool needsMotorType(EasyModelType type)
{
  return type == EASYMODE_AIRPLANE || type == EASYMODE_GLIDER ||
         type == EASYMODE_HELICOPTER;
}

static bool needsChannelAileron(EasyModelType type)
{
  return type == EASYMODE_AIRPLANE || type == EASYMODE_GLIDER ||
         type == EASYMODE_HELICOPTER || type == EASYMODE_MULTIROTOR;
}

static bool needsAileron2(const EasyModeData& em)
{
  return (em.wingType == EASYWING_DUAL_AIL || em.wingType == EASYWING_FLAPERON ||
          em.wingType == EASYWING_2AIL_1FLAP || em.wingType == EASYWING_2AIL_2FLAP ||
          em.wingType == EASYWING_2AIL_4FLAP || em.wingType == EASYWING_ELEVON ||
          em.wingType == EASYWING_DELTA) && needsChannelAileron(em.modelType);
}

static bool needsFlap(const EasyModeData& em)
{
  return em.wingType == EASYWING_1AIL_1FLAP || em.wingType == EASYWING_2AIL_1FLAP ||
         em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP;
}

static bool needsFlap2(const EasyModeData& em)
{
  return em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP;
}

static bool needsFlap34(const EasyModeData& em)
{
  return em.wingType == EASYWING_2AIL_4FLAP;
}

// ============================================================
// Embedded LZ4Bitmap image lookup tables
// ============================================================

// Thumbnail images for Step 1 model type buttons (indexed by EasyModelType)
static const uint8_t* const thumbImages[] = {
  nullptr,                     // EASYMODE_NONE
  easymode_img_thumb_airplane,
  easymode_img_thumb_helicopter,
  easymode_img_thumb_glider,
  easymode_img_thumb_multirotor,
  easymode_img_thumb_car,
  easymode_img_thumb_boat,
};

// Diagram images for Step 2 config page (indexed by EasyModelType)
static const uint8_t* const diagImages[] = {
  nullptr,                    // EASYMODE_NONE
  easymode_img_diag_airplane,
  easymode_img_diag_helicopter,
  easymode_img_diag_glider,
  easymode_img_diag_multirotor,
  easymode_img_diag_car,
  easymode_img_diag_boat,
};

// ============================================================
// Step progress indicator (3 dots showing current wizard step)
// ============================================================

static void buildStepIndicator(Window* parent, EasyModeWizardStep current)
{
  auto* bar = new Window(parent, rect_t{});
  bar->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM);
  lv_obj_set_width(bar->getLvObj(), lv_pct(100));
  lv_obj_set_style_flex_main_place(bar->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_style_flex_cross_place(bar->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_style_pad_ver(bar->getLvObj(), PAD_TINY, 0);

  for (int i = 0; i < WIZARD_STEP_COUNT; i++) {
    coord_t sz = (i == current) ? 10 : 8;
    auto* dot = new Window(bar, rect_t{0, 0, sz, sz});
    lv_obj_set_style_radius(dot->getLvObj(), LV_RADIUS_CIRCLE, 0);
    if (i == current)
      etx_solid_bg(dot->getLvObj(), COLOR_THEME_FOCUS_INDEX);
    else if (i < current)
      etx_solid_bg(dot->getLvObj(), COLOR_THEME_ACTIVE_INDEX);
    else
      etx_solid_bg(dot->getLvObj(), COLOR_THEME_DISABLED_INDEX);
  }
}

// ============================================================
// ModelEasyModePage implementation
// ============================================================

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

ModelEasyModePage::ModelEasyModePage(const PageDef& pageDef) :
    PageGroupItem(pageDef)
{
}

void ModelEasyModePage::rebuildUI()
{
  diagram = nullptr;  // will be recreated in build()
  if (parentWindow) {
    parentWindow->clear();
    build(parentWindow);
  }
}

void ModelEasyModePage::refreshDiagram()
{
  if (diagram) {
    diagram->update(g_easyMode);
  }
}

void ModelEasyModePage::setStep(EasyModeWizardStep step)
{
  wizardStep = step;
  rebuildUI();
}

void ModelEasyModePage::build(Window* window)
{
  parentWindow = window;

  // Restrict to vertical scrolling only (prevent horizontal overflow)
  lv_obj_set_scroll_dir(window->getLvObj(), LV_DIR_VER);

  auto& em = g_easyMode;

  // If no model type selected, always show model type step
  // If model type is set and we haven't been navigated yet, show config step
  if (em.modelType == EASYMODE_NONE) {
    wizardStep = WIZARD_MODEL_TYPE;
  } else if (wizardStep == WIZARD_MODEL_TYPE && !hasNavigated) {
    wizardStep = WIZARD_CONFIG;
    hasNavigated = true;
  }

  switch (wizardStep) {
    case WIZARD_MODEL_TYPE:
      buildModelTypeStep(window);
      break;
    case WIZARD_CONFIG:
      buildConfigStep(window);
      break;
    case WIZARD_OPTIONS:
      buildOptionsStep(window);
      break;
    default:
      break;
  }
}

// --- Step 1: Model Type Selection with images ---

void ModelEasyModePage::buildModelTypeStep(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_SMALL);

  buildStepIndicator(window, WIZARD_MODEL_TYPE);

  new StaticText(window, rect_t{}, STR_EASYMODE_MODEL_TYPE,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));

  // 3x2 grid of model type buttons with thumbnail images
  auto* grid = new Window(window, rect_t{});
  grid->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_SMALL);
  lv_obj_set_width(grid->getLvObj(), lv_pct(100));
  lv_obj_set_style_flex_main_place(grid->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);

  struct TypeBtn { EasyModelType type; const char* label; };
  static const TypeBtn types[] = {
    { EASYMODE_AIRPLANE,   STR_EASYMODE_AIRPLANE },
    { EASYMODE_HELICOPTER, STR_EASYMODE_HELICOPTER },
    { EASYMODE_GLIDER,     STR_EASYMODE_GLIDER },
    { EASYMODE_MULTIROTOR, STR_EASYMODE_MULTIROTOR },
    { EASYMODE_CAR,        STR_EASYMODE_CAR },
    { EASYMODE_BOAT,       STR_EASYMODE_BOAT },
  };

  coord_t btnW = (LCD_W - 48) / 3;
  coord_t btnH = 88;

  for (auto& t : types) {
    auto type = t.type;
    auto* btn = new TextButton(grid, rect_t{0, 0, btnW, btnH}, t.label,
      [this, type]() {
        auto& em = g_easyMode;
        em.modelType = type;
        easyModeSetDefaults(em);
        easyModeApply(em);
        storageDirty(EE_MODEL);
        setStep(WIZARD_CONFIG);
        return 0;
      });

    // Set up as column flex: image on top, label below
    lv_obj_set_flex_flow(btn->getLvObj(), LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(btn->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_main_place(btn->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_row(btn->getLvObj(), 2, 0);
    lv_obj_set_style_pad_all(btn->getLvObj(), 4, 0);
    lv_obj_set_width(btn->getLvObj(), btnW);
    lv_obj_set_height(btn->getLvObj(), btnH);

    // Add thumbnail image above the text label
    if (t.type < (int)(sizeof(thumbImages)/sizeof(thumbImages[0])) &&
        thumbImages[t.type]) {
      auto* img = new StaticLZ4Image(btn, 0, 0,
                      (const LZ4Bitmap*)thumbImages[t.type]);
      lv_obj_move_to_index(img->getLvObj(), 0);
    }

    // Highlight selected type
    if (g_easyMode.modelType == t.type) {
      lv_obj_set_style_border_width(btn->getLvObj(), 2, 0);
      lv_obj_set_style_border_color(btn->getLvObj(),
          makeLvColor(COLOR_THEME_FOCUS), 0);
      etx_solid_bg(btn->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    }
  }

  // If a model type is already selected, show a "Next" button
  if (g_easyMode.modelType != EASYMODE_NONE) {
    auto* navBar = new Window(window, rect_t{});
    navBar->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
    lv_obj_set_width(navBar->getLvObj(), lv_pct(100));
    lv_obj_set_style_flex_main_place(navBar->getLvObj(), LV_FLEX_ALIGN_END, 0);

    new TextButton(navBar, rect_t{0, 0, 90, 32}, STR_NEXT,
      [this]() { setStep(WIZARD_CONFIG); return 0; });
  }
}

// --- Step 2: Configuration + Channel Map with Image ---

void ModelEasyModePage::buildConfigStep(Window* window)
{
  auto& em = g_easyMode;

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  buildStepIndicator(window, WIZARD_CONFIG);

  // Content row: image left, options right
  auto* content = new Window(window, rect_t{});
  content->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY);
  lv_obj_set_width(content->getLvObj(), lv_pct(100));
  lv_obj_set_flex_grow(content->getLvObj(), 1);

  // Left panel: dynamic vehicle diagram (38% width)
  auto* leftPanel = new Window(content, rect_t{});
  leftPanel->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);
  lv_obj_set_width(leftPanel->getLvObj(), lv_pct(38));
  lv_obj_set_height(leftPanel->getLvObj(), LV_SIZE_CONTENT);
  lv_obj_set_style_flex_cross_place(leftPanel->getLvObj(), LV_FLEX_ALIGN_CENTER, 0);

  // Display dynamic diagram showing control surfaces with channel labels
  if (em.modelType > EASYMODE_NONE) {
    diagram = new EasyModeDiagram(leftPanel, em);
  }

  // Right panel: selectors and channel editors (scrollable, 60% width)
  coord_t contentH = LCD_H - 76;
  auto* rightPanel = new Window(content, rect_t{});
  rightPanel->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);
  lv_obj_set_width(rightPanel->getLvObj(), lv_pct(60));
  lv_obj_set_height(rightPanel->getLvObj(), contentH);
  lv_obj_set_style_max_height(rightPanel->getLvObj(), contentH, 0);
  lv_obj_clear_flag(rightPanel->getLvObj(), LV_OBJ_FLAG_SCROLL_CHAIN);
  lv_obj_set_scroll_dir(rightPanel->getLvObj(), LV_DIR_VER);
  lv_obj_set_scrollbar_mode(rightPanel->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // Type selectors
  if (needsWingType(em.modelType)) {
    auto line = rightPanel->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_WING_TYPE);
    new Choice(line, rect_t{}, wingTypeValues,
      0, EASYWING_COUNT - 1,
      [&em]() { return (int)em.wingType; },
      [this, &em](int val) {
        em.wingType = (EasyWingType)val;
        easyModeApply(em);
        SET_DIRTY();
        rebuildUI();
      });
  }

  if (needsTailType(em.modelType)) {
    auto line = rightPanel->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_TAIL_TYPE);
    new Choice(line, rect_t{}, tailTypeValues,
      0, EASYTAIL_COUNT - 1,
      [&em]() { return (int)em.tailType; },
      [this, &em](int val) {
        em.tailType = (EasyTailType)val;
        easyModeApply(em);
        SET_DIRTY();
        rebuildUI();
      });
  }

  if (needsMotorType(em.modelType)) {
    auto line = rightPanel->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_MOTOR_TYPE);
    new Choice(line, rect_t{}, motorTypeValues,
      0, EASYMOTOR_COUNT - 1,
      [&em]() { return (int)em.motorType; },
      [this, &em](int val) {
        em.motorType = (EasyMotorType)val;
        easyModeApply(em);
        SET_DIRTY();
        rebuildUI();
      });
  }

  // Channel map section
  auto chLine = rightPanel->newLine(grid);
  new StaticText(chLine, rect_t{}, STR_EASYMODE_CHANNEL_MAP,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
  new StaticText(chLine, rect_t{});

  auto addChannelRow = [&](const char* label, int8_t& chRef) {
    auto line = rightPanel->newLine(grid);
    new StaticText(line, rect_t{}, label);
    new NumberEdit(line, rect_t{}, 1, EASYCH_MAX,
      [&chRef]() { return chRef + 1; },
      [&chRef, &em, this](int val) {
        chRef = val - 1;
        easyModeApply(em);
        SET_DIRTY();
        refreshDiagram();
      });
  };

  bool isHeli = (em.modelType == EASYMODE_HELICOPTER);
  bool isMulti = (em.modelType == EASYMODE_MULTIROTOR);
  bool useRotorNames = (isHeli || isMulti);

  if (needsChannelAileron(em.modelType)) {
    addChannelRow(useRotorNames ? STR_EASYMODE_CH_ROLL : STR_EASYMODE_CH_AILERON,
                  em.channels.aileron);
    if (needsAileron2(em))
      addChannelRow(STR_EASYMODE_CH_AIL2, em.channels.aileron2);
  }

  if (em.modelType != EASYMODE_CAR && em.modelType != EASYMODE_BOAT)
    addChannelRow(isHeli ? STR_EASYMODE_CH_NICK :
                  isMulti ? STR_EASYMODE_CH_NICK :
                  STR_EASYMODE_CH_ELEVATOR,
                  em.channels.elevator);

  if ((em.tailType == EASYTAIL_DUAL_ELEVATOR || em.tailType == EASYTAIL_AILEVATOR) &&
      em.channels.elevator2 >= 0)
    addChannelRow(STR_EASYMODE_CH_ELE2, em.channels.elevator2);

  if (em.modelType != EASYMODE_CAR && em.modelType != EASYMODE_BOAT)
    addChannelRow(useRotorNames ? STR_EASYMODE_CH_YAW : STR_EASYMODE_CH_RUDDER,
                  em.channels.rudder);

  // Helicopter: throttle channel = collective pitch; throttle is optional
  if (isHeli) {
    addChannelRow(STR_EASYMODE_CH_PITCH, em.channels.throttle);
  } else if (em.motorType != EASYMOTOR_NONE || em.modelType == EASYMODE_CAR ||
             em.modelType == EASYMODE_BOAT || isMulti) {
    addChannelRow(STR_EASYMODE_CH_THROTTLE, em.channels.throttle);

    // Glider with motor: motor switch source selector
    if (em.modelType == EASYMODE_GLIDER) {
      auto motorSrcLine = rightPanel->newLine(grid);
      new StaticText(motorSrcLine, rect_t{}, STR_EASYMODE_SRC_MOTOR);
      new SourceChoice(motorSrcLine, rect_t{}, MIXSRC_FIRST_STICK, MIXSRC_LAST_SWITCH,
        [&em]() { return (int16_t)em.sources.motorSource; },
        [&em, this](int16_t val) {
          em.sources.motorSource = val;
          easyModeApply(em);
          SET_DIRTY();
        });
    }
  }

  if (em.modelType == EASYMODE_CAR || em.modelType == EASYMODE_BOAT)
    addChannelRow(STR_EASYMODE_CH_STEERING, em.channels.steering);

  if (needsFlap(em)) {
    addChannelRow(STR_EASYMODE_CH_FLAP, em.channels.flap);

    // Flap input source selector
    auto flapSrcLine = rightPanel->newLine(grid);
    new StaticText(flapSrcLine, rect_t{}, STR_EASYMODE_SRC_FLAP);
    new SourceChoice(flapSrcLine, rect_t{}, MIXSRC_FIRST_STICK, MIXSRC_LAST_SWITCH,
      [&em]() { return (int16_t)em.sources.flapSource; },
      [&em, this](int16_t val) {
        em.sources.flapSource = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }
  if (needsFlap2(em))
    addChannelRow(STR_EASYMODE_CH_FLAP2, em.channels.flap2);
  if (needsFlap34(em)) {
    addChannelRow(STR_EASYMODE_CH_BRAKEL, em.channels.flap3);
    addChannelRow(STR_EASYMODE_CH_BRAKER, em.channels.flap4);
  }

  // Crow braking option (glider/airplane with flaps)
  if ((em.modelType == EASYMODE_GLIDER || em.modelType == EASYMODE_AIRPLANE) &&
      needsFlap(em)) {
    auto crowLine = rightPanel->newLine(grid);
    new StaticText(crowLine, rect_t{}, STR_EASYMODE_CROW);
    new ToggleSwitch(crowLine, rect_t{},
      [&em]() { return em.options.crowEnabled; },
      [&em, this](uint8_t val) {
        em.options.crowEnabled = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  // Aux1: gyro gain (heli) or arm switch (multi)
  if (isHeli && em.channels.aux1 >= 0) {
    addChannelRow(STR_EASYMODE_CH_GYRO, em.channels.aux1);
    auto srcLine = rightPanel->newLine(grid);
    new StaticText(srcLine, rect_t{}, STR_EASYMODE_SRC_GYRO);
    new SourceChoice(srcLine, rect_t{}, MIXSRC_FIRST_STICK, MIXSRC_LAST_SWITCH,
      [&em]() { return (int16_t)em.sources.aux1Source; },
      [&em, this](int16_t val) {
        em.sources.aux1Source = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  if (isMulti && em.channels.aux1 >= 0) {
    addChannelRow(STR_EASYMODE_CH_AUX1, em.channels.aux1);
    auto srcLine = rightPanel->newLine(grid);
    new StaticText(srcLine, rect_t{}, STR_EASYMODE_SRC_ARM);
    new SourceChoice(srcLine, rect_t{}, MIXSRC_FIRST_STICK, MIXSRC_LAST_SWITCH,
      [&em]() { return (int16_t)em.sources.aux1Source; },
      [&em, this](int16_t val) {
        em.sources.aux1Source = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  // Aux2: flight mode switch (multi)
  if (isMulti && em.channels.aux2 >= 0) {
    addChannelRow(STR_EASYMODE_CH_AUX2, em.channels.aux2);
    auto srcLine = rightPanel->newLine(grid);
    new StaticText(srcLine, rect_t{}, STR_EASYMODE_SRC_MODE);
    new SourceChoice(srcLine, rect_t{}, MIXSRC_FIRST_STICK, MIXSRC_LAST_SWITCH,
      [&em]() { return (int16_t)em.sources.aux2Source; },
      [&em, this](int16_t val) {
        em.sources.aux2Source = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  // Navigation bar
  auto* navBar = new Window(rightPanel, rect_t{});
  navBar->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
  lv_obj_set_width(navBar->getLvObj(), lv_pct(100));
  lv_obj_set_height(navBar->getLvObj(), 36);
  lv_obj_set_style_flex_main_place(navBar->getLvObj(), LV_FLEX_ALIGN_SPACE_BETWEEN, 0);

  new TextButton(navBar, rect_t{0, 0, 80, 32}, "Back",
    [this]() { setStep(WIZARD_MODEL_TYPE); return 0; });
  new TextButton(navBar, rect_t{0, 0, 80, 32}, STR_NEXT,
    [this]() { setStep(WIZARD_OPTIONS); return 0; });
}

// --- Step 3: Options ---

void ModelEasyModePage::buildOptionsStep(Window* window)
{
  auto& em = g_easyMode;

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  buildStepIndicator(window, WIZARD_OPTIONS);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // Expo section header
  auto hdr = window->newLine(grid);
  new StaticText(hdr, rect_t{}, STR_EASYMODE_EXPO,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
  new StaticText(hdr, rect_t{});

  if (needsChannelAileron(em.modelType)) {
    auto line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_CH_AILERON);
    new NumberEdit(line, rect_t{}, 0, 100,
      [&em]() { return em.options.expoAileron; },
      [&em](int val) { em.options.expoAileron = val; easyModeApply(em); SET_DIRTY(); });
  }

  if (em.modelType != EASYMODE_CAR && em.modelType != EASYMODE_BOAT) {
    auto line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_CH_ELEVATOR);
    new NumberEdit(line, rect_t{}, 0, 100,
      [&em]() { return em.options.expoElevator; },
      [&em](int val) { em.options.expoElevator = val; easyModeApply(em); SET_DIRTY(); });

    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_CH_RUDDER);
    new NumberEdit(line, rect_t{}, 0, 100,
      [&em]() { return em.options.expoRudder; },
      [&em](int val) { em.options.expoRudder = val; easyModeApply(em); SET_DIRTY(); });
  }

  // Rates & mixing section header
  auto ratesHdr = window->newLine(grid);
  new StaticText(ratesHdr, rect_t{}, STR_EASYMODE_OPTIONS,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
  new StaticText(ratesHdr, rect_t{});

  auto line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_EASYMODE_DUAL_RATE);
  new NumberEdit(line, rect_t{}, 0, 100,
    [&em]() { return em.options.dualRateLow; },
    [&em](int val) { em.options.dualRateLow = val; easyModeApply(em); SET_DIRTY(); });

  if (needsAileron2(em)) {
    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_AIL_DIFF);
    new NumberEdit(line, rect_t{}, 0, 100,
      [&em]() { return em.options.aileronDifferential; },
      [&em](int val) { em.options.aileronDifferential = val; easyModeApply(em); SET_DIRTY(); });
  }

  if (needsChannelAileron(em.modelType) && em.channels.rudder >= 0) {
    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_AIL2RUD);
    new NumberEdit(line, rect_t{}, 0, 100,
      [&em]() { return em.options.aileronToRudderMix; },
      [&em](int val) { em.options.aileronToRudderMix = val; easyModeApply(em); SET_DIRTY(); });
  }

  if (needsFlap(em)) {
    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_FLAP2ELE);
    new NumberEdit(line, rect_t{}, -100, 100,
      [&em]() { return em.options.flapToElevatorComp; },
      [&em](int val) { em.options.flapToElevatorComp = val; easyModeApply(em); SET_DIRTY(); });
  }

  if (em.modelType == EASYMODE_MULTIROTOR) {
    static const char* const chOrderValues[] = { "AETR", "TAER", "RETA", nullptr };
    line = window->newLine(grid);
    new StaticText(line, rect_t{}, STR_EASYMODE_CH_ORDER);
    new Choice(line, rect_t{}, chOrderValues,
      0, EASYMULTI_ORDER_COUNT - 1,
      [&em]() { return (int)em.options.multiChannelOrder; },
      [&em](int val) {
        em.options.multiChannelOrder = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  // Custom mixes section
  auto mixHdr = window->newLine(grid);
  new StaticText(mixHdr, rect_t{}, STR_EASYMODE_CUSTOM_MIX,
                 COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
  new StaticText(mixHdr, rect_t{});

  for (int i = 0; i < EASY_CUSTOM_MIXES; i++) {
    auto& cm = em.customMix[i];

    // Row 1: "Mix N" label + source selector
    char label[8];
    snprintf(label, sizeof(label), "Mix %d", i + 1);
    auto srcLine = window->newLine(grid);
    new StaticText(srcLine, rect_t{}, label);
    new SourceChoice(srcLine, rect_t{}, 0, MIXSRC_LAST,
      [&cm]() { return (int16_t)cm.source; },
      [&cm, &em, this](int16_t val) {
        cm.source = val;
        easyModeApply(em);
        SET_DIRTY();
      });

    // Row 2: channel + weight
    auto chLine = window->newLine(grid);
    new StaticText(chLine, rect_t{}, STR_CH);
    new NumberEdit(chLine, rect_t{}, 0, EASYCH_MAX,
      [&cm]() { return cm.destCh + 1; },
      [&cm, &em, this](int val) {
        cm.destCh = val - 1;
        easyModeApply(em);
        SET_DIRTY();
      });

    auto wtLine = window->newLine(grid);
    new StaticText(wtLine, rect_t{}, STR_WEIGHT);
    new NumberEdit(wtLine, rect_t{}, -100, 100,
      [&cm]() { return cm.weight; },
      [&cm, &em, this](int val) {
        cm.weight = val;
        easyModeApply(em);
        SET_DIRTY();
      });
  }

  // Navigation + Finish + Convert bar
  auto* navBar = new Window(window, rect_t{});
  navBar->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
  lv_obj_set_width(navBar->getLvObj(), lv_pct(100));
  lv_obj_set_height(navBar->getLvObj(), 40);
  lv_obj_set_style_flex_main_place(navBar->getLvObj(), LV_FLEX_ALIGN_SPACE_BETWEEN, 0);

  new TextButton(navBar, rect_t{0, 0, 80, 32}, "Back",
    [this]() { setStep(WIZARD_CONFIG); return 0; });

  auto* finishBtn = new TextButton(navBar, rect_t{0, 0, 100, 32},
    STR_SAVE, [this]() {
      easyModeApply(g_easyMode);
      storageDirty(EE_MODEL);
      // Navigate back by closing the quick menu
      QuickMenu::shutdownQuickMenu();
      return 0;
    });
  etx_solid_bg(finishBtn->getLvObj(), COLOR_THEME_ACTIVE_INDEX);
  etx_txt_color(finishBtn->getLvObj(), COLOR_THEME_PRIMARY2_INDEX);

  auto* convertBtn = new TextButton(navBar, rect_t{0, 0, 150, 32},
    STR_EASYMODE_CONVERT, [this]() {
      if (confirmationDialog(STR_EASYMODE_CONVERT, STR_EASYMODE_CONVERT_CONFIRM, false)) {
        easyModeConvertToExpert();
        // Reopen Quick Menu on Model Setup so page list rebuilds
        // (Easy Mode page disappears, Inputs/Mixes appear)
        QuickMenu::openPage(QM_MODEL_SETUP);
      }
      return 0;
    });
  etx_solid_bg(convertBtn->getLvObj(), COLOR_THEME_WARNING_INDEX);
  etx_txt_color(convertBtn->getLvObj(), COLOR_THEME_PRIMARY2_INDEX);
}
