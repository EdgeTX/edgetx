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

#include "ads79xx.h"
#include "stm32_spi.h"
#include "stm32_gpio_driver.h"
#include "delays_driver.h"

#include "stm32_hal_ll.h"

#define RESETCMD                       0x4000
#define MANUAL_MODE                    0x1000 // manual mode channel 0
#define MANUAL_MODE_CHANNEL(x)         (MANUAL_MODE | ((x) << 7))

#define ADS79XX_MAX_FREQ (20000000UL)

static inline uint16_t ads79xx_rw(const stm32_spi_t* spi, uint16_t value)
{
  return stm32_spi_transfer_word(spi, value);
}

void ads79xx_init(const stm32_spi_adc_t* adc)
{
  const auto* spi = &adc->spi;

  stm32_spi_init(spi, LL_SPI_DATAWIDTH_16BIT);
  stm32_spi_set_max_baudrate(spi, ADS79XX_MAX_FREQ);
  
  stm32_spi_unselect(spi);
  delay_01us(1);

  stm32_spi_select(spi);
  ads79xx_rw(spi, RESETCMD);
  stm32_spi_unselect(spi);
  delay_01us(1);

  stm32_spi_select(spi);
  ads79xx_rw(spi, MANUAL_MODE);
  stm32_spi_unselect(spi);
}

static void ads79xx_dummy_read(const stm32_spi_adc_t* adc, uint16_t start_channel)
{
  // A dummy command to get things started
  // (because the sampled data is lagging behind for two command cycles)
  const auto* spi = &adc->spi;
  stm32_spi_select(spi);
  delay_01us(1);
  ads79xx_rw(spi, MANUAL_MODE_CHANNEL(start_channel));
  stm32_spi_unselect(spi);
  delay_01us(1);
}

static uint32_t ads79xx_read_next_channel(const stm32_spi_adc_t* adc,
                                          uint8_t index,
                                          const stm32_adc_input_t* inputs,
                                          const uint8_t* chan, uint8_t nconv)
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

  const auto* spi = &adc->spi;
  for (uint8_t i = 0; i < 4; i++) {
    stm32_spi_select(spi);
    delay_01us(1);

    // command is changed to the next index for the last two readings
    // (because the sampled data is lagging behind for two command cycles)
    uint8_t chan_idx = (i > 1 ? index + 1 : index) % nconv;

    auto input_idx = chan[chan_idx];
    auto spi_chan = inputs[input_idx].ADC_Channel;
    
    uint16_t val = (0x0fff & ads79xx_rw(spi, MANUAL_MODE_CHANNEL(spi_chan)));

#if defined(JITTER_MEASURE)
    if (JITTER_MEASURE_ACTIVE()) {
      rawJitter[index].measure(val);
    }
#endif

    stm32_spi_unselect(spi);
    delay_01us(1);
    result += val;
  }

  return result >> 2;
}

bool ads79xx_adc_start_read(const stm32_spi_adc_t* adc, const stm32_adc_input_t* inputs)
{
  auto start_channel = inputs[adc->channels[0]].ADC_Channel;
  ads79xx_dummy_read(adc, start_channel);
  ads79xx_dummy_read(adc, start_channel);

  return true;
}

void ads79xx_adc_wait_completion(const stm32_spi_adc_t* adc, const stm32_adc_input_t* inputs)
{
  auto spi_channels = adc->n_channels;
  auto chans = adc->channels;
  
  // Fetch buffer from generic ADC driver
  auto adcValues = getAnalogValues();
  
  // At this point, the first conversion has been started
  // in ads79xx_adc_start_read()

  // for each SPI channel
  for (uint32_t i = 0; i < spi_channels; i++) {
    
    // read SPI channel
    auto input_channel = chans[i];
    auto adc_value = ads79xx_read_next_channel(adc, i, inputs, chans, spi_channels);

    if (inputs[input_channel].inverted)
      adcValues[input_channel] = ADC_INVERT_VALUE(adc_value);
    else
      adcValues[input_channel] = adc_value;
  }
}
