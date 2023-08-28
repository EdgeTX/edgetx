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

#pragma once

#include "stm32_spi.h"
#include <stdint.h>


/* R1 response bits (see sd spec. 7.3.2.1 Format R1) */
#define SD_R1_RESPONSE_PARAM_ERROR       (1<<6)
#define SD_R1_RESPONSE_ADDR_ERROR        (1<<5)
#define SD_R1_RESPONSE_ERASE_SEQ_ERROR   (1<<4)
#define SD_R1_RESPONSE_CMD_CRC_ERROR     (1<<3)
#define SD_R1_RESPONSE_ILLEGAL_CMD_ERROR (1<<2)
#define SD_R1_RESPONSE_ERASE_RESET       (1<<1)
#define SD_R1_RESPONSE_IN_IDLE_STATE     (0x01)
#define SD_INVALID_R1_RESPONSE           (1<<7)

#define R1_VALID(X) (((X) >> 7) == 0)
#define R1_PARAM_ERR(X)   ((((X) &SD_R1_RESPONSE_PARAM_ERROR) != 0))
#define R1_ADDR_ERR(X)    ((((X) &SD_R1_RESPONSE_ADDR_ERROR) != 0))
#define R1_ERASE_ERR(X)   ((((X) &SD_R1_RESPONSE_ERASE_SEQ_ERROR) != 0))
#define R1_CMD_CRC_ERR(X) ((((X) &SD_R1_RESPONSE_CMD_CRC_ERROR) != 0))
#define R1_ILL_CMD_ERR(X) ((((X) &SD_R1_RESPONSE_ILLEGAL_CMD_ERROR) != 0))
#define R1_IDLE_BIT_SET(X) (((X) &SD_R1_RESPONSE_IN_IDLE_STATE) != 0)
#define R1_ERROR(X) (R1_PARAM_ERR(X) || R1_ADDR_ERR(X) || R1_ERASE_ERR(X) || \
                     R1_CMD_CRC_ERR(X) || R1_ILL_CMD_ERR(X))

/* see sd spec. 7.3.3.1 Data Response Token */
#define DATA_RESPONSE_IS_VALID(X)  (((X) & 0x11) == 0x01)
#define DATA_RESPONSE_ACCEPTED(X)  (((X) & 0x0E) == 0x04)
#define DATA_RESPONSE_CRC_ERR(X)   (((X) & 0x0E) == 0x0A)
#define DATA_RESPONSE_WRITE_ERR(X) (((X) & 0x0E) == 0x0C)

/* see sd spec. 5.1 OCR register */
#define OCR_VOLTAGE_3_2_TO_3_3 (1L << 20)
#define OCR_VOLTAGE_3_3_TO_3_4 (1L << 21)

/* card capacity status (CCS=0: the card is SDSD; CCS=1: card is SDHC or SDXC) */
#define OCR_CCS (1L << 30)

/* This bit is set to low if the card has not finished power up routine */
#define OCR_POWER_UP_STATUS (1L << 31)

/* to ensure the voltage range check on init is done properly you need to
   define this according to your actual interface/wiring with the sd-card */
#define SYSTEM_VOLTAGE (OCR_VOLTAGE_3_2_TO_3_3 | OCR_VOLTAGE_3_2_TO_3_3)

/* see sd spec. 7.3.1.3 Detailed Command Description */
#define SD_CMD_PREFIX_MASK (1<<6)

#define SD_CMD_0 0   /* Resets the SD Memory Card */
#define SD_CMD_1 1   /* Sends host capacity support info and starts the cards init process */
#define SD_CMD_8 8   /* Sends SD Card interface condition incl. host supply voltage info */
#define SD_CMD_9 9   /* Asks the selected card to send its card-specific data (CSD) */
#define SD_CMD_10 10 /* Asks the selected card to send its card identification (CID) */
#define SD_CMD_12 12 /* Forces the card to stop transmission in Multiple Block Read Operation */
#define SD_CMD_13 13 /* Sent as ACMD13 asks the card to send it's SD status */

#define SD_CMD_16 16 /* In case of SDSC Card, block length is set by this command */
#define SD_CMD_17 17 /* Reads a block of the size selected by the SET_BLOCKLEN command */
#define SD_CMD_18 18 /* Continuously transfers data blocks from card to host
                        until interrupted by a STOP_TRANSMISSION command */
#define SD_CMD_23 23 /* Used for ACMD23: write block erase count */
#define SD_CMD_24 24 /* Writes a block of the size selected by the SET_BLOCKLEN command */
#define SD_CMD_25 25 /* Continuously writes blocks of data until 'Stop Tran'token is sent */
#define SD_CMD_41 41 /* Reserved (used for ACMD41) */
#define SD_CMD_55 55 /* Defines to the card that the next command is an application specific
                        command rather than a standard command */
#define SD_CMD_58 58 /* Reads the OCR register of a card */
#define SD_CMD_59 59 /* Turns the CRC option on or off. Argument: 1:on; 0:off */

#define SD_CMD_8_VHS_2_7_V_TO_3_6_V 0x01
#define SD_CMD_8_CHECK_PATTERN      0xB5
#define SD_CMD_NO_ARG     0x00000000
#define SD_ACMD_41_ARG_HC 0x40000000
#define SD_CMD_59_ARG_EN  0x00000001
#define SD_CMD_59_ARG_DIS 0x00000000

/* see sd spec. 7.3.3 Control Tokens */
#define SD_DATA_TOKEN_CMD_17_18_24 0xFE
#define SD_DATA_TOKEN_CMD_25       0xFC
#define SD_DATA_TOKEN_CMD_25_STOP  0xFD

#define SD_SIZE_OF_CID_AND_CSD_REG 16
#define SD_SIZE_OF_SD_STATUS 64
#define SD_BLOCKS_FOR_REG_READ 1
#define SD_GET_CSD_STRUCTURE(CSD_RAW_DATA) ((CSD_RAW_DATA)[0] >> 6)
#define SD_CSD_V1 0
#define SD_CSD_V2 1
#define SD_CSD_VUNSUPPORTED -1

#define SD_HC_BLOCK_SIZE   (512)

#define SDCARD_SPI_INIT_ERROR (-1)   /* returned on failed init */
#define SDCARD_SPI_OK         (0)    /* returned on successful init */

#define SD_SIZE_OF_OID 2 /* OID (OEM/application ID field in CID reg) */
#define SD_SIZE_OF_PNM 5 /* PNM (product name field in CID reg) */

#define SD_SIZE_OF_CID_AND_CSD_REG 16
#define SD_SIZE_OF_SD_STATUS 64
#define SD_BLOCKS_FOR_REG_READ 1
#define SD_GET_CSD_STRUCTURE(CSD_RAW_DATA) ((CSD_RAW_DATA)[0] >> 6)

#define SD_CSD_V1 0
#define SD_CSD_V2 1
#define SD_CSD_VUNSUPPORTED -1

/**
 * @brief   CID register see section 5.2 in SD-Spec v5.00
 */
typedef struct {
    uint8_t MID;              /**< Manufacturer ID */
    char OID[SD_SIZE_OF_OID]; /**< OEM/Application ID*/
    char PNM[SD_SIZE_OF_PNM]; /**< Product name */
    uint8_t PRV;              /**< Product revision */
    uint32_t PSN;             /**< Product serial number */
    uint16_t MDT;             /**< Manufacturing date */
    uint8_t CID_CRC;          /**< CRC7 checksum */
} cid_t;

/**
 * @brief   CSD register with csd structure version 1.0
 *          see section 5.3.2 in SD-Spec v5.00
 */
typedef struct {
    uint8_t CSD_STRUCTURE : 2;        /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t TAAC : 8;                 /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t NSAC : 8;                 /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t TRAN_SPEED : 8;           /**< see section 5.3.2 in SD-Spec v5.00 */
    uint16_t CCC : 12;                /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t READ_BL_LEN : 4;          /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t READ_BL_PARTIAL : 1;      /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t WRITE_BLK_MISALIGN : 1;   /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t READ_BLK_MISALIGN : 1;    /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t DSR_IMP : 1;              /**< see section 5.3.2 in SD-Spec v5.00 */
    uint16_t C_SIZE : 12;             /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t VDD_R_CURR_MIN : 3;       /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t VDD_R_CURR_MAX : 3;       /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t VDD_W_CURR_MIN : 3;       /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t VDD_W_CURR_MAX : 3;       /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t C_SIZE_MULT : 3;          /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t ERASE_BLK_EN : 1;         /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t SECTOR_SIZE : 7;          /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t WP_GRP_SIZE : 7;          /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t WP_GRP_ENABLE : 1;        /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t R2W_FACTOR : 3;           /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t WRITE_BL_LEN : 4;         /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t WRITE_BL_PARTIAL : 1;     /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t FILE_FORMAT_GRP : 1;      /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t COPY : 1;                 /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t PERM_WRITE_PROTECT : 1;   /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t TMP_WRITE_PROTECT : 1;    /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t FILE_FORMAT : 2;          /**< see section 5.3.2 in SD-Spec v5.00 */
    uint8_t CSD_CRC : 8;              /**< see section 5.3.2 in SD-Spec v5.00 */
} csd_v1_t;

/**
 * @brief   CSD register with csd structure version 2.0
 *          see section 5.3.3 in SD-Spec v5.00
 */
typedef struct {
    uint8_t CSD_STRUCTURE : 2;        /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t TAAC : 8;                 /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t NSAC : 8;                 /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t TRAN_SPEED : 8;           /**< see section 5.3.3 in SD-Spec v5.00 */
    uint16_t CCC : 12;                /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t READ_BL_LEN : 4;          /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t READ_BL_PARTIAL : 1;      /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t WRITE_BLK_MISALIGN : 1;   /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t READ_BLK_MISALIGN : 1;    /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t DSR_IMP : 1;              /**< see section 5.3.3 in SD-Spec v5.00 */
    uint32_t C_SIZE : 22;             /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t ERASE_BLK_EN : 1;         /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t SECTOR_SIZE : 7;          /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t WP_GRP_SIZE : 7;          /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t WP_GRP_ENABLE : 1;        /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t R2W_FACTOR : 3;           /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t WRITE_BL_LEN : 4;         /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t WRITE_BL_PARTIAL : 1;     /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t FILE_FORMAT_GRP : 1;      /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t COPY : 1;                 /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t PERM_WRITE_PROTECT : 1;   /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t TMP_WRITE_PROTECT : 1;    /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t FILE_FORMAT : 2;          /**< see section 5.3.3 in SD-Spec v5.00 */
    uint8_t CSD_CRC : 8;              /**< see section 5.3.3 in SD-Spec v5.00 */
} csd_v2_t;

/**
 * @brief   CSD register (see section 5.3 in SD-Spec v5.00)
 */
typedef union {
    csd_v1_t v1;   /**< see section 5.3.2 in SD-Spec v5.00 */
    csd_v2_t v2;   /**< see section 5.3.3 in SD-Spec v5.00 */
} csd_t;

/**
 * @brief   SD status register (see section 4.10.2 in SD-Spec v5.00)
 */
typedef struct {
    uint32_t SIZE_OF_PROTECTED_AREA : 32;   /**< see section 4.10.2 in SD-Spec v5.00 */
    uint32_t SUS_ADDR : 22;                 /**< see section 4.10.2.12 in SD-Spec v5.00 */
    uint32_t VSC_AU_SIZE : 10;              /**< see section 4.10.2.11 in SD-Spec v5.00 */
    uint16_t SD_CARD_TYPE : 16;             /**< see section 4.10.2 in SD-Spec v5.00 */
    uint16_t ERASE_SIZE : 16;               /**< see section 4.10.2.5 in SD-Spec v5.00 */
    uint8_t  SPEED_CLASS : 8;               /**< see section 4.10.2.2 in SD-Spec v5.00 */
    uint8_t  PERFORMANCE_MOVE : 8;          /**< see section 4.10.2.3 in SD-Spec v5.00 */
    uint8_t  VIDEO_SPEED_CLASS : 8;         /**< see section 4.10.2.10 in SD-Spec v5.00 */
    uint8_t  ERASE_TIMEOUT : 6;             /**< see section 4.10.2.6 in SD-Spec v5.00 */
    uint8_t  ERASE_OFFSET : 2;              /**< see section 4.10.2.7 in SD-Spec v5.00 */
    uint8_t  UHS_SPEED_GRADE : 4;           /**< see section 4.10.2.8 in SD-Spec v5.00 */
    uint8_t  UHS_AU_SIZE : 4;               /**< see section 4.10.2.9 in SD-Spec v5.00 */
    uint8_t  AU_SIZE : 4;                   /**< see section 4.10.2.4 in SD-Spec v5.00 */
    uint8_t  DAT_BUS_WIDTH : 2;             /**< see section 4.10.2 in SD-Spec v5.00 */
    uint8_t  SECURED_MODE : 1;              /**< see section 4.10.2 in SD-Spec v5.00 */
} sd_status_t;

/**
 * @brief   version type of SD-card
 */
typedef enum {
    SD_V2,                 /**< SD version 2  */
    SD_V1,                 /**< SD version 1  */
    MMC_V3,                /**< MMC version 3 */
    SD_UNKNOWN             /**< SD-version unknown */
} sd_version_t;

/**
 * @brief   sdcard info
 */
typedef struct {
  sd_version_t card_type;
  cid_t        cid;
  csd_t        csd;
  int          csd_structure;
  bool         use_block_addr;
} sdcard_info_t;

typedef struct {
  const stm32_spi_t* spi;
  bool               use_block_addr;
} sdcard_spi_t;

/**
 * @brief   sdcard_spi r/w-operation return values
 */
typedef enum {
    SD_RW_OK = 0,           /**< no error */
    SD_RW_NO_TOKEN,         /**< no token was received (on block read) */
    SD_RW_TIMEOUT,          /**< cmd timed out (not-busy-state wasn't entered) */
    SD_RW_RX_TX_ERROR,      /**< error while performing SPI read/write */
    SD_RW_WRITE_ERROR,      /**< data-packet response indicates error */
    SD_RW_CRC_MISMATCH,     /**< CRC-mismatch of received data */
    SD_RW_NOT_SUPPORTED     /**< operation not supported on used card */
} sd_rw_response_t;

int sdcard_spi_init(const stm32_spi_t* spi, sdcard_info_t* card);

uint64_t sdcard_spi_get_capacity(sdcard_info_t* card);

uint32_t sdcard_spi_get_sector_count(sdcard_info_t* card);

int sdcard_spi_read_blocks(uint32_t blockaddr, uint8_t* data,
                           uint16_t blocksize, uint16_t nblocks,
                           sd_rw_response_t* state);

int sdcard_spi_write_blocks(uint32_t blockaddr, const uint8_t* data,
                            uint16_t blocksize, uint16_t nblocks,
                            sd_rw_response_t* state);

int sdcard_spi_wait_for_not_busy();
