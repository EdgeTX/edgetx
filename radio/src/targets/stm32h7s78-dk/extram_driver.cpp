#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "aps256xx.h"
#include "bsp_errno.h"

#define XSPI_RAM_FREQ             200000000U
#define XSPI_RAM_VARIABLE_LATENCY 0U
#define XSPI_RAM_READ_LATENCY     APS256XX_READ_LATENCY_7
#define XSPI_RAM_WRITE_LATENCY    APS256XX_WRITE_LATENCY_7
#define XSPI_RAM_IO_MODE          APS256XX_MR8_X8_X16

#define DEFAULT_READ_LATENCY      APS256XX_READ_LATENCY_5
#define DEFAULT_WRITE_LATENCY     APS256XX_WRITE_LATENCY_5

#define CONF_HSPI_DS   APS256XX_MR0_DS_HALF
#define CONF_HSPI_PASR APS256XX_MR4_PASR_FULL
#define CONF_HSPI_RF   APS256XX_MR4_RF_4X

/* Definition for XSPI RAM power resources */
#define XSPI_RAM_PWR_ENABLE()                 HAL_PWREx_EnableXSPIM1()

/* Definition for XSPI RAM clock resources */
#define XSPI_RAM_CLK_ENABLE()                 __HAL_RCC_XSPI1_CLK_ENABLE()
#define XSPI_RAM_CLK_DISABLE()                __HAL_RCC_XSPI1_CLK_DISABLE()

#define XSPI_RAM_CLK_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOO_CLK_ENABLE()
#define XSPI_RAM_DQS0_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOO_CLK_ENABLE()
#define XSPI_RAM_DQS1_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOO_CLK_ENABLE()
#define XSPI_RAM_CS_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOO_CLK_ENABLE()
#define XSPI_RAM_D0_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D1_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D2_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D3_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D4_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D5_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D6_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D7_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D8_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D9_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D10_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D11_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D12_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D13_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D14_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()
#define XSPI_RAM_D15_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOP_CLK_ENABLE()

#define XSPI_RAM_FORCE_RESET()                __HAL_RCC_XSPI1_FORCE_RESET()
#define XSPI_RAM_RELEASE_RESET()              __HAL_RCC_XSPI1_RELEASE_RESET()

/* Definition for XSPI RAM Pins */
/* XSPI_CLK */
#define XSPI_RAM_CLK_PIN                      GPIO_PIN_4
#define XSPI_RAM_CLK_GPIO_PORT                GPIOO
#define XSPI_RAM_CLK_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_DQS0 */
#define XSPI_RAM_DQS0_PIN                     GPIO_PIN_2
#define XSPI_RAM_DQS0_GPIO_PORT               GPIOO
#define XSPI_RAM_DQS0_PIN_AF                  GPIO_AF9_XSPIM_P1
/* XSPI_DQS1 */
#define XSPI_RAM_DQS1_PIN                     GPIO_PIN_3
#define XSPI_RAM_DQS1_GPIO_PORT               GPIOO
#define XSPI_RAM_DQS1_PIN_AF                  GPIO_AF9_XSPIM_P1
/* XSPI_CS */
#define XSPI_RAM_CS_PIN                       GPIO_PIN_0
#define XSPI_RAM_CS_GPIO_PORT                 GPIOO
#define XSPI_RAM_CS_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D0 */
#define XSPI_RAM_D0_PIN                       GPIO_PIN_0
#define XSPI_RAM_D0_GPIO_PORT                 GPIOP
#define XSPI_RAM_D0_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D1 */
#define XSPI_RAM_D1_PIN                       GPIO_PIN_1
#define XSPI_RAM_D1_GPIO_PORT                 GPIOP
#define XSPI_RAM_D1_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D2 */
#define XSPI_RAM_D2_PIN                       GPIO_PIN_2
#define XSPI_RAM_D2_GPIO_PORT                 GPIOP
#define XSPI_RAM_D2_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D3 */
#define XSPI_RAM_D3_PIN                       GPIO_PIN_3
#define XSPI_RAM_D3_GPIO_PORT                 GPIOP
#define XSPI_RAM_D3_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D4 */
#define XSPI_RAM_D4_PIN                       GPIO_PIN_4
#define XSPI_RAM_D4_GPIO_PORT                 GPIOP
#define XSPI_RAM_D4_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D5 */
#define XSPI_RAM_D5_PIN                       GPIO_PIN_5
#define XSPI_RAM_D5_GPIO_PORT                 GPIOP
#define XSPI_RAM_D5_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D6 */
#define XSPI_RAM_D6_PIN                       GPIO_PIN_6
#define XSPI_RAM_D6_GPIO_PORT                 GPIOP
#define XSPI_RAM_D6_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D7 */
#define XSPI_RAM_D7_PIN                       GPIO_PIN_7
#define XSPI_RAM_D7_GPIO_PORT                 GPIOP
#define XSPI_RAM_D7_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D8 */
#define XSPI_RAM_D8_PIN                       GPIO_PIN_8
#define XSPI_RAM_D8_GPIO_PORT                 GPIOP
#define XSPI_RAM_D8_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D9 */
#define XSPI_RAM_D9_PIN                       GPIO_PIN_9
#define XSPI_RAM_D9_GPIO_PORT                 GPIOP
#define XSPI_RAM_D9_PIN_AF                    GPIO_AF9_XSPIM_P1
/* XSPI_D10 */
#define XSPI_RAM_D10_PIN                      GPIO_PIN_10
#define XSPI_RAM_D10_GPIO_PORT                GPIOP
#define XSPI_RAM_D10_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_D11 */
#define XSPI_RAM_D11_PIN                      GPIO_PIN_11
#define XSPI_RAM_D11_GPIO_PORT                GPIOP
#define XSPI_RAM_D11_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_D12 */
#define XSPI_RAM_D12_PIN                      GPIO_PIN_12
#define XSPI_RAM_D12_GPIO_PORT                GPIOP
#define XSPI_RAM_D12_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_D13 */
#define XSPI_RAM_D13_PIN                      GPIO_PIN_13
#define XSPI_RAM_D13_GPIO_PORT                GPIOP
#define XSPI_RAM_D13_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_D14 */
#define XSPI_RAM_D14_PIN                      GPIO_PIN_14
#define XSPI_RAM_D14_GPIO_PORT                GPIOP
#define XSPI_RAM_D14_PIN_AF                   GPIO_AF9_XSPIM_P1
/* XSPI_D15 */
#define XSPI_RAM_D15_PIN                      GPIO_PIN_15
#define XSPI_RAM_D15_GPIO_PORT                GPIOP
#define XSPI_RAM_D15_PIN_AF                   GPIO_AF9_XSPIM_P1

static XSPI_HandleTypeDef hxspi_ram;


static uint32_t _get_max_xspi_freq(APS256XX_ReadLatencyCode_t ReadLatencyCode,
                                   APS256XX_WriteLatencyCode_t WriteLatencyCode)
{
  uint32_t xspi_max_freq = 0U;

  if ((ReadLatencyCode == APS256XX_READ_LATENCY_3) ||
      (WriteLatencyCode == APS256XX_WRITE_LATENCY_3)) {
    /* In case of latency 3, Fmax of memory is 66 MHz */
    xspi_max_freq = 66000000U;
  } else if ((ReadLatencyCode == APS256XX_READ_LATENCY_4) ||
             (WriteLatencyCode == APS256XX_WRITE_LATENCY_4)) {
    /* In case of latency 4, Fmax of memory is 109 MHz */
    xspi_max_freq = 109000000U;
  } else if ((ReadLatencyCode == APS256XX_READ_LATENCY_5) ||
             (WriteLatencyCode == APS256XX_WRITE_LATENCY_5)) {
    /* In case of latency 5, Fmax of memory is 133 MHz */
    xspi_max_freq = 133000000U;
  } else if ((ReadLatencyCode == APS256XX_READ_LATENCY_6) ||
             (WriteLatencyCode == APS256XX_WRITE_LATENCY_6)) {
    /* In case of latency 6, Fmax of memory is 166 MHz */
    xspi_max_freq = 166000000U;
  } else {
    /* In case of latency 7, Fmax of memory is 200 MHz */
    xspi_max_freq = 200000000U;
  }

  return xspi_max_freq;
}

/**
 * @brief  Initializes the XSPI MSP.
 * @param  hxspi XSPI handle
 * @retval None
 */
static void XSPI_RAM_MspInit()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Enable the SBS Clock */
  __HAL_RCC_SBS_CLK_ENABLE();

  /* Enable the power of XSPI */
  XSPI_RAM_PWR_ENABLE() ;

  /* Enable the XSPI memory interface clock */
  XSPI_RAM_CLK_ENABLE();

  /* Reset the XSPI memory interface */
  XSPI_RAM_FORCE_RESET();
  XSPI_RAM_RELEASE_RESET();

  /* Enable GPIO clocks */
  XSPI_RAM_CLK_GPIO_CLK_ENABLE();
  XSPI_RAM_DQS0_GPIO_CLK_ENABLE();
  XSPI_RAM_DQS1_GPIO_CLK_ENABLE();
  XSPI_RAM_CS_GPIO_CLK_ENABLE();
  XSPI_RAM_D0_GPIO_CLK_ENABLE();
  XSPI_RAM_D1_GPIO_CLK_ENABLE();
  XSPI_RAM_D2_GPIO_CLK_ENABLE();
  XSPI_RAM_D3_GPIO_CLK_ENABLE();
  XSPI_RAM_D4_GPIO_CLK_ENABLE();
  XSPI_RAM_D5_GPIO_CLK_ENABLE();
  XSPI_RAM_D6_GPIO_CLK_ENABLE();
  XSPI_RAM_D7_GPIO_CLK_ENABLE();
  XSPI_RAM_D8_GPIO_CLK_ENABLE();
  XSPI_RAM_D9_GPIO_CLK_ENABLE();
  XSPI_RAM_D10_GPIO_CLK_ENABLE();
  XSPI_RAM_D11_GPIO_CLK_ENABLE();
  XSPI_RAM_D12_GPIO_CLK_ENABLE();
  XSPI_RAM_D13_GPIO_CLK_ENABLE();
  XSPI_RAM_D14_GPIO_CLK_ENABLE();
  XSPI_RAM_D15_GPIO_CLK_ENABLE();

  /* SBS: Enable HSLV on XSPI1 */
  HAL_SBS_EnableIOSpeedOptimize(SBS_IO_XSPI1_HSLV);

  /* XSPI CS GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = XSPI_RAM_CS_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_CS_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI DQS0 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_DQS0_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_DQS0_PIN_AF;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(XSPI_RAM_DQS0_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI DQS1 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_DQS1_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_DQS1_PIN_AF;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(XSPI_RAM_DQS1_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI CLK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_CLK_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = XSPI_RAM_CLK_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D0 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D0_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D0_PIN_AF;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(XSPI_RAM_D0_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D1 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D1_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D1_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D1_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D2 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D2_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D2_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D2_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D3 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D3_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D3_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D3_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D4 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D4_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D4_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D4_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D5 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D5_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D5_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D5_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D6 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D6_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D6_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D6_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D7 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D7_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D7_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D7_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D8 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D8_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D8_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D8_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D9 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D9_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D9_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D9_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D10 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D10_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D10_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D10_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D11 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D11_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D11_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D11_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D12 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D12_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D12_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D12_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D13 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D13_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D13_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D13_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D14 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D14_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D14_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D14_GPIO_PORT, &GPIO_InitStruct);

  /* XSPI D15 GPIO pin configuration  */
  GPIO_InitStruct.Pin       = XSPI_RAM_D15_PIN;
  GPIO_InitStruct.Alternate = XSPI_RAM_D15_PIN_AF;
  HAL_GPIO_Init(XSPI_RAM_D15_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Set 16-bits Octal RAM to desired configuration. And this instance becomes current instance.
  *         If current instance running at MMP mode then this function doesn't work.
  * @retval BSP status
  */
static int32_t XSPI_RAM_Config16BitsOctalRAM()
{
  int32_t ret = BSP_ERROR_NONE;
  uint8_t reg[2] = {0};

  /* Reading the configuration of Mode Register 0 ***********************/
  if (APS256XX_ReadReg(&hxspi_ram, APS256XX_MR0_ADDRESS, reg,
                       (uint32_t)APS256XX_READ_REG_LATENCY(
                           (uint32_t)(DEFAULT_READ_LATENCY))) != APS256XX_OK) {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  } else {
    /* Configure the 16-bits Octal RAM memory ***************************/
    MODIFY_REG(reg[0],
               ((uint8_t)APS256XX_MR0_LATENCY_TYPE |
                (uint8_t)APS256XX_MR0_READ_LATENCY_CODE |
                (uint8_t)APS256XX_MR0_DRIVE_STRENGTH),
               ((uint8_t)(XSPI_RAM_VARIABLE_LATENCY) | (uint8_t)(XSPI_RAM_READ_LATENCY) |
                (uint8_t)CONF_HSPI_DS));

    if (APS256XX_WriteReg(&hxspi_ram, APS256XX_MR0_ADDRESS, reg[0]) !=
        APS256XX_OK) {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  if (ret == BSP_ERROR_NONE) {
    /* Reading the configuration of Mode Register 4 ***********************/
    if (APS256XX_ReadReg(&hxspi_ram, APS256XX_MR4_ADDRESS, reg,
                         (uint32_t)APS256XX_READ_REG_LATENCY((
                             uint32_t)(DEFAULT_READ_LATENCY))) != APS256XX_OK) {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    } else {
      /* Configure the 16-bits Octal RAM memory ***************************/
      WRITE_REG(reg[0], ((uint8_t)XSPI_RAM_WRITE_LATENCY |
                         (uint8_t)CONF_HSPI_RF | (uint8_t)CONF_HSPI_PASR));

      if (APS256XX_WriteReg(&hxspi_ram, APS256XX_MR4_ADDRESS, reg[0]) !=
          APS256XX_OK) {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  if (ret == BSP_ERROR_NONE) {
    /* Reading the configuration of Mode Register 8 ***********************/
    if (APS256XX_ReadReg(&hxspi_ram, APS256XX_MR8_ADDRESS, reg,
                         (uint32_t)APS256XX_READ_REG_LATENCY((
                             uint32_t)(DEFAULT_READ_LATENCY))) != APS256XX_OK) {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    } else {
      /* Configure the 16-bits Octal RAM memory ***************************/
      MODIFY_REG(reg[0], (uint8_t)APS256XX_MR8_X8_X16,
                 (uint8_t)(XSPI_RAM_IO_MODE));

      if (APS256XX_WriteReg(&hxspi_ram, APS256XX_MR8_ADDRESS, reg[0]) !=
          APS256XX_OK) {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the XSPI interface.
  * @param  Instance   XSPI Instance
  * @param  Init       XSPI Init structure
  * @retval BSP status
  */
extern "C" int32_t ExtRAM_Init()
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t xspi_clk, xspi_max_freq, xspi_prescaler;

  /* Msp XSPI initialization */
  XSPI_RAM_MspInit();

  /* Compute prescaler */
  xspi_max_freq =
      _get_max_xspi_freq(XSPI_RAM_READ_LATENCY, XSPI_RAM_WRITE_LATENCY);

  xspi_clk = LL_RCC_GetXSPIClockFreq(LL_RCC_XSPI1_CLKSOURCE);
  xspi_prescaler = (xspi_clk / xspi_max_freq);

  if ((xspi_clk % xspi_max_freq) == 0U) {
    xspi_prescaler = xspi_prescaler - 1U;
  }

  /* STM32 XSPI interface initialization */
  uint32_t hspi_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI1);

  /* XSPI initialization */
  hxspi_ram.Instance = XSPI1;

  hxspi_ram.Init.FifoThresholdByte       = 4U;
  hxspi_ram.Init.MemoryType              = HAL_XSPI_MEMTYPE_APMEM_16BITS;
  hxspi_ram.Init.MemoryMode              = HAL_XSPI_SINGLE_MEM;
  hxspi_ram.Init.MemorySize              = HAL_XSPI_SIZE_256MB;
  hxspi_ram.Init.MemorySelect            = HAL_XSPI_CSSEL_NCS1;
  hxspi_ram.Init.ChipSelectHighTimeCycle = 5U; /* tCPH = 24 ns min */
  hxspi_ram.Init.ClockMode               = HAL_XSPI_CLOCK_MODE_0;
  hxspi_ram.Init.ClockPrescaler          = xspi_prescaler;
  hxspi_ram.Init.SampleShifting          = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi_ram.Init.DelayHoldQuarterCycle   = HAL_XSPI_DHQC_DISABLE;
  hxspi_ram.Init.ChipSelectBoundary      = HAL_XSPI_BONDARYOF_16KB;
  hxspi_ram.Init.FreeRunningClock        = HAL_XSPI_FREERUNCLK_DISABLE;
  /* tCEM = 2 us max => REFRESH+4 clock cycles for read */
  hxspi_ram.Init.Refresh =
      ((2U * (hspi_clk / (xspi_prescaler + 1U)) / 1000000U) - 4U);
  hxspi_ram.Init.WrapSize                = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi_ram.Init.MaxTran                 = 0U;

  if (HAL_XSPI_Init(&hxspi_ram) != HAL_OK) {
    ret = BSP_ERROR_PERIPH_FAILURE;
  } else if (APS256XX_Reset(&hxspi_ram) != APS256XX_OK) {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  } else if (XSPI_RAM_Config16BitsOctalRAM() != BSP_ERROR_NONE) {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  } else if (APS256XX_EnableMemoryMappedMode(
                 &hxspi_ram,
                 (uint32_t)APS256XX_READ_LATENCY(
                     (uint32_t)(XSPI_RAM_READ_LATENCY),
                     (uint32_t)(XSPI_RAM_VARIABLE_LATENCY)),
                 (uint32_t)APS256XX_WRITE_LATENCY(
                     (uint32_t)(XSPI_RAM_WRITE_LATENCY)),
                 (uint32_t)(XSPI_RAM_IO_MODE), 0U) != APS256XX_OK) {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  /* Return BSP status */
  return ret;
}

