/*
 * Copyright (C) EdgeTX
 *
 * Widget Studio sim-only Lua module.  Compiled for the simu target only and
 * gated by WIDGET_STUDIO (see radio/src/targets/simu/CMakeLists.txt). Keep
 * this module limited to development automation; it must not grow into
 * a firmware-facing API.
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#if defined(COLORLCD) && defined(SIMU) && defined(WIDGET_STUDIO)
extern LROT_TABLE(simulib);
#endif
