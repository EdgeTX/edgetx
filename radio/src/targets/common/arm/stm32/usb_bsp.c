/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "usb_bsp.h"
#include "board.h"
#include "usbd_conf.h"

extern uint32_t SystemCoreClock;

/**
* @brief  USB_OTG_BSP_Init
*         Initilizes BSP configurations
* @param  None
* @retval None
*/

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  
  RCC_AHB1PeriphClockCmd(USB_RCC_AHB1Periph_GPIO, ENABLE);
  
  /* Configure DM and DP Pins */
  GPIO_InitStructure.GPIO_Pin = USB_GPIO_PIN_DM | USB_GPIO_PIN_DP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(USB_GPIO, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(USB_GPIO, USB_GPIO_PinSource_DM, USB_GPIO_AF);
  GPIO_PinAFConfig(USB_GPIO, USB_GPIO_PinSource_DP, USB_GPIO_AF);
  
  /* Configure VBUS Pin */
  GPIO_InitStructure.GPIO_Pin = USB_GPIO_PIN_VBUS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(USB_GPIO, &GPIO_InitStructure);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ; 
}

void USB_OTG_BSP_Deinit(USB_OTG_CORE_HANDLE *pdev)
{
  //nothing to do  
}

/**
* @brief  USB_OTG_BSP_EnableInterrupt
*         Enable USB Global interrupt
* @param  None
* @retval None
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
  // FreeRTOS syscall prio is 5
  NVIC_SetPriority(OTG_FS_IRQn, 11); // Lower priority interrupt
  NVIC_EnableIRQ(OTG_FS_IRQn);
}

/**
* @brief  USB_OTG_BSP_DisableInterrupt
*         Disable USB Global interrupt
* @param  None
* @retval None
*/
void USB_OTG_BSP_DisableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
  NVIC_DisableIRQ(OTG_FS_IRQn);
}


/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
  delay_us(usec);
}


/**
* @brief  USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
  delay_ms(msec);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
