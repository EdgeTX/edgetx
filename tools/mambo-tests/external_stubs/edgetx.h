// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include "pulses/modules_constants.h"
#include "targets/mambo/hal.h"
constexpr unsigned EXTERNAL_MODULE = 1;
struct TestModuleData { uint8_t type = MODULE_TYPE_NONE; };
struct TestModel { TestModuleData moduleData[2]; };
extern TestModel g_model;
