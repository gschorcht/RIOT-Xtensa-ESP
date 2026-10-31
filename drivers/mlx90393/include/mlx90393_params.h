/*
 * Copyright (C) 2023 mi6527ri
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mlx90393
 *
 * @{
 * @file
 * @brief       Default configuration for MLX90393 device driver
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 */

#ifndef MLX90393_PARAMS_H
#define MLX90393_PARAMS_H

#include "board.h"
#include "mlx90393.h"
#include "mlx90393_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the MLX90393
 * @{
 */
#ifndef MLX90393_PARAM_MODE
/**
 * @brief   Default measurement mode
 */
#define MLX90393_PARAM_MODE             (MLX90393_MODE_BURST)
#endif

#if IS_USED(MODULE_MLX90393_INT) || DOXYGEN
#ifndef MLX90393_PARAM_INT_PIN
/**
 * @brief   Default interrupt pin
 */
#define MLX90393_PARAM_INT_PIN          .int_pin = (GPIO_PIN(PORT_C, 8)),
#endif
#else
#define MLX90393_PARAM_INT_PIN
#endif

#ifndef MLX90393_PARAM_GAIN
/**
 * @brief   Default gain
 */
#define MLX90393_PARAM_GAIN             (MLX90393_GAIN_1X)
#endif

#ifndef MLX90393_PARAM_RES
/**
 * @brief   Default resolution
 */
#define MLX90393_PARAM_RES              (MLX90393_RES_19)
#endif

#ifndef MLX90393_PARAM_ODR
/**
 * @brief   Default sampling rate
 */
#define MLX90393_PARAM_ODR              (MLX90393_ODR_10HZ)
#endif

#ifndef MLX90393_PARAM_OSR
/**
 * @brief   Default oversampling ratio
 */
#define MLX90393_PARAM_OSR                          \
{                                                   \
    .mag            = MLX90393_OSR_1,               \
    .temp           = MLX90393_OSR_1                \
}
#endif

#ifndef MLX90393_PARAM_DIG_FILT
/**
 * @brief   Default digital filter
 */
#define MLX90393_PARAM_DIG_FILT         (MLX90393_DIG_FILT_1)
#endif

#if IS_USED(MODULE_MLX90393_WOC) || DOXYGEN
#ifndef MLX90393_PARAM_THRESHOLD
/**
 * @brief   Default thresholds for Wake-up On Change mode
 */
#define MLX90393_PARAM_THRESHOLD                    \
.threshold =                                        \
{                                                   \
    .xy             = 0xFFFF,                       \
    .z              = 1000,                         \
    .temp           = 0xFFFF                        \
},
#endif
#else
#define MLX90393_PARAM_THRESHOLD
#endif

/* Default configuration for SPI mode */
#if IS_USED(MODULE_MLX90393_SPI) || DOXYGEN

#ifndef MLX90393_PARAM_SPI
/**
 * @brief   Default SPI device
 */
#define MLX90393_PARAM_SPI              (SPI_DEV(0))
#endif

#ifndef MLX90393_PARAM_SPI_CS_PIN
/**
 * @brief   Default SPI chip select pin
 */
#define MLX90393_PARAM_SPI_CS_PIN       (GPIO_PIN(PORT_B, 6))
#endif

#ifndef MLX90393_PARAM_SPI_CLK
/**
 * @brief   Default SPI clock speed
 */
#define MLX90393_PARAM_SPI_CLK          (SPI_CLK_10MHZ)
#endif

#ifndef MLX90393_PARAMS_SPI
/**
 * @brief   Default SPI params
 */
#define MLX90393_PARAMS_SPI {                                                   \
                                .spi            = MLX90393_PARAM_SPI,           \
                                .cs_pin         = MLX90393_PARAM_SPI_CS_PIN,    \
                                .clk            = MLX90393_PARAM_SPI_CLK,       \
                                .mode           = MLX90393_PARAM_MODE,          \
                                .gain           = MLX90393_PARAM_GAIN,          \
                                .resolution     = MLX90393_PARAM_RES,           \
                                .odr            = MLX90393_PARAM_ODR,           \
                                .oversampling   = MLX90393_PARAM_OSR,           \
                                .dig_filt       = MLX90393_PARAM_DIG_FILT,      \
                                MLX90393_PARAM_INT_PIN                          \
                                MLX90393_PARAM_THRESHOLD                        \
                            }
#endif

/* Default configuration for I2C mode */
#elif IS_USED(MODULE_MLX90393_I2C) || DOXYGEN

#ifndef MLX90393_PARAM_I2C
/**
 * @brief   Default I2C device
 */
#define MLX90393_PARAM_I2C            (I2C_DEV(0))
#endif

#ifndef MLX90393_PARAM_I2C_ADDR
/**
 * @brief   Default I2C device address
 */
#define MLX90393_PARAM_I2C_ADDR       (0x0C)
#endif

#ifndef MLX90393_PARAMS_I2C
/**
 * @brief   Default I2C params
 */
#define MLX90393_PARAMS_I2C {                                                   \
                                .i2c            = MLX90393_PARAM_I2C,           \
                                .addr           = MLX90393_PARAM_I2C_ADDR,      \
                                .mode           = MLX90393_PARAM_MODE,          \
                                .gain           = MLX90393_PARAM_GAIN,          \
                                .resolution     = MLX90393_PARAM_RES,           \
                                .odr            = MLX90393_PARAM_ODR,           \
                                .oversampling   = MLX90393_PARAM_OSR,           \
                                .dig_filt       = MLX90393_PARAM_DIG_FILT,      \
                                MLX90393_PARAM_INT_PIN                          \
                                MLX90393_PARAM_THRESHOLD                        \
                            }
#endif
#endif
/**@}*/

/**
 * @brief   Configure params for MLX90393
 */
static const mlx90393_params_t mlx90393_params[] =
{
#if IS_USED(MODULE_MLX90393_SPI)
    MLX90393_PARAMS_SPI
#elif IS_USED(MODULE_MLX90393_I2C)
    MLX90393_PARAMS_I2C
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* MLX90393_PARAMS_H */
/** @} */
