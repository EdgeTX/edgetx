/**
  ******************************************************************************
  * @file    system_stm32h5xx.c
  * @author  MCD Application Team
  * @brief   CMSIS Cortex-M33 Device Peripheral Access Layer System Source File
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  *   This file provides two functions and one global variable to be called from
  *   user application:
  *      - SystemInit(): This function is called at startup just after reset and
  *                      before branch to main program. This call is made inside
  *                      the "startup_stm32h5xx.s" file.
  *
  *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
  *                                  by the user application to setup the SysTick
  *                                  timer or configure other parameters.
  *
  *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
  *                                 be called whenever the core clock is changed
  *                                 during program execution.
  *
  *   After each device reset the HSI (64 MHz) is used as system clock source.
  *   Then SystemInit() function is called, in "startup_stm32h5xx.s" file, to
  *   configure the system clock before to branch to main program.
  *
  *   This file configures the system clock as follows:
  *=============================================================================
  *-----------------------------------------------------------------------------
  *        System Clock source                     | HSI
  *-----------------------------------------------------------------------------
  *        SYSCLK(Hz)                              | 64000000
  *-----------------------------------------------------------------------------
  *        HCLK(Hz)                                | 64000000
  *-----------------------------------------------------------------------------
  *        AHB Prescaler                           | 1
  *-----------------------------------------------------------------------------
  *        APB1 Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB2 Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB3 Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        HSI Division factor                     | 1
  *-----------------------------------------------------------------------------
  *        PLL1_SRC                                | No clock
  *-----------------------------------------------------------------------------
  *        PLL1_M                                  | Prescaler disabled
  *-----------------------------------------------------------------------------
  *        PLL1_N                                  | 129
  *-----------------------------------------------------------------------------
  *        PLL1_P                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL1_Q                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL1_R                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL1_FRACN                              | 0
  *-----------------------------------------------------------------------------
  *        PLL2_SRC                                | No clock
  *-----------------------------------------------------------------------------
  *        PLL2_M                                  | Prescaler disabled
  *-----------------------------------------------------------------------------
  *        PLL2_N                                  | 129
  *-----------------------------------------------------------------------------
  *        PLL2_P                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL2_Q                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL2_R                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL2_FRACN                              | 0
  *-----------------------------------------------------------------------------
  *        PLL3_SRC                                | No clock
  *-----------------------------------------------------------------------------
  *        PLL3_M                                  | Prescaler disabled
  *-----------------------------------------------------------------------------
  *        PLL3_N                                  | 129
  *-----------------------------------------------------------------------------
  *        PLL3_P                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL3_Q                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL3_R                                  | 2
  *-----------------------------------------------------------------------------
  *        PLL3_FRACN                              | 0
  *-----------------------------------------------------------------------------
  *=============================================================================
  */

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup STM32H5xx_system
  * @{
  */

/** @addtogroup STM32H5xx_System_Private_Includes
  * @{
  */

#include "stm32h5xx.h"

/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_Defines
  * @{
  */

#if !defined  (HSE_VALUE)
  #define HSE_VALUE    (25000000UL) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (CSI_VALUE)
  #define CSI_VALUE    (4000000UL)  /*!< Value of the Internal oscillator in Hz*/
#endif /* CSI_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    (64000000UL) /*!< Value of the Internal oscillator in Hz */
#endif /* HSI_VALUE */

/************************* Miscellaneous Configuration ************************/
/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00U /*!< Vector Table base offset field.
                                   This value must be a multiple of 0x200. */
/******************************************************************************/

/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_Variables
  * @{
  */
  /* The SystemCoreClock variable is updated in three ways:
      1) by calling CMSIS function SystemCoreClockUpdate()
      2) by calling HAL API function HAL_RCC_GetHCLKFreq()
      3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
         Note: If you use this function to configure the system clock; then there
               is no need to call the 2 first functions listed above, since SystemCoreClock
               variable is updated automatically.
  */
  uint32_t SystemCoreClock = 64000000U;

  const uint8_t  AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
  const uint8_t  APBPrescTable[8] =  {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};
/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @addtogroup STM32H5xx_System_Private_Functions
  * @{
  */

/**
  * @brief  Setup the microcontroller system.
  * @param  None
  * @retval None
  */

void SystemInit(void)
{
  uint32_t reg_opsr;

  /* FPU settings ------------------------------------------------------------*/
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
   SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
  #endif

  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
  RCC->CR = RCC_CR_HSION;

  /* Reset CFGR register */
  RCC->CFGR1 = 0U;
  RCC->CFGR2 = 0U;

  /* Reset HSEON, HSECSSON, HSEBYP, HSEEXT, HSIDIV, HSIKERON, CSION, CSIKERON, HSI48 and PLLxON bits */
#if defined(RCC_CR_PLL3ON)
  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSECSSON | RCC_CR_HSEBYP | RCC_CR_HSEEXT | RCC_CR_HSIDIV | RCC_CR_HSIKERON | \
               RCC_CR_CSION | RCC_CR_CSIKERON |RCC_CR_HSI48ON | RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
#else
  RCC->CR &= ~(RCC_CR_HSEON | RCC_CR_HSECSSON | RCC_CR_HSEBYP | RCC_CR_HSEEXT | RCC_CR_HSIDIV | RCC_CR_HSIKERON | \
               RCC_CR_CSION | RCC_CR_CSIKERON |RCC_CR_HSI48ON | RCC_CR_PLL1ON | RCC_CR_PLL2ON);
#endif

  /* Reset PLLxCFGR register */
  RCC->PLL1CFGR = 0U;
  RCC->PLL2CFGR = 0U;
#if defined(RCC_CR_PLL3ON)
  RCC->PLL3CFGR = 0U;
#endif /* RCC_CR_PLL3ON */

  /* Reset PLL1DIVR register */
  RCC->PLL1DIVR = 0x01010280U;
  /* Reset PLL1FRACR register */
  RCC->PLL1FRACR = 0x00000000U;
  /* Reset PLL2DIVR register */
  RCC->PLL2DIVR = 0x01010280U;
  /* Reset PLL2FRACR register */
  RCC->PLL2FRACR = 0x00000000U;
#if defined(RCC_CR_PLL3ON)
  /* Reset PLL3DIVR register */
  RCC->PLL3DIVR = 0x01010280U;
  /* Reset PLL3FRACR register */
  RCC->PLL3FRACR = 0x00000000U;
#endif /* RCC_CR_PLL3ON */

  /* Reset HSEBYP bit */
  RCC->CR &= ~(RCC_CR_HSEBYP);

  /* Disable all interrupts */
  RCC->CIER = 0U;

  /* Configure the Vector Table location add offset address ------------------*/
  #ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM1_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
  #else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
  #endif /* VECT_TAB_SRAM */

  /* Check OPSR register to verify if there is an ongoing swap or option bytes update interrupted by a reset */
  reg_opsr = FLASH->OPSR & FLASH_OPSR_CODE_OP;
  if ((reg_opsr == FLASH_OPSR_CODE_OP) || (reg_opsr == (FLASH_OPSR_CODE_OP_2 | FLASH_OPSR_CODE_OP_1)))
  {
    /* Check FLASH Option Control Register access */
    if ((FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) != 0U)
    {
      /* Authorizes the Option Byte registers programming */
      FLASH->OPTKEYR = 0x08192A3BU;
      FLASH->OPTKEYR = 0x4C5D6E7FU;
    }
    /* Launch the option bytes change operation */
    FLASH->OPTCR |= FLASH_OPTCR_OPTSTART;

    /* Lock the FLASH Option Control Register access */
    FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK;
  }
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is CSI, SystemCoreClock will contain the CSI_VALUE(*)
  *
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(**)
  *
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(***)
  *
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(***)
  *             or HSI_VALUE(**) or CSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  *         (*) CSI_VALUE is a constant defined in stm32h5xx_hal.h file (default value
  *             4 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.
  *
  *         (**) HSI_VALUE is a constant defined in stm32h5xx_hal.h file (default value
  *              64 MHz) but the real value may vary depending on the variations
  *              in voltage and temperature.
  *
  *         (***) HSE_VALUE is a constant defined in stm32h5xx_hal.h file (default value
  *              25 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  *
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate(void)
{
  uint32_t pllp, pllsource, pllm, pllfracen, hsivalue, tmp;
  float_t fracn1, pllvco;

  /* Get SYSCLK source -------------------------------------------------------*/
  switch (RCC->CFGR1 & RCC_CFGR1_SWS)
  {
  case 0x00UL:  /* HSI used as system clock source */
    SystemCoreClock = (uint32_t) (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV)>> 3));
    break;

  case 0x08UL:  /* CSI used as system clock  source */
    SystemCoreClock = CSI_VALUE;
    break;

  case 0x10UL:  /* HSE used as system clock  source */
    SystemCoreClock = HSE_VALUE;
    break;

  case 0x18UL:  /* PLL1 used as system clock source */
    /* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN
    SYSCLK = PLL_VCO / PLLR
    */
    pllsource = (RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1SRC);
    pllm = ((RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1M)>> RCC_PLL1CFGR_PLL1M_Pos);
    pllfracen = ((RCC->PLL1CFGR & RCC_PLL1CFGR_PLL1FRACEN)>>RCC_PLL1CFGR_PLL1FRACEN_Pos);
    fracn1 = (float_t)(uint32_t)(pllfracen* ((RCC->PLL1FRACR & RCC_PLL1FRACR_PLL1FRACN)>> RCC_PLL1FRACR_PLL1FRACN_Pos));

    switch (pllsource)
    {
    case 0x01UL:  /* HSI used as PLL clock source */
      hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV)>> 3)) ;
      pllvco = ((float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + \
                (fracn1/(float_t)0x2000) +(float_t)1 );
      break;

    case 0x02UL:  /* CSI used as PLL clock source */
      pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + \
                (fracn1/(float_t)0x2000) +(float_t)1 );
      break;

    case 0x03UL:  /* HSE used as PLL clock source */
      pllvco = ((float_t)HSE_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1N) + \
                (fracn1/(float_t)0x2000) +(float_t)1 );
      break;

    default:  /* No clock sent to PLL*/
      pllvco = (float_t) 0U;
      break;
    }

    pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_PLL1P) >>RCC_PLL1DIVR_PLL1P_Pos) + 1U ) ;
    SystemCoreClock =  (uint32_t)(float_t)(pllvco/(float_t)pllp);

    break;

  default:
    SystemCoreClock = HSI_VALUE;
    break;
  }
  /* Compute HCLK clock frequency --------------------------------------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR2 & RCC_CFGR2_HPRE) >> RCC_CFGR2_HPRE_Pos)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;
}


void HardFault_Handler() { asm("BKPT"); }
void MemManage_Handler() {  asm("BKPT"); }
void BusFault_Handler() {  asm("BKPT"); }
void UsageFault_Handler() {  asm("BKPT"); }
void SecureFault_Handler() {  asm("BKPT"); }
void SVC_Handler() {  asm("BKPT"); }
void DebugMon_Handler() {  asm("BKPT"); }
void PendSV_Handler() {  asm("BKPT"); }
void SysTick_Handler() {  asm("BKPT"); }
void WWDG_IRQHandler() {  asm("BKPT"); }
void PVD_AVD_IRQHandler() {  asm("BKPT"); }
void RTC_IRQHandler() {  asm("BKPT"); }
void RTC_S_IRQHandler() {  asm("BKPT"); }
void TAMP_IRQHandler() {  asm("BKPT"); }
void RAMCFG_IRQHandler() {  asm("BKPT"); }
void FLASH_IRQHandler() {  asm("BKPT"); }
void FLASH_S_IRQHandler() {  asm("BKPT"); }
void GTZC_IRQHandler() {  asm("BKPT"); }
void RCC_IRQHandler() {  asm("BKPT"); }
void RCC_S_IRQHandler() {  asm("BKPT"); }
void EXTI0_IRQHandler() {  asm("BKPT"); }
void EXTI1_IRQHandler() {  asm("BKPT"); }
// void EXTI2_IRQHandler() {  asm("BKPT"); }
// void EXTI3_IRQHandler() {  asm("BKPT"); }
void EXTI4_IRQHandler() {  asm("BKPT"); }
void EXTI5_IRQHandler() {  asm("BKPT"); }
void EXTI6_IRQHandler() {  asm("BKPT"); }
void EXTI7_IRQHandler() {  asm("BKPT"); }
void EXTI8_IRQHandler() {  asm("BKPT"); }
void EXTI9_IRQHandler() {  asm("BKPT"); }
void EXTI10_IRQHandler() {  asm("BKPT"); }
void EXTI11_IRQHandler() {  asm("BKPT"); }
void EXTI12_IRQHandler() {  asm("BKPT"); }
void EXTI13_IRQHandler() {  asm("BKPT"); }
void EXTI14_IRQHandler() {  asm("BKPT"); }
void EXTI15_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel0_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel1_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel2_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel3_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel4_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel5_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel6_IRQHandler() {  asm("BKPT"); }
void GPDMA1_Channel7_IRQHandler() {  asm("BKPT"); }
void IWDG_IRQHandler() {  asm("BKPT"); }
void ADC1_IRQHandler() {  asm("BKPT"); }
void DAC1_IRQHandler() {  asm("BKPT"); }
void FDCAN1_IT0_IRQHandler() {  asm("BKPT"); }
void FDCAN1_IT1_IRQHandler() {  asm("BKPT"); }
void TIM1_BRK_IRQHandler() {  asm("BKPT"); }
void TIM1_UP_IRQHandler() {  asm("BKPT"); }
void TIM1_TRG_COM_IRQHandler() {  asm("BKPT"); }
void TIM1_CC_IRQHandler() {  asm("BKPT"); }
void TIM2_IRQHandler() {  asm("BKPT"); }
void TIM3_IRQHandler() {  asm("BKPT"); }
void TIM4_IRQHandler() {  asm("BKPT"); }
void TIM5_IRQHandler() {  asm("BKPT"); }
void TIM6_IRQHandler() {  asm("BKPT"); }
void TIM7_IRQHandler() {  asm("BKPT"); }
void I2C1_EV_IRQHandler() {  asm("BKPT"); }
void I2C1_ER_IRQHandler() {  asm("BKPT"); }
void I2C2_EV_IRQHandler() {  asm("BKPT"); }
void I2C2_ER_IRQHandler() {  asm("BKPT"); }
void SPI1_IRQHandler() {  asm("BKPT"); }
void SPI2_IRQHandler() {  asm("BKPT"); }
void SPI3_IRQHandler() {  asm("BKPT"); }
// void USART1_IRQHandler() {  asm("BKPT"); }
// void USART2_IRQHandler() {  asm("BKPT"); }
// void USART3_IRQHandler() {  asm("BKPT"); }
// void UART4_IRQHandler() {  asm("BKPT"); }
void UART5_IRQHandler() {  asm("BKPT"); }
void LPUART1_IRQHandler() {  asm("BKPT"); }
void LPTIM1_IRQHandler() {  asm("BKPT"); }
void TIM8_BRK_IRQHandler() {  asm("BKPT"); }
void TIM8_UP_IRQHandler() {  asm("BKPT"); }
void TIM8_TRG_COM_IRQHandler() {  asm("BKPT"); }
void TIM8_CC_IRQHandler() {  asm("BKPT"); }
void ADC2_IRQHandler() {  asm("BKPT"); }
void LPTIM2_IRQHandler() {  asm("BKPT"); }
void TIM15_IRQHandler() {  asm("BKPT"); }
void TIM16_IRQHandler() {  asm("BKPT"); }
void TIM17_IRQHandler() {  asm("BKPT"); }
void USB_DRD_FS_IRQHandler() {  asm("BKPT"); }
void CRS_IRQHandler() {  asm("BKPT"); }
void UCPD1_IRQHandler() {  asm("BKPT"); }
void FMC_IRQHandler() {  asm("BKPT"); }
void OCTOSPI1_IRQHandler() {  asm("BKPT"); }
// void SDMMC1_IRQHandler() {  asm("BKPT"); }
void I2C3_EV_IRQHandler() {  asm("BKPT"); }
void I2C3_ER_IRQHandler() {  asm("BKPT"); }
void SPI4_IRQHandler() {  asm("BKPT"); }
void SPI5_IRQHandler() {  asm("BKPT"); }
void SPI6_IRQHandler() {  asm("BKPT"); }
// void USART6_IRQHandler() {  asm("BKPT"); }
void USART10_IRQHandler() {  asm("BKPT"); }
void USART11_IRQHandler() {  asm("BKPT"); }
void SAI1_IRQHandler() {  asm("BKPT"); }
void SAI2_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel0_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel1_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel2_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel3_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel4_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel5_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel6_IRQHandler() {  asm("BKPT"); }
void GPDMA2_Channel7_IRQHandler() {  asm("BKPT"); }
// void UART7_IRQHandler() {  asm("BKPT"); }
// void UART8_IRQHandler() {  asm("BKPT"); }
void UART9_IRQHandler() {  asm("BKPT"); }
void UART12_IRQHandler() {  asm("BKPT"); }
void FPU_IRQHandler() {  asm("BKPT"); }
void ICACHE_IRQHandler() {  asm("BKPT"); }
void DCACHE1_IRQHandler() {  asm("BKPT"); }
void DCMI_PSSI_IRQHandler() {  asm("BKPT"); }
void CORDIC_IRQHandler() {  asm("BKPT"); }
void FMAC_IRQHandler() {  asm("BKPT"); }
void DTS_IRQHandler() {  asm("BKPT"); }
void RNG_IRQHandler() {  asm("BKPT"); }
void HASH_IRQHandler() {  asm("BKPT"); }
void CEC_IRQHandler() {  asm("BKPT"); }
void TIM12_IRQHandler() {  asm("BKPT"); }
void TIM13_IRQHandler() {  asm("BKPT"); }
// void TIM14_IRQHandler() {  asm("BKPT"); }
void I3C1_EV_IRQHandler() {  asm("BKPT"); }
void I3C1_ER_IRQHandler() {  asm("BKPT"); }
void I2C4_EV_IRQHandler() {  asm("BKPT"); }
void I2C4_ER_IRQHandler() {  asm("BKPT"); }
void LPTIM3_IRQHandler() {  asm("BKPT"); }
void LPTIM4_IRQHandler() {  asm("BKPT"); }
void LPTIM5_IRQHandler() {  asm("BKPT"); }
void LPTIM6_IRQHandler() {  asm("BKPT"); }

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

