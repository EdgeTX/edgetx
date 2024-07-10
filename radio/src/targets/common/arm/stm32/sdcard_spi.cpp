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

#include "sdcard_spi.h"
#include "stm32_spi.h"
#include "timers_driver.h"
#include "delays_driver.h"

#include "debug.h"
#include "crc.h"

#include <string.h>

#define SD_SPI_CLK_400K (400000UL)

#if !defined(SD_SPI_CLK_MAX)
#define SD_SPI_CLK_MAX (25000000UL)
#endif

// Disable CRC support by default
//
// #if !defined(SD_CARD_SPI_DISABLE_CRC)
// #define SD_CARD_SPI_ENABLE_CRC
// #endif

#define US_PER_MS                   (1000UL)
#define INIT_CMD_RETRY_US           (750 * US_PER_MS)
#define INIT_CMD0_RETRY_US          (100UL)
#define R1_POLLING_RETRY_US         (100 * US_PER_MS)
#define SD_DATA_TOKEN_RETRY_US      (100 * US_PER_MS)
#define SD_WAIT_FOR_NOT_BUSY_US     (250 * US_PER_MS)

#define SD_BLOCK_READ_CMD_RETRY_US  (100UL)
#define SD_BLOCK_WRITE_CMD_RETRY_US (100UL)

#define SD_CARD_DUMMY_BYTE (0xFF)

typedef enum {
  SD_INIT_START,
  SD_INIT_SPI_POWER_SEQ,
  SD_INIT_SEND_CMD0,
  SD_INIT_SEND_CMD8,
  SD_INIT_CARD_UNKNOWN,
  SD_INIT_SEND_ACMD41_HCS,
  SD_INIT_SEND_ACMD41,
  SD_INIT_SEND_CMD1,
  SD_INIT_SEND_CMD58,
  SD_INIT_SEND_CMD16,
  SD_INIT_ENABLE_CRC,
  SD_INIT_READ_CID,
  SD_INIT_READ_CSD,
  SD_INIT_SET_MAX_SPI_SPEED,
  SD_INIT_FINISH
} sd_init_fsm_state_t;

static sdcard_spi_t _sdcard_spi = {nullptr, false};

static inline void _send_dummy_byte(const stm32_spi_t* spi)
{
  stm32_spi_transfer_byte(spi, SD_CARD_DUMMY_BYTE);
}

static bool _wait_for_not_busy(const stm32_spi_t* spi, uint32_t retry_us)
{
  uint32_t timeout = timersGetUsTick();

  do {
    uint8_t read_byte = stm32_spi_transfer_byte(spi, SD_CARD_DUMMY_BYTE);
    if (read_byte == 0xFF) {
      return true;
    }
    if ((read_byte & 0x0F) != 0x00) {
      // shift by some bits???
      stm32_spi_unselect(spi);
      _send_dummy_byte(spi);
      stm32_spi_select(spi);
    }
  } while (timersGetUsTick() - timeout < retry_us);

  TRACE("_wait_for_not_busy: [FAILED]");
  return false;
}

static uint8_t _crc_7(const uint8_t* data, int n)
{
  uint8_t crc = 0;

  for (int i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (int j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) {
        crc ^= 0x09;
      }
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}

static inline uint16_t _transfer_bytes(const stm32_spi_t* spi, const uint8_t* out,
                                       uint8_t* in, uint16_t length)
{
  return stm32_spi_transfer_bytes(spi, out, in, length);
}

static void _flush_block(const stm32_spi_t* spi)
{
  for (int i = 0; i < 512 + 2; i++) {
    _send_dummy_byte(spi);
  }
}

static inline uint8_t _wait_for_r1(const stm32_spi_t* spi, uint32_t retry_us)
{
  uint32_t timeout = timersGetUsTick();
  uint8_t r1;

  do {
    r1 = stm32_spi_transfer_byte(spi, SD_CARD_DUMMY_BYTE);

    if (R1_VALID(r1)) {
      return r1;
    }

  } while (timersGetUsTick() - timeout < retry_us);

  return r1;
}

static inline bool _wait_for_token(const stm32_spi_t* spi, uint8_t token, uint32_t retry_us)
{
  uint32_t timeout = timersGetUsTick();
  uint8_t read_byte;

  do {
    read_byte = stm32_spi_transfer_byte(spi, SD_CARD_DUMMY_BYTE);
    if (read_byte == token) {
      return true;
    }
    
  } while ((read_byte == 0xFF) && (timersGetUsTick() - timeout < retry_us));

  // hack to get rid of a packet in case
  // the token was shifted
  if (read_byte != 0xFF) {
    _flush_block(spi);
  }

  return false;
}

static uint8_t sdcard_spi_send_cmd(const stm32_spi_t* spi, uint8_t sd_cmd_idx,
                                   uint32_t argument, uint32_t retry_us)
{
  uint8_t r1_resu;
  uint8_t cmd_data[6];

  cmd_data[0] = SD_CMD_PREFIX_MASK | sd_cmd_idx;
  cmd_data[1] = argument >> (3 * 8);
  cmd_data[2] = (argument >> (2 * 8)) & 0xFF;
  cmd_data[3] = (argument >> 8) & 0xFF;
  cmd_data[4] = argument & 0xFF;
  cmd_data[5] = _crc_7(cmd_data, sizeof(cmd_data) - 1);

  uint32_t timeout = timersGetUsTick();
  do {
    if (!_wait_for_not_busy(spi, SD_WAIT_FOR_NOT_BUSY_US)) {
      TRACE(
          "sdcard_spi_send_cmd: timeout while waiting for bus to be not busy!");
      r1_resu = SD_INVALID_R1_RESPONSE;
      continue;
    }

    if (_transfer_bytes(spi, cmd_data, nullptr, sizeof(cmd_data)) != sizeof(cmd_data)) {
      TRACE("sdcard_spi_send_cmd: _transfer_bytes: send cmd [%d]: [ERROR]",
            sd_cmd_idx);
      r1_resu = SD_INVALID_R1_RESPONSE;
      continue;
    }

    /* received byte after cmd12 is a dummy byte and should be ignored */
    if (sd_cmd_idx == SD_CMD_12) {
      _send_dummy_byte(spi);
    }

    r1_resu = _wait_for_r1(spi, R1_POLLING_RETRY_US);

    if (R1_VALID(r1_resu)) {
      break;
    } else {
      TRACE("sdcard_spi_send_cmd: R1_TIMEOUT (0x%02x)", r1_resu);
      r1_resu = SD_INVALID_R1_RESPONSE;
    }

  } while (timersGetUsTick() - timeout < retry_us);

  return r1_resu;
}

static uint8_t sdcard_spi_send_acmd(const stm32_spi_t* spi, uint8_t sd_cmd_idx,
                                    uint32_t argument, uint32_t retry_us)
{
  uint32_t timeout = timersGetUsTick();
  uint8_t r1_resu;

  do {
    r1_resu = sdcard_spi_send_cmd(spi, SD_CMD_55, SD_CMD_NO_ARG, 0);
    if (R1_VALID(r1_resu) && !R1_ERROR(r1_resu)) {
      r1_resu = sdcard_spi_send_cmd(spi, sd_cmd_idx, argument, 0);

      if (R1_VALID(r1_resu) && !R1_ERROR(r1_resu)) {
        return r1_resu;
      }
    }
  } while (timersGetUsTick() - timeout < retry_us);

  TRACE("sdcard_spi_send_acmd: [TIMEOUT]");
  return r1_resu;
}

static sd_rw_response_t _read_cid(const stm32_spi_t* spi, sdcard_info_t* card);
static sd_rw_response_t _read_csd(const stm32_spi_t* spi, sdcard_info_t* card);

static sd_init_fsm_state_t _init_sd_fsm_step(const stm32_spi_t* spi,
                                             sdcard_info_t* card,
                                             sd_init_fsm_state_t state)
{
  switch (state) {
  case SD_INIT_START:
    TRACE("SD_INIT_START");
    stm32_spi_init(spi, LL_SPI_DATAWIDTH_8BIT);
    stm32_spi_set_max_baudrate(spi, SD_SPI_CLK_400K);
    return SD_INIT_SPI_POWER_SEQ;

  case SD_INIT_SPI_POWER_SEQ:
    TRACE("SD_INIT_SPI_POWER_SEQ");
    {
      // wait minimum 50 ms until card is powered
      uint32_t power_on_timeout = timersGetUsTick();
      while(timersGetUsTick() - power_on_timeout < 50 * US_PER_MS);
    }

    // unselect sdcard for power up sequence
    stm32_spi_unselect(spi);

    // powersequence: perform at least 74 clock cycles with mosi_pin being
    // high (same as sending dummy bytes with 0xFF)
    for (int i = 0; i < 10; i++) {
      stm32_spi_transfer_byte(spi, 0xff);
    }
    {
      // wait for "end of transfer"
      uint32_t power_on_timeout = timersGetUsTick();
      while(timersGetUsTick() - power_on_timeout <= 200);
    }
    return SD_INIT_SEND_CMD0;

  case SD_INIT_SEND_CMD0:
    TRACE("SD_INIT_SEND_CMD0");
    stm32_spi_select(spi);
    {
      uint8_t cmd0_r1 = sdcard_spi_send_cmd(spi, SD_CMD_0, SD_CMD_NO_ARG, INIT_CMD0_RETRY_US);
      stm32_spi_unselect(spi);

      if (R1_VALID(cmd0_r1) && !R1_ERROR(cmd0_r1) && R1_IDLE_BIT_SET(cmd0_r1)) {
        TRACE("CMD0: [OK]");
        return SD_INIT_ENABLE_CRC;
      } else {
        TRACE("CMD0: [FAILED]");
      }
    }

    return SD_INIT_CARD_UNKNOWN;

  case SD_INIT_ENABLE_CRC:
#if defined(SD_CARD_SPI_ENABLE_CRC)
    TRACE("SD_INIT_ENABLE_CRC");
    stm32_spi_select(spi);
    {
      uint8_t r1 = sdcard_spi_send_cmd(spi, SD_CMD_59, SD_CMD_59_ARG_EN,
                                       INIT_CMD_RETRY_US);
      stm32_spi_unselect(spi);

      if (R1_VALID(r1) && !R1_ERROR(r1)) {
        TRACE("CMD59: [OK]");
        return SD_INIT_SEND_CMD8;
      }
    }
    return SD_INIT_CARD_UNKNOWN;
#endif
  /* if SD_CARD_SPI_ENABLE_CRC is not enabled,
     let's continue directly with CMD8
   */

  case SD_INIT_SEND_CMD8:
    TRACE("SD_INIT_SEND_CMD8");
    stm32_spi_select(spi);
    {
      int cmd8_arg = (SD_CMD_8_VHS_2_7_V_TO_3_6_V << 8) | SD_CMD_8_CHECK_PATTERN;
      uint8_t cmd8_r1 = sdcard_spi_send_cmd(spi, SD_CMD_8, cmd8_arg, INIT_CMD_RETRY_US);

      if (R1_VALID(cmd8_r1) && !R1_ERROR(cmd8_r1)) {
        TRACE("CMD8: [OK] --> reading remaining bytes for R7");

        uint8_t r7[4];

        if (_transfer_bytes(spi, nullptr, &r7[0], sizeof(r7)) == sizeof(r7)) {
          TRACE("R7 response: 0x%02x 0x%02x 0x%02x 0x%02x", r7[0], r7[1], r7[2], r7[3]);
          /* check if lower 12 bits (voltage range and check pattern) of
             response and arg are equal to verify compatibility and
             communication is working properly */
          if (((r7[2] & 0x0F) == ((cmd8_arg >> 8) & 0x0F)) &&
              (r7[3] == (cmd8_arg & 0xFF))) {
            TRACE("CMD8: [R7 MATCH]");
            return SD_INIT_SEND_ACMD41_HCS;
          }

          TRACE("CMD8: [R7 MISMATCH]");
        } else {
          TRACE("CMD8: _transfer_bytes (R7): [ERROR]");
        }

        stm32_spi_unselect(spi);
        return SD_INIT_CARD_UNKNOWN;
      }
    }

    TRACE("CMD8: [ERROR / NO RESPONSE]");
    return SD_INIT_SEND_ACMD41;

  case SD_INIT_CARD_UNKNOWN:
    TRACE("SD_INIT_CARD_UNKNOWN");
    card->card_type = SD_UNKNOWN;
    return SD_INIT_FINISH;

  case SD_INIT_SEND_ACMD41_HCS:
    TRACE("SD_INIT_SEND_ACMD41_HCS");
    {
      uint32_t acmd41_hcs_retry_timeout = timersGetUsTick();
      do {
        uint8_t acmd41hcs_r1 = sdcard_spi_send_acmd(spi, SD_CMD_41, SD_ACMD_41_ARG_HC, 0);
        if (R1_VALID(acmd41hcs_r1) && !R1_ERROR(acmd41hcs_r1) &&
            !R1_IDLE_BIT_SET(acmd41hcs_r1)) {
          TRACE("ACMD41: [OK]");
          return SD_INIT_SEND_CMD58;
        }
      } while (timersGetUsTick() - acmd41_hcs_retry_timeout < INIT_CMD_RETRY_US);
    }
    stm32_spi_unselect(spi);
    TRACE("ACMD41_HCS: [ERROR]");
    return SD_INIT_CARD_UNKNOWN;

  case SD_INIT_SEND_ACMD41:
    TRACE("SD_INIT_SEND_ACMD41");
    {
      uint32_t acmd41_retry_timeout = timersGetUsTick();
      do {
        uint8_t acmd41_r1 =
          sdcard_spi_send_acmd(spi, SD_CMD_41, SD_CMD_NO_ARG, 0);
        if (R1_VALID(acmd41_r1) && !R1_ERROR(acmd41_r1) &&
            !R1_IDLE_BIT_SET(acmd41_r1)) {
          TRACE("ACMD41: [OK]");
          card->use_block_addr = false;
          card->card_type = SD_V1;
          return SD_INIT_SEND_CMD16;
        }
      } while (timersGetUsTick() - acmd41_retry_timeout < INIT_CMD_RETRY_US);
    }
    TRACE("ACMD41: [ERROR]");
    return SD_INIT_SEND_CMD1;

  case SD_INIT_SEND_CMD1:
    TRACE("SD_INIT_SEND_CMD1");
    TRACE("COULD TRY CMD1 (for MMC-card)-> currently not supported");
    stm32_spi_unselect(spi);
    return SD_INIT_CARD_UNKNOWN;

  case SD_INIT_SEND_CMD58:
    TRACE("SD_INIT_SEND_CMD58");
    {
      uint8_t cmd58_r1 = sdcard_spi_send_cmd(spi, SD_CMD_58, SD_CMD_NO_ARG,
                                             INIT_CMD_RETRY_US);
      if (R1_VALID(cmd58_r1) && !R1_ERROR(cmd58_r1)) {
        TRACE("CMD58: [OK]");
        card->card_type = SD_V2;

        uint8_t r3[4];
        if (_transfer_bytes(spi, nullptr, r3, sizeof(r3)) == sizeof(r3)) {
          uint32_t ocr = ((uint32_t)r3[0] << (3 * 8)) |
            ((uint32_t)r3[1] << (2 * 8)) | (r3[2] << 8) | r3[3];
          TRACE("R3 RESPONSE: 0x%02x 0x%02x 0x%02x 0x%02x",
                r3[0], r3[1], r3[2], r3[3]);
          TRACE("OCR: 0x%" PRIx32, ocr);

          if ((ocr & SYSTEM_VOLTAGE) != 0) {
            TRACE("OCR: SYS VOLTAGE SUPPORTED");

            /* if power up outine is finished */
            if ((ocr & OCR_POWER_UP_STATUS) != 0) {
              TRACE("OCR: POWER UP ROUTINE FINISHED");
              /* if sd card is sdhc */
              if ((ocr & OCR_CCS) != 0) {
                TRACE("OCR: CARD TYPE IS SDHC (SD_V2 with block addressing)");
                card->use_block_addr = true;
                stm32_spi_unselect(spi);
                return SD_INIT_READ_CID;
              }

              TRACE("OCR: CARD TYPE IS SDSC (SD_v2 with byte addressing)");
              card->use_block_addr = false;
              return SD_INIT_SEND_CMD16;
            }

            TRACE("OCR: POWER UP ROUTINE NOT FINISHED!");
            /* poll status till power up is finished */
            return SD_INIT_SEND_CMD58;
          }

          TRACE("OCR: SYS VOLTAGE NOT SUPPORTED!");
        }

        TRACE("CMD58 response: [READ ERROR]");
      }
    }

    TRACE("CMD58: [ERROR]");
    stm32_spi_unselect(spi);
    return SD_INIT_CARD_UNKNOWN;

  case SD_INIT_SEND_CMD16:
    TRACE("SD_INIT_SEND_CMD16");
    {
      uint8_t r1_16 = sdcard_spi_send_cmd(spi, SD_CMD_16, SD_HC_BLOCK_SIZE,
                                          INIT_CMD_RETRY_US);
      if (R1_VALID(r1_16) && !R1_ERROR(r1_16)) {
        TRACE("CARD TYPE IS SDSC (SD_V1 with byte addressing)");
        stm32_spi_unselect(spi);
        return SD_INIT_READ_CID;
      } else {
        stm32_spi_unselect(spi);
        return SD_INIT_CARD_UNKNOWN;
      }
    }
    // unreachable
    break;

  case SD_INIT_READ_CID:
    TRACE("SD_INIT_READ_CID");
    if (_read_cid(spi, card) == SD_RW_OK) {
      return SD_INIT_READ_CSD;
    } else {
      TRACE("reading cid register failed!");
      return SD_INIT_CARD_UNKNOWN;
    }

  case SD_INIT_READ_CSD:
    TRACE("SD_INIT_READ_CSD");
    if (_read_csd(spi, card) == SD_RW_OK) {
      if (card->csd_structure == SD_CSD_V1) {
        TRACE("csd_structure is version 1");
      } else if (card->csd_structure == SD_CSD_V2) {
        TRACE("csd_structure is version 2");
      }
      return SD_INIT_SET_MAX_SPI_SPEED;
    } else {
      TRACE("reading csd register failed!");
      return SD_INIT_CARD_UNKNOWN;
    }

  case SD_INIT_SET_MAX_SPI_SPEED:
    TRACE("SD_INIT_SET_MAX_SPI_SPEED");
    stm32_spi_set_max_baudrate(spi, SD_SPI_CLK_MAX);
    TRACE("SD_INIT_SET_MAX_SPI_SPEED: [OK]");
    return SD_INIT_FINISH;

  default:
    TRACE("SD-INIT-FSM REACHED INVALID STATE!");
    return SD_INIT_CARD_UNKNOWN;
  }
}

int sdcard_spi_init(const stm32_spi_t* spi, sdcard_info_t* card)
{
  sd_init_fsm_state_t state = SD_INIT_START;

  do {
    state = _init_sd_fsm_step(spi, card, state);
  } while (state != SD_INIT_FINISH);

  if (card->card_type != SD_UNKNOWN) {
    _sdcard_spi.spi = spi;
    _sdcard_spi.use_block_addr = card->use_block_addr;
    return SDCARD_SPI_OK;
  }

  TRACE("sdcard_spi_init: [FAILED]");
  return SDCARD_SPI_INIT_ERROR;
}

static sd_rw_response_t _read_data_packet(const stm32_spi_t* spi, uint8_t token,
                                          uint8_t *data, uint16_t size)
{
  if (!_wait_for_token(spi, token, SD_DATA_TOKEN_RETRY_US)) {
    return SD_RW_NO_TOKEN;
  }

  if (stm32_spi_dma_receive_bytes(spi, data, size) == size) {

    uint8_t crc_bytes[2];
    if (_transfer_bytes(spi, nullptr, crc_bytes, sizeof(crc_bytes)) == sizeof(crc_bytes)) {

#if defined(SD_CARD_SPI_ENABLE_CRC)
      uint16_t data_crc16 = (crc_bytes[0] << 8) | crc_bytes[1];
      if (crc16(CRC_1021, data, size) != data_crc16) {
        return SD_RW_CRC_MISMATCH;
      }
#endif
      return SD_RW_OK;
    }

    return SD_RW_RX_TX_ERROR;
  }

  return SD_RW_RX_TX_ERROR;
}

static uint16_t _read_blocks(const sdcard_spi_t* card, uint8_t cmd_idx,
                             uint32_t bladdr, uint8_t* data,
                             uint16_t blsz, uint16_t nbl,
                             sd_rw_response_t* state)
{
  const auto* spi = card->spi;
  stm32_spi_select(spi);
  uint16_t reads = 0;

  uint32_t addr = card->use_block_addr ? bladdr : (bladdr * SD_HC_BLOCK_SIZE);
  uint8_t cmd_r1_resu = sdcard_spi_send_cmd(spi, cmd_idx, addr, SD_BLOCK_READ_CMD_RETRY_US);

  if (R1_VALID(cmd_r1_resu) && !R1_ERROR(cmd_r1_resu)) {

    for (uint16_t i = 0; i < nbl; i++) {
      *state = _read_data_packet(spi, SD_DATA_TOKEN_CMD_17_18_24, &(data[i * blsz]), blsz);
      if (*state != SD_RW_OK) { break; }
      reads++;
    }

    /* if this was a multi-block read */
    if (cmd_idx == SD_CMD_18) {
      cmd_r1_resu = sdcard_spi_send_cmd(spi, SD_CMD_12, 0, 1);
      if (!R1_VALID(cmd_r1_resu) || R1_ERROR(cmd_r1_resu)) {
        *state =  SD_RW_RX_TX_ERROR;
      }
    }
  }
  else {
    TRACE("_read_blocks: send CMD%d: [RX_TX_ERROR]", cmd_idx);
    *state = SD_RW_RX_TX_ERROR;
  }

  stm32_spi_unselect(spi);
  _send_dummy_byte(spi);
  // TODO: delay 100us?
  delay_us(100);

  return reads;
}

int sdcard_spi_read_blocks(uint32_t blockaddr, uint8_t* data,
                           uint16_t blocksize, uint16_t nblocks,
                           sd_rw_response_t* state)
{
  *state = SD_RW_OK;
  if (nblocks > 1) {
    return _read_blocks(&_sdcard_spi, SD_CMD_18, blockaddr, data, blocksize,
                        nblocks, state);
  } else {
    return _read_blocks(&_sdcard_spi, SD_CMD_17, blockaddr, data, blocksize,
                        nblocks, state);
  }
}

static sd_rw_response_t _write_data_packet(const stm32_spi_t* spi, uint8_t token,
                                           const uint8_t *data, uint16_t size)
{
  if (!_wait_for_not_busy(spi, SD_WAIT_FOR_NOT_BUSY_US)) {
    return SD_RW_TIMEOUT;
  }

  stm32_spi_transfer_byte(spi, token);

  if (stm32_spi_dma_transmit_bytes(spi, data, size) != size) {
    TRACE("_write_data_packet: [RX_TX_ERROR] (while transmitting payload)");
    return SD_RW_RX_TX_ERROR;
  }

#if defined(SD_CARD_SPI_ENABLE_CRC)
  uint16_t data_crc16 = crc16(CRC_1021, data, size);
  uint8_t crc[sizeof(uint16_t)] = {
    (uint8_t)(data_crc16 >> 8),
    (uint8_t)(data_crc16 & 0xFF)
  };
#else
  uint8_t crc[sizeof(uint16_t)] = { 0xFF, 0xFF };
#endif

  if (_transfer_bytes(spi, crc, nullptr, sizeof(crc)) != sizeof(crc)) {
    TRACE("_write_data_packet: [RX_TX_ERROR] (while transmitting CRC16)");
    return SD_RW_RX_TX_ERROR;
  }

  uint32_t timeout = timersGetUsTick();

  do {
    uint8_t data_response = stm32_spi_transfer_byte(spi, SD_CARD_DUMMY_BYTE);
    if (data_response == 0xFF) continue;

    if (!DATA_RESPONSE_IS_VALID(data_response)) {
      return SD_RW_RX_TX_ERROR;
    }

    if (DATA_RESPONSE_ACCEPTED(data_response)) {
      return SD_RW_OK;
    }

    if (DATA_RESPONSE_WRITE_ERR(data_response) ||
        DATA_RESPONSE_CRC_ERR(data_response)) {
      break;
    }

  } while(timersGetUsTick() - timeout < 10 * US_PER_MS);

  return SD_RW_WRITE_ERROR;
}

static uint16_t _write_blocks(const sdcard_spi_t* card, uint8_t cmd_idx,
                              uint32_t bladdr, const uint8_t *data,
                              uint16_t blsz, uint16_t nbl,
                              sd_rw_response_t *state)
{
  const auto* spi = card->spi;
  stm32_spi_select(spi);
  uint16_t written = 0;

  uint8_t token;
  if (cmd_idx == SD_CMD_25) {
    token = SD_DATA_TOKEN_CMD_25;
    sdcard_spi_send_acmd(spi, SD_CMD_23, nbl, SD_BLOCK_WRITE_CMD_RETRY_US);
  } else {
    token = SD_DATA_TOKEN_CMD_17_18_24;
  }

  uint32_t addr = card->use_block_addr ? bladdr : (bladdr * SD_HC_BLOCK_SIZE);
  uint8_t cmd_r1_resu = sdcard_spi_send_cmd(spi, cmd_idx, addr, SD_BLOCK_WRITE_CMD_RETRY_US);

  if (R1_VALID(cmd_r1_resu) && !R1_ERROR(cmd_r1_resu)) {

    for (uint16_t i = 0; i < nbl; i++) {
      *state = _write_data_packet(spi, token, &(data[i * blsz]), blsz);
      if (*state != SD_RW_OK) {
        break;
      }
      written++;
    }

    /* if this is a multi-block write it is needed to issue a stop
       command */
    if (cmd_idx == SD_CMD_25) {
      if (!_wait_for_not_busy(spi, SD_WAIT_FOR_NOT_BUSY_US)) {
        *state = SD_RW_TIMEOUT;
      } else {
        stm32_spi_transfer_byte(spi, SD_DATA_TOKEN_CMD_25_STOP);
      }
    }
  }
  else {
    *state = SD_RW_RX_TX_ERROR;
  }

  stm32_spi_unselect(spi);
  _send_dummy_byte(spi);
  // TODO: delay 100us?
  delay_us(100);

  return written;
}

int sdcard_spi_write_blocks(uint32_t blockaddr, const uint8_t* data,
                            uint16_t blocksize, uint16_t nblocks,
                            sd_rw_response_t* state)
{
  *state = SD_RW_OK;
  if (nblocks > 1) {
    return _write_blocks(&_sdcard_spi, SD_CMD_25, blockaddr, data, blocksize,
                         nblocks, state);
  } else {
    return _write_blocks(&_sdcard_spi, SD_CMD_24, blockaddr, data, blocksize,
                         nblocks, state);
  }
}

int sdcard_spi_wait_for_not_busy()
{
  auto spi = _sdcard_spi.spi;
  int res = -1;

  stm32_spi_select(spi);
  if (_wait_for_not_busy(spi, SD_WAIT_FOR_NOT_BUSY_US)) {
    res = 0;
  }
  stm32_spi_unselect(spi);

  return res;
}

static sd_rw_response_t _read_cid(const stm32_spi_t* spi, sdcard_info_t* card)
{
  uint8_t cid_raw_data[SD_SIZE_OF_CID_AND_CSD_REG];
  sd_rw_response_t state;

  sdcard_spi_t card_spi = { spi, card->use_block_addr };
  int nbl = _read_blocks(&card_spi, SD_CMD_10, 0, cid_raw_data, SD_SIZE_OF_CID_AND_CSD_REG,
                         SD_BLOCKS_FOR_REG_READ, &state);

  uint8_t crc7 = _crc_7(&(cid_raw_data[0]), SD_SIZE_OF_CID_AND_CSD_REG - 1);

  if (nbl == SD_BLOCKS_FOR_REG_READ) {
    if (crc7 == cid_raw_data[SD_SIZE_OF_CID_AND_CSD_REG - 1]) {
      card->cid.MID = cid_raw_data[0];
      memcpy(&card->cid.OID[0], &cid_raw_data[1], SD_SIZE_OF_OID);
      memcpy(&card->cid.PNM[0], &cid_raw_data[2], SD_SIZE_OF_PNM);
      card->cid.PRV = cid_raw_data[8];
      memcpy((uint8_t *)&card->cid.PSN, &cid_raw_data[9], 4);
      card->cid.MDT = (cid_raw_data[13] << 4) | cid_raw_data[14];
      card->cid.CID_CRC = cid_raw_data[15];
      TRACE("_read_cid: [OK]");
      return SD_RW_OK;
    }
    else {
      TRACE("_read_cid: [SD_RW_CRC_MISMATCH] (data-crc: 0x%02x | calc-crc: 0x%02x)",
            cid_raw_data[SD_SIZE_OF_CID_AND_CSD_REG - 1], crc7);
      return SD_RW_CRC_MISMATCH;
    }
  }
  return state;
}

static sd_rw_response_t _read_csd(const stm32_spi_t* spi, sdcard_info_t* card)
{
  uint8_t c[SD_SIZE_OF_CID_AND_CSD_REG];
  sd_rw_response_t state;

  sdcard_spi_t card_spi = { spi, card->use_block_addr };
  int read_resu = _read_blocks(&card_spi, SD_CMD_9, 0, c, SD_SIZE_OF_CID_AND_CSD_REG,
                               SD_BLOCKS_FOR_REG_READ, &state);

  if (read_resu == SD_BLOCKS_FOR_REG_READ) {
    if (_crc_7(c, SD_SIZE_OF_CID_AND_CSD_REG - 1) == c[SD_SIZE_OF_CID_AND_CSD_REG - 1]) {
      if (SD_GET_CSD_STRUCTURE(c) == SD_CSD_V1) {
        card->csd.v1.CSD_STRUCTURE = c[0] >> 6;
        card->csd.v1.TAAC = c[1];
        card->csd.v1.NSAC = c[2];
        card->csd.v1.TRAN_SPEED = c[3];
        card->csd.v1.CCC = (c[4] << 4) | ((c[5] & 0xF0) >> 4);
        card->csd.v1.READ_BL_LEN = (c[5] & 0x0F);
        card->csd.v1.READ_BL_PARTIAL = (c[6] & (1 << 7)) >> 7;
        card->csd.v1.WRITE_BLK_MISALIGN = (c[6] & (1 << 6)) >> 6;
        card->csd.v1.READ_BLK_MISALIGN = (c[6] & (1 << 5)) >> 5;
        card->csd.v1.DSR_IMP = (c[6] & (1 << 4)) >> 4;
        card->csd.v1.C_SIZE = ((c[6] & 0x03) << 10) | (c[7] << 2) | (c[8] >> 6);
        card->csd.v1.VDD_R_CURR_MIN = (c[8] & 0x38) >> 3;
        card->csd.v1.VDD_R_CURR_MAX = (c[8] & 0x07);
        card->csd.v1.VDD_W_CURR_MIN = (c[9] & 0xE0) >> 5;
        card->csd.v1.VDD_W_CURR_MAX = (c[9] & 0x1C) >> 2;
        card->csd.v1.C_SIZE_MULT = ((c[9] & 0x03) << 1) | (c[10] >> 7);
        card->csd.v1.ERASE_BLK_EN = (c[10] & (1 << 6)) >> 6;
        card->csd.v1.SECTOR_SIZE = ((c[10] & 0x3F) << 1) | (c[11] >> 7);
        card->csd.v1.WP_GRP_SIZE = (c[11] & 0x7F);
        card->csd.v1.WP_GRP_ENABLE = c[12] >> 7;
        card->csd.v1.R2W_FACTOR = (c[12] & 0x1C) >> 2;
        card->csd.v1.WRITE_BL_LEN = (c[12] & 0x03) << 2 | (c[13] >> 6);
        card->csd.v1.WRITE_BL_PARTIAL = (c[13] & (1 << 5)) >> 5;
        card->csd.v1.FILE_FORMAT_GRP = (c[14] & (1 << 7)) >> 7;
        card->csd.v1.COPY = (c[14] & (1 << 6)) >> 6;
        card->csd.v1.PERM_WRITE_PROTECT = (c[14] & (1 << 5)) >> 5;
        card->csd.v1.TMP_WRITE_PROTECT = (c[14] & (1 << 4)) >> 4;
        card->csd.v1.FILE_FORMAT = (c[14] & 0x0C) >> 2;
        card->csd.v1.CSD_CRC = c[15];
        card->csd_structure = SD_CSD_V1;
        return SD_RW_OK;
      } else if (SD_GET_CSD_STRUCTURE(c) == SD_CSD_V2) {
        card->csd.v2.CSD_STRUCTURE = c[0] >> 6;
        card->csd.v2.TAAC = c[1];
        card->csd.v2.NSAC = c[2];
        card->csd.v2.TRAN_SPEED = c[3];
        card->csd.v2.CCC = (c[4] << 4) | ((c[5] & 0xF0) >> 4);
        card->csd.v2.READ_BL_LEN = (c[5] & 0x0F);
        card->csd.v2.READ_BL_PARTIAL = (c[6] & (1 << 7)) >> 7;
        card->csd.v2.WRITE_BLK_MISALIGN = (c[6] & (1 << 6)) >> 6;
        card->csd.v2.READ_BLK_MISALIGN = (c[6] & (1 << 5)) >> 5;
        card->csd.v2.DSR_IMP = (c[6] & (1 << 4)) >> 4;
        card->csd.v2.C_SIZE = (((uint32_t)c[7] & 0x3F) << 16) | (c[8] << 8) | c[9];
        card->csd.v2.ERASE_BLK_EN = (c[10] & (1 << 6)) >> 6;
        card->csd.v2.SECTOR_SIZE = (c[10] & 0x3F) << 1 | (c[11] >> 7);
        card->csd.v2.WP_GRP_SIZE = (c[11] & 0x7F);
        card->csd.v2.WP_GRP_ENABLE = (c[12] & (1 << 7)) >> 7;
        card->csd.v2.R2W_FACTOR = (c[12] & 0x1C) >> 2;
        card->csd.v2.WRITE_BL_LEN = ((c[12] & 0x03) << 2) | (c[13] >> 6);
        card->csd.v2.WRITE_BL_PARTIAL = (c[13] & (1 << 5)) >> 5;
        card->csd.v2.FILE_FORMAT_GRP = (c[14] & (1 << 7)) >> 7;
        card->csd.v2.COPY = (c[14] & (1 << 6)) >> 6;
        card->csd.v2.PERM_WRITE_PROTECT = (c[14] & (1 << 5)) >> 5;
        card->csd.v2.TMP_WRITE_PROTECT = (c[14] & (1 << 4)) >> 4;
        card->csd.v2.FILE_FORMAT = (c[14] & 0x0C) >> 2;
        card->csd.v2.CSD_CRC = c[15];
        card->csd_structure = SD_CSD_V2;
        return SD_RW_OK;
      } else {
        return SD_RW_NOT_SUPPORTED;
      }

    } else {
      return SD_RW_CRC_MISMATCH;
    }
  }
  return state;
}

uint64_t sdcard_spi_get_capacity(sdcard_info_t *card)
{
  if (card->csd_structure == SD_CSD_V1) {
    uint32_t block_len = (1 << card->csd.v1.READ_BL_LEN);
    uint32_t mult = 1 << (card->csd.v1.C_SIZE_MULT + 2);
    uint32_t blocknr = (card->csd.v1.C_SIZE + 1) * mult;
    return blocknr * block_len;
  }
  else if (card->csd_structure == SD_CSD_V2) {
    return (card->csd.v2.C_SIZE + 1) * (((uint64_t)SD_HC_BLOCK_SIZE) << 10);
  }
  return 0;
}

uint32_t sdcard_spi_get_sector_count(sdcard_info_t *card)
{
  return sdcard_spi_get_capacity(card) / SD_HC_BLOCK_SIZE;
}
