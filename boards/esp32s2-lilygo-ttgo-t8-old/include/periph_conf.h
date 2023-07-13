/*
 * Copyright (C) 2022 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_esp32s2_lilygo_ttgo_t8
 * @brief       Peripheral configurations for the LILYGO TTGO T8 ESP32-S2 board
 * @{
 *
 * @file
 * @author      Gunar Schorcht <gunar@schorcht.net>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#if !DOXYGEN

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    SPI configuration
 *
 * SPI_DEV(0) is used for the Display. The GPIOs are not broken out.<br>
 * SPI_DEV(1) is used for the SD Card slot. The GPIOs are broken out and can
 * also be used by other devices.
 *
 * @note The GPIOs listed in the configuration are first initialized as SPI
 * signals when the corresponding SPI interface is used for the first time
 * by either calling the `spi_init_cs` function or the `spi_acquire`
 * function. That is, they are not allocated as SPI signals before and can
 * be used for other purposes as long as the SPI interface is not used.
 * @{
 */

#ifndef SPI0_CTRL
#define SPI0_CTRL   FSPI    /**< FSPI (SPI Controller 2) is used as SPI_DEV(0) */
#endif
#ifndef SPI0_SCK
#define SPI0_SCK    GPIO36  /**< FSPICLK used as signal `OLED_CLK` */
#endif
#ifndef SPI0_MISO
#define SPI0_MISO   GPIO9   /**< FSPIHD dummy (not broken out), GPIO37 is used as DCX */
#endif
#ifndef SPI0_MOSI
#define SPI0_MOSI   GPIO35  /**< FSPID used as Display signal `OLED_MOSI` */
#endif
#ifndef SPI0_CS0
#define SPI0_CS0    GPIO34  /**< FSPICS0 used as Display signal `OLED_CS` */
#endif

#ifndef SPI1_CTRL
#define SPI1_CTRL   HSPI    /**< HSPI (SPI Controller 3) is used as SPI_DEV(1) */
#endif
#ifndef SPI1_SCK
#define SPI1_SCK    GPIO12  /**< SPI SCK */
#endif
#ifndef SPI1_MISO
#define SPI1_MISO   GPIO13  /**< SPI MISO */
#endif
#ifndef SPI1_MOSI
#define SPI1_MOSI   GPIO11  /**< SPI MOSI */
#endif
#ifndef SPI1_CS0
#define SPI1_CS0    GPIO10  /**< SPI CS0 used for Display */
#endif
/** @} */

/**
 * @name   UART configuration
 *
 * ESP32-S2 provides 2 UART interfaces at maximum:
 *
 * UART_DEV(0) uses fixed standard configuration.<br>
 * UART_DEV(1) is not used.<br>
 *
 * @{
 */
#define UART0_TXD   GPIO43  /**< direct I/O pin for UART_DEV(0) TxD, can't be changed */
#define UART0_RXD   GPIO44  /**< direct I/O pin for UART_DEV(0) RxD, can't be changed */

/** @} */

#ifdef __cplusplus
} /* end extern "C" */
#endif

/* include common peripheral definitions as last step */
#include "periph_conf_common.h"

#endif

#endif /* PERIPH_CONF_H */
/** @} */

