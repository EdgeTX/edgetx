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

// TODO merge this file with board_sky9x.h

#ifndef _BOARD_H_
#define _BOARD_H_

#include <stdint.h>
#include "chip.h"
#include "AT91SAM3S4.h"

#if !defined(SIMU)
#include "core_cm3.h"
#endif

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par
/// This page lists several definition related to the board description.
///
/// Definitions
/// - BOARD_NAME

/// Name of the board.
// #define BOARD_NAME "AT91SAM3S-EK"
/// Board definition.
#define at91sam3sek
/// Family definition (already defined).
#define at91sam3s
/// Core definition
#define cortexm3

// TODO remove these definitions
//#define BOARD_REV_A
#define BOARD_REV_B

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par sam3s_ek_opfreq "SAM3S-EK - Operating frequencies"
/// This page lists several definition related to the board operating frequency
/// (when using the initialization done by board_lowlevel.c).
///
/// !Definitions
/// - BOARD_MAINOSC
/// - BOARD_MCK

/// Frequency of the board main oscillator.
#define BOARD_MAINOSC           12000000

/// Master clock frequency (when using board_lowlevel.c).
// #define BOARD_MCK               64000000

//------------------------------------------------------------------------------
// ADC
//------------------------------------------------------------------------------

/// Startup time max, return from Idle mode (in �s)
#define ADC_STARTUP_TIME_MAX       15
/// Track and hold Acquisition Time min (in ns)
#define ADC_TRACK_HOLD_TIME_MIN  1200

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// USB attributes configuration descriptor (bus or self powered, remote wakeup)
//#define BOARD_USB_BMATTRIBUTES              USBConfigurationDescriptor_SELFPOWERED_RWAKEUP
#define BOARD_USB_BMATTRIBUTES            USBConfigurationDescriptor_SELFPOWERED_NORWAKEUP
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par sam3s_ek_piodef "SAM3S-EK - PIO definitions"
/// This pages lists all the pio definitions contained in board.h. The constants
/// are named using the following convention: PIN_* for a constant which defines
/// a single Pin instance (but may include several PIOs sharing the same
/// controller), and PINS_* for a list of Pin instances.
///
/// !ADC
/// - PIN_ADC0_AD0
/// - PIN_ADC0_AD1
/// - PIN_ADC0_AD2
/// - PIN_ADC0_AD3
/// - PIN_ADC0_AD4
/// - PIN_ADC0_AD5
/// - PIN_ADC0_AD6
/// - PIN_ADC0_AD7
/// - PINS_ADC0
///
/// !UART
/// - PINS_UART
///
/// !EBI
/// - PIN_EBI_DATA_BUS
/// - PIN_EBI_NCS0
/// - PIN_EBI_NRD
/// - PIN_EBI_NWE
/// - PIN_EBI_ADDR_BUS
/// - PIN_EBI_PSRAM_NBS
/// - PIN_EBI_A1
/// - PIN_EBI_LCD_RS
///
/// !LEDs
/// - PIN_LED_0
/// - PIN_LED_1
/// - PIN_LED_2
/// - PINS_LEDS
///
/// !MCI
/// - PINS_MCI
///
/// !Push buttons
/// - PIN_PUSHBUTTON_1
/// - PIN_PUSHBUTTON_2
/// - PINS_PUSHBUTTONS
/// - PUSHBUTTON_BP1
/// - PUSHBUTTON_BP2
///
/// !PWMC
/// - PIN_PWMC_PWMH0
/// - PIN_PWMC_PWML0
/// - PIN_PWMC_PWMH1
/// - PIN_PWMC_PWML1
/// - PIN_PWMC_PWMH2
/// - PIN_PWMC_PWML2
/// - PIN_PWMC_PWMH3
/// - PIN_PWMC_PWML3
/// - PIN_PWM_LED0
/// - PIN_PWM_LED1
/// - PIN_PWM_LED2
/// - CHANNEL_PWM_LED0
/// - CHANNEL_PWM_LED1
/// - CHANNEL_PWM_LED2
///
/// !SPI
/// - PIN_SPI_MISO
/// - PIN_SPI_MOSI
/// - PIN_SPI_SPCK
/// - PINS_SPI
/// - PIN_SPI_NPCS0_PA11
///
/// ! SSC
/// - PIN_SSC_TD
/// - PIN_SSC_TK
/// - PIN_SSC_TF
/// - PINS_SSC_CODEC
///
/// ! PCK0
/// - PIN_PCK0
///
/// !TWI
/// - PIN_TWI_TWD0
/// - PIN_TWI_TWCK0
/// - PINS_TWI
///
/// !USART0
/// - PIN_USART0_RXD
/// - PIN_USART0_TXD
/// - PIN_USART0_CTS
/// - PIN_USART0_RTS
/// - PIN_USART0_SCK
///
/// !USB
/// - PIN_USB_PULLUP
///

/// ADC_AD0 pin definition.
#define PIN_ADC0_AD0 {1 << 21, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD1 pin definition.
#define PIN_ADC0_AD1 {1 << 30, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD2 pin definition.
#define PIN_ADC0_AD2 {1 << 3, PIOB, ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD3 pin definition.
#define PIN_ADC0_AD3 {1 << 4, PIOB, ID_PIOB, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD4 pin definition.
#define PIN_ADC0_AD4 {1 << 15, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD5 pin definition.
#define PIN_ADC0_AD5 {1 << 16, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD6 pin definition.
#define PIN_ADC0_AD6 {1 << 17, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
/// ADC_AD7 pin definition.
#define PIN_ADC0_AD7 {1 << 18, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}

/// Pins ADC
#define PINS_ADC PIN_ADC0_AD0, PIN_ADC0_AD1, PIN_ADC0_AD2, PIN_ADC0_AD3, PIN_ADC0_AD4, PIN_ADC0_AD5, PIN_ADC0_AD6, PIN_ADC0_AD7

/** UART pins (UTXD0 and URXD0) definitions, PA9,10. */
#define PINS_UART  {0x00000600, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/// EBI
#define PIN_EBI_DATA_BUS            {0xFF,    PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NRD                 {1 << 11, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NWE                 {1 << 8,  PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}

#define PIN_EBI_NCS0                {1 << 20, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_PSRAM_ADDR_BUS      {0x3f00fff, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_PSRAM_NBS           {1 << 7, PIOB, ID_PIOB, PIO_PERIPH_B, PIO_PULLUP}, \
                                    {1 << 15, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_A1                  {1 << 19, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}

#define PIN_EBI_NCS1                {1 << 15, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP} /* LCD CS pin */
#define PIN_EBI_LCD_RS              {1 << 19, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP} /* LCD RS pin */

#ifdef BOARD_REV_A
/** LED #0 pin definition. */
#define PIN_LED_0   {1 << 20, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
/** LED #1 pin definition. */
#define PIN_LED_1   {1 << 21, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
/** LED #2 pin definition. */
#define PIN_LED_2   {1 << 22, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#endif
#ifdef BOARD_REV_B
/** LED #0 pin definition. */
#define PIN_LED_0   {1 << 19, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
/** LED #1 pin definition. */
#define PIN_LED_1   {1 << 20, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
/** LED #2 pin definition. */
#define PIN_LED_2   {1 << 20, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#endif

/** List of all LEDs definitions. */
#define PINS_LEDS   PIN_LED_0, PIN_LED_1, PIN_LED_2

/** MCI pins definition. */
#define PINS_MCI   {0x3fUL << 26, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_PULLUP}
/** MCI pin Card Detect. */
#define PIN_MCI_CD {PIO_PB7, PIOB, ID_PIOB, PIO_INPUT, PIO_PULLUP}

/** Push button #0 definition. Attributes = pull-up + debounce + interrupt on rising edge. */
#define PIN_PUSHBUTTON_1    {1 << 3, PIOB, ID_PIOB, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE}
/** Push button #1 definition. Attributes = pull-up + debounce + interrupt on falling edge. */
#define PIN_PUSHBUTTON_2    {1 << 12, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE}
/** List of all push button definitions. */
#define PINS_PUSHBUTTONS    PIN_PUSHBUTTON_1, PIN_PUSHBUTTON_2

/** Push button #1 index. */
#define PUSHBUTTON_BP1   0
/** Push button #2 index. */
#define PUSHBUTTON_BP2   1

/// Simulated joystick LEFT index.
#define JOYSTICK_LEFT       0
/// Simulated joystick RIGHT index.
#define JOYSTICK_RIGHT      1

/** PWMC PWM0 pin definition. */
#define PIN_PWMC_PWMH0  {1 << 18, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWMC_PWML0  {1 << 19, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** PWMC PWM1 pin definition. */
#define PIN_PWMC_PWMH1  {1 << 19, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWMC_PWML1  {1 << 20, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
/** PWMC PWM2 pin definition. */
#define PIN_PWMC_PWMH2  {1 << 20, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWMC_PWML2  {1 << 16, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}
/** PWMC PWM3 pin definition. */
#define PIN_PWMC_PWMH3  {1 << 21, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWMC_PWML3  {1 << 15, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}
/** PWM pin definition for LED0 */
#define PIN_PWM_LED0 PIN_PWMC_PWMH0, PIN_PWMC_PWML0
/** PWM pin definition for LED1 */
#define PIN_PWM_LED1 PIN_PWMC_PWMH2, PIN_PWMC_PWML2
/** PWM pin definition for LED2 */
#define PIN_PWM_LED2 PIN_PWMC_PWMH3, PIN_PWMC_PWML3
/** PWM channel for LED0 */
#define CHANNEL_PWM_LED0 0
/** PWM channel for LED1 */
#define CHANNEL_PWM_LED1 2
/** PWM channel for LED2 */
#define CHANNEL_PWM_LED2 3

/** SPI MISO pin definition. */
#define PIN_SPI_MISO  {1 << 12, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI MOSI pin definition. */
#define PIN_SPI_MOSI  {1 << 13, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI SPCK pin definition. */
#define PIN_SPI_SPCK  {1 << 14, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** SPI chip select pin definition. */
#define PIN_SPI_NPCS0_PA11  {1 << 11, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
/** List of SPI pin definitions (MISO, MOSI & SPCK). */
#define PINS_SPI      PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SPCK

/// SSC pins definition.
#define PIN_SSC_TD      {0x1 << 26, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SSC_TK      {0x1 << 28, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SSC_TF      {0x1 << 30, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_SSC_CODEC       PIN_SSC_TD,  PIN_SSC_TK, PIN_SSC_TF

/// PCK0
#define PIN_PCK0        {0x1 << 21, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}

/// TWI pins definition.
#define TWI_V3XX
#define PIN_TWI_TWD0    {0x1 << 9, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_TWI_TWCK0    {0x1 << 10, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_TWI0     PIN_TWI_TWD0, PIN_TWI_TWCK0
#define PIN_TWI_TWD1    {0x1 << 24, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_TWI_TWCK1    {0x1 << 25, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_TWI1     PIN_TWI_TWD1, PIN_TWI_TWCK1

/// USART0
#define PIN_USART0_RXD    {0x1 << 19, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_TXD    {0x1 << 18, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_CTS    {0x1 << 8, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_RTS    {0x1 << 7, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_SCK    {0x1 << 17, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/// USART1
#define PIN_USART1_RXD    {0x1 << 21, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_TXD    {0x1 << 22, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_CTS    {0x1 << 25, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_RTS    {0x1 << 24, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_EN     {0x1 << 23, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}

/// USB VBus monitoring pin definition.
#ifdef BOARD_REV_A
#define PIN_USB_VBUS    {1 << 23, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
#endif
#ifdef BOARD_REV_B
#define PIN_USB_VBUS    {1 << 21, PIOC, ID_PIOC, PIO_INPUT, PIO_DEFAULT}
#endif

#define usbPlugged() (PIOC->PIO_PDSR & PIO_PC25)

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par sam3s_ek_extcomp "SAM3S-EK - External components"
/// This page lists the definitions related to external on-board components
/// located in the board.h file for the AT91SAM3S-EK.
///
/// !AT45 Dataflash Card
/// - BOARD_AT45_A_SPI_BASE
/// - BOARD_AT45_A_SPI_ID
/// - BOARD_AT45_A_SPI_PINS
/// - BOARD_AT45_A_SPI
/// - BOARD_AT45_A_NPCS
/// - BOARD_AT45_A_NPCS_PIN
///
/// !AT45 Dataflash (serial onboard DataFlash)
/// - BOARD_AT45_B_SPI_BASE
/// - BOARD_AT45_B_SPI_ID
/// - BOARD_AT45_B_SPI_PINS
/// - BOARD_AT45_B_SPI
/// - BOARD_AT45_B_NPCS
/// - BOARD_AT45_B_NPCS_PIN
///
/// !AT26 Serial Flash
/// - BOARD_AT26_A_SPI_BASE
/// - BOARD_AT26_A_SPI_ID
/// - BOARD_AT26_A_SPI_PINS
/// - BOARD_AT26_A_SPI
/// - BOARD_AT26_A_NPCS
/// - BOARD_AT26_A_NPCS_PIN
///
/// !SD Card
/// - MCI2_INTERFACE
/// - BOARD_SD_MCI_BASE
/// - BOARD_SD_MCI_ID
/// - BOARD_SD_PINS
/// - BOARD_SD_SLOT
///
/// !PSRAM
/// - BOARD_PSRAM_PINS
///
/// !LCD
/// - BOARD_LCD_ILI9325
/// - BOARD_LCD_PINS
/// - BOARD_BACKLIGHT_PIN
/// - BOARD_LCD_BASE
/// - BOARD_LCD_RS
/// - BOARD_LCD_WIDTH
/// - BOARD_LCD_HEIGHT
///
/// !TouchScreen
/// - BOARD_TSC_ADS7843
/// - PIN_TCS_IRQ
/// - PIN_TCS_BUSY
/// - BOARD_TSC_SPI_BASE
/// - BOARD_TSC_SPI_ID
/// - BOARD_TSC_SPI_PINS
/// - BOARD_TSC_NPCS
/// - BOARD_TSC_NPCS_PIN
///
/// Base address of SPI peripheral connected to the dataflash.
//#define BOARD_AT45_A_SPI_BASE         SPI0
///// Identifier of SPI peripheral connected to the dataflash.
//#define BOARD_AT45_A_SPI_ID           ID_SPI0
///// Pins of the SPI peripheral connected to the dataflash.
//#define BOARD_AT45_A_SPI_PINS         PINS_SPI0
///// Dataflahs SPI number.
//#define BOARD_AT45_A_SPI              0
///// Chip select connected to the dataflash.
//#define BOARD_AT45_A_NPCS             3
///// Chip select pin connected to the dataflash.
//#define BOARD_AT45_A_NPCS_PIN         PIN_SPI0_NPCS3

/// Base address of SPI peripheral connected to the dataflash.
//#define BOARD_AT45_B_SPI_BASE         SPI1
///// Identifier of SPI peripheral connected to the dataflash.
//#define BOARD_AT45_B_SPI_ID           ID_SPI1
///// Pins of the SPI peripheral connected to the dataflash.
//#define BOARD_AT45_B_SPI_PINS         PINS_SPI1
///// Dataflahs SPI number.
//#define BOARD_AT45_B_SPI              1
///// Chip select connected to the dataflash.
//#define BOARD_AT45_B_NPCS             3
///// Chip select pin connected to the dataflash.
//#define BOARD_AT45_B_NPCS_PIN         PIN_SPI1_NPCS3

/// Base address of SPI peripheral connected to the serialflash.
//#define BOARD_AT26_A_SPI_BASE         SPI0
///// Identifier of SPI peripheral connected to the serialflash.
//#define BOARD_AT26_A_SPI_ID           ID_SPI0
///// Pins of the SPI peripheral connected to the serialflash.
//#define BOARD_AT26_A_SPI_PINS         PINS_SPI0
///// Serialflash SPI number.
//#define BOARD_AT26_A_SPI              0
///// Chip select connected to the serialflash.
//#define BOARD_AT26_A_NPCS             3
///// Chip select pin connected to the serialflash.
//#define BOARD_AT26_A_NPCS_PIN         PIN_SPI0_NPCS3

/// ISO7816
/// - PIN_SMARTCARD_CONNECT
/// - PIN_ISO7816_RSTMC
/// - PINS_ISO7816

/// Smartcard detection pin
//#define PIN_SMARTCARD_CONNECT   {1 << 5, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
/// PIN used for reset the smartcard
//#define PIN_ISO7816_RSTMC       {1 << 7, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
/// Pins used for connect the smartcard
//#define PINS_ISO7816            PIN_USART0_TXD, PIN_USART0_SCK, PIN_ISO7816_RSTMC

/// Dma channel number
#define BOARD_MCI_DMA_CHANNEL                         0
/// MCI0 DMA hardware handshaking ID
#define DMA_HW_SRC_REQ_ID_MCI0      AT91C_HDMA_SRC_PER_0
#define DMA_HW_DEST_REQ_ID_MCI0     AT91C_HDMA_DST_PER_0
/// MCI1 DMA hardware handshaking ID
#define DMA_HW_SRC_REQ_ID_MCI1      AT91C_HDMA_SRC_PER_13
#define DMA_HW_DEST_REQ_ID_MCI1     AT91C_HDMA_DST_PER_13
/// SD DMA hardware handshaking ID
#define BOARD_SD_DMA_HW_SRC_REQ_ID      DMA_HW_SRC_REQ_ID_MCI0
#define BOARD_SD_DMA_HW_DEST_REQ_ID     DMA_HW_DEST_REQ_ID_MCI0


/// HS MCI interface
#define MCI2_INTERFACE
/// Base address of the MCI peripheral connected to the SD card.
#define BOARD_SD_MCI_BASE           MCI0//MCI
///// Peripheral identifier of the MCI connected to the SD card.
#define BOARD_SD_MCI_ID             AT91C_ID_MCI0 ID_MCI0 //ID_MCI
///// MCI pins that shall be configured to access the SD card.
#define BOARD_SD_PINS               PINS_MCI
///// MCI slot to which the SD card is connected to.
#define BOARD_SD_SLOT               MCI_SD_SLOTA
///// MCI Card Detect pin.
#define BOARD_SD_PIN_CD             PIN_MCI_CD

#define BLOCK_SIZE                     512
#define MSD_BUFFER_SIZE                (12*BLOCK_SIZE) // Size of the MSD IO buffer in bytes (6K, more the better)
#define SD_CARD_PRESENT()              ( (PIOB->PIO_PDSR & PIO_PB7) == 0 )

// States for initialising card
#define SD_ST_ERR               -1
#define SD_ST_STARTUP           0
#define SD_ST_EMPTY             1
#define SD_ST_INIT1             2
#define SD_ST_INIT2             3
#define SD_ST_IDLE              4
#define SD_ST_READY             5
#define SD_ST_IDENT             6
#define SD_ST_STBY              7
#define SD_ST_TRAN              8
#define SD_ST_DATA              9
#define SD_ST_MOUNTED           10

extern uint32_t Cmd_A41_resp;
extern int32_t Card_state;
extern volatile uint32_t Card_initialized;
extern uint32_t Card_CSD[4];
extern uint32_t transSpeed;

#define OCR_SD_CCS             (1UL << 30)

#define SD_CSD(csd, bitfield, bits)   ((csd[3-(bitfield)/32] >> ((bitfield)%32)) & ((1 << (bits)) - 1))

#define SD_CSD_TRAN_SPEED(pSd)         SD_CSD(pSd, 96,  8) ///< Max. data transfer rate

#define SD_CSD_C_SIZE(pSd)             ((SD_CSD(pSd, 72,  2) << 10) + \
                                        (SD_CSD(pSd, 64,  8) << 2)  + \
                                        SD_CSD(pSd, 62,  2)) ///< Device size

#define SD_CSD_C_SIZE_HC(csd)          ((SD_CSD(csd, 64,  6) << 16) + \
                                        (SD_CSD(csd, 56,  8) << 8)  + \
                                        SD_CSD(csd, 48,  8)) ///< Device size v2.0 High Capacity

#define SD_CSD_C_SIZE_MULT(pSd)        SD_CSD(pSd, 47,  3) ///< Device size multiplier

#define SD_CSD_MULT(pSd)               (1 << (SD_CSD_C_SIZE_MULT(pSd) + 2))

#define SD_CSD_READ_BL_LEN(pSd)        SD_CSD(pSd, 80,  4) ///< Max. read data block length

#define SD_CSD_BLKLEN(pSd)             (1 << (SD_CSD_READ_BL_LEN(pSd) - 9))

#define SD_CSD_BLOCKNR(pSd)            ((SD_CSD_C_SIZE(pSd) + 1) * SD_CSD_MULT(pSd)) * SD_CSD_BLKLEN(pSd)

#define SD_CSD_BLOCKNR_HC(pSd)         ((SD_CSD_C_SIZE_HC(pSd) + 1) * 1024)

#if !defined(SIMU)
  #define SD_IS_HC()                     (Cmd_A41_resp & OCR_SD_CCS)
  #define SD_GET_BLOCKNR()               (SD_IS_HC() ? (SD_CSD_BLOCKNR_HC(Card_CSD)) : (SD_CSD_BLOCKNR(Card_CSD)))
  #define SD_GET_SPEED()                 (transSpeed)
#else
  #define SD_IS_HC()              (0)
  #define SD_GET_BLOCKNR()        (0)
  #define SD_GET_SPEED()          (0)
#endif

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par sam3s_ek_mem "SAM3S-EK - Memories"
/// This page lists definitions related to internal & external on-board memories.
///
/// !Embedded Flash
/// - BOARD_FLASH_EFC

/// Internal SRAM address
#define AT91C_ISRAM                   AT91C_IRAM
#define AT91C_ISRAM_SIZE              0x00008000

#define AT91C_IFLASH_SIZE               (0x40000)
#define AT91C_IFLASH_PAGE_SIZE              (256) // Internal FLASH 0 Page Size: 256 bytes
#define AT91C_IFLASH_NB_OF_PAGES           (1024) // Internal FLASH 0 Number of Pages: 512
#define AT91C_IFLASH_LOCK_REGION_SIZE     (16384) // Internal FLASH 0 Lock Region Size: 16 Kbytes
#define AT91C_IFLASH_NB_OF_LOCK_BITS         (16) // Internal FLASH 0 Number of Lock Bits: 16

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// \section sam3s_ek_extcomp "SAM3S-EK - External components"
/// This page lists the definitions related to external on-board components
/// located in the board.h file for the SAM3S-EK.
///
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \par sam3s_ek_chipdef "SAM3S-EK - Individual chip definition"
/// This page lists the definitions related to different chip's definition
/// located in the board.h file for the SAM3S-EK.

/// DBGU
#define BOARD_DBGU_ID               ID_DBGU

/// Rtc
#define BOARD_RTC_ID                ID_RTC

/// Twi eeprom
#define BOARD_ID_TWI_EEPROM         ID_TWI1
#define BOARD_BASE_TWI_EEPROM       TWI1
#define BOARD_PINS_TWI_EEPROM       PINS_TWI1

/// USART
#define BOARD_PIN_USART_RXD        PIN_USART1_RXD
#define BOARD_PIN_USART_TXD        PIN_USART1_TXD
#define BOARD_PIN_USART_CTS        PIN_USART1_CTS
#define BOARD_PIN_USART_RTS        PIN_USART1_RTS
#define BOARD_PIN_USART_EN         PIN_USART1_EN
#define BOARD_USART_BASE           USART1
#define BOARD_ID_USART             ID_USART1

// BlueTooth
#define BT_USART       UART1
#define BT_ID          ID_UART1

//------------------------------------------------------------------------------


#define PIN_EBI_NANDOE          {1 << 9,  PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NANDWE          {1 << 10, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NANDCLE         {1 << 17, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NANDALE         {1 << 16, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_EBI_NANDIO          {0x000000FF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_PULLUP}

/// Nandflash chip enable pin definition.
#define BOARD_NF_CE_PIN         {1 << 14, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
/// Nandflash ready/busy pin definition.
#define BOARD_NF_RB_PIN         {1 << 18, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP}

/// Nandflash controller peripheral pins definition.
#define PINS_NANDFLASH          PIN_EBI_NANDIO, BOARD_NF_CE_PIN, BOARD_NF_RB_PIN, PIN_EBI_NANDOE, \
                                PIN_EBI_NANDWE, PIN_EBI_NANDCLE, PIN_EBI_NANDALE

/// Address for transferring command bytes to the nandflash.
#define BOARD_NF_COMMAND_ADDR   0x60400000
/// Address for transferring address bytes to the nandflash.
#define BOARD_NF_ADDRESS_ADDR   0x60200000
/// Address for transferring data bytes to the nandflash.
#define BOARD_NF_DATA_ADDR      0x60000000

extern uint32_t Master_frequency ;

struct t_serial_tx
{
  uint8_t *buffer ;
  uint16_t size ;
  volatile uint16_t ready ;
};

#endif // _BOARD_H_
