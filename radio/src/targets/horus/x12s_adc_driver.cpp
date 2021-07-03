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

#include "opentx.h"

uint16_t adcValues[NUM_ANALOGS] __DMA;
uint16_t rtcBatteryVoltage;

#define ADC_CS_HIGH()                  (ADC_SPI_GPIO->BSRRL = ADC_SPI_PIN_CS)
#define ADC_CS_LOW()                   (ADC_SPI_GPIO->BSRRH = ADC_SPI_PIN_CS)

#define SPI_STICK1                     0
#define SPI_STICK2                     1
#define SPI_STICK3                     2
#define SPI_STICK4                     3
#define SPI_S1                         4
#define SPI_6POS                       5
#define SPI_S2                         6
#define SPI_LS                         7
#define SPI_RS                         8
#define SPI_TX_VOLTAGE                 9
#define SPI_L2                         10
#define SPI_L1                         11
#define RESETCMD                       0x4000
#define MANUAL_MODE                    0x1000 // manual mode channel 0

uint16_t SPIx_ReadWriteByte(uint16_t value)
{
  while (SPI_I2S_GetFlagStatus(ADC_SPI, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(ADC_SPI, value);

  while (SPI_I2S_GetFlagStatus(ADC_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  return SPI_I2S_ReceiveData(ADC_SPI);
}

static void ADS7952_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef SPI_InitStructure;

  GPIO_InitStructure.GPIO_Pin = ADC_SPI_PIN_MISO | ADC_SPI_PIN_SCK | ADC_SPI_PIN_MOSI;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ADC_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = ADC_SPI_PIN_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ADC_SPI_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_SCK, ADC_GPIO_AF);
  GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_MISO, ADC_GPIO_AF);
  GPIO_PinAFConfig(ADC_SPI_GPIO, ADC_SPI_PinSource_MOSI, ADC_GPIO_AF);

  SPI_I2S_DeInit(ADC_SPI);

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(ADC_SPI, &SPI_InitStructure);
  SPI_Cmd(ADC_SPI, ENABLE);
  SPI_I2S_ITConfig(ADC_SPI, SPI_I2S_IT_TXE, DISABLE);
  SPI_I2S_ITConfig(ADC_SPI, SPI_I2S_IT_RXNE, DISABLE);

  ADC_CS_HIGH();
  delay_01us(1);
  ADC_CS_LOW();
  SPIx_ReadWriteByte(RESETCMD);
  ADC_CS_HIGH();
  delay_01us(1);
  ADC_CS_LOW();
  SPIx_ReadWriteByte(MANUAL_MODE);
  ADC_CS_HIGH();
}

void adcInit()
{
  ADS7952_Init();

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = ADC_GPIO_PIN_MOUSE1 | ADC_GPIO_PIN_MOUSE2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(ADC_GPIO_MOUSE, &GPIO_InitStructure);

  ADC3->CR1 = ADC_CR1_SCAN;
  ADC3->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC3->SQR1 = (2 - 1) << 20;
  ADC3->SQR2 = 0;
  ADC3->SQR3 = (ADC_CHANNEL_MOUSE1 << 0) + (ADC_CHANNEL_MOUSE2 << 5);
  ADC3->SMPR1 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24);
  ADC3->SMPR2 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24) + (ADC_SAMPTIME << 27);
  ADC->CCR = 0;

  // Enable the DMA channel here, DMA2 stream 1, channel 2
  ADC_DMA_Stream->CR = DMA_SxCR_PL | DMA_SxCR_CHSEL_1 | DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC;
  ADC_DMA_Stream->PAR = CONVERT_PTR_UINT(&ADC3->DR);
  ADC_DMA_Stream->M0AR = CONVERT_PTR_UINT(&adcValues[MOUSE1]);
  ADC_DMA_Stream->NDTR = 2;
  ADC_DMA_Stream->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

  ADC->CCR = ADC_CCR_VBATE; // Enable RTC coin cell sampling bridge
  
  ADC1->CR1 = ADC_CR1_SCAN;
  ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_DMA | ADC_CR2_DDS;
  ADC1->SQR1 = (1 - 1) << 20;
  ADC1->SQR2 = 0;
  ADC1->SQR3 = (ADC_Channel_Vbat << 0);
  ADC1->SMPR1 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24);
  ADC1->SMPR2 = (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24) + (ADC_SAMPTIME << 27);
}

uint16_t getRTCBatteryVoltage()
{
  return (rtcBatteryVoltage * 2 * ADC_VREF_PREC2) / 2048;
}

const uint16_t adcCommands[MOUSE1+2] =
{
  MANUAL_MODE | (SPI_STICK1 << 7),
  MANUAL_MODE | (SPI_STICK2 << 7),
  MANUAL_MODE | (SPI_STICK3 << 7),
  MANUAL_MODE | (SPI_STICK4 << 7),
  MANUAL_MODE | (SPI_S1 << 7),
  MANUAL_MODE | (SPI_6POS << 7),
  MANUAL_MODE | (SPI_S2 << 7),
  MANUAL_MODE | (SPI_L1 << 7),
  MANUAL_MODE | (SPI_L2 << 7),
  MANUAL_MODE | (SPI_LS << 7),
  MANUAL_MODE | (SPI_RS << 7),
  MANUAL_MODE | (SPI_TX_VOLTAGE << 7),
  MANUAL_MODE | (0 << 7),     // small joystick left/right
  MANUAL_MODE | (0 << 7)      // small joystick up/down
};

void adcReadSPIDummy()
{
  // A dummy command to get things started
  // (because the sampled data is lagging behind for two command cycles)
  ADC_CS_LOW();
  delay_01us(1);
  SPIx_ReadWriteByte(adcCommands[0]);
  ADC_CS_HIGH();
  delay_01us(1);
}

uint32_t adcReadNextSPIChannel(uint8_t index)
{
  uint32_t result = 0;

  // This delay is to allow charging of ADC input capacitor
  // after the MUX changes from one channel to the other.
  // It was determined experimentally. Biggest problem seems to be
  // the cross-talk between A4:S1 and A5:MULTIPOS. Changing S1 for one extreme
  // to the other resulted in A5 change of:
  //
  //        delay value       A5 change     Time needed for adcRead()
  //          1               16            0.154ms - 0.156ms
  //         38               5             0.197ms - 0.199ms
  //         62               0             0.225ms - 0.243ms
  delay_01us(40);

  for (uint8_t i = 0; i < 4; i++) {
    ADC_CS_LOW();
    delay_01us(1);
    // command is changed to the next index for the last two readings
    // (because the sampled data is lagging behind for two command cycles)
    uint16_t val = (0x0fff & SPIx_ReadWriteByte(adcCommands[(i>1) ? index+1 : index]));
#if defined(JITTER_MEASURE)
    if (JITTER_MEASURE_ACTIVE()) {
      rawJitter[index].measure(val);
    }
#endif
    ADC_CS_HIGH();
    delay_01us(1);
    result += val;
  }

  return result >> 2;
}

void adcOnChipReadStart()
{
  ADC_DMA_Stream->CR &= ~DMA_SxCR_EN;           // Disable DMA
  ADC3->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC_DMA->LIFCR = DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0; // Write ones to clear bits
  ADC_DMA_Stream->M0AR = CONVERT_PTR_UINT(&adcValues[MOUSE1]);
  ADC_DMA_Stream->NDTR = 2;
  ADC_DMA_Stream->CR |= DMA_SxCR_EN;            // Enable DMA
  ADC3->CR2 |= (uint32_t)ADC_CR2_SWSTART;

  ADC1->SR &= ~(uint32_t)(ADC_SR_EOC | ADC_SR_STRT | ADC_SR_OVR);
  ADC1->CR2 |= (uint32_t)ADC_CR2_SWSTART;
}

bool adcOnChipReadFinished()
{
  return (ADC_DMA->LISR & DMA_LISR_TCIF0);
}

void adcRead()
{
  uint16_t temp[NUM_ANALOGS-MOUSE1] = { 0 };
  uint8_t noInternalReads = 0;

  adcOnChipReadStart();
  adcReadSPIDummy();
  adcReadSPIDummy();
  for (uint32_t index=0; index<MOUSE1; index++) {
    adcValues[index] = adcReadNextSPIChannel(index);
    if (noInternalReads < 4 && adcOnChipReadFinished()) {
      for (uint8_t x=0; x<NUM_ANALOGS-MOUSE1; x++) {
        uint16_t val = adcValues[MOUSE1+x];
#if defined(JITTER_MEASURE)
        if (JITTER_MEASURE_ACTIVE()) {
          rawJitter[MOUSE1+x].measure(val);
        }
#endif
        temp[x] += val;
      }
      if (++noInternalReads < 4) {
        adcOnChipReadStart();
      }
    }
  }

#if defined(DEBUG)
  if (noInternalReads != 4) {
    TRACE("Internal ADC problem: reads: %d", noInternalReads);
  }
#endif

  for (uint8_t x=0; x<NUM_ANALOGS-MOUSE1; x++) {
    adcValues[MOUSE1+x] = temp[x] >> 2;
  }

  if (isVBatBridgeEnabled()) {
    rtcBatteryVoltage = ADC1->DR;
  }
}

#if !defined(SIMU)
const int8_t ana_direction[NUM_ANALOGS] = {1,-1,1,-1,  -1,1,-1,  -1,-1,  -1,1, 0,0,0};

uint16_t getAnalogValue(uint8_t index)
{
  if (ana_direction[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
