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

#include <stdint.h>

#include "stm32_hal.h"
#include "lcd.h"

#include "stm32_hal_ll.h"

#define LCD_ERROR_NONE               0
#define LCD_ERROR_WRONG_PARAM       -1
#define LCD_ERROR_PERIPH_FAILURE    -2
#define LCD_ERROR_UNKNOWN_COMPONENT -3
#define LCD_ERROR_COMPONENT_FAILURE -4
#define LCD_ERROR_BUS_FAILURE       -5

#define LCD_PIXEL_FORMAT_ARGB8888        0x00000000U   /*!< ARGB8888 LTDC pixel format */
#define LCD_PIXEL_FORMAT_RGB888          0x00000001U   /*!< RGB888 LTDC pixel format   */
#define LCD_PIXEL_FORMAT_RGB565          0x00000002U   /*!< RGB565 LTDC pixel format   */
#define LCD_PIXEL_FORMAT_ARGB1555        0x00000003U   /*!< ARGB1555 LTDC pixel format */
#define LCD_PIXEL_FORMAT_ARGB4444        0x00000004U   /*!< ARGB4444 LTDC pixel format */
#define LCD_PIXEL_FORMAT_L8              0x00000005U   /*!< L8 LTDC pixel format       */
#define LCD_PIXEL_FORMAT_AL44            0x00000006U   /*!< AL44 LTDC pixel format     */
#define LCD_PIXEL_FORMAT_AL88            0x00000007U   /*!< AL88 LTDC pixel format     */

#define OTM8009A_FORMAT_RGB888    ((uint32_t)0x00) /* Pixel format chosen is RGB888 : 24 bpp */
#define OTM8009A_FORMAT_RBG565    ((uint32_t)0x02) /* Pixel format chosen is RGB565 : 16 bpp */

/**
  * @brief  RK050HR18 Timing
  */
#define  RK050HR18_HSYNC            ((uint16_t)4)   /* Horizontal synchronization */
#define  RK050HR18_HBP              ((uint16_t)8)   /* Horizontal back porch      */
#define  RK050HR18_HFP              ((uint16_t)8)   /* Horizontal front porch     */
#define  RK050HR18_VSYNC            ((uint16_t)4)   /* Vertical synchronization   */
#define  RK050HR18_VBP              ((uint16_t)8)   /* Vertical back porch        */
#define  RK050HR18_VFP              ((uint16_t)8)   /* Vertical front porch       */


/* Landscape orientation choice of LCD screen */
#define LCD_ORIENTATION_LANDSCAPE        0x01U

/* Display resolution */
#define LCD_DEFAULT_WIDTH                800
#define LCD_DEFAULT_HEIGHT               480

/* LCD display enable pin */
#define LCD_DISP_EN_PIN                       GPIO_PIN_15
#define LCD_DISP_EN_GPIO_PORT                 GPIOE
#define LCD_DISP_EN_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_DISP_EN_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOE_CLK_DISABLE()

/* LCD BackLight control pin */
#define LCD_BL_CTRL_PIN                  GPIO_PIN_15
#define LCD_BL_CTRL_GPIO_PORT            GPIOG
#define LCD_BL_CTRL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOG_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOG_CLK_DISABLE()

/* LCD Brightness default value */
#define LCD_BRIGHTNESS_DEFAULT_VALUE   50U

/*LCD LPTimer brightness control parameters */
#define LCD_LPTIMx                     LPTIM1
#define LCD_LPTIMx_CLK_ENABLE()        __HAL_RCC_LPTIM1_CLK_ENABLE()
#define LCD_LPTIMx_CLK_DISABLE()       __HAL_RCC_LPTIM1_CLK_DISABLE()
#define LCD_LPTIMx_CHANNEL             TIM_CHANNEL_2
#define LCD_LPTIMx_CHANNEL_AF          GPIO_AF1_LPTIM1
#define LCD_LPTIMX_PERIOD_VALUE        ((uint32_t)10000) /* Period Value    */
#define LCD_LPTIMX_PULSE_VALUE         (((uint32_t)(LCD_LPTIMX_PERIOD_VALUE +1U)*LCD_BRIGHTNESS_DEFAULT_VALUE/100U)-1U)
#define LCD_LPTIMX_PRESCALER_VALUE     LPTIM_PRESCALER_DIV1  /* Prescaler divider Value */
#define LCD_LPTIMX_CLOCK_SOURCE        LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC /* internal clock source */


typedef struct
{
  uint32_t XSize;
  uint32_t YSize;
  uint32_t ActiveLayer;
  uint32_t PixelFormat;
  uint32_t BppFactor;
  uint32_t IsMspCallbacksValid;
  uint32_t ReloadEnable;
} BSP_LCD_Ctx_t;

typedef struct
{
  uint32_t X0;
  uint32_t X1;
  uint32_t Y0;
  uint32_t Y1;
  uint32_t PixelFormat;
  uint32_t Address;
} LTDC_LayerConfig_t;

static void* _initial_frame_buffer = nullptr;

static LTDC_HandleTypeDef  hlcd_ltdc;


/**
  * @brief  Configure LCD control pins (Back-light, Display Enable and TE)
  * @retval None
  */
static void LCD_InitSequence(void)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* LCD_BL_CTRL GPIO configuration */
  LCD_BL_CTRL_GPIO_CLK_ENABLE();

  gpio_init_structure.Pin       = LCD_BL_CTRL_PIN;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_PORT, &gpio_init_structure);

  /* Assert back-light LCD_BL_CTRL pin */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_PORT, LCD_BL_CTRL_PIN, GPIO_PIN_SET);

  /** @brief NVIC configuration for LTDC interrupt that is now enabled */
  NVIC_SetPriority(LTDC_IRQn, 4); // LTDC_IRQ_PRIO
  NVIC_EnableIRQ(LTDC_IRQn);
}

static void LTDC_MspInit()
{
  /** Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitTypeDef  gpio_init_structure;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  /* Data */
  gpio_init_structure.Pin = GPIO_PIN_13 |GPIO_PIN_15;
  gpio_init_structure.Alternate = GPIO_AF10_LTDC;
  HAL_GPIO_Init(GPIOB, &gpio_init_structure);

  /* Data */
  gpio_init_structure.Pin =  GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_10| GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14;
  gpio_init_structure.Alternate = GPIO_AF13_LTDC;
  HAL_GPIO_Init(GPIOB, &gpio_init_structure);


  gpio_init_structure.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_11 |GPIO_PIN_12| GPIO_PIN_15;
  gpio_init_structure.Alternate = GPIO_AF13_LTDC;
  HAL_GPIO_Init(GPIOA, &gpio_init_structure);


  gpio_init_structure.Pin = GPIO_PIN_6;
  gpio_init_structure.Alternate = GPIO_AF12_LTDC;
  HAL_GPIO_Init(GPIOA, &gpio_init_structure);

  gpio_init_structure.Pin = GPIO_PIN_9|GPIO_PIN_10;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &gpio_init_structure);


  gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1| GPIO_PIN_2 | GPIO_PIN_13|GPIO_PIN_14;
  gpio_init_structure.Alternate = GPIO_AF13_LTDC;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /* VSYNC */
  gpio_init_structure.Pin = GPIO_PIN_11;
  gpio_init_structure.Alternate = GPIO_AF11_LTDC;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  gpio_init_structure.Pin       = GPIO_PIN_0 ;
  gpio_init_structure.Alternate = GPIO_AF11_LTDC;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  gpio_init_structure.Pin   = GPIO_PIN_10|GPIO_PIN_11;
  gpio_init_structure.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOF , &gpio_init_structure);

  gpio_init_structure.Pin   = GPIO_PIN_9 | GPIO_PIN_7 | GPIO_PIN_15;
  gpio_init_structure.Alternate = GPIO_AF13_LTDC;
  HAL_GPIO_Init(GPIOF , &gpio_init_structure);

  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init_structure.Pin       = LCD_DISP_EN_PIN;
  HAL_GPIO_Init(LCD_DISP_EN_GPIO_PORT, &gpio_init_structure);

  /** Toggle Sw reset of LTDC IP */
  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_RELEASE_RESET();
}

/**
  * @brief  LTDC Clock Config for LCD TFT display.
  * @param  hltdc  LTDC Handle
  *         Being __weak it can be overwritten by the application
  * @retval HAL_status
  */
static HAL_StatusTypeDef LTDC_ClockConfig(LTDC_HandleTypeDef *hltdc)
{
  LL_RCC_PLL3R_Enable();
  LL_RCC_PLL3FRACN_Disable();
  LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
  LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL3_SetM(2);
  LL_RCC_PLL3_SetN(50);
  LL_RCC_PLL3_SetP(2);
  LL_RCC_PLL3_SetQ(2);
  LL_RCC_PLL3_SetR(24);
  LL_RCC_PLL3_Enable();
  while (LL_RCC_PLL3_IsReady() != 1) {
  }
  
  return HAL_OK;
}

/**
  * @brief  Initializes the LTDC.
  * @param  hltdc  LTDC handle
  * @param  Width  LTDC width
  * @param  Height LTDC height
  * @retval HAL status
  */
static HAL_StatusTypeDef LTDC_Init(LTDC_HandleTypeDef *hltdc, uint32_t Width, uint32_t Height)
{
  hltdc->Instance = LTDC;
  hltdc->Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc->Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc->Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc->Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  hltdc->Init.HorizontalSync     = RK050HR18_HSYNC - 1;
  hltdc->Init.AccumulatedHBP     = RK050HR18_HSYNC + RK050HR18_HBP - 1;
  hltdc->Init.AccumulatedActiveW = RK050HR18_HSYNC + Width + RK050HR18_HBP - 1;
  hltdc->Init.TotalWidth         = RK050HR18_HSYNC + Width + RK050HR18_HBP + RK050HR18_HFP - 1;
  hltdc->Init.VerticalSync       = RK050HR18_VSYNC - 1;
  hltdc->Init.AccumulatedVBP     = RK050HR18_VSYNC + RK050HR18_VBP - 1;
  hltdc->Init.AccumulatedActiveH = RK050HR18_VSYNC + Height + RK050HR18_VBP - 1;
  hltdc->Init.TotalHeigh         = RK050HR18_VSYNC + Height + RK050HR18_VBP + RK050HR18_VFP - 1;

  hltdc->Init.Backcolor.Blue  = 0x00;
  hltdc->Init.Backcolor.Green = 0x00;
  hltdc->Init.Backcolor.Red   = 0x00;

  return HAL_LTDC_Init(hltdc);
}

/**
  * @brief  MX LTDC layer configuration.
  * @param  hltdc      LTDC handle
  * @param  LayerIndex Layer 0 or 1
  * @param  Config     Layer configuration
  * @retval HAL status
  */
static HAL_StatusTypeDef LTDC_ConfigLayer(LTDC_HandleTypeDef *hltdc, uint32_t LayerIndex,
                                          LTDC_LayerConfig_t *Config)
{
  LTDC_LayerCfgTypeDef pLayerCfg;

  pLayerCfg.WindowX0 = Config->X0;
  pLayerCfg.WindowX1 = Config->X1;
  pLayerCfg.WindowY0 = Config->Y0;
  pLayerCfg.WindowY1 = Config->Y1;
  pLayerCfg.PixelFormat = Config->PixelFormat;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = Config->Address;
  pLayerCfg.ImageWidth = (Config->X1 - Config->X0);
  pLayerCfg.ImageHeight = (Config->Y1 - Config->Y0);
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  return HAL_LTDC_ConfigLayer(hltdc, &pLayerCfg, LayerIndex);
}

/**
  * @brief  Initializes the LCD.
  * @param  Instance    LCD Instance
  * @param  Orientation LCD_ORIENTATION_LANDSCAPE
  * @param  PixelFormat LCD_PIXEL_FORMAT_RBG565 or LCD_PIXEL_FORMAT_RBG888
  * @param  Width       Display width
  * @param  Height      Display height
  * @retval BSP status
  */
static int32_t LCD_InitEx(uint32_t Orientation, uint32_t PixelFormat,
                          uint32_t Width, uint32_t Height)
{
  int32_t ret = LCD_ERROR_NONE;
  uint32_t ltdc_pixel_format;
  LTDC_LayerConfig_t config;

  if (Orientation > LCD_ORIENTATION_LANDSCAPE) {
    ret = LCD_ERROR_WRONG_PARAM;
  } else {
    switch (PixelFormat) {
      case LCD_PIXEL_FORMAT_RGB565: /*LCD_PIXEL_FORMAT_RBG565*/
        ltdc_pixel_format = LTDC_PIXEL_FORMAT_RGB565;
        break;
      case LCD_PIXEL_FORMAT_RGB888 /*LCD_PIXEL_FORMAT_RBG888*/:
      default:
        ltdc_pixel_format = LTDC_PIXEL_FORMAT_ARGB8888;
        break;
    }

    /* Initialize LCD special pins GPIOs */
    LCD_InitSequence();

    /* Initializes peripherals instance value */
    hlcd_ltdc.Instance = LTDC;

    /* MSP initialization */
    LTDC_MspInit();

    /* Assert display enable LCD_DISP_CTRL pin */
    HAL_GPIO_WritePin(LCD_DISP_EN_GPIO_PORT, LCD_DISP_EN_PIN, GPIO_PIN_SET);

    if (LTDC_ClockConfig(&hlcd_ltdc) != HAL_OK) {
      ret = LCD_ERROR_PERIPH_FAILURE;
    } else if (LTDC_Init(&hlcd_ltdc, Width, Height) != HAL_OK) {
      ret = LCD_ERROR_PERIPH_FAILURE;
    }

    if (ret == LCD_ERROR_NONE) {
      /* Configure default LTDC Layer 0. This configuration can be override by
      calling LCD_ConfigLayer() at application level */
      config.X0 = 0;
      config.X1 = Width;
      config.Y0 = 0;
      config.Y1 = Height;
      config.PixelFormat = ltdc_pixel_format;
      config.Address = (uintptr_t)_initial_frame_buffer;

      if (LTDC_ConfigLayer(&hlcd_ltdc, 0, &config) != HAL_OK) {
        ret = LCD_ERROR_PERIPH_FAILURE;
      }
    }
  }

  return ret;
}

/* LPTimer handler declaration */
static LPTIM_HandleTypeDef hlcd_lptim;
static LPTIM_OC_ConfigTypeDef  hlcd_lptim_config = {0};

/**
  * @brief  Initializes TIM MSP.
  * @param  hlptim LPTIM handle
  * @retval None
  */
static void LPTIMx_PWM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hlptim);

  GPIO_InitTypeDef gpio_init_structure;

  LCD_BL_CTRL_GPIO_CLK_ENABLE();

  /* LPTIMx Peripheral clock enable */
  LCD_LPTIMx_CLK_ENABLE();

  /* Timer channel configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init_structure.Alternate = LCD_LPTIMx_CHANNEL_AF;
  gpio_init_structure.Pin       = LCD_BL_CTRL_PIN; /* BL_CTRL */

  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_PORT, &gpio_init_structure);
}

/**
  * @brief  Initializes LPTIM in PWM mode
  * @param  hlptim LPTIM handle
  * @retval None
  */
static void LPTIMx_PWM_Init(LPTIM_HandleTypeDef *hlptim)
{
  LPTIMx_PWM_MspInit(hlptim);

  hlptim->Instance = LCD_LPTIMx;
  (void)HAL_LPTIM_DeInit(hlptim);

  hlptim->Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim->Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim->Init.Clock.Source = LCD_LPTIMX_CLOCK_SOURCE;
  hlptim->Init.Clock.Prescaler = LCD_LPTIMX_PRESCALER_VALUE;
  hlptim->Init.UltraLowPowerClock.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;
  hlptim->Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim->Init.Trigger.ActiveEdge = LPTIM_ACTIVEEDGE_RISING;
  hlptim->Init.Trigger.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;
  hlptim->Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim->Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  hlptim->Init.Period = LCD_LPTIMX_PERIOD_VALUE;
  hlptim->Init.RepetitionCounter = 0;
  hlptim->Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING;

  (void)HAL_LPTIM_Init(hlptim);

  hlcd_lptim_config.Pulse      = LCD_LPTIMX_PULSE_VALUE;
  hlcd_lptim_config.OCPolarity = LPTIM_OCPOLARITY_LOW;
  (void)HAL_LPTIM_OC_ConfigChannel(hlptim, &hlcd_lptim_config, LPTIM_CHANNEL_2);
  (void)HAL_LPTIM_PWM_Start(hlptim, LPTIM_CHANNEL_2);
}

void LCD_SetBrightness(uint32_t Brightness)
{
  hlcd_lptim_config.Pulse =
      ((uint32_t)(LCD_LPTIMX_PERIOD_VALUE + 1U) * Brightness / 100U) - 1U;
  (void)HAL_LPTIM_OC_ConfigChannel(&hlcd_lptim, &hlcd_lptim_config,
                                   LPTIM_CHANNEL_2);
}

extern "C" void backlightInit()
{
  LPTIMx_PWM_Init(&hlcd_lptim);
}

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  SCB_CleanDCache();

  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)buffer;

  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;

  __HAL_LTDC_ENABLE_IT(&hlcd_ltdc, LTDC_IT_LI);

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while(_frame_addr_reloaded == 0);
}

extern "C" void lcdSetInitalFrameBuffer(void* fbAddress)
{
  _initial_frame_buffer = fbAddress;
}

extern "C" void lcdInit(void)
{
  LCD_InitEx(LCD_ORIENTATION_LANDSCAPE, LTDC_PIXEL_FORMAT_RGB565,
             LCD_DEFAULT_WIDTH, LCD_DEFAULT_HEIGHT);

  __HAL_RCC_DMA2D_CLK_ENABLE();
  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  // clear interrupt flag
  __HAL_LTDC_CLEAR_FLAG(&hlcd_ltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hlcd_ltdc, LTDC_IT_LI);
  _frame_addr_reloaded = 1;
}
