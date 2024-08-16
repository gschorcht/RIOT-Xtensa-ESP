/*
 * Copyright (C) 2023 Michael Ristau
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mlx90393
 * @{
 *
 * @file
 * @brief       Test application for the MLX90393 magnetometer driver.
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 *
 * ## About
 *
 * The test application demonstrates the use of different functions of
 * the MLX90393 sensor driver depending on the used modules and configuration params.
 *
 * Pseudomodule        | Functionality
 * :-------------------|:-------------------------------------------------------
 * `mlx90393_i2c`      | Use I2C bus
 * `mlx90393_spi`      | Use SPI bus
 * `mlx90393_int`      | Data ready interrupt handling
 * `mlx90393_woc`      | Wake-up on change mode
 *
 * By default the test application uses the I2C bus, polling and the default params set
 * defined in file mlx90393_params.h. The default params use the Burst mode.
 * To use data ready interrupts instead of polling for new data, the `mlx90393_int` module
 * has to be used.
 *
 * ## Usage
 * To compile and execute the test application, use command in the test directory:
 * make BOARD=... flash
 *
 * To test the different driver functions you can overwrite the parameters in the
 * default configuration set or add modules.
 *
 * Some examples:
 *
 * Wake-up on change mode absolute:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * CFLAGS="-DMLX90393_PARAM_MODE=MLX90393_MODE_WOC_ABSOLUTE" \
 * USEMODULE='mlx90393_woc' \
 * make BOARD=... flash
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Single measurement mode:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * CFLAGS="-DMLX90393_PARAM_MODE=MLX90393_MODE_SINGLE_MEASUREMENT" \
 * make BOARD=... flash
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Burst mode with interrupt:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * CFLAGS="-DMLX90393_PARAM_MODE=MLX90393_MODE_BURST" \
 * USEMODULE='mlx90393_int' \
 * make BOARD=... flash
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * To test the sensor with the SPI Interface you can use the mlx90393_spi module:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DRIVER='mlx90393_spi' \
 * make BOARD=... flash
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "mlx90393.h"
#include "mlx90393_params.h"
#include "ztimer.h"

#define BUS_ERROR                   -EIO
#define DEVICE_NOT_AVAILABLE        -ENXIO
#define INVALID_PARAM               -EINVAL
#define DEVICE_ERROR                -EFAULT

static void print_error(int error)
{
    switch (error)
    {
    case 0:
        puts("No error");
        break;
    case BUS_ERROR:
        puts("Communication bus error");
        break;
    case DEVICE_NOT_AVAILABLE:
        puts("Connectivity error, device not available");
        break;
    case INVALID_PARAM:
        puts("Invalid configuration parameter");
        break;
    case DEVICE_ERROR:
        puts("Device error");
        break;
    default:
        break;
    }
}

#if IS_USED(MODULE_MLX90393_WOC)
void woc_cb(void *arg)
{
    *(int*)arg = 1;
}
#endif

int main(void)
{
    puts("MLX90393 magnetometer driver test application\n\r");
#if IS_USED(MODULE_MLX90393_SPI)
    printf("Initializing MLX90393 magnetometer at SPI_%i", mlx90393_params[0].spi);
#elif IS_USED(MODULE_MLX90393_I2C)
    printf("Initializing MLX90393 magnetometer at I2C_%i", mlx90393_params[0].i2c);
#endif

    mlx90393_t dev;
    int error = 0;

#if IS_USED(MODULE_MLX90393_WOC)
    int woc_triggered = 0;
#endif

    if ((error = mlx90393_init(&dev, &mlx90393_params[0])) != 0) {
        puts("[FAILED]");
        print_error(error);
        return -1;
    }
    puts("[SUCCESS]");

    unsigned count = 0;
    mlx90393_data_t data;

    puts("Starting read data from the device");

#if IS_USED(MODULE_MLX90393_WOC)
    mlx90393_enable_woc(&dev, woc_cb, &woc_triggered);
#endif
    while (1) {

#if IS_USED(MODULE_MLX90393_WOC)
        if (woc_triggered) {
            if ((error = mlx90393_read(&dev, &data)) != 0) {
                puts("Failed to read data from the device");
                print_error(error);
                return -1;
            }
            printf("Field strength: X: %ld uT Y: %ld uT Z: %ld uT\n\r",
                data.x_axis, data.y_axis, data.z_axis);
            printf("Temperature: %d d°C\n\r", data.temp);
            count++;
            woc_triggered = 0;
            mlx90393_enable_woc(&dev, woc_cb, &woc_triggered);
        }
#endif
        if (dev.params->mode == MLX90393_MODE_BURST ||
            dev.params->mode == MLX90393_MODE_SINGLE_MEASUREMENT) {
            if ((error = mlx90393_read(&dev, &data)) != 0) {
                puts("Failed to read data from the device");
                print_error(error);
                return -1;
            }
            printf("Field strength: X: %ld uT Y: %ld uT Z: %ld uT\n\r",
                data.x_axis, data.y_axis, data.z_axis);
            printf("Temperature: %d d°C\n\r", data.temp);

            if (dev.params->mode == MLX90393_MODE_SINGLE_MEASUREMENT) {
                ztimer_sleep(ZTIMER_SEC, 1);
            }
            count++;
        }

        /*
         * the continuous measurement is stopped, the sensor is set to idle mode
         * and started again after 5 seconds every 50 cycles
         */
        if (dev.params->mode != MLX90393_MODE_SINGLE_MEASUREMENT && count == 50) {
            mlx90393_stop_cont(&dev);
            puts("Measurement stopped and sensor set to idle mode.");
            ztimer_sleep(ZTIMER_SEC, 5);
            mlx90393_start_cont(&dev);
            puts("Measurement started again.");
            count = 0;
        }
    }
    return 0;
}
