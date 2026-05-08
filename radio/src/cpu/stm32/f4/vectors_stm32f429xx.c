
#include "stm32f429xx.h"
#include "cortex_m_isr.h"

void Default_Handler() { default_isr_handler(); }

/* stm32f429xx specific interrupt vectors */
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
WEAK_DEFAULT void WWDG_IRQHandler();
WEAK_DEFAULT void PVD_IRQHandler();
WEAK_DEFAULT void TAMP_STAMP_IRQHandler();
WEAK_DEFAULT void RTC_WKUP_IRQHandler();
WEAK_DEFAULT void FLASH_IRQHandler();
WEAK_DEFAULT void RCC_IRQHandler();
WEAK_DEFAULT void EXTI0_IRQHandler();
WEAK_DEFAULT void EXTI1_IRQHandler();
WEAK_DEFAULT void EXTI2_IRQHandler();
WEAK_DEFAULT void EXTI3_IRQHandler();
WEAK_DEFAULT void EXTI4_IRQHandler();
WEAK_DEFAULT void DMA1_Stream0_IRQHandler();
WEAK_DEFAULT void DMA1_Stream1_IRQHandler();
WEAK_DEFAULT void DMA1_Stream2_IRQHandler();
WEAK_DEFAULT void DMA1_Stream3_IRQHandler();
WEAK_DEFAULT void DMA1_Stream4_IRQHandler();
WEAK_DEFAULT void DMA1_Stream5_IRQHandler();
WEAK_DEFAULT void DMA1_Stream6_IRQHandler();
WEAK_DEFAULT void ADC_IRQHandler();
WEAK_DEFAULT void CAN1_TX_IRQHandler();
WEAK_DEFAULT void CAN1_RX0_IRQHandler();
WEAK_DEFAULT void CAN1_RX1_IRQHandler();
WEAK_DEFAULT void CAN1_SCE_IRQHandler();
WEAK_DEFAULT void EXTI9_5_IRQHandler();
WEAK_DEFAULT void TIM1_BRK_TIM9_IRQHandler();
WEAK_DEFAULT void TIM1_UP_TIM10_IRQHandler();
WEAK_DEFAULT void TIM1_TRG_COM_TIM11_IRQHandler();
WEAK_DEFAULT void TIM1_CC_IRQHandler();
WEAK_DEFAULT void TIM2_IRQHandler();
WEAK_DEFAULT void TIM3_IRQHandler();
WEAK_DEFAULT void TIM4_IRQHandler();
WEAK_DEFAULT void I2C1_EV_IRQHandler();
WEAK_DEFAULT void I2C1_ER_IRQHandler();
WEAK_DEFAULT void I2C2_EV_IRQHandler();
WEAK_DEFAULT void I2C2_ER_IRQHandler();
WEAK_DEFAULT void SPI1_IRQHandler();
WEAK_DEFAULT void SPI2_IRQHandler();
WEAK_DEFAULT void USART1_IRQHandler();
WEAK_DEFAULT void USART2_IRQHandler();
WEAK_DEFAULT void USART3_IRQHandler();
WEAK_DEFAULT void EXTI15_10_IRQHandler();
WEAK_DEFAULT void RTC_Alarm_IRQHandler();
WEAK_DEFAULT void OTG_FS_WKUP_IRQHandler();
WEAK_DEFAULT void TIM8_BRK_TIM12_IRQHandler();
WEAK_DEFAULT void TIM8_UP_TIM13_IRQHandler();
WEAK_DEFAULT void TIM8_TRG_COM_TIM14_IRQHandler();
WEAK_DEFAULT void TIM8_CC_IRQHandler();
WEAK_DEFAULT void DMA1_Stream7_IRQHandler();
WEAK_DEFAULT void FMC_IRQHandler();
WEAK_DEFAULT void SDIO_IRQHandler();
WEAK_DEFAULT void TIM5_IRQHandler();
WEAK_DEFAULT void SPI3_IRQHandler();
WEAK_DEFAULT void UART4_IRQHandler();
WEAK_DEFAULT void UART5_IRQHandler();
WEAK_DEFAULT void TIM6_DAC_IRQHandler();
WEAK_DEFAULT void TIM7_IRQHandler();
WEAK_DEFAULT void DMA2_Stream0_IRQHandler();
WEAK_DEFAULT void DMA2_Stream1_IRQHandler();
WEAK_DEFAULT void DMA2_Stream2_IRQHandler();
WEAK_DEFAULT void DMA2_Stream3_IRQHandler();
WEAK_DEFAULT void DMA2_Stream4_IRQHandler();
WEAK_DEFAULT void ETH_IRQHandler();
WEAK_DEFAULT void ETH_WKUP_IRQHandler();
WEAK_DEFAULT void CAN2_TX_IRQHandler();
WEAK_DEFAULT void CAN2_RX0_IRQHandler();
WEAK_DEFAULT void CAN2_RX1_IRQHandler();
WEAK_DEFAULT void CAN2_SCE_IRQHandler();
WEAK_DEFAULT void OTG_FS_IRQHandler();
WEAK_DEFAULT void DMA2_Stream5_IRQHandler();
WEAK_DEFAULT void DMA2_Stream6_IRQHandler();
WEAK_DEFAULT void DMA2_Stream7_IRQHandler();
WEAK_DEFAULT void USART6_IRQHandler();
WEAK_DEFAULT void I2C3_EV_IRQHandler();
WEAK_DEFAULT void I2C3_ER_IRQHandler();
WEAK_DEFAULT void OTG_HS_EP1_OUT_IRQHandler();
WEAK_DEFAULT void OTG_HS_EP1_IN_IRQHandler();
WEAK_DEFAULT void OTG_HS_WKUP_IRQHandler();
WEAK_DEFAULT void OTG_HS_IRQHandler();
WEAK_DEFAULT void DCMI_IRQHandler();
WEAK_DEFAULT void HASH_RNG_IRQHandler();
WEAK_DEFAULT void FPU_IRQHandler();
WEAK_DEFAULT void UART7_IRQHandler();
WEAK_DEFAULT void UART8_IRQHandler();
WEAK_DEFAULT void SPI4_IRQHandler();
WEAK_DEFAULT void SPI5_IRQHandler();
WEAK_DEFAULT void SPI6_IRQHandler();
WEAK_DEFAULT void SAI1_IRQHandler();
WEAK_DEFAULT void LTDC_IRQHandler();
WEAK_DEFAULT void LTDC_ER_IRQHandler();
WEAK_DEFAULT void DMA2D_IRQHandler();

/* CPU specific interrupt vector table */
const isr_t g_pnfVectors[] ISR_VECTOR = {
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
  [WWDG_IRQn + 16                     ] = WWDG_IRQHandler,
  [PVD_IRQn + 16                      ] = PVD_IRQHandler,
  [TAMP_STAMP_IRQn + 16               ] = TAMP_STAMP_IRQHandler,
  [RTC_WKUP_IRQn + 16                 ] = RTC_WKUP_IRQHandler,
  [FLASH_IRQn + 16                    ] = FLASH_IRQHandler,
  [RCC_IRQn + 16                      ] = RCC_IRQHandler,
  [EXTI0_IRQn + 16                    ] = EXTI0_IRQHandler,
  [EXTI1_IRQn + 16                    ] = EXTI1_IRQHandler,
  [EXTI2_IRQn + 16                    ] = EXTI2_IRQHandler,
  [EXTI3_IRQn + 16                    ] = EXTI3_IRQHandler,
  [EXTI4_IRQn + 16                    ] = EXTI4_IRQHandler,
  [DMA1_Stream0_IRQn + 16             ] = DMA1_Stream0_IRQHandler,
  [DMA1_Stream1_IRQn + 16             ] = DMA1_Stream1_IRQHandler,
  [DMA1_Stream2_IRQn + 16             ] = DMA1_Stream2_IRQHandler,
  [DMA1_Stream3_IRQn + 16             ] = DMA1_Stream3_IRQHandler,
  [DMA1_Stream4_IRQn + 16             ] = DMA1_Stream4_IRQHandler,
  [DMA1_Stream5_IRQn + 16             ] = DMA1_Stream5_IRQHandler,
  [DMA1_Stream6_IRQn + 16             ] = DMA1_Stream6_IRQHandler,
  [ADC_IRQn + 16                      ] = ADC_IRQHandler,
  [CAN1_TX_IRQn + 16                  ] = CAN1_TX_IRQHandler,
  [CAN1_RX0_IRQn + 16                 ] = CAN1_RX0_IRQHandler,
  [CAN1_RX1_IRQn + 16                 ] = CAN1_RX1_IRQHandler,
  [CAN1_SCE_IRQn + 16                 ] = CAN1_SCE_IRQHandler,
  [EXTI9_5_IRQn + 16                  ] = EXTI9_5_IRQHandler,
  [TIM1_BRK_TIM9_IRQn + 16            ] = TIM1_BRK_TIM9_IRQHandler,
  [TIM1_UP_TIM10_IRQn + 16            ] = TIM1_UP_TIM10_IRQHandler,
  [TIM1_TRG_COM_TIM11_IRQn + 16       ] = TIM1_TRG_COM_TIM11_IRQHandler,
  [TIM1_CC_IRQn + 16                  ] = TIM1_CC_IRQHandler,
  [TIM2_IRQn + 16                     ] = TIM2_IRQHandler,
  [TIM3_IRQn + 16                     ] = TIM3_IRQHandler,
  [TIM4_IRQn + 16                     ] = TIM4_IRQHandler,
  [I2C1_EV_IRQn + 16                  ] = I2C1_EV_IRQHandler,
  [I2C1_ER_IRQn + 16                  ] = I2C1_ER_IRQHandler,
  [I2C2_EV_IRQn + 16                  ] = I2C2_EV_IRQHandler,
  [I2C2_ER_IRQn + 16                  ] = I2C2_ER_IRQHandler,
  [SPI1_IRQn + 16                     ] = SPI1_IRQHandler,
  [SPI2_IRQn + 16                     ] = SPI2_IRQHandler,
  [USART1_IRQn + 16                   ] = USART1_IRQHandler,
  [USART2_IRQn + 16                   ] = USART2_IRQHandler,
  [USART3_IRQn + 16                   ] = USART3_IRQHandler,
  [EXTI15_10_IRQn + 16                ] = EXTI15_10_IRQHandler,
  [RTC_Alarm_IRQn + 16                ] = RTC_Alarm_IRQHandler,
  [OTG_FS_WKUP_IRQn + 16              ] = OTG_FS_WKUP_IRQHandler,
  [TIM8_BRK_TIM12_IRQn + 16           ] = TIM8_BRK_TIM12_IRQHandler,
  [TIM8_UP_TIM13_IRQn + 16            ] = TIM8_UP_TIM13_IRQHandler,
  [TIM8_TRG_COM_TIM14_IRQn + 16       ] = TIM8_TRG_COM_TIM14_IRQHandler,
  [TIM8_CC_IRQn + 16                  ] = TIM8_CC_IRQHandler,
  [DMA1_Stream7_IRQn + 16             ] = DMA1_Stream7_IRQHandler,
  [FMC_IRQn + 16                      ] = FMC_IRQHandler,
  [SDIO_IRQn + 16                     ] = SDIO_IRQHandler,
  [TIM5_IRQn + 16                     ] = TIM5_IRQHandler,
  [SPI3_IRQn + 16                     ] = SPI3_IRQHandler,
  [UART4_IRQn + 16                    ] = UART4_IRQHandler,
  [UART5_IRQn + 16                    ] = UART5_IRQHandler,
  [TIM6_DAC_IRQn + 16                 ] = TIM6_DAC_IRQHandler,
  [TIM7_IRQn + 16                     ] = TIM7_IRQHandler,
  [DMA2_Stream0_IRQn + 16             ] = DMA2_Stream0_IRQHandler,
  [DMA2_Stream1_IRQn + 16             ] = DMA2_Stream1_IRQHandler,
  [DMA2_Stream2_IRQn + 16             ] = DMA2_Stream2_IRQHandler,
  [DMA2_Stream3_IRQn + 16             ] = DMA2_Stream3_IRQHandler,
  [DMA2_Stream4_IRQn + 16             ] = DMA2_Stream4_IRQHandler,
  [ETH_IRQn + 16                      ] = ETH_IRQHandler,
  [ETH_WKUP_IRQn + 16                 ] = ETH_WKUP_IRQHandler,
  [CAN2_TX_IRQn + 16                  ] = CAN2_TX_IRQHandler,
  [CAN2_RX0_IRQn + 16                 ] = CAN2_RX0_IRQHandler,
  [CAN2_RX1_IRQn + 16                 ] = CAN2_RX1_IRQHandler,
  [CAN2_SCE_IRQn + 16                 ] = CAN2_SCE_IRQHandler,
  [OTG_FS_IRQn + 16                   ] = OTG_FS_IRQHandler,
  [DMA2_Stream5_IRQn + 16             ] = DMA2_Stream5_IRQHandler,
  [DMA2_Stream6_IRQn + 16             ] = DMA2_Stream6_IRQHandler,
  [DMA2_Stream7_IRQn + 16             ] = DMA2_Stream7_IRQHandler,
  [USART6_IRQn + 16                   ] = USART6_IRQHandler,
  [I2C3_EV_IRQn + 16                  ] = I2C3_EV_IRQHandler,
  [I2C3_ER_IRQn + 16                  ] = I2C3_ER_IRQHandler,
  [OTG_HS_EP1_OUT_IRQn + 16           ] = OTG_HS_EP1_OUT_IRQHandler,
  [OTG_HS_EP1_IN_IRQn + 16            ] = OTG_HS_EP1_IN_IRQHandler,
  [OTG_HS_WKUP_IRQn + 16              ] = OTG_HS_WKUP_IRQHandler,
  [OTG_HS_IRQn + 16                   ] = OTG_HS_IRQHandler,
  [DCMI_IRQn + 16                     ] = DCMI_IRQHandler,
  [HASH_RNG_IRQn + 16                 ] = HASH_RNG_IRQHandler,
  [FPU_IRQn + 16                      ] = FPU_IRQHandler,
  [UART7_IRQn + 16                    ] = UART7_IRQHandler,
  [UART8_IRQn + 16                    ] = UART8_IRQHandler,
  [SPI4_IRQn + 16                     ] = SPI4_IRQHandler,
  [SPI5_IRQn + 16                     ] = SPI5_IRQHandler,
  [SPI6_IRQn + 16                     ] = SPI6_IRQHandler,
  [SAI1_IRQn + 16                     ] = SAI1_IRQHandler,
  [LTDC_IRQn + 16                     ] = LTDC_IRQHandler,
  [LTDC_ER_IRQn + 16                  ] = LTDC_ER_IRQHandler,
  [DMA2D_IRQn + 16                    ] = DMA2D_IRQHandler,
};

