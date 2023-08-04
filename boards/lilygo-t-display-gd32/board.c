/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_stm32f746g-disco
 * @{
 *
 * @file
 * @brief       Board specific implementations for the STM32F746G-DISCO board
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "kernel_defines.h"
#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    gpio_init(BACKLIGHT_PIN, GPIO_OUT);
#if 0
    if (IS_USED(CONFIG_LILYGO_T_DISPLY_GD32_WITH_TFT)) {
        gpio_set(BACKLIGHT_PIN);
    }
    else {
        gpio_clear(BACKLIGHT_PIN);
    }
#else
    gpio_set(BACKLIGHT_PIN);
#endif
}
