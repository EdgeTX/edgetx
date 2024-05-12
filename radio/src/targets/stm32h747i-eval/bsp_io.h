#pragma once

#include "hal/gpio.h"

int bsp_io_init();

int bsp_sd_transceiver_init();
void bsp_sd_transceiver_sel_ldo(bool low_voltage);

int bsp_touch_enable_irq(gpio_cb_t cb);

