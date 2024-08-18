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

#include "stm32_i2c_driver.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "timers_driver.h"
#include "debug.h"

#define MAX_I2C_DEVICES 2

struct stm32_i2c_device {
  I2C_HandleTypeDef handle;
  const stm32_i2c_hw_def_t* hw_def;
};

static stm32_i2c_device _i2c_devs[MAX_I2C_DEVICES] = {};

static stm32_i2c_device* i2c_get_device(uint8_t bus)
{
  if (bus >= MAX_I2C_DEVICES) return nullptr;
  return &_i2c_devs[bus];
}

static I2C_HandleTypeDef* i2c_get_handle(uint8_t bus)
{
  if (bus >= MAX_I2C_DEVICES) return nullptr;
  return &_i2c_devs[bus].handle;
}

#if defined(STM32H7) || defined(STM32H7RS)

/* ST's I2C timing computation */

#define I2C_VALID_TIMING_NBR                   8U
#define I2C_SPEED_FREQ_STANDARD                0U    /* 100 kHz */
#define I2C_SPEED_FREQ_FAST                    1U    /* 400 kHz */
#define I2C_SPEED_FREQ_FAST_PLUS               2U    /* 1 MHz */
#define I2C_ANALOG_FILTER_DELAY_MIN            50U   /* ns */
#define I2C_ANALOG_FILTER_DELAY_MAX            260U  /* ns */
#define I2C_USE_ANALOG_FILTER                  1U
#define I2C_DIGITAL_FILTER_COEF                0U
#define I2C_PRESC_MAX                          16U
#define I2C_SCLDEL_MAX                         16U
#define I2C_SDADEL_MAX                         16U
#define I2C_SCLH_MAX                           256U
#define I2C_SCLL_MAX                           256U
#define SEC2NSEC                               1000000000UL

typedef struct
{
  uint32_t freq;       /* Frequency in Hz */
  uint32_t freq_min;   /* Minimum frequency in Hz */
  uint32_t freq_max;   /* Maximum frequency in Hz */
  uint32_t hddat_min;  /* Minimum data hold time in ns */
  uint32_t vddat_max;  /* Maximum data valid time in ns */
  uint32_t sudat_min;  /* Minimum data setup time in ns */
  uint32_t lscl_min;   /* Minimum low period of the SCL clock in ns */
  uint32_t hscl_min;   /* Minimum high period of SCL clock in ns */
  uint32_t trise;      /* Rise time in ns */
  uint32_t tfall;      /* Fall time in ns */
  uint32_t dnf;        /* Digital noise filter coefficient */
} I2C_Charac_t;

typedef struct
{
  uint32_t presc;      /* Timing prescaler */
  uint32_t tscldel;    /* SCL delay */
  uint32_t tsdadel;    /* SDA delay */
  uint32_t sclh;       /* SCL high period */
  uint32_t scll;       /* SCL low period */
} I2C_Timings_t;

static const I2C_Charac_t I2C_Charac[] =
{
  /* I2C_SPEED_FREQ_STANDARD */
  {
    .freq = 100000,
    .freq_min = 80000,
    .freq_max = 120000,
    .hddat_min = 0,
    .vddat_max = 3450,
    .sudat_min = 250,
    .lscl_min = 4700,
    .hscl_min = 4000,
    .trise = 640,
    .tfall = 20,
    .dnf = I2C_DIGITAL_FILTER_COEF,
  },
  /* I2C_SPEED_FREQ_FAST */
  {
    .freq = 400000,
    .freq_min = 320000,
    .freq_max = 480000,
    .hddat_min = 0,
    .vddat_max = 900,
    .sudat_min = 100,
    .lscl_min = 1300,
    .hscl_min = 600,
    .trise = 250,
    .tfall = 100,
    .dnf = I2C_DIGITAL_FILTER_COEF,
  },
  /* I2C_SPEED_FREQ_FAST_PLUS */
  {
    .freq = 1000000,
    .freq_min = 800000,
    .freq_max = 1200000,
    .hddat_min = 0,
    .vddat_max = 450,
    .sudat_min = 50,
    .lscl_min = 500,
    .hscl_min = 260,
    .trise = 60,
    .tfall = 100,
    .dnf = I2C_DIGITAL_FILTER_COEF,
  },
};

static I2C_Timings_t I2c_valid_timing[I2C_VALID_TIMING_NBR];
static uint32_t      I2c_valid_timing_nbr = 0;

static void I2C_Compute_PRESC_SCLDEL_SDADEL(uint32_t clock_src_freq, uint32_t I2C_speed);
static uint32_t I2C_Compute_SCLL_SCLH (uint32_t clock_src_freq, uint32_t I2C_speed);

/**
  * @brief  Compute I2C timing according current I2C clock source and required I2C clock.
  * @param  clock_src_freq I2C clock source in Hz.
  * @param  i2c_freq Required I2C clock in Hz.
  * @retval I2C timing or 0 in case of error.
  */
static uint32_t I2C_GetTiming(uint32_t clock_src_freq, uint32_t i2c_freq)
{
  uint32_t ret = 0;
  uint32_t speed;
  uint32_t idx;

  if((clock_src_freq != 0U) && (i2c_freq != 0U))
  {
    for ( speed = 0 ; speed <=  (uint32_t)I2C_SPEED_FREQ_FAST_PLUS ; speed++)
    {
      if ((i2c_freq >= I2C_Charac[speed].freq_min) &&
          (i2c_freq <= I2C_Charac[speed].freq_max))
      {
        I2c_valid_timing_nbr = 0;
        I2C_Compute_PRESC_SCLDEL_SDADEL(clock_src_freq, speed);
        idx = I2C_Compute_SCLL_SCLH(clock_src_freq, speed);

        if (idx < I2C_VALID_TIMING_NBR)
        {
          ret = ((I2c_valid_timing[idx].presc  & 0x0FU) << 28) |\
                ((I2c_valid_timing[idx].tscldel & 0x0FU) << 20) |\
                ((I2c_valid_timing[idx].tsdadel & 0x0FU) << 16) |\
                ((I2c_valid_timing[idx].sclh & 0xFFU) << 8) |\
                ((I2c_valid_timing[idx].scll & 0xFFU) << 0);
        }
        break;
      }
    }
  }

  return ret;
}

/**
  * @brief  Compute PRESC, SCLDEL and SDADEL.
  * @param  clock_src_freq I2C source clock in HZ.
  * @param  I2C_speed I2C frequency (index).
  * @retval None.
  */
static void I2C_Compute_PRESC_SCLDEL_SDADEL(uint32_t clock_src_freq, uint32_t I2C_speed)
{
  uint32_t prev_presc = I2C_PRESC_MAX;
  uint32_t ti2cclk;
  int32_t  tsdadel_min, tsdadel_max;
  int32_t  tscldel_min;
  uint32_t presc, scldel, sdadel;
  uint32_t tafdel_min, tafdel_max;

  ti2cclk   = (SEC2NSEC + (clock_src_freq / 2U))/ clock_src_freq;

  tafdel_min = (I2C_USE_ANALOG_FILTER == 1U) ? I2C_ANALOG_FILTER_DELAY_MIN : 0U;
  tafdel_max = (I2C_USE_ANALOG_FILTER == 1U) ? I2C_ANALOG_FILTER_DELAY_MAX : 0U;

  /* tDNF = DNF x tI2CCLK
     tPRESC = (PRESC+1) x tI2CCLK
     SDADEL >= {tf +tHD;DAT(min) - tAF(min) - tDNF - [3 x tI2CCLK]} / {tPRESC}
     SDADEL <= {tVD;DAT(max) - tr - tAF(max) - tDNF- [4 x tI2CCLK]} / {tPRESC} */

  tsdadel_min = (int32_t)I2C_Charac[I2C_speed].tfall + (int32_t)I2C_Charac[I2C_speed].hddat_min -
    (int32_t)tafdel_min - (int32_t)(((int32_t)I2C_Charac[I2C_speed].dnf + 3) * (int32_t)ti2cclk);

  tsdadel_max = (int32_t)I2C_Charac[I2C_speed].vddat_max - (int32_t)I2C_Charac[I2C_speed].trise -
    (int32_t)tafdel_max - (int32_t)(((int32_t)I2C_Charac[I2C_speed].dnf + 4) * (int32_t)ti2cclk);


  /* {[tr+ tSU;DAT(min)] / [tPRESC]} - 1 <= SCLDEL */
  tscldel_min = (int32_t)I2C_Charac[I2C_speed].trise + (int32_t)I2C_Charac[I2C_speed].sudat_min;

  if (tsdadel_min <= 0)
  {
    tsdadel_min = 0;
  }

  if (tsdadel_max <= 0)
  {
    tsdadel_max = 0;
  }

  for (presc = 0; presc < I2C_PRESC_MAX; presc++)
  {
    for (scldel = 0; scldel < I2C_SCLDEL_MAX; scldel++)
    {
      /* TSCLDEL = (SCLDEL+1) * (PRESC+1) * TI2CCLK */
      uint32_t tscldel = (scldel + 1U) * (presc + 1U) * ti2cclk;

      if (tscldel >= (uint32_t)tscldel_min)
      {
        for (sdadel = 0; sdadel < I2C_SDADEL_MAX; sdadel++)
        {
          /* TSDADEL = SDADEL * (PRESC+1) * TI2CCLK */
          uint32_t tsdadel = (sdadel * (presc + 1U)) * ti2cclk;

          if ((tsdadel >= (uint32_t)tsdadel_min) && (tsdadel <= (uint32_t)tsdadel_max))
          {
            if(presc != prev_presc)
            {
              I2c_valid_timing[I2c_valid_timing_nbr].presc = presc;
              I2c_valid_timing[I2c_valid_timing_nbr].tscldel = scldel;
              I2c_valid_timing[I2c_valid_timing_nbr].tsdadel = sdadel;
              prev_presc = presc;
              I2c_valid_timing_nbr ++;

              if(I2c_valid_timing_nbr >= I2C_VALID_TIMING_NBR)
              {
                return;
              }
            }
          }
        }
      }
    }
  }
}

/**
  * @brief  Calculate SCLL and SCLH and find best configuration.
  * @param  clock_src_freq I2C source clock in HZ.
  * @param  I2C_speed I2C frequency (index).
  * @retval config index (0 to I2C_VALID_TIMING_NBR], 0xFFFFFFFF for no valid config.
  */
static uint32_t I2C_Compute_SCLL_SCLH (uint32_t clock_src_freq, uint32_t I2C_speed)
{
 uint32_t ret = 0xFFFFFFFFU;
  uint32_t ti2cclk;
  uint32_t ti2cspeed;
  uint32_t prev_error;
  uint32_t dnf_delay;
  uint32_t clk_min, clk_max;
  uint32_t scll, sclh;
  uint32_t tafdel_min;

  ti2cclk   = (SEC2NSEC + (clock_src_freq / 2U))/ clock_src_freq;
  ti2cspeed   = (SEC2NSEC + (I2C_Charac[I2C_speed].freq / 2U))/ I2C_Charac[I2C_speed].freq;

  tafdel_min = (I2C_USE_ANALOG_FILTER == 1U) ? I2C_ANALOG_FILTER_DELAY_MIN : 0U;

  /* tDNF = DNF x tI2CCLK */
  dnf_delay = I2C_Charac[I2C_speed].dnf * ti2cclk;

  clk_max = SEC2NSEC / I2C_Charac[I2C_speed].freq_min;
  clk_min = SEC2NSEC / I2C_Charac[I2C_speed].freq_max;

  prev_error = ti2cspeed;

  for (uint32_t count = 0; count < I2c_valid_timing_nbr; count++)
  {
    /* tPRESC = (PRESC+1) x tI2CCLK*/
    uint32_t tpresc = (I2c_valid_timing[count].presc + 1U) * ti2cclk;

    for (scll = 0; scll < I2C_SCLL_MAX; scll++)
    {
      /* tLOW(min) <= tAF(min) + tDNF + 2 x tI2CCLK + [(SCLL+1) x tPRESC ] */
      uint32_t tscl_l = tafdel_min + dnf_delay + (2U * ti2cclk) + ((scll + 1U) * tpresc);


      /* The I2CCLK period tI2CCLK must respect the following conditions:
      tI2CCLK < (tLOW - tfilters) / 4 and tI2CCLK < tHIGH */
      if ((tscl_l > I2C_Charac[I2C_speed].lscl_min) && (ti2cclk < ((tscl_l - tafdel_min - dnf_delay) / 4U)))
      {
        for (sclh = 0; sclh < I2C_SCLH_MAX; sclh++)
        {
          /* tHIGH(min) <= tAF(min) + tDNF + 2 x tI2CCLK + [(SCLH+1) x tPRESC] */
          uint32_t tscl_h = tafdel_min + dnf_delay + (2U * ti2cclk) + ((sclh + 1U) * tpresc);

          /* tSCL = tf + tLOW + tr + tHIGH */
          uint32_t tscl = tscl_l + tscl_h + I2C_Charac[I2C_speed].trise + I2C_Charac[I2C_speed].tfall;

          if ((tscl >= clk_min) && (tscl <= clk_max) && (tscl_h >= I2C_Charac[I2C_speed].hscl_min) && (ti2cclk < tscl_h))
          {
            int32_t error = (int32_t)tscl - (int32_t)ti2cspeed;

            if (error < 0)
            {
              error = -error;
            }

            /* look for the timings with the lowest clock error */
            if ((uint32_t)error < prev_error)
            {
              prev_error = (uint32_t)error;
              I2c_valid_timing[count].scll = scll;
              I2c_valid_timing[count].sclh = sclh;
              ret = count;
            }
          }
        }
      }
    }
  }

  return ret;
}


static int i2c_init_clock_source(I2C_TypeDef* instance)
{
#if defined(LL_RCC_I2C1_CLKSOURCE)
  if (instance == I2C1) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_RCC_SetClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
  }
#endif

#if defined(LL_RCC_I2C23_CLKSOURCE)
  if (instance == I2C2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
    LL_RCC_SetClockSource(LL_RCC_I2C23_CLKSOURCE_PCLK1);
  }
#endif

#if defined(LL_RCC_I2C123_CLKSOURCE)
  LL_RCC_SetClockSource(LL_RCC_I2C123_CLKSOURCE_PCLK1);
#endif

  return 0;
}

#endif

static int i2c_enable_clock(I2C_TypeDef* instance)
{
  /* Peripheral clock enable */
  if (instance == I2C1) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
  } else if (instance == I2C2) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
  } else if (instance == I2C3) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C3);
  } else {
    return -1;
  }

  return 0;
}

static int i2c_disable_clock(I2C_TypeDef* instance)
{
  /* Peripheral clock disable */
  if (instance == I2C1)
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C1);
  else if (instance == I2C2)
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C2);
  else if (instance == I2C3)
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_I2C3);
  else
    return -1;

  return 0;
}

static int i2c_gpio_init(const stm32_i2c_hw_def_t* hw_def)
{
  gpio_set(hw_def->SCL_GPIO);
  gpio_init(hw_def->SCL_GPIO, GPIO_OD_PU, GPIO_PIN_SPEED_MEDIUM);
  gpio_set_af(hw_def->SCL_GPIO, hw_def->GPIO_AF);

  gpio_set(hw_def->SDA_GPIO);
  gpio_init(hw_def->SDA_GPIO, GPIO_OD_PU, GPIO_PIN_SPEED_MEDIUM);
  gpio_set_af(hw_def->SDA_GPIO, hw_def->GPIO_AF);

  if (hw_def->set_pwr) {
    hw_def->set_pwr(true);
    // Add some delay to leave enought time
    // for devices to boot before querying them
    HAL_Delay(20);
  }
  
  return 0;
}

static int i2c_gpio_deinit(const stm32_i2c_hw_def_t* hw_def)
{
  // reconfigure pins as open-drain input
  gpio_init(hw_def->SCL_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(hw_def->SDA_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);

  if (hw_def->set_pwr) {
    hw_def->set_pwr(false);
  }

  return 0;
}


int stm32_i2c_init(uint8_t bus, uint32_t clock_rate, const stm32_i2c_hw_def_t* hw_def)
{
  auto dev = i2c_get_device(bus);
  if (!dev) return -1;

  if (!dev->hw_def) {
    dev->hw_def = hw_def;
    dev->handle.Instance = hw_def->I2Cx;
  }

  auto h = &dev->handle;
  I2C_InitTypeDef& init = h->Init;

#if defined(STM32H7) || defined(STM32H7RS)
  i2c_init_clock_source(h->Instance);
# if defined(LL_RCC_I2C123_CLKSOURCE_PCLK1)
  uint32_t pclk_freq = LL_RCC_GetI2CClockFreq(LL_RCC_I2C123_CLKSOURCE_PCLK1);
# elif defined(LL_RCC_I2C1_CLKSOURCE_PCLK1)
  uint32_t pclk_freq = LL_RCC_GetI2CClockFreq(LL_RCC_I2C1_CLKSOURCE_PCLK1);
# endif
  init.Timing = I2C_GetTiming(pclk_freq, clock_rate);
#else
  if (init.ClockSpeed > 0) {
    if (init.ClockSpeed != clock_rate) return -1;
    return 0;
  }
  init.ClockSpeed = clock_rate;
  init.DutyCycle = I2C_DUTYCYCLE_16_9;
#endif

  init.OwnAddress1 = 0;
  init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  init.OwnAddress2 = 0;
  init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (i2c_gpio_init(hw_def) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspInit() I2C_GPIO misconfiguration");
    return -1;
  }

  if (i2c_enable_clock(hw_def->I2Cx) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspInit() I2C misconfiguration");
    return -1;
  }

  if (HAL_I2C_Init(h) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2C_Init() failed");
    return -1;
  }

#if defined(I2C_FLTR_ANOFF) && defined(I2C_FLTR_DNF) || defined(STM32H7) || \
    defined(STM32H7RS)
  // Configure Analogue filter
  if (HAL_I2CEx_ConfigAnalogFilter(h, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2CEx_ConfigAnalogFilter() failed");
    return -1;
  }

  // Configure Digital filter
  if (HAL_I2CEx_ConfigDigitalFilter(h, 0) != HAL_OK) {
    TRACE("I2C ERROR: HAL_I2CEx_ConfigDigitalFilter() failed");
    return -1;
  }
#endif

  return 1;
}

int stm32_i2c_deinit(uint8_t bus)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;  

  if (HAL_I2C_DeInit(h) != HAL_OK) return -1;
  
  if (i2c_disable_clock(h->Instance) < 0) {
    TRACE("I2C ERROR: HAL_I2C_MspDeInit() I2C misconfiguration");
    return -1;
  }

  auto dev = i2c_get_device(bus);
  if (!dev) return -1;

  i2c_gpio_deinit(dev->hw_def);

  return 0;
}

int stm32_i2c_master_tx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Master_Transmit(h, addr << 1, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_master_rx(uint8_t bus, uint16_t addr, uint8_t *data, uint16_t len,
                        uint32_t timeout)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Master_Receive(h, addr << 1, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_read(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                   uint8_t* data, uint16_t len, uint32_t timeout)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;  

  if (HAL_I2C_Mem_Read(h, addr << 1, reg, reg_size, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_write(uint8_t bus, uint16_t addr, uint16_t reg, uint16_t reg_size,
                    uint8_t* data, uint16_t len, uint32_t timeout)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;  
  
  if (HAL_I2C_Mem_Write(h, addr << 1, reg, reg_size, data, len, timeout) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_i2c_is_dev_ready(uint8_t bus, uint16_t addr, uint32_t retries, uint32_t timeout)
{
  I2C_HandleTypeDef* h = i2c_get_handle(bus);
  if (!h) return -1;

  HAL_StatusTypeDef err = HAL_I2C_IsDeviceReady(h, addr << 1, retries, timeout);
  if (err != HAL_OK) return -1;

  return 0;
}

int stm32_i2c_is_dev_ready(uint8_t bus, uint16_t addr, uint32_t timeout)
{
  return stm32_i2c_is_dev_ready(bus, addr, 1, timeout);
}
