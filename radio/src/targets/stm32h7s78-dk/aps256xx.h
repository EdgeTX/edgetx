#pragma once

#include "stm32_hal.h"

/**
  * @brief  APS256XX Configuration
  */
#define APS256XX_RAM_SIZE               0x2000000U  /* 256 MBits => 32 MBytes                */

/**
  * @brief  APS256XX Error codes
  */
#define APS256XX_OK                     (0)
#define APS256XX_ERROR                  (-1)

/******************************************************************************
  * @brief  APS256XX Registers
  ****************************************************************************/
/* Mode Register 0 */
#define APS256XX_MR0_ADDRESS            0x00000000U

#define APS256XX_MR0_DRIVE_STRENGTH     0x03U       /*!< Drive Strength                      */
#define APS256XX_MR0_DS_FULL            0x00U       /*!< Drive Strength : Full (25 Ohm)      */
#define APS256XX_MR0_DS_HALF            0x01U       /*!< Drive Strength : Half (50 Ohm)      */
#define APS256XX_MR0_DS_QUART           0x02U       /*!< Drive Strength : 1/4 (100 Ohm)      */
#define APS256XX_MR0_DS_EIGHT           0x03U       /*!< Drive Strength : 1/8 (200 Ohm)      */

#define APS256XX_MR0_READ_LATENCY_CODE  0x1CU       /*!< Read Latency Code                   */
#define APS256XX_MR0_RLC_3              0x00U       /*!< Read Latency Code : 3               */
#define APS256XX_MR0_RLC_4              0x04U       /*!< Read Latency Code : 4               */
#define APS256XX_MR0_RLC_5              0x08U       /*!< Read Latency Code : 5               */
#define APS256XX_MR0_RLC_6              0x0CU       /*!< Read Latency Code : 6               */
#define APS256XX_MR0_RLC_7              0x10U       /*!< Read Latency Code : 7               */

#define APS256XX_MR0_LATENCY_TYPE       0x20U       /*!< Latency Type                        */

/* Mode Register 1 */
#define APS256XX_MR1_ADDRESS            0x00000001U

#define APS256XX_MR1_VENDOR_ID          0x1FU       /*!< Vendor Identifier                   */
#define APS256XX_MR1_VENDOR_ID_APM      0x0DU       /*!< Vendor Identifier : APM             */

#define APS256XX_MR1_ULP                0x80U       /*!< Ultra Low Power Device              */

/* Mode Register 2 */
#define APS256XX_MR2_ADDRESS            0x00000002U

#define APS256XX_MR2_DENSITY            0x07U       /*!< Device Density                      */
#define APS256XX_MR2_DENSITY_128_MB     0x05U       /*!< Device Density : 128 Mb             */
#define APS256XX_MR2_DENSITY_256_MB     0x07U       /*!< Device Density : 256 Mb             */
#define APS256XX_MR2_DENSITY_512_MB     0x06U       /*!< Device Density : 512 Mb             */

#define APS256XX_MR2_DEVICE_ID          0x18U       /*!< Device Identifier                   */
#define APS256XX_MR2_DEVID_GEN_1        0x00U       /*!< Device Identifier : Generation 1    */
#define APS256XX_MR2_DEVID_GEN_2        0x08U       /*!< Device Identifier : Generation 2    */
#define APS256XX_MR2_DEVID_GEN_3        0x10U       /*!< Device Identifier : Generation 3    */
#define APS256XX_MR2_DEVID_GEN_4        0x18U       /*!< Device Identifier : Generation 4    */

#define APS256XX_MR2_KGD                0xE0U       /*!< Good-Die Identifier                 */
#define APS256XX_MR2_KGD_GOOD_DIE_ID    0xC0U       /*!< Good-Die Identifier : PASS          */

/* Mode Register 3 */
#define APS256XX_MR3_ADDRESS            0x00000003U

#define APS256XX_MR3_SRF                0x30U       /*!< Self Refresh Flag                   */
#define APS256XX_MR3_SRF_0_5X           0x10U       /*!< Self Refresh Flag : 0.5x Refresh    */
#define APS256XX_MR3_SRF_1X             0x00U       /*!< Self Refresh Flag : 1x Refresh      */
#define APS256XX_MR3_SRF_4X             0x20U       /*!< Self Refresh Flag : 4x Refresh      */

#define APS256XX_MR3_RBXEN              0x80U       /*!< Row Boundary Crossing Enable        */

/* Mode Register 4 */
#define APS256XX_MR4_ADDRESS            0x00000004U

#define APS256XX_MR4_PASR               0x07U       /*!< Partially Address Space Refresh     */
#define APS256XX_MR4_PASR_FULL          0x00U       /*!< PASR : Full Array                   */
#define APS256XX_MR4_PASR_BOTTOM_HALF   0x01U       /*!< PASR : Bottom 1/2 Array             */
#define APS256XX_MR4_PASR_BOTTOM_QUART  0x02U       /*!< PASR : Bottom 1/4 Array             */
#define APS256XX_MR4_PASR_BOTTOM_EIGHT  0x03U       /*!< PASR : Bottom 1/8 Array             */
#define APS256XX_MR4_PASR_NONE          0x04U       /*!< PASR : None                         */
#define APS256XX_MR4_PASR_TOP_HALF      0x05U       /*!< PASR : Top 1/2 Array                */
#define APS256XX_MR4_PASR_TOP_QUART     0x06U       /*!< PASR : Top 1/4 Array                */
#define APS256XX_MR4_PASR_TOP_EIGHT     0x07U       /*!< PASR : Top 1/8 Array                */

#define APS256XX_MR4_RF_RATE            0x18U       /*!< Refresh Frequency Setting           */
#define APS256XX_MR4_RF_4X              0x00U       /*!< RF : Always 4x Refresh              */
#define APS256XX_MR4_RF_1X              0x08U       /*!< RF : Enables 1x Refresh             */
#define APS256XX_MR4_RF_0_5X            0x18U       /*!< RF : Enables 0.5x Refresh           */

#define APS256XX_MR4_WRITE_LATENCY_CODE 0xE0U       /*!< Write Latency Code                  */
#define APS256XX_MR4_WLC_3              0x00U       /*!< Write Latency Code : 3              */
#define APS256XX_MR4_WLC_4              0x80U       /*!< Write Latency Code : 4              */
#define APS256XX_MR4_WLC_5              0x40U       /*!< Write Latency Code : 5              */
#define APS256XX_MR4_WLC_6              0xC0U       /*!< Write Latency Code : 6              */
#define APS256XX_MR4_WLC_7              0x20U       /*!< Write Latency Code : 7              */

/* Mode Register 6 */
#define APS256XX_MR6_ADDRESS            0x00000006U

#define APS256XX_MR6_HALF_SLEEP         0xF0U       /*!< Half Sleep                          */
#define APS256XX_MR6_HS_HALF_SPEED_MODE 0xF0U       /*!< Half Sleep : Half Speed Mode        */
#define APS256XX_MR6_HS_DEEP_POWER_DOWN 0xC0U       /*!< Half Sleep : Deep Power Down Mode   */

/* Mode Register 8 */
#define APS256XX_MR8_ADDRESS            0x00000008U

#define APS256XX_MR8_BL                 0x03U       /*!< Burst Length                        */
#define APS256XX_MR8_BL_16_BYTES        0x00U       /*!< Burst Length : 16 Byte/Word Wrap    */
#define APS256XX_MR8_BL_32_BYTES        0x01U       /*!< Burst Length : 32 Byte/Word Wrap    */
#define APS256XX_MR8_BL_64_BYTES        0x02U       /*!< Burst Length : 64 Byte/Word Wrap    */
#define APS256XX_MR8_BL_2K_BYTES        0x03U       /*!< Burst Length : 2K Byte/1K Word Wrap */

#define APS256XX_MR8_BT                 0x04U       /*!< Burst Type                          */

#define APS256XX_MR8_RBX                0x08U       /*!< Row Boundary Crossing Read Enable   */

#define APS256XX_MR8_X8_X16             0x40U       /*!< IO X8/X16 Mode                      */

/******************************************************************************
  * @brief  APS256XX Commands
  ****************************************************************************/
/* Read Operations */
#define APS256XX_READ_CMD               0x00U       /*!< Synchronous Read                    */
#define APS256XX_READ_LINEAR_BURST_CMD  0x20U       /*!< Linear Burst Read                   */

/* Write Operations */
#define APS256XX_WRITE_CMD              0x80U       /*!< Synchronous Write                   */
#define APS256XX_WRITE_LINEAR_BURST_CMD 0xA0U       /*!< Linear Burst Write                  */

/* Reset Operations */
#define APS256XX_RESET_CMD              0xFFU       /*!< Global Reset                        */

/* Register Operations */
#define APS256XX_READ_REG_CMD           0x40U       /*!< Mode Register Read                  */
#define APS256XX_WRITE_REG_CMD          0xC0U       /*!< Mode Register Write                 */

/**
  * @}
  */
/** @defgroup APS256XX_Exported_Types APS256XX Exported Types
  * @{
  */
typedef enum
{
  APS256XX_BURST_16_BYTES = APS256XX_MR8_BL_16_BYTES,
  APS256XX_BURST_32_BYTES = APS256XX_MR8_BL_32_BYTES,
  APS256XX_BURST_64_BYTES = APS256XX_MR8_BL_64_BYTES,
  APS256XX_BURST_2_KBYTES = APS256XX_MR8_BL_2K_BYTES
} APS256XX_BurstLength_t;

typedef enum
{
  APS256XX_READ_LATENCY_3 = APS256XX_MR0_RLC_3,
  APS256XX_READ_LATENCY_4 = APS256XX_MR0_RLC_4,
  APS256XX_READ_LATENCY_5 = APS256XX_MR0_RLC_5,
  APS256XX_READ_LATENCY_6 = APS256XX_MR0_RLC_6,
  APS256XX_READ_LATENCY_7 = APS256XX_MR0_RLC_7
} APS256XX_ReadLatencyCode_t;

typedef enum
{
  APS256XX_WRITE_LATENCY_3 = APS256XX_MR4_WLC_3,
  APS256XX_WRITE_LATENCY_4 = APS256XX_MR4_WLC_4,
  APS256XX_WRITE_LATENCY_5 = APS256XX_MR4_WLC_5,
  APS256XX_WRITE_LATENCY_6 = APS256XX_MR4_WLC_6,
  APS256XX_WRITE_LATENCY_7 = APS256XX_MR4_WLC_7
} APS256XX_WriteLatencyCode_t;
/**
  * @}
  */

/** @defgroup APS256XX_Exported_Macros APS256XX Exported Macros
  * @{
  */
#define APS256XX_READ_REG_LATENCY(rlc)   ((((rlc) & APS256XX_MR0_READ_LATENCY_CODE) >> 2U) + 3U)

#define APS256XX_READ_LATENCY(rlc, type) ((((type) & APS256XX_MR0_LATENCY_TYPE) == 0U) ? \
                                          ((((rlc) & APS256XX_MR0_READ_LATENCY_CODE) >> 2U) + 3U) : \
                                          (((((rlc) & APS256XX_MR0_READ_LATENCY_CODE) >> 2U) + 3U)*2U))

#define APS256XX_WRITE_LATENCY(wlc)      ((((wlc) & APS256XX_MR4_WRITE_LATENCY_CODE) == 0U) ? 3U : \
                                          ((((wlc) & APS256XX_MR4_WLC_7)             != 0U) ? 7U : \
                                           ((((wlc) & APS256XX_MR4_WLC_5)            == 0U) ? 4U : \
                                            ((((wlc) & APS256XX_MR4_WLC_4)           == 0U) ? 5U : 6U))))
/**
  * @}
  */

/** @defgroup APS256XX_Exported_Functions APS256XX Exported Functions
  * @{
  */
/* Read/Write Array Commands **************************************************/
int32_t APS256XX_Read(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size, uint32_t LatencyCode,
                      uint32_t IOMode, uint32_t BurstType);
int32_t APS256XX_Read_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size,
                          uint32_t LatencyCode, uint32_t IOMode, uint32_t BurstType);
int32_t APS256XX_Write(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size, uint32_t LatencyCode,
                       uint32_t IOMode, uint32_t BurstType);
int32_t APS256XX_Write_DMA(XSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size,
                           uint32_t LatencyCode, uint32_t IOMode, uint32_t BurstType);
int32_t APS256XX_EnableMemoryMappedMode(XSPI_HandleTypeDef *Ctx, uint32_t ReadLatencyCode, uint32_t WriteLatencyCode,
                                        uint32_t IOMode, uint32_t BurstType);

/* Register/Setting Commands **************************************************/
int32_t APS256XX_ReadReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t *Value, uint32_t LatencyCode);
int32_t APS256XX_WriteReg(XSPI_HandleTypeDef *Ctx, uint32_t Address, uint8_t Value);

/* ID Commands ****************************************************************/
int32_t APS256XX_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *ID, uint32_t LatencyCode);

/* Power down Commands ********************************************************/
int32_t APS256XX_EnterPowerDown(XSPI_HandleTypeDef *Ctx);
int32_t APS256XX_LeavePowerDown(XSPI_HandleTypeDef *Ctx);

/* Reset Command **************************************************************/
int32_t APS256XX_Reset(XSPI_HandleTypeDef *Ctx);
