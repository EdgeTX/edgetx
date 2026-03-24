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

#include "gtests.h"

class SpecialFunctionsTest : public EdgeTxTest {};

TEST_F(SpecialFunctionsTest, SwitchFiledSize)
{
  // test the size of swtch member
  g_model.customFn[0].swtch = SWSRC_LAST;
  EXPECT_EQ(g_model.customFn[0].swtch, SWSRC_LAST)
      << "CustomFunctionData.swtch member is too small to hold all possible "
         "values";
  g_model.customFn[0].swtch = -SWSRC_LAST;
  EXPECT_EQ(g_model.customFn[0].swtch, -SWSRC_LAST)
      << "CustomFunctionData.swtch member is too small to hold all possible "
         "values";
}

TEST_F(SpecialFunctionsTest, FlightReset)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  g_model.customFn[0].swtch = swPos;
  g_model.customFn[0].func = FUNC_RESET;
  g_model.customFn[0].all.val = FUNC_RESET_FLIGHT;
  g_model.customFn[0].active = true;

  mainRequestFlags = 0;
  simuSetSwitch(sw, 0);
  EXPECT_FALSE(getSwitch(swPos));

  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_FALSE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));

  // now trigger SA0
  simuSetSwitch(sw, -1);
  EXPECT_TRUE(getSwitch(swPos));

  // flightReset() should be called
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_TRUE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));

  // now reset mainRequestFlags, and it should stay reset (flightReset() should not be called again)
  mainRequestFlags = 0;
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_FALSE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));
}

#if defined(GVARS)
TEST_F(SpecialFunctionsTest, GvarsInc)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  simuSetSwitch(sw, 0);    // SA-

  g_model.customFn[0].swtch = swPos;
  g_model.customFn[0].func = FUNC_ADJUST_GVAR;
  g_model.customFn[0].all.mode = FUNC_ADJUST_GVAR_INCDEC;
  g_model.customFn[0].all.param = 0; // GV1
  g_model.customFn[0].all.val = -1;   // inc/dec value
  g_model.customFn[0].active = true;

  g_model.flightModeData[0].gvars[0] = 10;  // GV1 = 10;
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 10);

  // now trigger SA0
  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 9);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 9);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 9);

  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 8);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 8);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 8);

  g_model.customFn[0].all.val = 10;   // inc/dec value
 
  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 18);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 18);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 18);

  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 28);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 28);
}
#endif // #if defined(GVARS)

#if defined(GVARS)
// Incrementing a GVar past its model maximum must clamp at MODEL_GVAR_MAX,
// not wrap or exceed the boundary.
TEST_F(SpecialFunctionsTest, GvarIncSaturatesAtMax)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  g_model.customFn[0].swtch = swPos;
  g_model.customFn[0].func = FUNC_ADJUST_GVAR;
  g_model.customFn[0].all.mode = FUNC_ADJUST_GVAR_INCDEC;
  g_model.customFn[0].all.param = 0;  // GV1
  g_model.customFn[0].all.val = 100;  // step = +100
  g_model.customFn[0].active = true;

  const int16_t maxVal = MODEL_GVAR_MAX(0);  // GVAR_MAX - g_model.gvars[0].max

  // Prime to one step below the ceiling.
  g_model.flightModeData[0].gvars[0] = maxVal - 50;

  simuSetSwitch(sw, -1);  // switch on (edge trigger: activeSwitches not yet set)
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], maxVal)
      << "GVar should be clamped to MODEL_GVAR_MAX, not exceed it";
}

// Decrementing a GVar past its model minimum must clamp at MODEL_GVAR_MIN.
TEST_F(SpecialFunctionsTest, GvarDecSaturatesAtMin)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  g_model.customFn[0].swtch = swPos;
  g_model.customFn[0].func = FUNC_ADJUST_GVAR;
  g_model.customFn[0].all.mode = FUNC_ADJUST_GVAR_INCDEC;
  g_model.customFn[0].all.param = 0;   // GV1
  g_model.customFn[0].all.val = -100;  // step = -100
  g_model.customFn[0].active = true;

  const int16_t minVal = MODEL_GVAR_MIN(0);

  // Prime to one step above the floor.
  g_model.flightModeData[0].gvars[0] = minVal + 50;

  simuSetSwitch(sw, -1);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], minVal)
      << "GVar should be clamped to MODEL_GVAR_MIN, not go below it";
}
#endif // #if defined(GVARS)

#if defined(OVERRIDE_CHANNEL_FUNCTION)
// When FUNC_OVERRIDE_CHANNEL is active, the mixer must output the override
// value for that channel regardless of the normal mix result.
TEST_F(SpecialFunctionsTest, channelOverrideForcesOutput)
{
  int sw;
  for (sw = 0; sw < switchGetMaxAllSwitches(); sw += 1)
    if (g_model.getSwitchType(sw) == SWITCH_3POS)
      break;
  int swPos = (sw * 3) + SWSRC_FIRST_SWITCH;

  g_model.customFn[0].swtch = swPos;
  g_model.customFn[0].func = FUNC_OVERRIDE_CHANNEL;
  g_model.customFn[0].all.param = 0;   // CH1 (0-based)
  g_model.customFn[0].all.val = 100;   // override value: +100%
  g_model.customFn[0].active = true;

  // Switch off — channel output should not be overridden.
  simuSetSwitch(sw, 0);
  customFunctionsReset();
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(safetyCh[0], OVERRIDE_CHANNEL_UNDEFINED)
      << "safetyCh should be undefined when switch is inactive";

  // Switch on — safetyCh[0] must be set to the override value.
  simuSetSwitch(sw, -1);
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ(safetyCh[0], 100)
      << "safetyCh should hold the override value when switch is active";

  // Mixer must honour safetyCh and output calc100toRESX(100).
  evalMixes(1);
  EXPECT_EQ(channelOutputs[0], calc100toRESX(100))
      << "channelOutputs[0] should equal calc100toRESX(100) under override";
}
#endif // #if defined(OVERRIDE_CHANNEL_FUNCTION)
