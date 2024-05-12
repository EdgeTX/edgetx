#include "hal.h"
#include "mfxstm32l152.h"
#include "bsp_errno.h"

#include "stm32_i2c_driver.h"
#include "stm32_gpio.h"
#include "hal/gpio.h"

#include "stm32_hal.h"

#define IO_I2C_CLK_RATE                  400000
#define IO_I2C_ADDRESS                   (0x84U >> 1)
#define IO_I2C_ADDRESS_2                 (0x86U >> 1)

static bool _bsp_io_initialized = false;
static MFXSTM32L152_Object_t Io_CompObj;

static int32_t BSP_I2C1_Init()
{
  if (stm32_i2c_init(I2C_Bus_1, IO_I2C_CLK_RATE) >= 0) {
    return BSP_ERROR_NONE;
  }

  return BSP_ERROR_BUS_FAILURE;
}

static int32_t BSP_I2C1_DeInit()
{
  if (stm32_i2c_deinit(I2C_Bus_1) == 0) {
    return BSP_ERROR_NONE;
  }

  return BSP_ERROR_BUS_FAILURE;
}

static int32_t BSP_I2C1_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData,
                                uint16_t Length)
{
  if (stm32_i2c_read(I2C_Bus_1, DevAddr, Reg, 1, pData, Length, 100) == 0) {
    return BSP_ERROR_NONE;
  }

  return BSP_ERROR_BUS_FAILURE;
}

static int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData,
                                 uint16_t Length)
{
  if (stm32_i2c_write(I2C_Bus_1, DevAddr, Reg, 1, pData, Length, 100)) {
    return BSP_ERROR_NONE;
  }

  return BSP_ERROR_NONE;
}

int bsp_io_init()
{
  if (!_bsp_io_initialized) {
    uint8_t i2c_address[] = {IO_I2C_ADDRESS, IO_I2C_ADDRESS_2};
    MFXSTM32L152_IO_t IOCtx = {
        .Init = BSP_I2C1_Init,
        .DeInit = BSP_I2C1_DeInit,
        .WriteReg = BSP_I2C1_WriteReg,
        .ReadReg = BSP_I2C1_ReadReg,
        .GetTick = (MFXSTM32L152_GetTick_Func)HAL_GetTick,
    };

    if (MFXSTM32L152_Probe(&IOCtx, &Io_CompObj, i2c_address, 2) !=
        BSP_ERROR_NONE) {
      return -1;
    }

    if (MFXSTM32L152_IO_Driver.IO_Start(
            &Io_CompObj, MFXSTM32L152_GPIO_PINS_ALL) != MFXSTM32L152_OK) {
      return -1;
    }

    _bsp_io_initialized = true;
  }

  return 0;
}

static gpio_cb_t _touch_cb = nullptr;

static void _touch_cb_wrap()
{
  if (_touch_cb) _touch_cb();
  MFXSTM32L152_IO_Driver.IO_ClearIT(&Io_CompObj, MFXSTM32L152_GPIO_PIN_14);
}

int bsp_touch_enable_irq(gpio_cb_t cb)
{
  if (bsp_io_init() != 0) {
    return -1;
  }

  MFXSTM32L152_IO_Init_t io_init_structure = {
      .Pin  = MFXSTM32L152_GPIO_PIN_14,
      .Mode = MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE,
      .Pull = MFXSTM32L152_GPIO_PULL_UP,
  };

  if (MFXSTM32L152_IO_Driver.Init(&Io_CompObj, &io_init_structure) != MFXSTM32L152_OK) {
    return -1;
  }

  _touch_cb = cb;
  gpio_init_int(TOUCH_INT_GPIO, GPIO_IN, GPIO_BOTH, _touch_cb_wrap);

  return 0;
}

int bsp_joystick_init()
{
  if (bsp_io_init() != 0) {
    return -1;
  }

  return 0; 
}
