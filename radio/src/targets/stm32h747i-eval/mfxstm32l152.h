/**
  ******************************************************************************
  * @file    mfxstm32l152.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          mfxstm32l152.c IO expander driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MFXSTM32L152_H
#define MFXSTM32L152_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define MFXSTM32L152_OK                      (0)
#define MFXSTM32L152_ERROR                   (-1)

/* MFXSTM32L152_REG_ADR_ID values */
#define MFXSTM32L152_ID                       0x7BU
#define MFXSTM32L152_ID_2                     0x79U

/* MFXSTM32L152_REG_ADR_SYS_CTRL values */
#define MFXSTM32L152_SWRST                    0x80U
#define MFXSTM32L152_STANDBY                  0x40U
#define MFXSTM32L152_ALTERNATE_GPIO_EN        0x08U /* If IDD and TS are enabled they take automatically the AF pins*/
// #define MFXSTM32L152_IDD_EN                   0x04U
// #define MFXSTM32L152_TS_EN                    0x02U
#define MFXSTM32L152_GPIO_EN                  0x01U

/* MFXSTM32L152_REG_ADR_MFX_IRQ_OUT values */
#define MFXSTM32L152_OUT_PIN_TYPE_OPENDRAIN   0x00U
#define MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL    0x01U
#define MFXSTM32L152_OUT_PIN_POLARITY_LOW     0x00U
#define MFXSTM32L152_OUT_PIN_POLARITY_HIGH    0x02U

/* REG_ADR_IRQ_SRC_EN, REG_ADR_IRQ_PENDING & REG_ADR_IRQ_ACK values */
#define MFXSTM32L152_IRQ_TS_OVF               0x80U  /* TouchScreen FIFO Overflow irq*/
#define MFXSTM32L152_IRQ_TS_FULL              0x40U  /* TouchScreen FIFO Full irq*/
#define MFXSTM32L152_IRQ_TS_TH                0x20U  /* TouchScreen FIFO threshold triggered irq*/
#define MFXSTM32L152_IRQ_TS_NE                0x10U  /* TouchScreen FIFO Not Empty irq*/
#define MFXSTM32L152_IRQ_TS_DET               0x08U  /* TouchScreen Detect irq*/
#define MFXSTM32L152_IRQ_ERROR                0x04U  /* Error message from MFXSTM32L152 firmware irq */
#define MFXSTM32L152_IRQ_IDD                  0x02U  /* IDD function irq */
#define MFXSTM32L152_IRQ_GPIO                 0x01U  /* General GPIO irq (only for SRC_EN and PENDING) */
#define MFXSTM32L152_IRQ_ALL                  0xFFU  /* All global interrupts          */
#define MFXSTM32L152_IRQ_TS                   (MFXSTM32L152_IRQ_TS_DET\
                                               | MFXSTM32L152_IRQ_TS_NE\
                                               |  MFXSTM32L152_IRQ_TS_TH\
                                               | MFXSTM32L152_IRQ_TS_FULL\
                                               | MFXSTM32L152_IRQ_TS_OVF)

/* GPIO: IO Pins definition */
#define MFXSTM32L152_GPIO_PIN_0                0x0001U
#define MFXSTM32L152_GPIO_PIN_1                0x0002U
#define MFXSTM32L152_GPIO_PIN_2                0x0004U
#define MFXSTM32L152_GPIO_PIN_3                0x0008U
#define MFXSTM32L152_GPIO_PIN_4                0x0010U
#define MFXSTM32L152_GPIO_PIN_5                0x0020U
#define MFXSTM32L152_GPIO_PIN_6                0x0040U
#define MFXSTM32L152_GPIO_PIN_7                0x0080U

#define MFXSTM32L152_GPIO_PIN_8                0x0100U
#define MFXSTM32L152_GPIO_PIN_9                0x0200U
#define MFXSTM32L152_GPIO_PIN_10               0x0400U
#define MFXSTM32L152_GPIO_PIN_11               0x0800U
#define MFXSTM32L152_GPIO_PIN_12               0x1000U
#define MFXSTM32L152_GPIO_PIN_13               0x2000U
#define MFXSTM32L152_GPIO_PIN_14               0x4000U
#define MFXSTM32L152_GPIO_PIN_15               0x8000U

#define MFXSTM32L152_GPIO_PIN_16               0x010000U
#define MFXSTM32L152_GPIO_PIN_17               0x020000U
#define MFXSTM32L152_GPIO_PIN_18               0x040000U
#define MFXSTM32L152_GPIO_PIN_19               0x080000U
#define MFXSTM32L152_GPIO_PIN_20               0x100000U
#define MFXSTM32L152_GPIO_PIN_21               0x200000U
#define MFXSTM32L152_GPIO_PIN_22               0x400000U
#define MFXSTM32L152_GPIO_PIN_23               0x800000U

#define MFXSTM32L152_AGPIO_PIN_0               MFXSTM32L152_GPIO_PIN_16
#define MFXSTM32L152_AGPIO_PIN_1               MFXSTM32L152_GPIO_PIN_17
#define MFXSTM32L152_AGPIO_PIN_2               MFXSTM32L152_GPIO_PIN_18
#define MFXSTM32L152_AGPIO_PIN_3               MFXSTM32L152_GPIO_PIN_19
#define MFXSTM32L152_AGPIO_PIN_4               MFXSTM32L152_GPIO_PIN_20
#define MFXSTM32L152_AGPIO_PIN_5               MFXSTM32L152_GPIO_PIN_21
#define MFXSTM32L152_AGPIO_PIN_6               MFXSTM32L152_GPIO_PIN_22
#define MFXSTM32L152_AGPIO_PIN_7               MFXSTM32L152_GPIO_PIN_23

#define MFXSTM32L152_GPIO_PINS_ALL             0xFFFFFFU

/* GPIO: constant */
#define MFXSTM32L152_GPIO_DIR_IN                0x0U
#define MFXSTM32L152_GPIO_DIR_OUT               0x1U
#define MFXSTM32L152_IRQ_GPI_EVT_LEVEL          0x0U
#define MFXSTM32L152_IRQ_GPI_EVT_EDGE           0x1U
#define MFXSTM32L152_IRQ_GPI_TYPE_LLFE          0x0U  /* Low Level Falling Edge */
#define MFXSTM32L152_IRQ_GPI_TYPE_HLRE          0x1U  /*High Level Raising Edge */
#define MFXSTM32L152_GPI_WITHOUT_PULL_RESISTOR  0x0U
#define MFXSTM32L152_GPI_WITH_PULL_RESISTOR     0x1U
#define MFXSTM32L152_GPO_PUSH_PULL              0x0U
#define MFXSTM32L152_GPO_OPEN_DRAIN             0x1U
#define MFXSTM32L152_GPIO_PULL_DOWN             0x0U
#define MFXSTM32L152_GPIO_PULL_UP               0x1U

#define MFXSTM32L152_GPIO_NOPULL                0x0U   /*!< No Pull-up or Pull-down activation  */
#define MFXSTM32L152_GPIO_PULLUP                0x1U   /*!< Pull-up activation                  */
#define MFXSTM32L152_GPIO_PULLDOWN              0x2U   /*!< Pull-down activation                */

#define MFXSTM32L152_GPIO_MODE_OFF              0x0U  /* when pin isn't used*/
#define MFXSTM32L152_GPIO_MODE_ANALOG           0x1U  /* analog mode */
#define MFXSTM32L152_GPIO_MODE_INPUT            0x2U  /* input floating */
#define MFXSTM32L152_GPIO_MODE_OUTPUT_OD        0x3U  /* Open Drain output without internal resistor */
#define MFXSTM32L152_GPIO_MODE_OUTPUT_PP        0x4U  /* PushPull output without internal resistor */
#define MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE   0x5U  /* float input - irq detect on rising edge */
#define MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE  0x6U  /* float input - irq detect on falling edge */
#define MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL     0x7U  /* float input - irq detect on low level */
#define MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL    0x8U  /* float input - irq detect on high level */

/**
  * @brief Register address: chip IDs (R)
  */
#define MFXSTM32L152_REG_ADR_ID                 0x00U

/**
  * @brief Register address: chip FW_VERSION  (R)
  */
#define MFXSTM32L152_REG_ADR_FW_VERSION_MSB     0x01U
#define MFXSTM32L152_REG_ADR_FW_VERSION_LSB     0x00U

/**
  * @brief Register address: System Control Register (R/W)
  */
#define MFXSTM32L152_REG_ADR_SYS_CTRL           0x40U

/**
  * @brief Register address: Vdd monitoring (R)
 */
#define MFXSTM32L152_REG_ADR_VDD_REF_MSB        0x06U
#define MFXSTM32L152_REG_ADR_VDD_REF_LSB        0x07U

/**
  * @brief Register address: Error source
  */
#define MFXSTM32L152_REG_ADR_ERROR_SRC          0x03U

/**
  * @brief Register address: Error Message
  */
#define MFXSTM32L152_REG_ADR_ERROR_MSG          0x04U

/**
  * @brief Reg Addr IRQs: to config the pin that informs Main MCU that MFX events appear
  */
#define MFXSTM32L152_REG_ADR_MFX_IRQ_OUT        0x41U

/**
  * @brief Reg Addr IRQs: to select the events which activate the MFXSTM32L152_IRQ_OUT signal
  */
#define MFXSTM32L152_REG_ADR_IRQ_SRC_EN         0x42U

/**
  * @brief Reg Addr IRQs: the Main MCU must read the IRQ_PENDING register to know the interrupt reason
  */
#define MFXSTM32L152_REG_ADR_IRQ_PENDING        0x08U

/**
  * @brief Reg Addr IRQs: the Main MCU must acknowledge it thanks to a writing access to the IRQ_ACK register
  */
#define MFXSTM32L152_REG_ADR_IRQ_ACK            0x44U


/**
  * @brief  GPIO: 24 programmable input/output called MFXSTM32L152_GPIO[23:0] are provided
  */

/**
  * @brief Reg addr: GPIO DIRECTION (R/W): GPIO pins direction: (0) input, (1) output.
  */
#define MFXSTM32L152_REG_ADR_GPIO_DIR1          0x60U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_DIR2          0x61U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_DIR3          0x62U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO TYPE (R/W): If GPIO in output: (0) output push pull, (1) output open drain.
             If GPIO in input: (0) input without pull resistor, (1) input with pull resistor.
  */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE1         0x64U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE2         0x65U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_TYPE3         0x66U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO PULL_UP_PULL_DOWN (R/W)
  */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD1         0x68U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD2         0x69U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_PUPD3         0x6AU  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO SET (W): When GPIO is in output mode, write (1) puts
           the corresponding GPO in High level.
 */
#define MFXSTM32L152_REG_ADR_GPO_SET1           0x6CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPO_SET2           0x6DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPO_SET3           0x6EU  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO CLEAR (W): When GPIO is in output mode, write (1) puts
           the corresponding GPO in Low level.
 */
#define MFXSTM32L152_REG_ADR_GPO_CLR1           0x70U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPO_CLR2           0x71U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPO_CLR3           0x72U  /* agpio [0:7] */

/**
  * @brief Reg addr: GPIO STATE (R): Give state of the GPIO pin.
  */
#define MFXSTM32L152_REG_ADR_GPIO_STATE1         0x10U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_GPIO_STATE2         0x11U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_GPIO_STATE3         0x12U  /* agpio [0:7] */

/* GPIOs can INDIVIDUALLY generate interruption to the Main MCU thanks to the MFXSTM32L152_IRQ_OUT signal */
/* the general MFXSTM32L152_IRQ_GPIO_SRC_EN shall be enabled too          */
/* GPIO IRQ_GPI_SRC1/2/3 (R/W): registers enable or not the feature to generate irq */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1       0x48U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC2       0x49U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_SRC3       0x4AU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_EVT1/2/3 (R/W): Irq generated on level (0) or edge (1).
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT1       0x4CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT2       0x4DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_EVT3       0x4EU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_TYPE1/2/3 (R/W): Irq generated on (0) : Low level or Falling edge.
          (1) : High level or Rising edge.
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE1      0x50U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE2      0x51U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE3      0x52U  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_PENDING1/2/3 (R): irq occurs
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING1   0x0CU  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING2   0x0DU  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING3   0x0EU  /* agpio [0:7] */

/**
  * @brief GPIO IRQ_GPI_ACK1/2/3 (W): Write (1) to acknowledge IRQ event
  */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK1       0x54U  /* gpio [0:7] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK2       0x55U  /* gpio [8:15] */
#define MFXSTM32L152_REG_ADR_IRQ_GPI_ACK3       0x56U  /* agpio [0:7] */


typedef int32_t (*MFXSTM32L152_Write_Func)(void *, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*MFXSTM32L152_Read_Func)(void *, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  MFXSTM32L152_Write_Func   WriteReg;
  MFXSTM32L152_Read_Func    ReadReg;
  void                      *handle;
  /* Internal resources */
} mfxstm32l152_ctx_t;

typedef int32_t (*MFXSTM32L152_Init_Func)(void);
typedef int32_t (*MFXSTM32L152_DeInit_Func)(void);
typedef int32_t (*MFXSTM32L152_GetTick_Func)(void);
typedef int32_t (*MFXSTM32L152_Delay_Func)(uint32_t);
typedef int32_t (*MFXSTM32L152_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*MFXSTM32L152_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  MFXSTM32L152_Init_Func          Init;
  MFXSTM32L152_DeInit_Func        DeInit;
  uint16_t                        Address;
  MFXSTM32L152_WriteReg_Func      WriteReg;
  MFXSTM32L152_ReadReg_Func       ReadReg;
  MFXSTM32L152_GetTick_Func       GetTick;
} MFXSTM32L152_IO_t;


typedef struct
{
  MFXSTM32L152_IO_t         IO;
  mfxstm32l152_ctx_t        Ctx;
  uint8_t                   IsInitialized;
} MFXSTM32L152_Object_t;

typedef struct
{
  uint32_t Pin;       /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref GPIO_pins_define */

  uint32_t Mode;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref IO_ModeTypedef */

  uint32_t Pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull_define */
} MFXSTM32L152_IO_Init_t;

/* IO driver structure initialization */
typedef struct
{
  int32_t (*Init)(MFXSTM32L152_Object_t *, MFXSTM32L152_IO_Init_t *);
  int32_t (*DeInit)(MFXSTM32L152_Object_t *);
  int32_t (*ReadID)(MFXSTM32L152_Object_t *, uint32_t *);
  int32_t (*Reset)(MFXSTM32L152_Object_t *);
  int32_t (*IO_Start)(MFXSTM32L152_Object_t *, uint32_t);
  int32_t (*IO_WritePin)(MFXSTM32L152_Object_t *, uint32_t, uint8_t);
  int32_t (*IO_ReadPin)(MFXSTM32L152_Object_t *, uint32_t);
  int32_t (*IO_EnableIT)(MFXSTM32L152_Object_t *);
  int32_t (*IO_DisableIT)(MFXSTM32L152_Object_t *);
  int32_t (*IO_ITStatus)(MFXSTM32L152_Object_t *, uint32_t);
  int32_t (*IO_ClearIT)(MFXSTM32L152_Object_t *, uint32_t);
} MFXSTM32L152_IO_Mode_t;

/* IO driver structure */
extern MFXSTM32L152_IO_Mode_t MFXSTM32L152_IO_Driver;

/**
  * @brief  Initialize the mfxstm32l152 and configure the needed hardware resources
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Init(MFXSTM32L152_Object_t *pObj);

/**
  * @brief  Register Bus Io to component
  * @param  Component object pointer
  * @retval Component status
  */
int32_t MFXSTM32L152_RegisterBusIO(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_t *pIO);

/**
  * @brief  Read the MFXSTM32L152 IO Expander device ID.
  * @param  pObj   Pointer to component object.
  * @retval The Device ID (two bytes).
  */
int32_t MFXSTM32L152_ReadID(MFXSTM32L152_Object_t *pObj, uint32_t *Id);

int32_t MFXSTM32L152_Probe(MFXSTM32L152_IO_t *pIOCtx,
                           MFXSTM32L152_Object_t *pObj,
                           uint8_t *i2c_address, uint32_t i2c_addr_len);

#ifdef __cplusplus
}
#endif
#endif
