/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   OpenTX - https://github.com/opentx/opentx
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
 *
 * The content of this file partially stems from STM32F4 HAL by STMicroelectronics.
 */


#pragma once

#include <stdint.h>

#define     __IO    volatile             /*!< Defines 'read / write' permissions */

#define GPIO_NUMBER        16U

#define GPIO_GET_INDEX(__GPIOx__)    (uint8_t)(((__GPIOx__) == (GPIOA))? 0U :\
                                               ((__GPIOx__) == (GPIOB))? 1U :\
                                               ((__GPIOx__) == (GPIOC))? 2U :\
                                               ((__GPIOx__) == (GPIOD))? 3U :\
                                               ((__GPIOx__) == (GPIOE))? 4U :\
                                               ((__GPIOx__) == (GPIOF))? 5U :\
                                               ((__GPIOx__) == (GPIOG))? 6U :\
                                               ((__GPIOx__) == (GPIOH))? 7U :\
                                               ((__GPIOx__) == (GPIOI))? 8U :\
                                               ((__GPIOx__) == (GPIOJ))? 9U : 10U)

#define HAL_MAX_DELAY      0xFFFFFFFFU

/* AHB1 Peripheral Clock Enable Disable
 * @brief  Enable or disable the AHB1 peripheral clock.
 * @note   After reset, the peripheral clock (used for registers read/write access)
 *         is disabled and the application software has to enable this clock before
 *         using it.
 */
#define __HAL_RCC_GPIOA_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_GPIOB_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_GPIOC_CLK_ENABLE()  do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOCEN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_GPIOH_CLK_ENABLE()  do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOHEN);\
                                        UNUSED(tmpreg); \
                                         } while(0U)

#define __HAL_RCC_GPIOA_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOAEN))
#define __HAL_RCC_GPIOB_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOBEN))
#define __HAL_RCC_GPIOC_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOCEN))
#define __HAL_RCC_GPIOH_CLK_DISABLE()        (RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOHEN))

/* APB1 Peripheral Clock Enable Disable
 * @brief  Enable or disable the Low Speed APB (APB1) peripheral clock.
 * @note   After reset, the peripheral clock (used for registers read/write access)
 *         is disabled and the application software has to enable this clock before
 *         using it.
 */
#define __HAL_RCC_I2C1_CLK_ENABLE()     do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_I2C2_CLK_ENABLE()     do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C2EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C2EN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_I2C3_CLK_ENABLE()     do { \
                                        __IO uint32_t tmpreg = 0x00U; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C3EN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C3EN);\
                                        UNUSED(tmpreg); \
                                          } while(0U)
#define __HAL_RCC_I2C1_CLK_DISABLE()   (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C1EN))
#define __HAL_RCC_I2C2_CLK_DISABLE()   (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C2EN))
#define __HAL_RCC_I2C3_CLK_DISABLE()   (RCC->APB1ENR &= ~(RCC_APB1ENR_I2C3EN))

#define __HAL_LOCK(__HANDLE__)                                           \
                              do{                                        \
                                  if((__HANDLE__)->Lock == HAL_LOCKED)   \
                                  {                                      \
                                     return HAL_BUSY;                    \
                                  }                                      \
                                  else                                   \
                                  {                                      \
                                     (__HANDLE__)->Lock = HAL_LOCKED;    \
                                  }                                      \
                                }while (0U)

#define __HAL_UNLOCK(__HANDLE__)                                          \
                                do{                                       \
                                    (__HANDLE__)->Lock = HAL_UNLOCKED;    \
                                  }while (0U)


/* Enable the specified I2C peripheral.
 * @param  __HANDLE__ specifies the I2C Handle.
 * @retval None
 */
#define __HAL_I2C_ENABLE(__HANDLE__)                  SET_BIT((__HANDLE__)->Instance->CR1, I2C_CR1_PE)

/* Disable the specified I2C peripheral.
 * @param  __HANDLE__ specifies the I2C Handle.
 * @retval None
 */
#define __HAL_I2C_DISABLE(__HANDLE__)                 CLEAR_BIT((__HANDLE__)->Instance->CR1, I2C_CR1_PE)

// I2C Private Constants
#define I2C_FLAG_MASK                   0x0000FFFFU
#define I2C_MIN_PCLK_FREQ_STANDARD      2000000U    /*!< 2 MHz                     */
#define I2C_MIN_PCLK_FREQ_FAST          4000000U    /*!< 4 MHz                     */

#define I2C_TIMEOUT_FLAG                35U         /*!< Timeout 35 ms             */
#define I2C_TIMEOUT_BUSY_FLAG           25U         /*!< Timeout 25 ms             */
#define I2C_TIMEOUT_STOP_FLAG           5U          /*!< Timeout 5 ms              */
#define I2C_NO_OPTION_FRAME             0xFFFF0000U /*!< XferOptions default value */

// I2C Private Macros
#define I2C_MIN_PCLK_FREQ(__PCLK__, __SPEED__)        (((__SPEED__) <= 100000U) ? ((__PCLK__) < I2C_MIN_PCLK_FREQ_STANDARD) : ((__PCLK__) < I2C_MIN_PCLK_FREQ_FAST))
#define I2C_CCR_CALCULATION(__PCLK__, __SPEED__, __COEFF__)     (((((__PCLK__) - 1U)/((__SPEED__) * (__COEFF__))) + 1U) & I2C_CCR_CCR)
#define I2C_FREQRANGE(__PCLK__)                       ((__PCLK__)/1000000U)
#define I2C_RISE_TIME(__FREQRANGE__, __SPEED__)       (((__SPEED__) <= 100000U) ? ((__FREQRANGE__) + 1U) : ((((__FREQRANGE__) * 300U) / 1000U) + 1U))
#define I2C_SPEED_STANDARD(__PCLK__, __SPEED__)            ((I2C_CCR_CALCULATION((__PCLK__), (__SPEED__), 2U) < 4U)? 4U:I2C_CCR_CALCULATION((__PCLK__), (__SPEED__), 2U))
#define I2C_SPEED_FAST(__PCLK__, __SPEED__, __DUTYCYCLE__) (((__DUTYCYCLE__) == I2C_DUTYCYCLE_2)? I2C_CCR_CALCULATION((__PCLK__), (__SPEED__), 3U) : (I2C_CCR_CALCULATION((__PCLK__), (__SPEED__), 25U) | I2C_DUTYCYCLE_16_9))
#define I2C_SPEED(__PCLK__, __SPEED__, __DUTYCYCLE__)      (((__SPEED__) <= 100000U)? (I2C_SPEED_STANDARD((__PCLK__), (__SPEED__))) : \
                                                                  ((I2C_SPEED_FAST((__PCLK__), (__SPEED__), (__DUTYCYCLE__)) & I2C_CCR_CCR) == 0U)? 1U : \
                                                                  ((I2C_SPEED_FAST((__PCLK__), (__SPEED__), (__DUTYCYCLE__))) | I2C_CCR_FS))
#define I2C_7BIT_ADD_WRITE(__ADDRESS__)                    ((uint8_t)((__ADDRESS__) & (uint8_t)(~I2C_OAR1_ADD0)))
#define I2C_7BIT_ADD_READ(__ADDRESS__)                     ((uint8_t)((__ADDRESS__) | I2C_OAR1_ADD0))

#define I2C_10BIT_ADDRESS(__ADDRESS__)                     ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)0x00FF)))
#define I2C_10BIT_HEADER_WRITE(__ADDRESS__)                ((uint8_t)((uint16_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)0x0300)) >> 7) | (uint16_t)0x00F0)))
#define I2C_10BIT_HEADER_READ(__ADDRESS__)                 ((uint8_t)((uint16_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)0x0300)) >> 7) | (uint16_t)(0x00F1))))

#define I2C_MEM_ADD_MSB(__ADDRESS__)                       ((uint8_t)((uint16_t)(((uint16_t)((__ADDRESS__) & (uint16_t)0xFF00)) >> 8)))
#define I2C_MEM_ADD_LSB(__ADDRESS__)                       ((uint8_t)((uint16_t)((__ADDRESS__) & (uint16_t)0x00FF)))

// I2C duty cycle in fast mode
#define I2C_DUTYCYCLE_2                 0x00000000U
#define I2C_DUTYCYCLE_16_9              I2C_CCR_DUTY

// I2C addressing mode
#define I2C_ADDRESSINGMODE_7BIT         0x00004000U
#define I2C_ADDRESSINGMODE_10BIT        (I2C_OAR1_ADDMODE | 0x00004000U)

// I2C dual addressing mode
#define I2C_DUALADDRESS_DISABLE        0x00000000U
#define I2C_DUALADDRESS_ENABLE         I2C_OAR2_ENDUAL

// I2C general call addressing mode
#define I2C_GENERALCALL_DISABLE        0x00000000U
#define I2C_GENERALCALL_ENABLE         I2C_CR1_ENGC

// I2C nostretch mode
#define I2C_NOSTRETCH_DISABLE          0x00000000U
#define I2C_NOSTRETCH_ENABLE           I2C_CR1_NOSTRETCH

// I2C Error Code definition
#define HAL_I2C_ERROR_NONE              0x00000000U    /*!< No error              */
#define HAL_I2C_ERROR_BERR              0x00000001U    /*!< BERR error            */
#define HAL_I2C_ERROR_ARLO              0x00000002U    /*!< ARLO error            */
#define HAL_I2C_ERROR_AF                0x00000004U    /*!< AF error              */
#define HAL_I2C_ERROR_OVR               0x00000008U    /*!< OVR error             */
#define HAL_I2C_ERROR_DMA               0x00000010U    /*!< DMA transfer error    */
#define HAL_I2C_ERROR_TIMEOUT           0x00000020U    /*!< Timeout Error         */
#define HAL_I2C_ERROR_SIZE              0x00000040U    /*!< Size Management error */
#define HAL_I2C_ERROR_DMA_PARAM         0x00000080U    /*!< DMA Parameter Error   */
#define HAL_I2C_WRONG_START             0x00000200U    /*!< Wrong start Error     */

// I2C XferOptions definition
#define  I2C_FIRST_FRAME                0x00000001U
#define  I2C_FIRST_AND_NEXT_FRAME       0x00000002U
#define  I2C_NEXT_FRAME                 0x00000004U
#define  I2C_FIRST_AND_LAST_FRAME       0x00000008U
#define  I2C_LAST_FRAME_NO_STOP         0x00000010U
#define  I2C_LAST_FRAME                 0x00000020U

// I2C states
#define I2C_STATE_MSK             ((uint32_t)((uint32_t)((uint32_t)HAL_I2C_STATE_BUSY_TX | (uint32_t)HAL_I2C_STATE_BUSY_RX) & (uint32_t)(~((uint32_t)HAL_I2C_STATE_READY)))) /*!< Mask State define, keep only RX and TX bits            */
#define I2C_STATE_NONE            ((uint32_t)(HAL_I2C_MODE_NONE))                                                        /*!< Default Value                                          */
#define I2C_STATE_MASTER_BUSY_TX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy TX, combinaison of State LSB and Mode enum */
#define I2C_STATE_MASTER_BUSY_RX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy RX, combinaison of State LSB and Mode enum */
#define I2C_STATE_SLAVE_BUSY_TX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy TX, combinaison of State LSB and Mode enum  */
#define I2C_STATE_SLAVE_BUSY_RX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy RX, combinaison of State LSB and Mode enum  */

// I2C Analog Filter
#define I2C_ANALOGFILTER_ENABLE        0x00000000U
#define I2C_ANALOGFILTER_DISABLE       I2C_FLTR_ANOFF

/* Clears the I2C ADDR pending flag.
 * @param  __HANDLE__ specifies the I2C Handle.
 *         This parameter can be I2C where x: 1, 2, or 3 to select the I2C peripheral.
 * @retval None
 */
#define __HAL_I2C_CLEAR_ADDRFLAG(__HANDLE__)    \
  do{                                           \
    __IO uint32_t tmpreg = 0x00U;               \
    tmpreg = (__HANDLE__)->Instance->SR1;       \
    tmpreg = (__HANDLE__)->Instance->SR2;       \
    UNUSED(tmpreg);                             \
  } while(0)

/* Clears the I2C pending flags which are cleared by writing 0 in a specific bit.
 * @param  __HANDLE__ specifies the I2C Handle.
 * @param  __FLAG__ specifies the flag to clear.
 *         This parameter can be any combination of the following values:
 *            @arg I2C_FLAG_OVR: Overrun/Underrun flag (Slave mode)
 *            @arg I2C_FLAG_AF: Acknowledge failure flag
 *            @arg I2C_FLAG_ARLO: Arbitration lost flag (Master mode)
 *            @arg I2C_FLAG_BERR: Bus error flag
 * @retval None
 */
#define __HAL_I2C_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->SR1 = (uint16_t)(~((__FLAG__) & I2C_FLAG_MASK)))

// HAL Status structures definition
typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

// HAL lock structures definition
typedef enum
{
 HAL_UNLOCKED = 0x00U,
 HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;

/* HAL State structure definition
  * HAL I2C State value coding follow below described bitmap :
  * b7-b6  Error information
  *   00 : No Error
  *   01 : Abort (Abort user request on going)
  *   10 : Timeout
  *   11 : Error
  * b5     Peripheral initialization status
  *   0  : Reset (Peripheral not initialized)
  *   1  : Init done (Peripheral initialized and ready to use. HAL I2C Init function called)
  * b4     (not used)
  *   x  : Should be set to 0
  * b3
  *   0  : Ready or Busy (No Listen mode ongoing)
  *   1  : Listen (Peripheral in Address Listen Mode)
  * b2     Intrinsic process state
  *   0  : Ready
  *   1  : Busy (Peripheral busy with some configuration or internal operations)
  * b1     Rx state
  *   0  : Ready (no Rx operation ongoing)
  *   1  : Busy (Rx operation ongoing)
  * b0     Tx state
  *   0  : Ready (no Tx operation ongoing)
  *   1  : Busy (Tx operation ongoing)
  */
typedef enum
{
  HAL_I2C_STATE_RESET             = 0x00U,   /*!< Peripheral is not yet Initialized         */
  HAL_I2C_STATE_READY             = 0x20U,   /*!< Peripheral Initialized and ready for use  */
  HAL_I2C_STATE_BUSY              = 0x24U,   /*!< An internal process is ongoing            */
  HAL_I2C_STATE_BUSY_TX           = 0x21U,   /*!< Data Transmission process is ongoing      */
  HAL_I2C_STATE_BUSY_RX           = 0x22U,   /*!< Data Reception process is ongoing         */
  HAL_I2C_STATE_LISTEN            = 0x28U,   /*!< Address Listen Mode is ongoing            */
  HAL_I2C_STATE_BUSY_TX_LISTEN    = 0x29U,   /*!< Address Listen Mode and Data Transmission
                                                 process is ongoing                         */
  HAL_I2C_STATE_BUSY_RX_LISTEN    = 0x2AU,   /*!< Address Listen Mode and Data Reception
                                                 process is ongoing                         */
  HAL_I2C_STATE_ABORT             = 0x60U,   /*!< Abort user request ongoing                */
  HAL_I2C_STATE_TIMEOUT           = 0xA0U,   /*!< Timeout state                             */
  HAL_I2C_STATE_ERROR             = 0xE0U    /*!< Error                                     */
} HAL_I2C_StateTypeDef;

/* HAL Mode structure definition
  * HAL I2C Mode value coding follow below described bitmap :
  * b7     (not used)
  *   x  : Should be set to 0
  * b6
  *   0  : None
  *   1  : Memory (HAL I2C communication is in Memory Mode)
  * b5
  *   0  : None
  *   1  : Slave (HAL I2C communication is in Slave Mode)
  * b4
  *   0  : None
  *   1  : Master (HAL I2C communication is in Master Mode)
  * b3-b2-b1-b0  (not used)
  * xxxx : Should be set to 0000
  */
typedef enum
{
  HAL_I2C_MODE_NONE               = 0x00U,   /*!< No I2C communication on going             */
  HAL_I2C_MODE_MASTER             = 0x10U,   /*!< I2C communication is in Master Mode       */
  HAL_I2C_MODE_SLAVE              = 0x20U,   /*!< I2C communication is in Slave Mode        */
  HAL_I2C_MODE_MEM                = 0x40U    /*!< I2C communication is in Memory Mode       */
} HAL_I2C_ModeTypeDef;

// I2C configuration structure definition
typedef struct
{
  uint32_t ClockSpeed;       /*!< Specifies the clock frequency.
                                  This parameter must be set to a value lower than 400kHz */
  uint32_t DutyCycle;        /*!< Specifies the I2C fast mode duty cycle.
                                  This parameter can be a value of @ref I2C_duty_cycle_in_fast_mode */
  uint32_t OwnAddress1;      /*!< Specifies the first device own address.
                                  This parameter can be a 7-bit or 10-bit address. */
  uint32_t AddressingMode;   /*!< Specifies if 7-bit or 10-bit addressing mode is selected.
                                  This parameter can be a value of @ref I2C_addressing_mode */
  uint32_t DualAddressMode;  /*!< Specifies if dual addressing mode is selected.
                                  This parameter can be a value of @ref I2C_dual_addressing_mode */
  uint32_t OwnAddress2;      /*!< Specifies the second device own address if dual addressing mode is selected
                                  This parameter can be a 7-bit address. */
  uint32_t GeneralCallMode;  /*!< Specifies if general call mode is selected.
                                  This parameter can be a value of @ref I2C_general_call_addressing_mode */
  uint32_t NoStretchMode;    /*!< Specifies if nostretch mode is selected.
                                  This parameter can be a value of @ref I2C_nostretch_mode */
} HAL_I2C_InitTypeDef; // WARNING: renamed from I2C_InitTypeDef due to conflict

// I2C handle structure definition
typedef struct
{
  I2C_TypeDef                *Instance;      /*!< I2C registers base address               */
  HAL_I2C_InitTypeDef        Init;           /*!< I2C communication parameters (MOD!)      */
  uint8_t                    *pBuffPtr;      /*!< Pointer to I2C transfer buffer           */
  uint16_t                   XferSize;       /*!< I2C transfer size                        */
  __IO uint16_t              XferCount;      /*!< I2C transfer counter                     */
  __IO uint32_t              XferOptions;    /*!< I2C transfer options                     */
  __IO uint32_t              PreviousState;  /*!< I2C communication Previous state and mode
                                                  context for internal usage               */
  //DMA_HandleTypeDef          *hdmatx;        /*!< I2C Tx DMA handle parameters             */
  //DMA_HandleTypeDef          *hdmarx;        /*!< I2C Rx DMA handle parameters             */
  HAL_LockTypeDef            Lock;           /*!< I2C locking object                       */
  __IO HAL_I2C_StateTypeDef  State;          /*!< I2C communication state                  */
  __IO HAL_I2C_ModeTypeDef   Mode;           /*!< I2C communication mode                   */
  __IO uint32_t              ErrorCode;      /*!< I2C Error code                           */
  __IO uint32_t              Devaddress;     /*!< I2C Target device address                */
  __IO uint32_t              Memaddress;     /*!< I2C Target memory address                */
  __IO uint32_t              MemaddSize;     /*!< I2C Target memory address  size          */
  __IO uint32_t              EventCount;     /*!< I2C Event counter                        */
} I2C_HandleTypeDef;

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter);
HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
