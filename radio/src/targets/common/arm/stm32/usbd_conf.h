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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USBD_CONF_H_
#define _USBD_CONF_H_

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>

#define USBD_CFG_MAX_NUM           1
#define USBD_ITF_MAX_NUM           1
#define USB_MAX_STR_DESC_SIZ       64

#define USBD_SELF_POWERED

/* Class Layer Parameter */

#define MSC_IN_EP                    0x81
#define MSC_OUT_EP                   0x01
#define MSC_MAX_PACKET               64

// Save some RAM on smaller STM32 processors. Slightly lowers USB mass storage speed
#if defined(STM32F2) && !defined(BOOT)
#define MSC_MEDIA_PACKET             512
#else
#define MSC_MEDIA_PACKET             4096
#endif

#define HID_IN_EP                    0x81
#define HID_OUT_EP                   0x01

#define HID_IN_PACKET                19
#define HID_OUT_PACKET               9

#define CDC_IN_EP                    0x81  /* EP1 for data IN */
#define CDC_OUT_EP                   0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                   0x82  /* EP2 for CDC commands */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_MAX_PACKET_SIZE     64   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE           8    /* Control Endpoint Packet size */

#if defined(STM32F2)
#define CDC_IN_FRAME_INTERVAL        4    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE             512  /* Total size of IN buffer: APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL */
#else
#define CDC_IN_FRAME_INTERVAL        15    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE             2048  /* Total size of IN buffer: APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL */
#endif
#define APP_FOPS                     VCP_fops

#endif // _USBD_CONF_H_


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
