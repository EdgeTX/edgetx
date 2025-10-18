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

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#include "memory_sections.h"

#if !defined(ALL_LANGS) || defined(BOOT)
typedef const char* STR_TYP;
#define STR_DEF(x) x
#define STR_VAL(x) x
#else
typedef const char* (*STR_TYP)();
#define STR_DEF(x) x##_FN
#define STR_VAL(x) x()
#endif

#if !defined(M_PI)
#define M_PI         3.14159265358979323846   // pi
#define M_PI_2       1.57079632679489661923   // pi/2
#define M_PI_4       0.785398163397448309616  // pi/4
#define M_1_PI       0.318309886183790671538  // 1/pi
#define M_2_PI       0.636619772367581343076  // 2/pi
#define M_2_SQRTPI   1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2      1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2    0.707106781186547524401  // 1/sqrt(2)
#define M_E          2.71828182845904523536   // e
#define M_LOG2E      1.44269504088896340736   // log2(e)
#define M_LOG10E     0.434294481903251827651  // log10(e)
#define M_LN2        0.693147180559945309417  // ln(2)
#define M_LN10       2.30258509299404568402   // ln(10)
#endif

#if !defined(UNUSED)
  #define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif

#if !defined(__ALIGNED)
#if defined(SIMU)
  #define __ALIGNED(x)
  #define __SECTION_USED(s)
#else
  #define __ALIGNED(x)        __attribute__((aligned(x)))
  #define __SECTION_USED(s)   __attribute__((section(s), used))
#endif
#endif

#if __GNUC__
  #define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#else
  #define PACK(__Declaration__) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#if defined(SIMU)
  #define PACK_NOT_SIMU(__Declaration__) __Declaration__
#else
  #define PACK_NOT_SIMU(__Declaration__) PACK(__Declaration__)
#endif

#if defined(SIMU)
  #define CONVERT_PTR_UINT(x) ((uint32_t)(uint64_t)(x))
  #define CONVERT_UINT_PTR(x) ((uint32_t*)(uint64_t)(x))
#else
  #define CONVERT_PTR_UINT(x) ((uint32_t)(x))
  #define CONVERT_UINT_PTR(x) ((uint32_t *)(x))
#endif

#if !defined(DIM)
  #define DIM(__arr) (sizeof((__arr)) / sizeof((__arr)[0]))
#endif

#if defined(__cplusplus)
  #undef  EXTERN_C
  #define EXTERN_C(__Declaration__) extern "C" { __Declaration__ ; }
  #define EXTERN_C_START extern "C" {
  #define EXTERN_C_END }
#else
  #undef  EXTERN_C
  #define EXTERN_C(__Declaration__) __Declaration__
  #define EXTERN_C_START
  #define EXTERN_C_END
#endif
