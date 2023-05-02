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

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "hal.h"
#include "debug.h"
#include "delays_driver.h"

#include "sdio_sd.h"

static SD_HandleTypeDef sdio;
static DMA_HandleTypeDef sdioTxDma;

void SD_LowLevel_Init(void)
{
  /* Enable the SDIO APB2 Clock */
  __HAL_RCC_SDIO_CLK_ENABLE();

  LL_GPIO_InitTypeDef  GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  /* Configure PC.08, PC.09, PC.10, PC.11 pins: D0, D1, D2, D3 pins */
  GPIO_InitStructure.Pin = LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_12; // SDIO
  LL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PD.02 CMD line */
  GPIO_InitStructure.Pin = LL_GPIO_PIN_2;
  LL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* Configure PC.12 pin: CLK pin */
  GPIO_InitStructure.Pin = LL_GPIO_PIN_12;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  // SDIO Interrupt ENABLE
  NVIC_SetPriority(SDIO_IRQn, 0);
  NVIC_EnableIRQ(SDIO_IRQn);
  // DMA2 STREAMx Interrupt ENABLE
  NVIC_SetPriority(SD_SDIO_DMA_IRQn, 0);
  NVIC_EnableIRQ(SD_SDIO_DMA_IRQn);

}

void SD_LowLevel_DMA_Config(uint32_t * BufferSRC, uint32_t BufferSize)
{
  sdioTxDma.Instance = SD_SDIO_DMA_STREAM;
  sdioTxDma.Init.Channel = SD_SDIO_DMA_CHANNEL;
//  sdioTxDma.Init.DMA_PeripheralBaseAddr = SD_SDIO_FIFO_ADDRESS;
//  sdioTxDma.Init.DMA_Memory0BaseAddr = (intptr_t)BufferSRC;
  sdioTxDma.Init.Direction = DMA_MEMORY_TO_PERIPH;
//  sdioTxDma.Init.DMA_BufferSize = 1;
  sdioTxDma.Init.PeriphInc = DMA_PINC_DISABLE;
  sdioTxDma.Init.MemInc = DMA_MINC_ENABLE;
  sdioTxDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  sdioTxDma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  sdioTxDma.Init.Mode = DMA_PFCTRL;
  sdioTxDma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  sdioTxDma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  sdioTxDma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  sdioTxDma.Init.MemBurst = DMA_MBURST_INC4;
  sdioTxDma.Init.PeriphBurst = DMA_PBURST_INC4;

  HAL_DMA_Init(&sdioTxDma);
  __HAL_LINKDMA(&sdio, hdmatx, sdioTxDma);
  __HAL_LINKDMA(&sdio, hdmarx, sdioTxDma);
}

static bool init = false;
SD_Error SD_Init(void)
{
  if(init) return SD_OK;
  init = true;
  __IO SD_Error errorstatus = SD_OK;


  /* SDIO Peripheral Low Level Init */
  SD_LowLevel_Init();
  SD_LowLevel_DMA_Config(nullptr, 0);
  __HAL_RCC_DMA2_CLK_ENABLE();

  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  sdio.Instance = SDIO;
  sdio.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  sdio.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  sdio.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  sdio.Init.BusWide = SDIO_BUS_WIDE_1B;
  sdio.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  sdio.Init.ClockDiv = SD_SDIO_TRANSFER_CLK_DIV;
  HAL_SD_DeInit(&sdio);

  HAL_StatusTypeDef halStatus = HAL_SD_Init(&sdio);
  if (halStatus != HAL_OK) {
    TRACE("SD_PowerON() status=%d", halStatus);
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return SD_ERROR;
  }

  HAL_SD_CardInfoTypeDef cardInfo;
  HAL_StatusTypeDef es = HAL_SD_GetCardInfo(&sdio, &cardInfo);
  if(es != HAL_OK)
    return SD_ERROR;
  HAL_SD_ConfigWideBusOperation(&sdio, SDIO_BUS_WIDE_4B);

  return errorstatus;
}


/**
  * @brief  Gets the cuurent sd card data transfer status.
  * @param  None
  * @retval SDTransferState: Data Transfer state.
  *   This value can be:
  *        - SD_TRANSFER_OK: No data transfer is acting
  *        - SD_TRANSFER_BUSY: Data transfer is acting
  */
SDTransferState SD_GetStatus(void)
{
  HAL_SD_CardStateTypeDef cardstate = HAL_SD_GetCardState(&sdio);

  if (cardstate == HAL_SD_CARD_TRANSFER) {
    return SD_TRANSFER_OK;
  }
  else if (cardstate == HAL_SD_CARD_ERROR) {
    return SD_TRANSFER_ERROR;
  }
  else {
    return SD_TRANSFER_BUSY;
  }
}

/**
 * @brief  Detect if SD card is correctly plugged in the memory slot.
 * @param  None
 * @retval Return if SD is detected or not
 */
uint8_t SD_Detect(void)
{
  __IO uint8_t status = SD_PRESENT;

  /*!< Check GPIO to detect SD */
  if ((LL_GPIO_ReadInputPort(SD_PRESENT_GPIO) & SD_PRESENT_LL_GPIO_PIN) != 0) {
    status = SD_NOT_PRESENT;
  }

  return status;
}

//---------------------------------------------------------------------------------------
//disable optimize
#define OPTIMIZE(level) __attribute__((optimize(level)))
/**
  * @brief  Returns information about specific card.
  * @param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card
  *         information.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo)
{
  if(HAL_SD_GetCardInfo(&sdio, cardinfo) != HAL_OK)
    return SD_ERROR;
  return SD_OK;
}

/**
  * @brief  Allows to read one block from a specified address in a card. The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_ReadBlock(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize)
{
  HAL_StatusTypeDef sdStatus = HAL_SD_ReadBlocks_DMA(&sdio, readbuff, ReadAddr, 1);
  if(sdStatus != HAL_OK)
    return SD_ERROR;

  return SD_OK;
}

/**
  * @brief  Allows to read blocks from a specified address  in a card.  The Data
  *         transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  readbuff: pointer to the buffer that will contain the received data.
  * @param  ReadAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  HAL_StatusTypeDef res = HAL_SD_ReadBlocks_DMA(&sdio, readbuff, ReadAddr, NumberOfBlocks);
  if(res == HAL_OK)
    return SD_OK;

  return SD_ERROR;
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_ReadMultiBlocks() function
  *         to insure that all data sent by the card are already transferred by
  *         the DMA controller.
  * @param  None.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_WaitReadOperation(uint32_t timeout)
{
  volatile HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&sdio);
  if(state == HAL_SD_CARD_READY || state == HAL_SD_CARD_TRANSFER)
    return SD_OK;
  timeout = 100;

  while((HAL_SD_GetCardState(&sdio) == HAL_SD_CARD_SENDING) && (timeout > 0)) {
    delay_ms(1);
    timeout--;
  }

  state = HAL_SD_GetCardState(&sdio);
  if(timeout > 0 && state == HAL_SD_CARD_TRANSFER)
    return SD_OK;

  return SD_ERROR;
}

/**
  * @brief  Allows to write one block starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode or Polling mode.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_WriteBlock(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize)
{
  HAL_StatusTypeDef res = HAL_SD_WriteBlocks_DMA(&sdio,  writebuff,  WriteAddr,  1);
  if(res == HAL_OK)
    return SD_OK;

  return SD_ERROR;
}

/**
  * @brief  Allows to write blocks starting from a specified address in a card.
  *         The Data transfer can be managed by DMA mode only.
  * @note   This operation should be followed by two functions to check if the
  *         DMA Controller and SD Card status.
  *          - SD_ReadWaitOperation(): this function insure that the DMA
  *            controller has finished all data transfer.
  *          - SD_GetStatus(): to check that the SD Card has finished the
  *            data transfer and it is ready for data.
  * @param  WriteAddr: Address from where data are to be read.
  * @param  writebuff: pointer to the buffer that contain the data to be transferred.
  * @param  BlockSize: the SD card Data block size. The Block size should be 512.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_WriteMultiBlocks(uint8_t *writebuff, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  HAL_StatusTypeDef res = HAL_SD_WriteBlocks_DMA(&sdio, writebuff, WriteAddr, NumberOfBlocks);
  if(res == HAL_OK)
    return SD_OK;
  return SD_ERROR;
}

/**
  * @brief  This function waits until the SDIO DMA data transfer is finished.
  *         This function should be called after SDIO_WriteBlock() and
  *         SDIO_WriteMultiBlocks() function to insure that all data sent by the
  *         card are already transferred by the DMA controller.
  * @param  None.
  * @retval SD_Error: SD Card Error code.
  */
OPTIMIZE("O0") SD_Error SD_WaitWriteOperation(uint32_t timeout)
{
  HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&sdio);
  if(state == HAL_SD_CARD_READY || state == HAL_SD_CARD_TRANSFER)
    return SD_OK;

  timeout = 1000;

  state = HAL_SD_GetCardState(&sdio);
  while((state == HAL_SD_CARD_RECEIVING || state == HAL_SD_CARD_PROGRAMMING) && (timeout > 0)) {
    delay_ms(1);
    timeout--;
    state = HAL_SD_GetCardState(&sdio);
  }

  state = HAL_SD_GetCardState(&sdio);
  if(timeout > 0 && state == HAL_SD_CARD_TRANSFER)
    return SD_OK;

  return SD_ERROR;
}

size_t SD_GetSectorCount()
{
  HAL_SD_CardInfoTypeDef cardInfo;

  if(SD_GetCardInfo(&cardInfo) != SD_OK)
    return 0;

  return cardInfo.LogBlockNbr;
}

size_t SD_GetSectorSize()
{
  HAL_SD_CardInfoTypeDef cardInfo;

  if(SD_GetCardInfo(&cardInfo) != SD_OK)
    return 0;

  return cardInfo.LogBlockSize;
}

size_t SD_GetBlockSize()
{
  HAL_SD_CardInfoTypeDef cardInfo;

  if(SD_GetCardInfo(&cardInfo) != SD_OK)
    return 0;

  return cardInfo.LogBlockSize / BLOCK_SIZE;
}


extern "C" void SDIO_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_SDIO);
  HAL_SD_IRQHandler(&sdio);
}
extern "C" void SD_SDIO_DMA_IRQHANDLER(void)
{
  DEBUG_INTERRUPT(INT_SDIO_DMA);
  HAL_DMA_IRQHandler(&sdioTxDma);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
