/**
  ******************************************************************************
  * @file    usbd_dfu_media_template.c
  * @author  MCD Application Team
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MEDIA_DESC_STR      "@External Flash   /0x70000000/2048*064Kg"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static uint16_t MEM_If_Init(void);
static uint16_t MEM_If_Erase(uint32_t Add);
static uint16_t MEM_If_Write(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint8_t *MEM_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len);
static uint16_t MEM_If_DeInit(void);
static uint16_t MEM_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer);

extern USBD_DFU_MediaTypeDef USBD_DFU_MEDIA_fops;

USBD_DFU_MediaTypeDef USBD_DFU_MEDIA_fops =
{
  (uint8_t *)MEDIA_DESC_STR,
  MEM_If_Init,
  MEM_If_DeInit,
  MEM_If_Erase,
  MEM_If_Write,
  MEM_If_Read,
  MEM_If_GetStatus,
};

/**
  * @brief  MEM_If_Init
  *         Memory initialization routine.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init(void)
{
  return 0;
}

/**
  * @brief  MEM_If_DeInit
  *         Memory deinitialization routine.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
static uint16_t MEM_If_DeInit(void)
{
  return 0;
}

/**
  * @brief  MEM_If_Erase
  *         Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
static uint16_t MEM_If_Erase(uint32_t Add)
{
  UNUSED(Add);

  return 0;
}

/**
  * @brief  MEM_If_Write
  *         Memory write routine.
  * @param  Add: Address to be written to.
  * @param  Len: Number of data to be written (in bytes).
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
static uint16_t MEM_If_Write(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  UNUSED(src);
  UNUSED(dest);
  UNUSED(Len);

  return 0;
}

/**
  * @brief  MEM_If_Read
  *         Memory read routine.
  * @param  Add: Address to be read from.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
static uint8_t *MEM_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  UNUSED(src);
  UNUSED(dest);
  UNUSED(Len);

  /* Return a valid address to avoid HardFault */
  return NULL;
}

/**
  * @brief  Flash_If_GetStatus
  *         Memory read routine.
  * @param  Add: Address to be read from.
  * @param  cmd: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
static uint16_t MEM_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  UNUSED(Add);
  UNUSED(buffer);

  switch (Cmd)
  {
    case DFU_MEDIA_PROGRAM:

      break;

    case DFU_MEDIA_ERASE:
    default:

      break;
  }
  return (0);
}
