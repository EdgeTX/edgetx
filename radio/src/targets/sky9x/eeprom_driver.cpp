/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"

volatile uint8_t Spi_complete = 1;
uint8_t Spi_tx_buf[24];

uint8_t eepromIsTransferComplete()
{
  return Spi_complete;
}

uint32_t eepromTransmitData(uint8_t *command, uint8_t *tx, uint8_t *rx, uint32_t comlen, uint32_t count)
{
  Spi * spiptr = SPI;
  uint32_t condition;
  static uint8_t discard_rx_command[4];

  if (comlen > 4) {
    return 0x4FFFF;
  }

  Spi_complete = 0;
  condition = SPI_SR_TXEMPTY;
  spiptr->SPI_CR = 1; // Enable
  (void) spiptr->SPI_RDR; // Dump any rx data
  (void) spiptr->SPI_SR; // Clear error flags
  spiptr->SPI_RPR = (uint32_t)discard_rx_command;
  spiptr->SPI_RCR = comlen;
  if (rx) {
    spiptr->SPI_RNPR = (uint32_t)rx;
    spiptr->SPI_RNCR = count;
    condition = SPI_SR_RXBUFF;
  }
  spiptr->SPI_TPR = (uint32_t)command;
  spiptr->SPI_TCR = comlen;
  if (tx) {
    spiptr->SPI_TNPR = (uint32_t)tx;
  }
  else {
    spiptr->SPI_TNPR = (uint32_t)rx;
  }
  spiptr->SPI_TNCR = count;

  spiptr->SPI_PTCR = SPI_PTCR_RXTEN | SPI_PTCR_TXTEN; // Start transfers

  // Wait for things to get started, avoids early interrupt
  for (count = 0; count < 1000; count += 1) {
    if ((spiptr->SPI_SR & SPI_SR_TXEMPTY) == 0)
    {
      break;
    }
  }
  spiptr->SPI_IER = condition;

  return 0;
}

uint8_t eepromTransmitByte(uint8_t out, bool skipFirst)
{
  Spi * spiptr = SPI;
  uint32_t delay;

  spiptr->SPI_CR = 1; // Enable
  (void) spiptr->SPI_RDR; // Dump any rx data

  spiptr->SPI_TDR = out;

  delay = 0;
  while ((spiptr->SPI_SR & SPI_SR_RDRF) == 0) {
    // wait for received
    if (++delay > 10000) {
      break;
    }
  }
  
  if (skipFirst) {
    (void) spiptr->SPI_RDR; // Dump the rx data
    spiptr->SPI_TDR = 0;
    delay = 0;
    while ((spiptr->SPI_SR & SPI_SR_RDRF) == 0) {
      // wait for received
      if (++delay > 10000) {
        break;
      }
    }
  }
  
  spiptr->SPI_CR = 2; // Disable
  return spiptr->SPI_RDR;
}

enum EepromCommand {
  COMMAND_WRITE_STATUS_REGISTER = 0x01,
  COMMAND_BYTE_PROGRAM = 0x02,
  COMMAND_READ_ARRAY = 0x03,
  COMMAND_READ_STATUS = 0x05,
  COMMAND_WRITE_ENABLE = 0x06,
  COMMAND_BLOCK_ERASE = 0x20,
};

void eepromPrepareCommand(EepromCommand command, uint32_t address)
{
  uint8_t * p = Spi_tx_buf;
  *p = command;
  *(p+1) = address >> 16;
  *(p+2) = address >> 8;
  *(p+3) = address;
}

uint8_t eepromReadStatus()
{
  uint8_t read_status = eepromTransmitByte(COMMAND_READ_STATUS, true);
  return !(read_status & 1);
}

void eepromWriteStatusRegister()
{
  eepromTransmitByte(COMMAND_WRITE_STATUS_REGISTER, true);
}

void eepromWriteEnable()
{
  eepromTransmitByte(COMMAND_WRITE_ENABLE, false);
}

void eepromStartWrite(uint8_t * buffer, size_t address, size_t size)
{
  eepromWriteEnable();
  eepromPrepareCommand(COMMAND_BYTE_PROGRAM, address);
  eepromTransmitData(Spi_tx_buf, buffer, 0, 4, size);
}

void eepromStartRead(uint8_t * buffer, size_t address, size_t size)
{
  eepromPrepareCommand(COMMAND_READ_ARRAY, address);
  eepromTransmitData(Spi_tx_buf, 0, buffer, 4, size);
}

void eepromBlockErase(uint32_t address)
{
  eepromWriteEnable();
  eepromPrepareCommand(COMMAND_BLOCK_ERASE, address);
  eepromTransmitData(Spi_tx_buf, 0, 0, 4, 0);
}

// SPI i/f to EEPROM (4Mb)
// Peripheral ID 21 (0x00200000)
// Connections:
// SS   PA11 (peripheral A)
// MISO PA12 (peripheral A)
// MOSI PA13 (peripheral A)
// SCK  PA14 (peripheral A)
// Set clock to 3 MHz, AT25 device is rated to 70MHz, 18MHz would be better
void eepromInit()
{
  Spi * spiptr = SPI;
  uint32_t timer;

  PMC->PMC_PCER0 |= 0x00200000L; // Enable peripheral clock to SPI
  /* Configure PIO */
  configure_pins(0x00007800, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA | PIN_NO_PULLUP);

  timer = (Master_frequency / 3000000) << 8; // Baud rate 3Mb/s
  spiptr->SPI_MR = 0x14000011; // 0001 0100 0000 0000 0000 0000 0001 0001 Master
  spiptr->SPI_CSR[0] = 0x01180009 | timer; // 0000 0001 0001 1000 xxxx xxxx 0000 1001
  NVIC_EnableIRQ(SPI_IRQn);

  eepromWriteEnable();
  eepromWriteStatusRegister();
}

extern "C" void SPI_IRQHandler()
{
  Spi * spiptr = SPI;
  SPI->SPI_IDR = 0x07FF; // All interrupts off
  spiptr->SPI_CR = 2; // Disable
  (void) spiptr->SPI_RDR; // Dump any rx data
  (void) spiptr->SPI_SR; // Clear error flags
  spiptr->SPI_PTCR = SPI_PTCR_RXTDIS | SPI_PTCR_TXTDIS; // Stop tramsfers
  Spi_complete = 1; // Indicate completion
}
