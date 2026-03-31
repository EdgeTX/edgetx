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
#include "mixes.h"
#include "expos.h"
#include "curves.h"
#include "customfn.h"

class SpecialFunctionsTest : public EdgeTxTest {};

TEST_F(SpecialFunctionsTest, SwitchFiledSize)
{
  // test that SwitchRef can round-trip through the swtch field
  auto trainerRef = SwitchRef_(SWITCH_TYPE_TRAINER, 0);
  (*customFnAllocAt(0)).swtch = trainerRef;
  EXPECT_EQ((*customFnAddress(0)).swtch, trainerRef)
      << "CustomFunctionData.swtch member cannot hold SWITCH_TYPE_TRAINER";

  auto trainerInv = SwitchRef_(SWITCH_TYPE_TRAINER, 0, SWITCH_FLAG_INVERTED);
  (*customFnAddress(0)).swtch = trainerInv;
  EXPECT_EQ((*customFnAddress(0)).swtch, trainerInv)
      << "CustomFunctionData.swtch member cannot hold inverted SWITCH_TYPE_TRAINER";
}

TEST_F(SpecialFunctionsTest, FlightReset)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  auto swRef = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));

  (*customFnAllocAt(0)).swtch = swRef;
  (*customFnAddress(0)).func = FUNC_RESET;
  (*customFnAddress(0)).all.val = FUNC_RESET_FLIGHT;
  (*customFnAddress(0)).active = true;

  mainRequestFlags = 0;
  simuSetSwitch(sw, 0);
  EXPECT_FALSE(getSwitch(swRef));

  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_FALSE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));

  // now trigger SA0
  simuSetSwitch(sw, -1);
  EXPECT_TRUE(getSwitch(swRef));

  // flightReset() should be called
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_TRUE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));

  // now reset mainRequestFlags, and it should stay reset (flightReset() should not be called again)
  mainRequestFlags = 0;
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_FALSE((bool)(mainRequestFlags & (1 << REQUEST_FLIGHT_RESET)));
}

#if defined(GVARS)
TEST_F(SpecialFunctionsTest, GvarsInc)
{
  int sw = findHwSwitch(SWITCH_3POS);
  if (sw < 0) return;
  auto swRef = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(sw * 3));

  simuSetSwitch(sw, 0);    // SA-

  (*customFnAllocAt(0)).swtch = swRef;
  (*customFnAddress(0)).func = FUNC_ADJUST_GVAR;
  (*customFnAddress(0)).all.mode = FUNC_ADJUST_GVAR_INCDEC;
  (*customFnAddress(0)).all.param = 0; // GV1
  (*customFnAddress(0)).all.val = -1;   // inc/dec value
  (*customFnAddress(0)).active = true;

  GVAR_VALUE(0, 0) = 10;  // GV1 = 10;
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 10);

  // now trigger SA0
  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 9);
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 9);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 9);

  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 8);
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 8);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 8);

  (*customFnAddress(0)).all.val = 10;   // inc/dec value

  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 18);
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 18);

  simuSetSwitch(sw, 0);    // SA-
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 18);

  simuSetSwitch(sw, -1);  // SAdown
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 28);
  evalFunctions(customFnAddress(0), g_modelArena.sectionCount(ARENA_CUSTOM_FN), modelFunctionsContext);
  EXPECT_EQ(GVAR_VALUE(0, 0), 28);
}
#endif // #if defined(GVARS)
