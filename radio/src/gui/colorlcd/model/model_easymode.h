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

#pragma once

#include "pagegroup.h"

class EasyModeDiagram;

enum EasyModeWizardStep : uint8_t {
  WIZARD_MODEL_TYPE = 0,
  WIZARD_CONFIG,
  WIZARD_OPTIONS,
  WIZARD_STEP_COUNT
};

class ModelEasyModePage : public PageGroupItem
{
 public:
  ModelEasyModePage(const PageDef& pageDef);
  void build(Window* window) override;

 protected:
  Window* parentWindow = nullptr;
  EasyModeWizardStep wizardStep = WIZARD_MODEL_TYPE;
  EasyModeDiagram* diagram = nullptr;
  bool hasNavigated = false;

  void rebuildUI();
  void setStep(EasyModeWizardStep step);
  void refreshDiagram();

  void buildModelTypeStep(Window* window);
  void buildConfigStep(Window* window);
  void buildOptionsStep(Window* window);
};
