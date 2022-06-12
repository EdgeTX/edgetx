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

/* Includes ------------------------------------------------------------------*/

extern "C" {
#include "usbd_usr.h"

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */


const USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,

  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,
};
}

/**
* @brief Whatever the user application needs to do when USB is initialised
* @param  None
* @retval None
*/
void USBD_USR_Init(void)
{

}

/**
* @brief  Displays the message on LCD on device reset event
* @param  speed : device speed
* @retval None
*/
void USBD_USR_DeviceReset (uint8_t speed)
{

}


/**
* @brief  Displays the message on LCD on device config event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConfigured (void)
{

}
/**
* @brief  Displays the message on LCD on device suspend event
* @param  None
* @retval None
*/
void USBD_USR_DeviceSuspended(void)
{

}


/**
* @brief  Displays the message on LCD on device resume event
* @param  None
* @retval None
*/
void USBD_USR_DeviceResumed(void)
{

}

/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConnected (void)
{

}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceDisconnected (void)
{
#if !defined(BOOT) && defined(EEPROM)
  // TODO is it really needed if we didn't write the EEPROM?
//  if (getSelectedUsbMode() == USB_MASS_STORAGE_MODE)
//    NVIC_SystemReset();
#endif
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
