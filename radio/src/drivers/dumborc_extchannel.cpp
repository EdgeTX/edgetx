/*
 * Copyright (C) EdgeTx
 *
 * Authors:
 *   FreedomLJS <freedom_ljs@qq.com>
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

#include "dumborc_extchannel.h"



#if ( DUMBORC_EXTCHANNEL_ENABLED )

#include <string.h>
#include <stdio.h>

#include "hal.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "delays_driver.h"

#include "os/time.h"
#include "os/task.h"
#include "os/sleep.h"
#include "tasks.h"



/*** ###### ###### ###### >>>>>> LJS : INNER <<<<<< ###### ###### ###### ***/

#define DUMBORC_EXTCHANNEL_COUNT_OF(__TARGET__)                         \
  (sizeof(__TARGET__) / sizeof((__TARGET__)[0]))


#define DUMBORC_EXTCHANNEL_CPU_FREQ_HZ    (CPU_FREQ)

#define DUMBORC_EXTCHANNEL_DELAY_US_TO_TICK(__TIME_US__)                \
  ((uint32_t)((__TIME_US__) * ((DUMBORC_EXTCHANNEL_CPU_FREQ_HZ) / 1000000UL)))

#define DUMBORC_EXTCHANNEL_DELAY_TICK_TO_US(__TIME_TICK__)              \
  ((uint32_t)((__TIME_TICK__) / ((DUMBORC_EXTCHANNEL_CPU_FREQ_HZ) / 1000000UL)))


#define DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_SDA    GPIOA
#define DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_SCL    GPIOA
#define DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_SDA     13
#define DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_SCL     14

#define DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(__GPIO__)                    \
  DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_##__GPIO__
#define DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_X(__GPIO__)                     \
  DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_##__GPIO__
#define DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(__GPIO__)                  \
  (1UL << DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_X(__GPIO__))

#define DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(__GPIO__)                     \
  LL_GPIO_ResetOutputPin(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(__GPIO__) , \
                         DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(__GPIO__))
#define DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(__GPIO__)                     \
  LL_GPIO_SetOutputPin(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(__GPIO__) ,   \
                       DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(__GPIO__))
#define DUMBORC_EXTCHANNEL_I2C_GPIO_GET_X(__GPIO__)                     \
  LL_GPIO_IsInputPinSet(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(__GPIO__) ,  \
                        DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(__GPIO__))


#define DUMBORC_EXTCHANNEL_I2C_ADDR_A    (0x27 << 1)
#define DUMBORC_EXTCHANNEL_I2C_ADDR_B    (0x26 << 1)


#define DUMBORC_EXTCHANNEL_TASK_STACK_SIZE    (64)
#define DUMBORC_EXTCHANNEL_TASK_PRIO          (CLI_TASK_PRIO)



static task_handle_t s_dumborc_extchannel_task_id = {0};
static TASK_DEFINE_STACK(s_dumborc_extchannel_task_stack , \
                         DUMBORC_EXTCHANNEL_TASK_STACK_SIZE);

static uint8_t s_dumborc_extchannel_i2c_req_buf_reg[2][8] = {0};
static uint32_t s_dumborc_extchannel_i2c_clk_cycle_tick_div3 = 0;
static uint32_t s_dumborc_extchannel_i2c_req_cnt = 0;
static uint32_t s_dumborc_extchannel_i2c_req_pins_status = 0;
static uint32_t s_dumborc_extchannel_task_status = 0;



/* LJS : Software I2C BEGIN >>> */
static void i_dumborc_extchannel_delay_init(void)
{
  delaysInit();
}

static void i_dumborc_extchannel_delay_uninit(void)
{
  return;
}

static uint32_t i_dumborc_extchannel_delay_get_tick(void)
{
  return ticksNow();
}

#pragma GCC push_options
#pragma GCC optimize("O0")
static void i_dumborc_extchannel_delay_us(uint32_t const time_us)
{
  uint32_t val_tick , val_tick_dt;


  val_tick = i_dumborc_extchannel_delay_get_tick();
  val_tick_dt = DUMBORC_EXTCHANNEL_DELAY_US_TO_TICK(time_us);
  while ( (i_dumborc_extchannel_delay_get_tick() - val_tick) < val_tick_dt );
}

static int i_dumborc_extchannel_delay_check(void)
{
  uint32_t val , val_tick;


  /* LJS : Get timer current tick  */
  val_tick = i_dumborc_extchannel_delay_get_tick();
  /* LJS : Simulate a 2us delay! */
  val = DUMBORC_EXTCHANNEL_DELAY_US_TO_TICK(2);
  while ( val-- > 0 ) {
    __NOP();
  }
  /* LJS : Check the change in timer tick  */
  if ( i_dumborc_extchannel_delay_get_tick() == val_tick ) {
    /* LJS : Timer hasn't started! (-1)  */
    return -1;
  }

  return 0;
}
#pragma GCC pop_options


static void i_dumborc_extchannel_i2c_gpio_init(void)
{
  /* LJS : Init Software-I2C SDA & SCL GPIO */
  LL_GPIO_InitTypeDef pinInit;


  stm32_gpio_enable_clock(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SDA));
  stm32_gpio_enable_clock(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SCL));

  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);

  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin        = DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(SDA);
  pinInit.Mode       = LL_GPIO_MODE_OUTPUT;
  pinInit.Speed      = LL_GPIO_SPEED_FREQ_MEDIUM;
  pinInit.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  pinInit.Pull       = LL_GPIO_PULL_UP;
  pinInit.Alternate  = LL_GPIO_AF_0;
  LL_GPIO_Init(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SDA) , &pinInit);
  pinInit.Pin = DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(SCL);
  LL_GPIO_Init(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SCL) , &pinInit);
}

static void i_dumborc_extchannel_i2c_gpio_uninit(void)
{
  /* LJS : Uninit Software-I2C SDA & SCL GPIO */
  LL_GPIO_InitTypeDef pinInit;


  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin        = DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(SDA);
  pinInit.Mode       = LL_GPIO_MODE_INPUT;
  pinInit.Speed      = LL_GPIO_SPEED_FREQ_LOW;
  pinInit.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  pinInit.Pull       = LL_GPIO_PULL_NO;
  pinInit.Alternate  = LL_GPIO_AF_0;
  LL_GPIO_Init(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SDA) , &pinInit);
  pinInit.Pin = DUMBORC_EXTCHANNEL_I2C_GPIO_PIN_LL_X(SCL);
  LL_GPIO_Init(DUMBORC_EXTCHANNEL_I2C_GPIO_PORT_X(SCL) , &pinInit);
}

static void i_dumborc_extchannel_i2c_bus_start(void)
{
  /**
   * SCL XXXXXX XXXXXX HHHHHH HHHHHH LLLLLL
   * SDA XXXXXX HHHHHH HHHHHH LLLLLL LLLLLL
   */
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
}

static void i_dumborc_extchannel_i2c_bus_stop(void)
{
  /**
   * SCL XXXXXX XXXXXX HHHHHH HHHHHH
   * SDA XXXXXX LLLLLL LLLLLL HHHHHH
   */
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
}

static uint8_t i_dumborc_extchannel_i2c_bus_read_1byte(void)
{
  uint32_t count , rx_byte;


  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);

  count = 8;
  rx_byte = 0;
  do {
    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);

    rx_byte <<= 1;
    if ( DUMBORC_EXTCHANNEL_I2C_GPIO_GET_X(SDA) ) {
      rx_byte |= 0x01;
    }

    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SCL);
    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  } while ( --count );

  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);

  return (rx_byte & 0xFF);
}

static void i_dumborc_extchannel_i2c_bus_write_1byte(uint8_t const data)
{
  uint32_t count , tx_byte;


  count = 8;
  tx_byte = data;
  do {
    if ( tx_byte & 0x80 ) {
      DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
    } else {
      DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SDA);
    }
    tx_byte <<= 1;

    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SCL);
    if ( count == 1 ) {
      DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SDA);
    }
    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  } while ( --count );
}

static uint8_t i_dumborc_extchannel_i2c_bus_get_ack(void)
{
  uint32_t count , status;


  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);

  count = 10;
  do {
    status = DUMBORC_EXTCHANNEL_I2C_GPIO_GET_X(SDA);
    if ( status == 0 ) {
      break;
    }
    i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  } while ( --count );

  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);

  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);

  return (!!status);
}

static void i_dumborc_extchannel_i2c_bus_set_ack(uint32_t const status)
{
  if ( status ) {
    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SDA);
  } else {
    DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SDA);
  }

  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_H(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
  DUMBORC_EXTCHANNEL_I2C_GPIO_SET_L(SCL);
  i_dumborc_extchannel_delay_us(s_dumborc_extchannel_i2c_clk_cycle_tick_div3);
}

static uint8_t i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(uint8_t const data)
{
  i_dumborc_extchannel_i2c_bus_write_1byte(data);

  return i_dumborc_extchannel_i2c_bus_get_ack();
}

static int i_dumborc_extchannel_i2c_bus_read(\
  uint8_t const dev_address                , \
  uint8_t const * const p_tx_buffer        , \
  uint32_t const tx_length                 , \
  uint8_t * const p_rx_buffer              , \
  uint32_t const rx_length)
{
  int status;
  uint32_t i , value;


  if ( (dev_address & 0x01) || (p_rx_buffer == NULL) || (rx_length == 0) ) {
    /* LJS : Input parameter error! (-1) */
    return -1;
  }

  /* LJS : READ-PROCESS BEGIN >>> */
  do {
    if ( (p_tx_buffer != NULL) && (tx_length != 0) ) {
      /* LJS : Need to write data! Bus start! */
      i_dumborc_extchannel_i2c_bus_start();
      status = -2;

      /* LJS : Write device address (write mode , bit0 = 0). */
      if ( i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(dev_address) ) {
        /* LJS : Hasn't ack (not exist device) , break it (READ-PROCESS). */
        break;
      }

      /* LJS : Write device data. */
      for (i = 0 ; i < tx_length ; ++i) {
        if ( i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(p_tx_buffer[i]) ) {
          /* LJS : Hasn't ack (wirte error) , break it (write device data). */
          break;
        }
      }
      if ( i != tx_length ) {
        /* LJS : Not enough quantity , break it (READ-PROCESS). */
        break;
      }
    }

    /* LJS : Need to read data! Bus restart! */
    i_dumborc_extchannel_i2c_bus_start();
    status = -3;

    /* LJS : Write device address (read mode , bit0 = 1). */
    if ( i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(dev_address | 0x01) ) {
      /* LJS : Hasn't ack (not exist device) , break it (READ-PROCESS). */
      break;
    }

    /* LJS : Read device data. */
    value = rx_length - 1;
    for (i = 0 ; i < rx_length ; ++i) {
      p_rx_buffer[i] = i_dumborc_extchannel_i2c_bus_read_1byte();
      i_dumborc_extchannel_i2c_bus_set_ack(i == value);
    }
    /* LJS : Process succeed! */
    status = 0;
  } while ( 0 );

  /* LJS : READ-PROCESS END <<< */
  i_dumborc_extchannel_i2c_bus_stop();

  return status;
}

static int i_dumborc_extchannel_i2c_bus_write(\
  uint8_t const dev_address                 , \
  uint8_t const * const p_tx_buffer         , \
  uint32_t const tx_length)
{
  int status;
  uint32_t i;


  if ( (dev_address & 0x01) || (p_tx_buffer == NULL) || (tx_length == 0) ) {
    /* LJS : Input parameter error! (-1) */
    return -1;
  }

  /* LJS : WRITE-PROCESS BEGIN >>> */
  do {
    /* LJS : Need to write data! Bus start! */
    i_dumborc_extchannel_i2c_bus_start();
    status = -2;

    /* LJS : Write device address (write mode , bit0 = 0). */
    if ( i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(dev_address) ) {
      /* LJS : Hasn't ack (not exist device) , break it (WRITE-PROCESS). */
      break;
    }

    /* LJS : Write device data. */
    for (i = 0 ; i < tx_length ; ++i) {
      if ( i_dumborc_extchannel_i2c_bus_write_1byte_with_ack(p_tx_buffer[i]) ) {
        /* LJS : Hasn't ack (wirte error) , break it (write device data). */
        break;
      }
    }
    if ( i != tx_length ) {
      /* LJS : Not enough quantity , break it (WRITE-PROCESS). */
      break;
    }
    /* LJS : Process succeed! */
    status = 0;
  } while ( 0 );

  /* LJS : WRITE-PROCESS END <<< */
  i_dumborc_extchannel_i2c_bus_stop();

  return status;
}
/* LJS : Software I2C END <<< */


/* LJS : Dumborc_extchannel handle task BEGIN <<< */
static void i_dumborc_extchannel_task_cb(void)
{
  for ( ; ; ) {
    while ( !s_dumborc_extchannel_task_status ) {
      sleep_ms(1);
    }

    dumborc_extchannel_handle();

    sleep_ms(100);
  }
}

static int i_dumborc_extchannel_task_init(void)
{
  if ( (s_dumborc_extchannel_task_id._stack_size) <= 0 ) {
    /* LJS : Task (static_task) create. */
    task_create(&s_dumborc_extchannel_task_id , \
                  i_dumborc_extchannel_task_cb , \
                  "DR_EC:P" , \
                  s_dumborc_extchannel_task_stack , \
                  DUMBORC_EXTCHANNEL_TASK_STACK_SIZE , \
                  DUMBORC_EXTCHANNEL_TASK_PRIO);
    if ( (s_dumborc_extchannel_task_id._stack_size) <= 0 ) {
      /* LJS : Task hasn't create! (-1)  */
      return -1;
    }
  }

  return 0;
}

static int i_dumborc_extchannel_task_uninit(void)
{
  return 0;
}
/* LJS : Dumborc_extchannel handle task END <<< */


static void i_dumborc_extchannel_para_reset(void)
{
  memset(s_dumborc_extchannel_i2c_req_buf_reg , 0x00 , \
         sizeof(s_dumborc_extchannel_i2c_req_buf_reg));
  s_dumborc_extchannel_i2c_clk_cycle_tick_div3 = \
    DUMBORC_EXTCHANNEL_DELAY_US_TO_TICK(1);
  s_dumborc_extchannel_i2c_req_cnt = 0;
  s_dumborc_extchannel_i2c_req_pins_status = 0;
  s_dumborc_extchannel_task_status = 0;
}

/*** ###### ###### ###### >>>>>> ----------- <<<<<< ###### ###### ###### ***/



/*** ###### ###### ###### >>>>>> LJS : OUTER <<<<<< ###### ###### ###### ***/

/**
 * [dumborc_extchannel_init (dumborc_extchannel init)]
 * @return  [error code (int)]
 *          [0 : succeed ; !0 : failed]
 */
int dumborc_extchannel_init(void)
{
  i_dumborc_extchannel_delay_init();
  i_dumborc_extchannel_para_reset();
  i_dumborc_extchannel_i2c_gpio_init();

  if ( i_dumborc_extchannel_delay_check() < 0 ) {
    /* LJS : Timer hasn't started! (-1)  */
    return -1;
  }

  if ( i_dumborc_extchannel_task_init() < 0 ) {
    /* LJS : Task hasn't init! (-2)  */
    return -2;
  }

  return 0;
}

/**
 * [dumborc_extchannel_uninit (dumborc_extchannel uninit)]
 * @return  [error code (int)]
 *          [0 : succeed ; !0 : failed]
 */
int dumborc_extchannel_uninit(void)
{
  i_dumborc_extchannel_i2c_gpio_uninit();
  i_dumborc_extchannel_para_reset();
  i_dumborc_extchannel_delay_uninit();

  if ( i_dumborc_extchannel_task_uninit() < 0 ) {
    /* LJS : Task hasn't uninit! (-1)  */
    return -1;
  }

  return 0;
}


/**
 * [dumborc_extchannel_handle (dumborc_extchannel handle)]
 */
void dumborc_extchannel_handle(void)
{
  static const uint8_t buf_cfg[][3] = {\
    {0x02 , 0xFF , 0xFF} , \
    {0x04 , 0x00 , 0x00} , \
    {0x06 , 0xFF , 0xFF} , \
  };
  int ret;
  uint32_t i;
  uint8_t reg;


  if ( !s_dumborc_extchannel_task_status ) {
    return;
  }

  /* LJS : PROCESS BEGIN >>> */
  if ( s_dumborc_extchannel_i2c_req_cnt <= 0 ) {
    /* LJS : First-time use requires initializing the configuration */
    for ( i = 0 , ret = 0 ; \
          i < DUMBORC_EXTCHANNEL_COUNT_OF(buf_cfg) ; \
          ++i ) {
      ret |= i_dumborc_extchannel_i2c_bus_write(\
               DUMBORC_EXTCHANNEL_I2C_ADDR_A , \
               buf_cfg[i] , sizeof(buf_cfg[0]));
      ret |= i_dumborc_extchannel_i2c_bus_write(\
               DUMBORC_EXTCHANNEL_I2C_ADDR_B , \
               buf_cfg[i] , sizeof(buf_cfg[0]));
    }
  }

  for ( i = 0 , ret = 0 ; \
        i < (DUMBORC_EXTCHANNEL_COUNT_OF(\
               s_dumborc_extchannel_i2c_req_buf_reg[0]) >> 1) ; \
        ++i ) {
    if ( (i >= 1) && \
         ((s_dumborc_extchannel_i2c_req_cnt % 10) != 0) ) {
      /* LJS : Execute every 10 times! (Slow refresh 'reg[2:7]') */
      break;
    }
    reg = (i << 1);
    ret |= i_dumborc_extchannel_i2c_bus_read(\
             DUMBORC_EXTCHANNEL_I2C_ADDR_A , \
             &reg , sizeof(reg) , \
             &(s_dumborc_extchannel_i2c_req_buf_reg[0][reg]) , 2);
    ret |= i_dumborc_extchannel_i2c_bus_read(\
             DUMBORC_EXTCHANNEL_I2C_ADDR_B , \
             &reg , sizeof(reg) , \
             &(s_dumborc_extchannel_i2c_req_buf_reg[1][reg]) , 2);
  }
  s_dumborc_extchannel_i2c_req_pins_status = \
    ( (((uint32_t)(s_dumborc_extchannel_i2c_req_buf_reg[0][0])) <<  0) | \
      (((uint32_t)(s_dumborc_extchannel_i2c_req_buf_reg[0][1])) <<  8) | \
      (((uint32_t)(s_dumborc_extchannel_i2c_req_buf_reg[1][0])) << 16) | \
      (((uint32_t)(s_dumborc_extchannel_i2c_req_buf_reg[1][1])) << 24) );

  ++s_dumborc_extchannel_i2c_req_cnt;
  /* LJS : PROCESS END <<< */
}


/**
 * [dumborc_extchannel_get_task_status (dumborc_extchannel get task status)]
 * @return  [task status (uint32_t)]
 *          [0 : stop ; !0 : running]
 */
uint32_t dumborc_extchannel_get_task_status(void)
{
  return s_dumborc_extchannel_task_status;
}

/**
 * [dumborc_extchannel_set_task_status (dumborc_extchannel set task status)]
 * @param status [task status (uint32_t)]
 *               [0 : stop ; !0 : running]
 */
void dumborc_extchannel_set_task_status(uint32_t const status)
{
  s_dumborc_extchannel_task_status = (!!status);
}


/**
 * [dumborc_extchannel_get_reg_data (dumborc_extchannel get register data)]
 * @param  idx [IC index (uint32_t)]
 * @param  reg [IC register (uint32_t)]
 * @return     [register data (uint32_t)]
 */
uint32_t dumborc_extchannel_get_reg_data(uint32_t const idx , uint32_t const reg)
{
  if ( (s_dumborc_extchannel_i2c_req_cnt <= 0) || \
       (idx >= DUMBORC_EXTCHANNEL_COUNT_OF(\
                 s_dumborc_extchannel_i2c_req_buf_reg)) || \
       (reg >= DUMBORC_EXTCHANNEL_COUNT_OF(\
                 s_dumborc_extchannel_i2c_req_buf_reg[0])) ) {
    /* LJS : Dumborc_extchannel hasn't run, or input parameter error! (0) */
    return 0;
  }

  return (s_dumborc_extchannel_i2c_req_buf_reg[idx][reg]);
}

/**
 * [dumborc_extchannel_get_pin_status (dumborc_extchannel get pin status)]
 * @param  pin_mask [pin mask (uint32_t)]
 * @return          [pin status (uint32_t)]
 */
uint32_t dumborc_extchannel_get_pin_status(uint32_t const pin_mask)
{
  if ( s_dumborc_extchannel_i2c_req_cnt <= 0 ) {
    /* LJS : Dumborc_extchannel hasn't run! (0) */
    return 0;
  }

  return (s_dumborc_extchannel_i2c_req_pins_status & pin_mask);
}



#if 1
#include "stm32_switch_driver.h"



/**
 * [boardSwitchGetPosition (System board swith get position , Override)]
 * @param  idx [System board switch index (uint8_t)]
 * @return     [System board switch position (SwitchHwPos)]
 */
SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  extern const stm32_switch_t * boardGetSwitchDef(uint8_t idx);
  static uint32_t time_ms_prev = 0;
  const stm32_switch_t * sw = boardGetSwitchDef(idx);
  SwitchHwPos ret = SWITCH_HW_UP;
  uint32_t time_ms_curr , pin_sta_hi , pin_sta_lo;


  time_ms_curr = time_get_ms();
  if ( ((time_ms_curr - time_ms_prev) >= (1 * 1000)) && \
       (dumborc_extchannel_get_pin_status(0xFFFFFFFF) == 0) ) {
    /* LJS : Dumborc_extchannel hasn't init! Reinit! */
    time_ms_prev = time_ms_curr;
    dumborc_extchannel_set_task_status(0);
    dumborc_extchannel_uninit();
    dumborc_extchannel_init();
    dumborc_extchannel_set_task_status(1);
  }

  if ( (sw->type == SWITCH_HW_ADC) || (sw->GPIOx_high != NULL) ) {
    /* LJS : Belongs to a system call! */
    return stm32_switch_get_position(sw);
  }

  pin_sta_hi = dumborc_extchannel_get_pin_status(sw->Pin_high);
  switch ( sw->type ) {
    case SWITCH_HW_2POS: {
      if ( !pin_sta_hi ) {
        ret = SWITCH_HW_DOWN;
      }
    } break;

    case SWITCH_HW_3POS: {
      pin_sta_lo = dumborc_extchannel_get_pin_status(sw->Pin_low);
      if ( pin_sta_hi && pin_sta_lo ) {
        ret = SWITCH_HW_MID;
      } else if ( !pin_sta_hi && pin_sta_lo ) {
        ret = SWITCH_HW_DOWN;
      }
    } break;

    default:{
    } break;
  }

  if ( sw->inverted ) {
    if ( ret == SWITCH_HW_UP ) {
      ret = SWITCH_HW_DOWN;
    } else if ( ret == SWITCH_HW_DOWN ) {
      ret = SWITCH_HW_UP;
    }
  }

  return ret;
}
#endif

/*** ###### ###### ###### >>>>>> ----------- <<<<<< ###### ###### ###### ***/

#endif    /* ( DUMBORC_EXTCHANNEL_ENABLED ) */


