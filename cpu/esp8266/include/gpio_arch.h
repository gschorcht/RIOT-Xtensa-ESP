/*
 * Copyright (C) 2019 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_esp8266
 * @{
 *
 * @file
 * @brief       Architecture specific GPIO functions for ESP8266
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @}
 */

#ifndef GPIO_ARCH_H
#define GPIO_ARCH_H

#include "gpio_arch_common.h"

#ifndef DOXYGEN

#ifdef __cplusplus
extern "C" {
#endif

/** Map of GPIO pin numbers to IOMUX pin numbers */
extern const uint8_t _gpio_to_iomux[];
/** Map of IOMUX pin numbers to GPIO pin numbers */
extern const uint8_t _iomux_to_gpio[];

/**
 * @brief   Called before the power management enters a light or deep sleep mode
 *
 * @param   mode    sleep mode that is entered
 */
void gpio_pm_sleep_enter(unsigned mode);

/**
 * @brief   Called after the power management left light sleep mode
 * @param   cause   wake-up cause
 */
void gpio_pm_sleep_exit(uint32_t cause);

#ifdef __cplusplus
}
#endif

#endif /* DOXYGEN */
#endif /* GPIO_ARCH_H */
