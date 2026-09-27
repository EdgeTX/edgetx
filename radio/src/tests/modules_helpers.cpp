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

class ModulesHelpersTest : public EdgeTxTest {};

#if defined(MULTIMODULE) && defined(HARDWARE_INTERNAL_MODULE)

// 'multi.rfProtocol' shares storage with other module types' data. An
// AFHDS2A module whose bound receiver ID starts with a FrSky Multi protocol
// number must not be treated as D16/R9 (it would open the FrSky bind menu
// instead of binding).
TEST_F(ModulesHelpersTest, multiProtocolChecksIgnoreOtherModuleTypes)
{
  const uint8_t frskyProtocols[] = {
      MODULE_SUBTYPE_MULTI_FRSKYX,
      MODULE_SUBTYPE_MULTI_FRSKYX2,
      MODULE_SUBTYPE_MULTI_FRSKY_R9,
  };

  for (auto proto : frskyProtocols) {
    setModuleType(INTERNAL_MODULE, MODULE_TYPE_FLYSKY_AFHDS2A);
    g_model.moduleData[INTERNAL_MODULE].flysky.rx_id[0] = proto;

    EXPECT_FALSE(isModuleD16(INTERNAL_MODULE)) << "rx_id[0]=" << (int)proto;
    EXPECT_FALSE(IS_D16_MULTI(INTERNAL_MODULE)) << "rx_id[0]=" << (int)proto;
    EXPECT_FALSE(IS_R9_MULTI(INTERNAL_MODULE)) << "rx_id[0]=" << (int)proto;
  }
}

TEST_F(ModulesHelpersTest, multiProtocolChecksMatchMultimodule)
{
  setModuleType(INTERNAL_MODULE, MODULE_TYPE_MULTIMODULE);

  g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol =
      MODULE_SUBTYPE_MULTI_FRSKYX;
  EXPECT_TRUE(isModuleD16(INTERNAL_MODULE));
  EXPECT_FALSE(IS_R9_MULTI(INTERNAL_MODULE));

  g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol =
      MODULE_SUBTYPE_MULTI_FRSKYX2;
  EXPECT_TRUE(isModuleD16(INTERNAL_MODULE));

  g_model.moduleData[INTERNAL_MODULE].multi.rfProtocol =
      MODULE_SUBTYPE_MULTI_FRSKY_R9;
  EXPECT_FALSE(isModuleD16(INTERNAL_MODULE));
  EXPECT_TRUE(IS_R9_MULTI(INTERNAL_MODULE));
}

#endif

