
#include "stm32h7s7xx.h"
#include "cortex_m_isr.h"

void Default_Handler() { default_isr_handler(); }

/* stm32h7s7xx specific interrupt vectors */
WEAK_DEFAULT void Reset_Handler();
WEAK_DEFAULT void NMI_Handler();
WEAK_DEFAULT void HardFault_Handler();
WEAK_DEFAULT void MemManage_Handler();
WEAK_DEFAULT void BusFault_Handler();
WEAK_DEFAULT void UsageFault_Handler();
WEAK_DEFAULT void SVC_Handler();
WEAK_DEFAULT void DebugMon_Handler();
WEAK_DEFAULT void PendSV_Handler();
WEAK_DEFAULT void SysTick_Handler();
WEAK_DEFAULT void PVD_PVM_IRQHandler();
WEAK_DEFAULT void DTS_IRQHandler();
WEAK_DEFAULT void IWDG_IRQHandler();
WEAK_DEFAULT void WWDG_IRQHandler();
WEAK_DEFAULT void RCC_IRQHandler();
WEAK_DEFAULT void FLASH_IRQHandler();
WEAK_DEFAULT void RAMECC_IRQHandler();
WEAK_DEFAULT void FPU_IRQHandler();
WEAK_DEFAULT void TAMP_IRQHandler();
WEAK_DEFAULT void EXTI0_IRQHandler();
WEAK_DEFAULT void EXTI1_IRQHandler();
WEAK_DEFAULT void EXTI2_IRQHandler();
WEAK_DEFAULT void EXTI3_IRQHandler();
WEAK_DEFAULT void EXTI4_IRQHandler();
WEAK_DEFAULT void EXTI5_IRQHandler();
WEAK_DEFAULT void EXTI6_IRQHandler();
WEAK_DEFAULT void EXTI7_IRQHandler();
WEAK_DEFAULT void EXTI8_IRQHandler();
WEAK_DEFAULT void EXTI9_IRQHandler();
WEAK_DEFAULT void EXTI10_IRQHandler();
WEAK_DEFAULT void EXTI11_IRQHandler();
WEAK_DEFAULT void EXTI12_IRQHandler();
WEAK_DEFAULT void EXTI13_IRQHandler();
WEAK_DEFAULT void EXTI14_IRQHandler();
WEAK_DEFAULT void EXTI15_IRQHandler();
WEAK_DEFAULT void RTC_IRQHandler();
WEAK_DEFAULT void SAES_IRQHandler();
WEAK_DEFAULT void CRYP_IRQHandler();
WEAK_DEFAULT void PKA_IRQHandler();
WEAK_DEFAULT void HASH_IRQHandler();
WEAK_DEFAULT void RNG_IRQHandler();
WEAK_DEFAULT void ADC1_2_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel0_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel1_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel2_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel3_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel4_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel5_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel6_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel7_IRQHandler();
WEAK_DEFAULT void TIM1_BRK_IRQHandler();
WEAK_DEFAULT void TIM1_UP_IRQHandler();
WEAK_DEFAULT void TIM1_TRG_COM_IRQHandler();
WEAK_DEFAULT void TIM1_CC_IRQHandler();
WEAK_DEFAULT void TIM2_IRQHandler();
WEAK_DEFAULT void TIM3_IRQHandler();
WEAK_DEFAULT void TIM4_IRQHandler();
WEAK_DEFAULT void TIM5_IRQHandler();
WEAK_DEFAULT void TIM6_IRQHandler();
WEAK_DEFAULT void TIM7_IRQHandler();
WEAK_DEFAULT void TIM9_IRQHandler();
WEAK_DEFAULT void SPI1_IRQHandler();
WEAK_DEFAULT void SPI2_IRQHandler();
WEAK_DEFAULT void SPI3_IRQHandler();
WEAK_DEFAULT void SPI4_IRQHandler();
WEAK_DEFAULT void SPI5_IRQHandler();
WEAK_DEFAULT void SPI6_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel0_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel1_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel2_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel3_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel4_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel5_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel6_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel7_IRQHandler();
WEAK_DEFAULT void SAI1_A_IRQHandler();
WEAK_DEFAULT void SAI1_B_IRQHandler();
WEAK_DEFAULT void SAI2_A_IRQHandler();
WEAK_DEFAULT void SAI2_B_IRQHandler();
WEAK_DEFAULT void I2C1_EV_IRQHandler();
WEAK_DEFAULT void I2C1_ER_IRQHandler();
WEAK_DEFAULT void I2C2_EV_IRQHandler();
WEAK_DEFAULT void I2C2_ER_IRQHandler();
WEAK_DEFAULT void I2C3_EV_IRQHandler();
WEAK_DEFAULT void I2C3_ER_IRQHandler();
WEAK_DEFAULT void USART1_IRQHandler();
WEAK_DEFAULT void USART2_IRQHandler();
WEAK_DEFAULT void USART3_IRQHandler();
WEAK_DEFAULT void UART4_IRQHandler();
WEAK_DEFAULT void UART5_IRQHandler();
WEAK_DEFAULT void UART7_IRQHandler();
WEAK_DEFAULT void UART8_IRQHandler();
WEAK_DEFAULT void I3C1_EV_IRQHandler();
WEAK_DEFAULT void I3C1_ER_IRQHandler();
WEAK_DEFAULT void OTG_HS_IRQHandler();
WEAK_DEFAULT void ETH_IRQHandler();
WEAK_DEFAULT void CORDIC_IRQHandler();
WEAK_DEFAULT void GFXTIM_IRQHandler();
WEAK_DEFAULT void DCMIPP_IRQHandler();
WEAK_DEFAULT void LTDC_IRQHandler();
WEAK_DEFAULT void LTDC_ER_IRQHandler();
WEAK_DEFAULT void DMA2D_IRQHandler();
WEAK_DEFAULT void JPEG_IRQHandler();
WEAK_DEFAULT void GFXMMU_IRQHandler();
WEAK_DEFAULT void I3C1_WKUP_IRQHandler();
WEAK_DEFAULT void MCE1_IRQHandler();
WEAK_DEFAULT void MCE2_IRQHandler();
WEAK_DEFAULT void MCE3_IRQHandler();
WEAK_DEFAULT void XSPI1_IRQHandler();
WEAK_DEFAULT void XSPI2_IRQHandler();
WEAK_DEFAULT void FMC_IRQHandler();
WEAK_DEFAULT void SDMMC1_IRQHandler();
WEAK_DEFAULT void SDMMC2_IRQHandler();
WEAK_DEFAULT void OTG_FS_IRQHandler();
WEAK_DEFAULT void TIM12_IRQHandler();
WEAK_DEFAULT void TIM13_IRQHandler();
WEAK_DEFAULT void TIM14_IRQHandler();
WEAK_DEFAULT void TIM15_IRQHandler();
WEAK_DEFAULT void TIM16_IRQHandler();
WEAK_DEFAULT void TIM17_IRQHandler();
WEAK_DEFAULT void LPTIM1_IRQHandler();
WEAK_DEFAULT void LPTIM2_IRQHandler();
WEAK_DEFAULT void LPTIM3_IRQHandler();
WEAK_DEFAULT void LPTIM4_IRQHandler();
WEAK_DEFAULT void LPTIM5_IRQHandler();
WEAK_DEFAULT void SPDIF_RX_IRQHandler();
WEAK_DEFAULT void MDIOS_IRQHandler();
WEAK_DEFAULT void ADF1_FLT0_IRQHandler();
WEAK_DEFAULT void CRS_IRQHandler();
WEAK_DEFAULT void UCPD1_IRQHandler();
WEAK_DEFAULT void CEC_IRQHandler();
WEAK_DEFAULT void PSSI_IRQHandler();
WEAK_DEFAULT void LPUART1_IRQHandler();
WEAK_DEFAULT void WAKEUP_PIN_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel8_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel9_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel10_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel11_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel12_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel13_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel14_IRQHandler();
WEAK_DEFAULT void GPDMA1_Channel15_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel8_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel9_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel10_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel11_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel12_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel13_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel14_IRQHandler();
WEAK_DEFAULT void HPDMA1_Channel15_IRQHandler();
WEAK_DEFAULT void GPU2D_IRQHandler();
WEAK_DEFAULT void GPU2D_ER_IRQHandler();
WEAK_DEFAULT void ICACHE_IRQHandler();
WEAK_DEFAULT void FDCAN1_IT0_IRQHandler();
WEAK_DEFAULT void FDCAN1_IT1_IRQHandler();
WEAK_DEFAULT void FDCAN2_IT0_IRQHandler();
WEAK_DEFAULT void FDCAN2_IT1_IRQHandler();

/* CPU specific interrupt vector table */
const isr_t _isr_vector[] ISR_VECTOR = {
  [0] = (isr_t)&_estack,
  [1                                  ] = Reset_Handler,
  [2                                  ] = NMI_Handler,
  [3                                  ] = HardFault_Handler,
  [4                                  ] = MemManage_Handler,
  [5                                  ] = BusFault_Handler,
  [6                                  ] = UsageFault_Handler,
  [7                                  ] = 0,
  [8                                  ] = 0,
  [9                                  ] = 0,
  [10                                 ] = 0,
  [11                                 ] = SVC_Handler,
  [12                                 ] = DebugMon_Handler,
  [13                                 ] = 0,
  [14                                 ] = PendSV_Handler,
  [15                                 ] = SysTick_Handler,
  [PVD_PVM_IRQn + 16                  ] = PVD_PVM_IRQHandler,
  [DTS_IRQn + 16                      ] = DTS_IRQHandler,
  [IWDG_IRQn + 16                     ] = IWDG_IRQHandler,
  [WWDG_IRQn + 16                     ] = WWDG_IRQHandler,
  [RCC_IRQn + 16                      ] = RCC_IRQHandler,
  [FLASH_IRQn + 16                    ] = FLASH_IRQHandler,
  [RAMECC_IRQn + 16                   ] = RAMECC_IRQHandler,
  [FPU_IRQn + 16                      ] = FPU_IRQHandler,
  [TAMP_IRQn + 16                     ] = TAMP_IRQHandler,
  [EXTI0_IRQn + 16                    ] = EXTI0_IRQHandler,
  [EXTI1_IRQn + 16                    ] = EXTI1_IRQHandler,
  [EXTI2_IRQn + 16                    ] = EXTI2_IRQHandler,
  [EXTI3_IRQn + 16                    ] = EXTI3_IRQHandler,
  [EXTI4_IRQn + 16                    ] = EXTI4_IRQHandler,
  [EXTI5_IRQn + 16                    ] = EXTI5_IRQHandler,
  [EXTI6_IRQn + 16                    ] = EXTI6_IRQHandler,
  [EXTI7_IRQn + 16                    ] = EXTI7_IRQHandler,
  [EXTI8_IRQn + 16                    ] = EXTI8_IRQHandler,
  [EXTI9_IRQn + 16                    ] = EXTI9_IRQHandler,
  [EXTI10_IRQn + 16                   ] = EXTI10_IRQHandler,
  [EXTI11_IRQn + 16                   ] = EXTI11_IRQHandler,
  [EXTI12_IRQn + 16                   ] = EXTI12_IRQHandler,
  [EXTI13_IRQn + 16                   ] = EXTI13_IRQHandler,
  [EXTI14_IRQn + 16                   ] = EXTI14_IRQHandler,
  [EXTI15_IRQn + 16                   ] = EXTI15_IRQHandler,
  [RTC_IRQn + 16                      ] = RTC_IRQHandler,
  [SAES_IRQn + 16                     ] = SAES_IRQHandler,
  [CRYP_IRQn + 16                     ] = CRYP_IRQHandler,
  [PKA_IRQn + 16                      ] = PKA_IRQHandler,
  [HASH_IRQn + 16                     ] = HASH_IRQHandler,
  [RNG_IRQn + 16                      ] = RNG_IRQHandler,
  [ADC1_2_IRQn + 16                   ] = ADC1_2_IRQHandler,
  [GPDMA1_Channel0_IRQn + 16          ] = GPDMA1_Channel0_IRQHandler,
  [GPDMA1_Channel1_IRQn + 16          ] = GPDMA1_Channel1_IRQHandler,
  [GPDMA1_Channel2_IRQn + 16          ] = GPDMA1_Channel2_IRQHandler,
  [GPDMA1_Channel3_IRQn + 16          ] = GPDMA1_Channel3_IRQHandler,
  [GPDMA1_Channel4_IRQn + 16          ] = GPDMA1_Channel4_IRQHandler,
  [GPDMA1_Channel5_IRQn + 16          ] = GPDMA1_Channel5_IRQHandler,
  [GPDMA1_Channel6_IRQn + 16          ] = GPDMA1_Channel6_IRQHandler,
  [GPDMA1_Channel7_IRQn + 16          ] = GPDMA1_Channel7_IRQHandler,
  [TIM1_BRK_IRQn + 16                 ] = TIM1_BRK_IRQHandler,
  [TIM1_UP_IRQn + 16                  ] = TIM1_UP_IRQHandler,
  [TIM1_TRG_COM_IRQn + 16             ] = TIM1_TRG_COM_IRQHandler,
  [TIM1_CC_IRQn + 16                  ] = TIM1_CC_IRQHandler,
  [TIM2_IRQn + 16                     ] = TIM2_IRQHandler,
  [TIM3_IRQn + 16                     ] = TIM3_IRQHandler,
  [TIM4_IRQn + 16                     ] = TIM4_IRQHandler,
  [TIM5_IRQn + 16                     ] = TIM5_IRQHandler,
  [TIM6_IRQn + 16                     ] = TIM6_IRQHandler,
  [TIM7_IRQn + 16                     ] = TIM7_IRQHandler,
  [TIM9_IRQn + 16                     ] = TIM9_IRQHandler,
  [SPI1_IRQn + 16                     ] = SPI1_IRQHandler,
  [SPI2_IRQn + 16                     ] = SPI2_IRQHandler,
  [SPI3_IRQn + 16                     ] = SPI3_IRQHandler,
  [SPI4_IRQn + 16                     ] = SPI4_IRQHandler,
  [SPI5_IRQn + 16                     ] = SPI5_IRQHandler,
  [SPI6_IRQn + 16                     ] = SPI6_IRQHandler,
  [HPDMA1_Channel0_IRQn + 16          ] = HPDMA1_Channel0_IRQHandler,
  [HPDMA1_Channel1_IRQn + 16          ] = HPDMA1_Channel1_IRQHandler,
  [HPDMA1_Channel2_IRQn + 16          ] = HPDMA1_Channel2_IRQHandler,
  [HPDMA1_Channel3_IRQn + 16          ] = HPDMA1_Channel3_IRQHandler,
  [HPDMA1_Channel4_IRQn + 16          ] = HPDMA1_Channel4_IRQHandler,
  [HPDMA1_Channel5_IRQn + 16          ] = HPDMA1_Channel5_IRQHandler,
  [HPDMA1_Channel6_IRQn + 16          ] = HPDMA1_Channel6_IRQHandler,
  [HPDMA1_Channel7_IRQn + 16          ] = HPDMA1_Channel7_IRQHandler,
  [SAI1_A_IRQn + 16                   ] = SAI1_A_IRQHandler,
  [SAI1_B_IRQn + 16                   ] = SAI1_B_IRQHandler,
  [SAI2_A_IRQn + 16                   ] = SAI2_A_IRQHandler,
  [SAI2_B_IRQn + 16                   ] = SAI2_B_IRQHandler,
  [I2C1_EV_IRQn + 16                  ] = I2C1_EV_IRQHandler,
  [I2C1_ER_IRQn + 16                  ] = I2C1_ER_IRQHandler,
  [I2C2_EV_IRQn + 16                  ] = I2C2_EV_IRQHandler,
  [I2C2_ER_IRQn + 16                  ] = I2C2_ER_IRQHandler,
  [I2C3_EV_IRQn + 16                  ] = I2C3_EV_IRQHandler,
  [I2C3_ER_IRQn + 16                  ] = I2C3_ER_IRQHandler,
  [USART1_IRQn + 16                   ] = USART1_IRQHandler,
  [USART2_IRQn + 16                   ] = USART2_IRQHandler,
  [USART3_IRQn + 16                   ] = USART3_IRQHandler,
  [UART4_IRQn + 16                    ] = UART4_IRQHandler,
  [UART5_IRQn + 16                    ] = UART5_IRQHandler,
  [UART7_IRQn + 16                    ] = UART7_IRQHandler,
  [UART8_IRQn + 16                    ] = UART8_IRQHandler,
  [I3C1_EV_IRQn + 16                  ] = I3C1_EV_IRQHandler,
  [I3C1_ER_IRQn + 16                  ] = I3C1_ER_IRQHandler,
  [OTG_HS_IRQn + 16                   ] = OTG_HS_IRQHandler,
  [ETH_IRQn + 16                      ] = ETH_IRQHandler,
  [CORDIC_IRQn + 16                   ] = CORDIC_IRQHandler,
  [GFXTIM_IRQn + 16                   ] = GFXTIM_IRQHandler,
  [DCMIPP_IRQn + 16                   ] = DCMIPP_IRQHandler,
  [LTDC_IRQn + 16                     ] = LTDC_IRQHandler,
  [LTDC_ER_IRQn + 16                  ] = LTDC_ER_IRQHandler,
  [DMA2D_IRQn + 16                    ] = DMA2D_IRQHandler,
  [JPEG_IRQn + 16                     ] = JPEG_IRQHandler,
  [GFXMMU_IRQn + 16                   ] = GFXMMU_IRQHandler,
  [I3C1_WKUP_IRQn + 16                ] = I3C1_WKUP_IRQHandler,
  [MCE1_IRQn + 16                     ] = MCE1_IRQHandler,
  [MCE2_IRQn + 16                     ] = MCE2_IRQHandler,
  [MCE3_IRQn + 16                     ] = MCE3_IRQHandler,
  [XSPI1_IRQn + 16                    ] = XSPI1_IRQHandler,
  [XSPI2_IRQn + 16                    ] = XSPI2_IRQHandler,
  [FMC_IRQn + 16                      ] = FMC_IRQHandler,
  [SDMMC1_IRQn + 16                   ] = SDMMC1_IRQHandler,
  [SDMMC2_IRQn + 16                   ] = SDMMC2_IRQHandler,
  [OTG_FS_IRQn + 16                   ] = OTG_FS_IRQHandler,
  [TIM12_IRQn + 16                    ] = TIM12_IRQHandler,
  [TIM13_IRQn + 16                    ] = TIM13_IRQHandler,
  [TIM14_IRQn + 16                    ] = TIM14_IRQHandler,
  [TIM15_IRQn + 16                    ] = TIM15_IRQHandler,
  [TIM16_IRQn + 16                    ] = TIM16_IRQHandler,
  [TIM17_IRQn + 16                    ] = TIM17_IRQHandler,
  [LPTIM1_IRQn + 16                   ] = LPTIM1_IRQHandler,
  [LPTIM2_IRQn + 16                   ] = LPTIM2_IRQHandler,
  [LPTIM3_IRQn + 16                   ] = LPTIM3_IRQHandler,
  [LPTIM4_IRQn + 16                   ] = LPTIM4_IRQHandler,
  [LPTIM5_IRQn + 16                   ] = LPTIM5_IRQHandler,
  [SPDIF_RX_IRQn + 16                 ] = SPDIF_RX_IRQHandler,
  [MDIOS_IRQn + 16                    ] = MDIOS_IRQHandler,
  [ADF1_FLT0_IRQn + 16                ] = ADF1_FLT0_IRQHandler,
  [CRS_IRQn + 16                      ] = CRS_IRQHandler,
  [UCPD1_IRQn + 16                    ] = UCPD1_IRQHandler,
  [CEC_IRQn + 16                      ] = CEC_IRQHandler,
  [PSSI_IRQn + 16                     ] = PSSI_IRQHandler,
  [LPUART1_IRQn + 16                  ] = LPUART1_IRQHandler,
  [WAKEUP_PIN_IRQn + 16               ] = WAKEUP_PIN_IRQHandler,
  [GPDMA1_Channel8_IRQn + 16          ] = GPDMA1_Channel8_IRQHandler,
  [GPDMA1_Channel9_IRQn + 16          ] = GPDMA1_Channel9_IRQHandler,
  [GPDMA1_Channel10_IRQn + 16         ] = GPDMA1_Channel10_IRQHandler,
  [GPDMA1_Channel11_IRQn + 16         ] = GPDMA1_Channel11_IRQHandler,
  [GPDMA1_Channel12_IRQn + 16         ] = GPDMA1_Channel12_IRQHandler,
  [GPDMA1_Channel13_IRQn + 16         ] = GPDMA1_Channel13_IRQHandler,
  [GPDMA1_Channel14_IRQn + 16         ] = GPDMA1_Channel14_IRQHandler,
  [GPDMA1_Channel15_IRQn + 16         ] = GPDMA1_Channel15_IRQHandler,
  [HPDMA1_Channel8_IRQn + 16          ] = HPDMA1_Channel8_IRQHandler,
  [HPDMA1_Channel9_IRQn + 16          ] = HPDMA1_Channel9_IRQHandler,
  [HPDMA1_Channel10_IRQn + 16         ] = HPDMA1_Channel10_IRQHandler,
  [HPDMA1_Channel11_IRQn + 16         ] = HPDMA1_Channel11_IRQHandler,
  [HPDMA1_Channel12_IRQn + 16         ] = HPDMA1_Channel12_IRQHandler,
  [HPDMA1_Channel13_IRQn + 16         ] = HPDMA1_Channel13_IRQHandler,
  [HPDMA1_Channel14_IRQn + 16         ] = HPDMA1_Channel14_IRQHandler,
  [HPDMA1_Channel15_IRQn + 16         ] = HPDMA1_Channel15_IRQHandler,
  [GPU2D_IRQn + 16                    ] = GPU2D_IRQHandler,
  [GPU2D_ER_IRQn + 16                 ] = GPU2D_ER_IRQHandler,
  [ICACHE_IRQn + 16                   ] = ICACHE_IRQHandler,
  [FDCAN1_IT0_IRQn + 16               ] = FDCAN1_IT0_IRQHandler,
  [FDCAN1_IT1_IRQn + 16               ] = FDCAN1_IT1_IRQHandler,
  [FDCAN2_IT0_IRQn + 16               ] = FDCAN2_IT0_IRQHandler,
  [FDCAN2_IT1_IRQn + 16               ] = FDCAN2_IT1_IRQHandler,
};

