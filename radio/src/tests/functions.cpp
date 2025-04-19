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
  EXPECT_EQ(false, getSwitch(swPos));

  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)), false);

  // now trigger SA0
  simuSetSwitch(sw, -1);
  EXPECT_EQ(true, getSwitch(swPos));

  // flightReset() should be called
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)), true);

  // now reset mainRequestFlags, and it should stay reset (flightReset() should not be called again)
  mainRequestFlags = 0;
  evalFunctions(g_model.customFn, modelFunctionsContext);
  EXPECT_EQ((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)), false);
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
