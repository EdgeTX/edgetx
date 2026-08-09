/*
 * Copyright (C) EdgeTx
 *
 * Authors:
 *   FreedomLJS <freedom_ljs@qq.com>
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

#ifndef __DUMBORC_EXTCHANNEL_H__
#define __DUMBORC_EXTCHANNEL_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>



#if ( DUMBORC_EXTCHANNEL_ENABLED )

#if ( LOG_ENABLED && DUMBORC_EXTCHANNEL_LOG_ENABLED )
# include "debug.h"
# define DUMBORC_EXTCHANNEL_LOG_INFO(...)    TRACE(__VA_ARGS__)
#else
# define DUMBORC_EXTCHANNEL_LOG_INFO(...)
#endif    /* ( LOG_ENABLED && DUMBORC_EXTCHANNEL_LOG_ENABLED ) */



/* LJS : Dumborc extchannel PIN_[0:31] define. */
#define DUMBORC_EXTCHANNEL_PIN_0     (1UL <<  0U)
#define DUMBORC_EXTCHANNEL_PIN_1     (1UL <<  1U)
#define DUMBORC_EXTCHANNEL_PIN_2     (1UL <<  2U)
#define DUMBORC_EXTCHANNEL_PIN_3     (1UL <<  3U)
#define DUMBORC_EXTCHANNEL_PIN_4     (1UL <<  4U)
#define DUMBORC_EXTCHANNEL_PIN_5     (1UL <<  5U)
#define DUMBORC_EXTCHANNEL_PIN_6     (1UL <<  6U)
#define DUMBORC_EXTCHANNEL_PIN_7     (1UL <<  7U)
#define DUMBORC_EXTCHANNEL_PIN_8     (1UL <<  8U)
#define DUMBORC_EXTCHANNEL_PIN_9     (1UL <<  9U)
#define DUMBORC_EXTCHANNEL_PIN_10    (1UL << 10U)
#define DUMBORC_EXTCHANNEL_PIN_11    (1UL << 11U)
#define DUMBORC_EXTCHANNEL_PIN_12    (1UL << 12U)
#define DUMBORC_EXTCHANNEL_PIN_13    (1UL << 13U)
#define DUMBORC_EXTCHANNEL_PIN_14    (1UL << 14U)
#define DUMBORC_EXTCHANNEL_PIN_15    (1UL << 15U)
#define DUMBORC_EXTCHANNEL_PIN_16    (1UL << 16U)
#define DUMBORC_EXTCHANNEL_PIN_17    (1UL << 17U)
#define DUMBORC_EXTCHANNEL_PIN_18    (1UL << 18U)
#define DUMBORC_EXTCHANNEL_PIN_19    (1UL << 19U)
#define DUMBORC_EXTCHANNEL_PIN_20    (1UL << 20U)
#define DUMBORC_EXTCHANNEL_PIN_21    (1UL << 21U)
#define DUMBORC_EXTCHANNEL_PIN_22    (1UL << 22U)
#define DUMBORC_EXTCHANNEL_PIN_23    (1UL << 23U)
#define DUMBORC_EXTCHANNEL_PIN_24    (1UL << 24U)
#define DUMBORC_EXTCHANNEL_PIN_25    (1UL << 25U)
#define DUMBORC_EXTCHANNEL_PIN_26    (1UL << 26U)
#define DUMBORC_EXTCHANNEL_PIN_27    (1UL << 27U)
#define DUMBORC_EXTCHANNEL_PIN_28    (1UL << 28U)
#define DUMBORC_EXTCHANNEL_PIN_29    (1UL << 29U)
#define DUMBORC_EXTCHANNEL_PIN_30    (1UL << 30U)
#define DUMBORC_EXTCHANNEL_PIN_31    (1UL << 31U)



/**
 * [dumborc_extchannel_init (dumborc_extchannel init)]
 * @return  [error code (int)]
 *          [0 : succeed ; !0 : failed]
 */
int dumborc_extchannel_init(void);

/**
 * [dumborc_extchannel_uninit (dumborc_extchannel uninit)]
 * @return  [error code (int)]
 *          [0 : succeed ; !0 : failed]
 */
int dumborc_extchannel_uninit(void);


/**
 * [dumborc_extchannel_get_task_status (dumborc_extchannel get task status)]
 * @return  [task status (uint32_t)]
 *          [0 : stop ; !0 : running]
 */
uint32_t dumborc_extchannel_get_task_status(void);

/**
 * [dumborc_extchannel_set_task_status (dumborc_extchannel set task status)]
 * @param status [task status (uint32_t)]
 *               [0 : stop ; !0 : running]
 */
void dumborc_extchannel_set_task_status(uint32_t const status);


/**
 * [dumborc_extchannel_get_reg_data (dumborc_extchannel get register data)]
 * @param  idx [IC index (uint32_t)]
 * @param  reg [IC register (uint32_t)]
 * @return     [register data (uint32_t)]
 */
uint32_t dumborc_extchannel_get_reg_data(uint32_t const idx , uint32_t const reg);

/**
 * [dumborc_extchannel_get_pin_status (dumborc_extchannel get pin status)]
 * @param  pin_mask [pin mask (uint32_t)]
 * @return          [pin status (uint32_t)]
 */
uint32_t dumborc_extchannel_get_pin_status(uint32_t const pin_mask);



#endif    /* ( DUMBORC_EXTCHANNEL_ENABLED ) */



#ifdef __cplusplus
}
#endif

#endif


