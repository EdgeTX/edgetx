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
#include "otm8009a.h"
#include "lcd_drv.h"
#include "lcd.h"

#include "stm32h7xx_ll_rcc.h"

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
  * @brief  OTM8009A_480X800 Timing parameters for Portrait orientation mode
  */
#define  OTM8009A_480X800_HSYNC             ((uint16_t)2)      /* Horizontal synchronization */
#define  OTM8009A_480X800_HBP               ((uint16_t)34)     /* Horizontal back porch      */
#define  OTM8009A_480X800_HFP               ((uint16_t)34)     /* Horizontal front porch     */
#define  OTM8009A_480X800_VSYNC             ((uint16_t)1)      /* Vertical synchronization   */
#define  OTM8009A_480X800_VBP               ((uint16_t)15)     /* Vertical back porch        */
#define  OTM8009A_480X800_VFP               ((uint16_t)16)     /* Vertical front porch       */

/**
  * @brief  OTM8009A_800X480 Timing parameters for Landscape orientation mode
  *         Same values as for Portrait mode in fact.
  */
#define  OTM8009A_800X480_HSYNC             OTM8009A_480X800_HSYNC  /* Horizontal synchronization */
#define  OTM8009A_800X480_HBP               OTM8009A_480X800_HBP    /* Horizontal back porch      */
#define  OTM8009A_800X480_HFP               OTM8009A_480X800_HFP    /* Horizontal front porch     */
#define  OTM8009A_800X480_VSYNC             OTM8009A_480X800_VSYNC  /* Vertical synchronization   */
#define  OTM8009A_800X480_VBP               OTM8009A_480X800_VBP    /* Vertical back porch        */
#define  OTM8009A_800X480_VFP               OTM8009A_480X800_VFP    /* Vertical front porch       */


/* Landscape orientation choice of LCD screen */
#define LCD_ORIENTATION_LANDSCAPE        0x01U

/* Display resolution */
#define LCD_DEFAULT_WIDTH                800
#define LCD_DEFAULT_HEIGHT               480

/* LCD reset pin */
#define LCD_RESET_PIN                    GPIO_PIN_10
#define LCD_RESET_PULL                   GPIO_NOPULL
#define LCD_RESET_GPIO_PORT              GPIOF
#define LCD_RESET_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()

/* LCD tearing effect pin */
#define LCD_TE_PIN                       GPIO_PIN_2
#define LCD_TE_GPIO_PORT                 GPIOJ
#define LCD_TE_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LCD_TE_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOJ_CLK_DISABLE()

/* Back-light control pin */
#define LCD_BL_CTRL_PIN                  GPIO_PIN_6
#define LCD_BL_CTRL_GPIO_PORT            GPIOA
#define LCD_BL_CTRL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOA_CLK_DISABLE()

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

static void* Lcd_CompObj = nullptr;
static LCD_Drv_t* Lcd_Drv = nullptr;

static DSI_HandleTypeDef   hlcd_dsi;
static DMA2D_HandleTypeDef hlcd_dma2d;
static LTDC_HandleTypeDef  hlcd_ltdc;
static BSP_LCD_Ctx_t Lcd_Ctx;


/**
  * @brief  BSP LCD Reset
  *         Hw reset the LCD DSI activating its XRES signal (active low for some time)
  *         and deactivating it later.
  * @param  Instance LCD Instance
  */
static void LCD_Reset()
{
  GPIO_InitTypeDef  gpio_init_structure;

  LCD_RESET_GPIO_CLK_ENABLE();

  /* Configure the GPIO Reset pin */
  gpio_init_structure.Pin   = LCD_RESET_PIN;
  gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull  = LCD_RESET_PULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_RESET_GPIO_PORT , &gpio_init_structure);

  /* Activate XRES active low */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT , LCD_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(20);/* wait 20 ms */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT , LCD_RESET_PIN, GPIO_PIN_SET);/* Deactivate XRES */
  HAL_Delay(10);/* Wait for 10ms after releasing XRES before sending commands */
}

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

  /* LCD_TE_CTRL GPIO configuration */
  LCD_TE_GPIO_CLK_ENABLE();

  gpio_init_structure.Pin       = LCD_TE_PIN;
  gpio_init_structure.Mode      = GPIO_MODE_INPUT;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_TE_GPIO_PORT, &gpio_init_structure);
  /* Assert back-light LCD_BL_CTRL pin */
  HAL_GPIO_WritePin(LCD_TE_GPIO_PORT, LCD_TE_PIN, GPIO_PIN_SET);

  /** @brief NVIC configuration for LTDC interrupt that is now enabled */
  NVIC_SetPriority(LTDC_IRQn, 4); // LTDC_IRQ_PRIO
  NVIC_EnableIRQ(LTDC_IRQn);

  /** @brief NVIC configuration for DMA2D interrupt that is now enabled */
  // NVIC_SetPriority(DMA2D_IRQn, 4);
  // NVIC_EnableIRQ(DMA2D_IRQn);

  /** @brief NVIC configuration for DSI interrupt that is now enabled */
  // NVIC_SetPriority(DSI_IRQn, 4);
  // NVIC_EnableIRQ(DSI_IRQn);
}

static void LTDC_MspInit()
{
  /** Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /** Toggle Sw reset of LTDC IP */
  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_RELEASE_RESET();
}

static void DSI_MspInit()
{
  /** Enable DSI Host and wrapper clocks */
  __HAL_RCC_DSI_CLK_ENABLE();

  /** Soft Reset the DSI Host and wrapper */
  __HAL_RCC_DSI_FORCE_RESET();
  __HAL_RCC_DSI_RELEASE_RESET();
}

/**
  * @brief  Initializes the DSIHOST.
  * @param  hdsi   DSI handle
  * @param  Width  Horizontal active width
  * @param  Height Vertical active height
  * @param  PixelFormat DSI color coding RGB888 or RGB565
  * @retval HAL status
  */
static HAL_StatusTypeDef DSIHOST_DSI_Init(DSI_HandleTypeDef *hdsi, uint32_t Width, uint32_t Height, uint32_t PixelFormat)
{
  DSI_PLLInitTypeDef PLLInit;
  DSI_VidCfgTypeDef VidCfg;

  hdsi->Instance = DSI;
  hdsi->Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi->Init.TXEscapeCkdiv = 4;
  hdsi->Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  PLLInit.PLLNDIV = 100;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV5;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(hdsi, &PLLInit) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Timing parameters for all Video modes */
  /*
  The lane byte clock is set 62500 Khz
  The pixel clock is set to 27429 Khz
  */
  VidCfg.VirtualChannelID = 0;
  VidCfg.ColorCoding = PixelFormat;
  VidCfg.LooselyPacked = DSI_LOOSELY_PACKED_DISABLE;
  VidCfg.Mode = DSI_VID_MODE_BURST;
  VidCfg.PacketSize = Width;
  VidCfg.NumberOfChunks = 0;
  VidCfg.NullPacketSize = 0xFFFU;
  VidCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  VidCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  VidCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  VidCfg.HorizontalSyncActive = (OTM8009A_480X800_HSYNC * 62500U)/27429U;
  VidCfg.HorizontalBackPorch = (OTM8009A_480X800_HBP * 62500U)/27429U;
  VidCfg.HorizontalLine = ((Width + OTM8009A_480X800_HSYNC + OTM8009A_480X800_HBP + OTM8009A_480X800_HFP) * 62500U)/27429U;
  VidCfg.VerticalSyncActive = OTM8009A_480X800_VSYNC;
  VidCfg.VerticalBackPorch = OTM8009A_480X800_VBP;
  VidCfg.VerticalFrontPorch = OTM8009A_480X800_VFP;
  VidCfg.VerticalActive = Height;
  VidCfg.LPCommandEnable = DSI_LP_COMMAND_ENABLE;
  VidCfg.LPLargestPacketSize = 4;
  VidCfg.LPVACTLargestPacketSize = 4;

  VidCfg.LPHorizontalFrontPorchEnable  = DSI_LP_HFP_ENABLE;
  VidCfg.LPHorizontalBackPorchEnable   = DSI_LP_HBP_ENABLE;
  VidCfg.LPVerticalActiveEnable        = DSI_LP_VACT_ENABLE;
  VidCfg.LPVerticalFrontPorchEnable    = DSI_LP_VFP_ENABLE;
  VidCfg.LPVerticalBackPorchEnable     = DSI_LP_VBP_ENABLE;
  VidCfg.LPVerticalSyncActiveEnable    = DSI_LP_VSYNC_ENABLE;
  VidCfg.FrameBTAAcknowledgeEnable     = DSI_FBTAA_DISABLE;

  if (HAL_DSI_ConfigVideoMode(hdsi, &VidCfg) != HAL_OK) {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  LTDC Clock Config for LCD DSI display.
  * @param  hltdc  LTDC Handle
  *         Being __weak it can be overwritten by the application
  * @retval HAL_status
  */
static HAL_StatusTypeDef LTDC_ClockConfig(LTDC_HandleTypeDef *hltdc)
{
  // LL_RCC_PLL3P_Enable();
  // LL_RCC_PLL3Q_Enable();
  LL_RCC_PLL3R_Enable();
  LL_RCC_PLL3FRACN_Disable();
  LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
  LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL3_SetM(5);
  LL_RCC_PLL3_SetN(132);
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

  hltdc->Init.HorizontalSync     = OTM8009A_480X800_HSYNC - 1;
  hltdc->Init.AccumulatedHBP     = OTM8009A_480X800_HSYNC + OTM8009A_480X800_HBP - 1;
  hltdc->Init.AccumulatedActiveW = OTM8009A_480X800_HSYNC + Width + OTM8009A_480X800_HBP - 1;
  hltdc->Init.TotalWidth         = OTM8009A_480X800_HSYNC + Width + OTM8009A_480X800_HBP + OTM8009A_480X800_HFP - 1;
  hltdc->Init.VerticalSync       = OTM8009A_480X800_VSYNC - 1;
  hltdc->Init.AccumulatedVBP     = OTM8009A_480X800_VSYNC + OTM8009A_480X800_VBP - 1;
  hltdc->Init.AccumulatedActiveH = OTM8009A_480X800_VSYNC + Height + OTM8009A_480X800_VBP - 1;
  hltdc->Init.TotalHeigh         = OTM8009A_480X800_VSYNC + Height + OTM8009A_480X800_VBP + OTM8009A_480X800_VFP - 1;

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
  * @brief  DCS or Generic short/long write command
  * @param  ChannelNbr Virtual channel ID
  * @param  Reg Register to be written
  * @param  pData pointer to a buffer of data to be write
  * @param  Size To precise command to be used (short or long)
  * @retval BSP status
  */
static int32_t DSI_IO_Write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
  int32_t ret = LCD_ERROR_NONE;

  if(Size <= 1U) {
    if(HAL_DSI_ShortWrite(&hlcd_dsi, ChannelNbr, DSI_DCS_SHORT_PKT_WRITE_P1,
                          Reg, (uint32_t)pData[Size]) != HAL_OK) {
      ret = LCD_ERROR_BUS_FAILURE;
    }
  } else {
    if(HAL_DSI_LongWrite(&hlcd_dsi, ChannelNbr, DSI_DCS_LONG_PKT_WRITE,
                         Size, (uint32_t)Reg, pData) != HAL_OK) {
      ret = LCD_ERROR_BUS_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DCS or Generic read command
  * @param  ChannelNbr Virtual channel ID
  * @param  Reg Register to be read
  * @param  pData pointer to a buffer to store the payload of a read back operation.
  * @param  Size  Data size to be read (in byte).
  * @retval BSP status
  */
static int32_t DSI_IO_Read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
  int32_t ret = LCD_ERROR_NONE;

  if(HAL_DSI_Read(&hlcd_dsi, ChannelNbr, pData, Size,
                  DSI_DCS_SHORT_PKT_READ, Reg, pData) != HAL_OK) {
    ret = LCD_ERROR_BUS_FAILURE;
  }

  return ret;
}

/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t OTM8009A_Probe(uint32_t ColorCoding, uint32_t Orientation)
{
  int32_t ret;
  uint32_t id;
  OTM8009A_IO_t              IOCtx;
  static OTM8009A_Object_t   OTM8009AObj;

  /* Configure the audio driver */
  IOCtx.Address     = 0;
  IOCtx.GetTick     = (OTM8009A_GetTick_Func)HAL_GetTick;
  IOCtx.WriteReg    = DSI_IO_Write;
  IOCtx.ReadReg     = DSI_IO_Read;

  if(OTM8009A_RegisterBusIO(&OTM8009AObj, &IOCtx) != OTM8009A_OK) {
    ret = LCD_ERROR_BUS_FAILURE;
  } else {
    Lcd_CompObj = &OTM8009AObj;

    if(OTM8009A_ReadID((OTM8009A_Object_t*)Lcd_CompObj, &id) != OTM8009A_OK) {
      ret = LCD_ERROR_COMPONENT_FAILURE;
    } else {
      Lcd_Drv = (LCD_Drv_t *)(void *) &OTM8009A_LCD_Driver;
      if(Lcd_Drv->Init(Lcd_CompObj, ColorCoding, Orientation) != OTM8009A_OK) {
        ret = LCD_ERROR_COMPONENT_FAILURE;
      } else {
        ret = LCD_ERROR_NONE;
      }
    }
  }

  return ret;
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
static int32_t LCD_InitEx(uint32_t Orientation, uint32_t PixelFormat, uint32_t Width, uint32_t Height)
{
  int32_t ret = LCD_ERROR_NONE;
  uint32_t ctrl_pixel_format, ltdc_pixel_format, dsi_pixel_format;
  LTDC_LayerConfig_t config;

  if(Orientation > LCD_ORIENTATION_LANDSCAPE) {
    ret = LCD_ERROR_WRONG_PARAM;
  } else {
    switch(PixelFormat) {
    case LCD_PIXEL_FORMAT_RGB565:/*LCD_PIXEL_FORMAT_RBG565*/
      ltdc_pixel_format = LTDC_PIXEL_FORMAT_RGB565;
      dsi_pixel_format = DSI_RGB565;
      ctrl_pixel_format = OTM8009A_FORMAT_RBG565;
      Lcd_Ctx.BppFactor=2U;
      break;
    case LCD_PIXEL_FORMAT_RGB888/*LCD_PIXEL_FORMAT_RBG888*/:
    default:
      ltdc_pixel_format = LTDC_PIXEL_FORMAT_ARGB8888;
      dsi_pixel_format = DSI_RGB888;
      ctrl_pixel_format = OTM8009A_FORMAT_RGB888;
      Lcd_Ctx.BppFactor=4U;
      break;
    }

    /* Store pixel format, xsize and ysize information */
    Lcd_Ctx.PixelFormat = PixelFormat;
    Lcd_Ctx.XSize  = Width;
    Lcd_Ctx.YSize  = Height;

    /* Toggle Hardware Reset of the LCD using its XRES signal (active low) */
    LCD_Reset();

    /* Initialize LCD special pins GPIOs */
    LCD_InitSequence();

    /* Initializes peripherals instance value */
    hlcd_ltdc.Instance = LTDC;
    hlcd_dma2d.Instance = DMA2D;
    hlcd_dsi.Instance = DSI;

    /* MSP initialization */
    LTDC_MspInit();
    DSI_MspInit();

    if(DSIHOST_DSI_Init(&hlcd_dsi, Width, Height, dsi_pixel_format) != HAL_OK) {
      ret = LCD_ERROR_PERIPH_FAILURE;
    }
    else if(LTDC_ClockConfig(&hlcd_ltdc) != HAL_OK) {
      ret = LCD_ERROR_PERIPH_FAILURE;
    } else {
     if(LTDC_Init(&hlcd_ltdc, Width, Height) != HAL_OK) {
       ret = LCD_ERROR_PERIPH_FAILURE;
     }
    }

    if(ret == LCD_ERROR_NONE) {
      /* Configure default LTDC Layer 0. This configuration can be override by calling
      LCD_ConfigLayer() at application level */
      config.X0          = 0;
      config.X1          = Width;
      config.Y0          = 0;
      config.Y1          = Height;
      config.PixelFormat = ltdc_pixel_format;
      config.Address     = (uintptr_t)_initial_frame_buffer;
      if(LTDC_ConfigLayer(&hlcd_ltdc, 0, &config) != HAL_OK) {
        ret = LCD_ERROR_PERIPH_FAILURE;
      } else {
        /* Enable the DSI host and wrapper after the LTDC initialization
        To avoid any synchronization issue, the DSI shall be started after enabling the LTDC */
        (void)HAL_DSI_Start(&hlcd_dsi);

        /* Enable the DSI BTW for read operations */
        (void)HAL_DSI_ConfigFlowControl(&hlcd_dsi, DSI_FLOW_CONTROL_BTA);

        // #if (USE_LCD_CTRL_OTM8009A == 1)
        /* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver)
        depending on configuration of DSI */
        if(OTM8009A_Probe(ctrl_pixel_format, Orientation) != LCD_ERROR_NONE) {
          ret = LCD_ERROR_UNKNOWN_COMPONENT;
        } else {
          ret = LCD_ERROR_NONE;
        }
        // #endif
      }
    /* By default the reload is activated and executed immediately */
    Lcd_Ctx.ReloadEnable = 1U;
   }
  }

  return ret;
}

void LCD_SetBrightness(uint32_t Brightness)
{
  Lcd_Drv->SetBrightness(Lcd_CompObj, Brightness);
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

extern "C" void backlightInit()
{
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
