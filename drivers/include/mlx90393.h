/*
 * Copyright (C) 2023 Michael Ristau
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_mlx90393 MLX90393 3-axis magnetometer
 * @ingroup     drivers_sensors
 * @ingroup     drivers_saul
 * @brief       Device driver for the MLX90393 3-axis magnetometer
 *
 * # Overview
 *
 * ## About the sensor
 *
 * The MLX90393 is a magnetic field sensor offering a 16-bit output proportional
 * to the magnetic flux density sensed along the X, Y, and Z axes.
 * In addition, the sensor offers a 16-bit temperature output signal. These values
 * are available via I2C and SPI. The sensor can be run in single shot and continuous
 * modes, see below.
 * To minimize host operations, interrupts can be used either when new sensor data is
 * ready to be read or when sensor values exceed configured thresholds.
 *
 * ## Supported Features
 *
 * Functionality the driver supports:
 * - Magnetic field and temperature sensing in single-shot or continuous mode
 * - Configurable thresholds for triggering interrupt
 * - Configurable output data rate
 * - Configurable analog chain gain
 * - Configurable 16-bit output value from the 19-bit ADC
 * - Configurable oversampling ratio for meagnetic and temperature sensor
 * - Digital filter applicable to ADC
 * - Support for I2C and SPI
 * - Measurement retrieval via interrupt or polling
 * - Setting the sensor into idle (sleep) mode
 * - Fixed configuration of the sensor by a default parameter set of
 *   type #mlx90393_params_t as defined in the file `mlx90393_params.h
 * - SAUL sensor interface
 *
 * The following pseudomodules are used to choose the communication bus or add
 * additional functionalities:
 * <center>
 * Pseudomodule        | Functionality
 * :-------------------|:-------------------------------------------------------
 * `mlx90393_i2c`      | Use I2C bus
 * `mlx90393_spi`      | Use SPI bus
 * `mlx90393_int`      | Data ready interrupt handling
 * `mlx90393_woc`      | Wake-up on change mode
 * </center>
 * <br>
 *
 * @note
 * - If the handling of data ready interrupts is enabled by module `mlx90393_int`,
 *   the GPIO pin for the interrupt signal must be defined by the configuration parameter
 *   mlx90393_params_t::int_pin. The default configuration of this GPIO pin
 *   is defined by #MLX90393_PARAM_INT_PIN that can be overridden by the
 *   board definition. The interrupt signal is HIGH active. With the use of the module
 *   `mlx90393_int` polling is no longer supported.
 * - For the Wake-up on change measurement mode the use of interrupt is mandatory.
 *   If the module `mlx90393_woc` is used, the module `mlx90393_int` is added automatically.
 *
 * # Using the driver
 *
 * ## Operation modes
 *
 * The MLX90393 can be used in three modes:
 *
 * - **Single measurement mode**<br>
 *   The master will ask for data via the corresponding interface (I2C or SPI),
 *   waking up the mlx90393 to make a single conversion, immediately followed by an
 *   automatic return to sleep mode (IDLE) until the next polling of the master.
 *
 * - **Burst mode**<br>
 *   When the sensor is operating in burst mode, it will make continuous conversions at
 *   configurable time intervals. Whenever the MLX90393 has made the selected conversions,
 *   the DRDY signal will be set (active High) on the INT and/or INT/TRG pin to indicate that
 *   the data is ready for read.
 *
 * - **Wake-up on change mode**<br>
 *   The Wake-Up on Change (WOC) functionality can be set to only receive interrupts when a
 *   certain threshold is crossed. In WOC mode, the sensor is operating in burst
 *   mode and compares each new burst value with a reference value to assert an
 *   interrupt if the difference between both exceeds a user-defined threshold.
 *   The reference value is defined as one of the following:
 *   -  The first measurement of WOC mode is stored once as reference value. This measurement at
 *      “t=0” is then the basis for comparison (Absolute mode).
 *   -  The reference for acquisition(t) is always acquisition(t-1) (Relative mode).
 *   For this mode the use of interrupt is mandatory.
 *
 * ## Initialization
 *
 * The **easiest way to use the driver** is simply to initialize the sensor
 * with function #mlx90393_init using the default configuration parameter set
 * #mlx90393_params as defined in file mlx90393_params.h.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
 * static mlx90393_t dev;
 *
 * if (mlx90393_init(&dev, &mlx90393_params[0]) != 0) {
 *     ... // error handling
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * After initialization, the sensor is configured according to the standard
 * configuration parameters and is fully operational.
 *
 * ## Fetching data
 *
 * - In single measurement mode the #mlx90393_read function starts the measurement.
 * - When interrupt is configured the #mlx90393_read function locks a mutex
 *   and sleeps until a measurement is ready and the sensor triggers an interrupt.
 *   In the corresponding ISR the mutex is unlocked and the function proceeds reading
 *   and converting the data.
 * - If interrupt is not used, the function either sleeps for the conversion time
 *   in single measurement mode or performs continuous queries for data in burst mode,
 *   with sleep intervals of 10 ms (polling).
 * - For the wake-up on change mode the use of interrupt is mandatory.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
 * while (1)
 * {
 *     mlx90393_data_t mag_data;
 *
 *     if (mlx90393_read(&dev, &mag_data) != 0) {
 *         ... // error handling
 *     }
 *     ...
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * ## Output data format
 *
 * Function #mlx90393_read returns magnetic field data in micro tesla and temperature
 * in deci celsius in parameter \p data. The range of the magnetic field value depends on:
 * - the analog chain gain defined by mlx90393_params_t::gain
 * - the resolution defined by mlx90393_params_t::resolution
 *
 * ## Using Interrupts
 *
 * The MLX90393 sensor supports the use of data-ready interrupts to signal when data is available.
 * To use interrupts, add the module `mlx90393_int`, connect the INT pin of the MLX90393 to the
 * designated interrupt pin on the MCU, as configured in the parameters.
 * In single measurement and burst mode, when calling the function #mlx90393_read,
 * it will block the calling thread until an interrupt is triggered. Once an interrupt is
 * triggered, the driver handles the interrupt with an internal ISR and then proceeds with
 * reading and converting data and finally returns from the #mlx90393_read function.
 * In wake-up on change mode the #mlx90393_enable_woc function can be used to activate the
 * interrupt which is triggered, when the user defined threshold is crossed.
 * Additionally a callback function for handling the interrupt and its argument are
 * passed to the function.
 * @warning
 * The passed callback function is called in interrupt context and should not be used to access
 * the sensor or execute time-consuming code.
 *
 * ## Power Saving
 *
 * The MLX90393 sensor can be shutdown into idle mode when no continuous measurements are
 * required using the function #mlx90393_stop_cont. The power consumption is then reduced to
 * a maximum of 5 uA. To restart the MLX90393 in previous continuous measurement mode,
 * the #mlx90393_start_cont function can be used.
 * In single measurement mode, once the measurement is completed, the sensor transitions into
 * idle mode, awaiting a new command from the master to initiate another acquisition.
 *
 * @{
 *
 * @file
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 */

#ifndef MLX90393_H
#define MLX90393_H

#include "periph/gpio.h"
#if IS_USED(MODULE_MLX90393_SPI)
#include "periph/spi.h"
#elif IS_USED(MODULE_MLX90393_I2C)
#include "periph/i2c.h"
#endif

/* Add header includes here */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Data container of the MLX90393 sensor
 */
typedef struct {
    int32_t x_axis;                  /**< Magnometer data from x-axis */
    int32_t y_axis;                  /**< Magnometer data from y_axis */
    int32_t z_axis;                  /**< Magnometer data from z_axis */
    int16_t temp;                    /**< Temperature */
} mlx90393_data_t;

/**
 * @brief   Measurement mode of the MLX90393 sensor
 */
typedef enum {
    MLX90393_MODE_BURST,                        /**< Burst mode */
    MLX90393_MODE_SINGLE_MEASUREMENT,           /**< Single measurement mode */
#if IS_USED(MODULE_MLX90393_WOC) || DOXYGEN
    MLX90393_MODE_WOC_ABSOLUTE,                 /**< Wake-up on change mode,
                                                     compared to first measurement */
    MLX90393_MODE_WOC_RELATIVE,                 /**< Wake-up on change mode,
                                                     compared to previous measurement */
#endif
} mlx90393_mode_t;

/**
 * @brief   Output data rate (sample rate) of the MLX90393 sensor
 *
 * @note    When MLX90393_ODR_MAX is used, the output data rata depends only
 *          on the configuration of oversampling ratio and digital filter.
 */
typedef enum {
    MLX90393_ODR_MAX = 0x00,        /**< Maximum output data rate */
    MLX90393_ODR_50HZ = 0x01,       /**< Output data rate 50 Hz */
    MLX90393_ODR_25HZ = 0x02,       /**< Output data rate 25 Hz */
    MLX90393_ODR_12_5HZ = 0x04,     /**< Output data rate 12.5 Hz */
    MLX90393_ODR_10HZ = 0x05,       /**< Output data rate 10 Hz */
    MLX90393_ODR_6_25HZ = 0x08,     /**< Output data rate 6.25 Hz */
    MLX90393_ODR_5HZ = 0x0A,        /**< Output data rate 5 Hz */
    MLX90393_ODR_2_5HZ = 0x14,      /**< Output data rate 2.5 Hz */
    MLX90393_ODR_2HZ = 0x19,        /**< Output data rate 2 Hz */
    MLX90393_ODR_1_25HZ = 0x28,     /**< Output data rate 1.25 Hz */
    MLX90393_ODR_1HZ = 0x32,        /**< Output data rate 1 Hz */
} mlx90393_odr_t;

/**
 * @brief   Analog chain gain of the MLX90393 sensor
 */
typedef enum {
    MLX90393_GAIN_5X,               /**< gain factor 5 */
    MLX90393_GAIN_4X,               /**< gain factor 4 */
    MLX90393_GAIN_3X,               /**< gain factor 3 */
    MLX90393_GAIN_2_5X,             /**< gain factor 2.5 */
    MLX90393_GAIN_2X,               /**< gain factor 2 */
    MLX90393_GAIN_1_67X,            /**< gain factor 1.67 */
    MLX90393_GAIN_1_33X,            /**< gain factor 1.33 */
    MLX90393_GAIN_1X,               /**< gain factor 1 */
} mlx90393_gain_t;

/**
 * @brief   Desired 16-bit output value from the 19-bit ADC of the MLX90393 sensor
 */
typedef enum {
    MLX90393_RES_16,                /**< resolution 16 bit */
    MLX90393_RES_17,                /**< resolution 17 bit */
    MLX90393_RES_18,                /**< resolution 18 bit */
    MLX90393_RES_19,                /**< resolution 19 bit */
} mlx90393_resolution_t;

/**
 * @brief   Oversampling ratio of the MLX90393 sensor
 */
typedef enum {
    MLX90393_OSR_0,                 /**< Oversampling ratio 0 */
    MLX90393_OSR_1,                 /**< Oversampling ratio 1 */
    MLX90393_OSR_2,                 /**< Oversampling ratio 2 */
    MLX90393_OSR_3,                 /**< Oversampling ratio 3 */
} mlx90393_oversampling_ratio_t;

/**
 * @brief   Digital filter applicable to ADC of the MLX90393 sensor
 */
typedef enum {
    MLX90393_DIG_FILT_0,            /**< Digital filter 0 */
    MLX90393_DIG_FILT_1,            /**< Digital filter 1 */
    MLX90393_DIG_FILT_2,            /**< Digital filter 2 */
    MLX90393_DIG_FILT_3,            /**< Digital filter 3 */
    MLX90393_DIG_FILT_4,            /**< Digital filter 4 */
    MLX90393_DIG_FILT_5,            /**< Digital filter 5 */
    MLX90393_DIG_FILT_6,            /**< Digital filter 6 */
    MLX90393_DIG_FILT_7,            /**< Digital filter 7 */
} mlx90393_digital_filter_t;

/**
 * @brief   Oversampling rates of magnetic and temperature sensors
 */
typedef struct {
    mlx90393_oversampling_ratio_t mag;      /**< Oversampling ratio magnetic sensor */
    mlx90393_oversampling_ratio_t temp;     /**< Oversampling ratio temperature sensor */
} mlx90393_oversampling_t;

#if IS_USED(MODULE_MLX90393_WOC) || DOXYGEN
/**
 * @brief   Thresholds for wake-up on change mode
 */
typedef struct {
    int xy;                             /**< Threshold for x and y axes */
    int z;                              /**< Threshold for z axis */
    int temp;                           /**< Threshold for temperature */
} mlx90393_threshold_t;

/**
 * @brief   Signature of callback function triggered from wake-up on change interrupt
 *
 * @param[in] arg       argument for the callback
 */
typedef void (*mlx90393_cb_t)(void *arg);

#endif

/**
 * @brief   Device configuration parameters
 */
typedef struct {
#if IS_USED(MODULE_MLX90393_SPI) || DOXYGEN
    spi_t spi;                              /**< SPI bus */
    gpio_t cs_pin;                          /**< Connected chip select pin */
    spi_clk_t clk;                          /**< clock speed for the SPI bus */
#elif IS_USED(MODULE_MLX90393_I2C) || DOXYGEN
    i2c_t i2c;                              /**< I2C device */
    uint8_t addr;                           /**< Magnometer I2C address */
#endif
    mlx90393_mode_t mode;                   /**< Measurement mode */
#if IS_USED(MODULE_MLX90393_INT) || DOXYGEN
    gpio_t int_pin;                         /**< Interrupt pin */
#endif
    mlx90393_gain_t gain;                   /**< Analog chain gain */
    mlx90393_resolution_t resolution;       /**< Desired 16-bit output value from the 19-bit ADC */
    mlx90393_odr_t odr;                     /**< Output data rate */
    mlx90393_oversampling_t oversampling;   /**< Oversampling ratio */
    mlx90393_digital_filter_t dig_filt;     /**< Digital filter applicable to ADC */
#if IS_USED(MODULE_MLX90393_WOC) || DOXYGEN
    mlx90393_threshold_t threshold;         /**< Threshold for wake-up on change mode */
#endif
} mlx90393_params_t;

/**
 * @brief   Device descriptor for the MLX90393 magnetometer
 */
typedef struct {
    const mlx90393_params_t *params;    /**< Device configuration parameters */
    uint16_t ref_temp;                  /**< Reference temperature for converting raw temp data
                                             in centi celsius */
#if !IS_USED(MODULE_MLX90393_INT) || DOXYGEN
    uint8_t conversion_time;            /**< Conversion time for single measurement mode */
#endif
} mlx90393_t;

/**
 * @brief   Initialize the MLX90393 sensor device
 *
 * This function resets the sensor and initializes it based on the given configuration
 * parameters. Burst and wake-up on change modes are activated immediately. In single
 * measurement mode, the measurement is not initiated by this function, mlx90393_read
 * must be called for that purpose.
 * If wake-up on change mode is used, configuring a valid interrupt pin is mandatory.
 *
 * @note Some configurations of oversampling and digital filter are not permitted:
 *       - #MLX90393_OSR_0 and #MLX90393_DIG_FILT_0
 *       - #MLX90393_OSR_0 and #MLX90393_DIG_FILT_1
 *       - #MLX90393_OSR_1 and #MLX90393_DIG_FILT_0
 *       In such cases, the function will return with -EINVAL.
 *
 * @note The conversion time increases with higher oversampling rates and digital filters.
 *       If the values are too high, it may result in the configured output data rate
 *       not being achieved.
 *
 * @param[inout] dev        Device descriptor of MLX90393 device to be initialized
 * @param[in]    params     Configuration parameters
 *
 * @retval 0                on success
 * @retval -ENXIO           device not available
 * @retval -EIO             SPI or I2C communication error
 * @retval -EINVAL          invalid parameter
 * @retval -EFAULT          device error
 */
int mlx90393_init(mlx90393_t *dev, const mlx90393_params_t *params);

/**
 * @brief   Read magnetic and temperature data from the given device and convert it
 *          to physical values
 *
 * This function performs a single measurement or retrieves data from continuous
 * measurements based on the configured mode. It supports both interrupt-based and
 * polling-based measurement retrieval.
 *
 * If interrupt is not used, the function either sleeps for the conversion time
 * in single measurement mode or performs continuous queries for data in burst mode,
 * with sleep intervals of 10 ms. When interrupt is used, a mutex is locked, and
 * the function sleeps until data is ready, triggering an interrupt.
 *
 * @note For wake-up on change mode the use of interrupt is mandatory.
 *
 * @pre
 * - Device must be properly initialized using mlx90393_init
 *
 * @param[in]   dev         Device descriptor of MLX90393 device to read from
 * @param[out]  data        MLX90393 data in micro Tesla and deci Celsius
 *
 * @retval 0                on success
 * @retval -EIO             SPI or I2C communication error
 * @retval -EFAULT          device error
 */
int mlx90393_read(mlx90393_t *dev, mlx90393_data_t *data);

/**
 * @brief   Stop measurements in continuous mode
 *
 * Burst and Wake-up on change measurement modes are stopped and the device
 * is forced into idle mode. Once stopped, the modes can be restarted with mlx90393_start_cont.
 *
 * @param[in]   dev         Device descriptor of MLX90393 device
 *
 * @retval 0                on success
 * @retval -EIO             SPI or I2C communication error
 * @retval -EFAULT          device error
 */
int mlx90393_stop_cont(mlx90393_t *dev);

/**
 * @brief   Start measurements in continuous mode
 *
 * Turns the device out of idle mode and starts a previously stopped burst or wake-up on change
 * mode with same parameters as defined in configuration parameters. This function does not
 * initialize the device and the continuous measurement mode.
 *
 * @pre
 * - Device must be initialized with mlx90393_init
 * - Continuous measurement was stopped with mlx90393_stop_cont
 *
 * @param[in]   dev         Device descriptor of MLX90393 device
 *
 * @retval 0                on success
 * @retval -EIO             SPI or I2C communication error
 * @retval -EFAULT          device error
 */
int mlx90393_start_cont(mlx90393_t *dev);

#if IS_USED(MODULE_MLX90393_WOC) || DOXYGEN
/**
 * @brief   Activate wake-up on change interrupt
 *
 * Activates the interrupt which is triggered if the user defined woc threshold is crossed.
 * The passed callback function is called when the interrupt occurs. After the interrupt
 * the measurement data can be read with mlx90393_read.
 *
 * @warning
 * The passed callback function is called in interrupt context and should not be used to access
 * the sensor or execute time-consuming code.
 *
 * @param[in]   dev         Device descriptor of MLX90393 device
 * @param       cb          Callback to be executed when interrupt occurs
 * @param       arg         Callback argument
 */
void mlx90393_enable_woc(mlx90393_t *dev, mlx90393_cb_t cb, void *arg);

#endif

#ifdef __cplusplus
}
#endif

#endif /* MLX90393_H */
/** @} */
