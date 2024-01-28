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
 * @brief       Device driver implementation for the MLX90393
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 *
 * @}
 */

#include "mlx90393.h"
#include "mlx90393_constants.h"
#include "mlx90393_params.h"
#include "ztimer.h"
#include "imath.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#if MODULE_MLX90393_SPI
#define DEV_SPI             (dev->params.spi)
#define DEV_CS_PIN          (dev->params.cs_pin)
#define DEV_CLK             (dev->params.clk)
#define SPI_MODE            (SPI_MODE_3)
#elif MODULE_MLX90393_I2C
#define DEV_I2C             (dev->params.i2c)
#define DEV_ADDR            (dev->params.addr)
#endif
#define DEV_MODE            (dev->params.mode)
#define DEV_INT_PIN         (dev->params.int_pin)
#define DEV_ODR             (dev->params.odr)
#define DEV_GAIN            (dev->params.gain)
#define DEV_RESOLUTION      (dev->params.resolution)
#define DEV_OSR_MAG         (dev->params.oversampling.mag)
#define DEV_OSR_TEMP        (dev->params.oversampling.temp)
#define DEV_DIG_FILT        (dev->params.dig_filt)
#define DEV_REF_TEMP        (dev->ref_temp)
#define CONN_TEST_DATA      (0xAF03)

#define MLX90393_BM_READ_TIMEOUT    (10)


/** Forward declaration of functions for internal use */
static int _init_bus(const mlx90393_t *dev);
static void _acquire(mlx90393_t *dev);
static void _release(mlx90393_t *dev);
static int _write_byte(mlx90393_t *dev, uint8_t data);
static int _read_byte(mlx90393_t *dev, uint8_t *buffer);
static int _write_bytes(mlx90393_t *dev, void *data, size_t len);
static int _read_bytes(mlx90393_t *dev, void *buffer, size_t len);
static int _check_status_byte(mlx90393_t *dev);
static int _write_register(mlx90393_t *dev, uint8_t addr, uint16_t value);
static int _read_register(mlx90393_t *dev, uint8_t addr, uint16_t *value);
static int _write_register_bits(mlx90393_t *dev, uint8_t addr, uint16_t mask, uint16_t value);
static int _calculate_temp(uint16_t raw_temp, uint16_t ref_temp);
static int _get_gain_factor(mlx90393_gain_t gain);
static int _reset(mlx90393_t *dev);
static int _exit(mlx90393_t *dev);
static int _is_avaiable(mlx90393_t *dev);
static void _calculate_conv_time(mlx90393_t *dev);
static int _read_measurement(mlx90393_t *dev, uint8_t *buffer);

int mlx90393_init(mlx90393_t *dev, const mlx90393_params_t *params)
{
    assert(dev);
    assert(params);
    dev->params = *params;
    int error = 0;
    if ((error = _init_bus(dev)) != MLX90393_SUCCESS) {
        return error;
    }
    _acquire(dev);
    
    /* exit all continuous measurement modes */
    if ((error = _exit(dev)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    ztimer_sleep(ZTIMER_MSEC, MLX90393_COMMAND_EX_TIMEOUT);
    /* reset mlx90393 */
    if ((error = _reset(dev)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    ztimer_sleep(ZTIMER_MSEC, MLX90393_COMMAND_RT_TIMEOUT);
    /* check availability of the sensor */
    if ((error = _is_avaiable(dev)) != MLX90393_SUCCESS) {
        _release(dev);
        DEBUG("[mlx90393] error: device not avaiable\n");
        return error;
    }
    /* store ref temp in dev */
    if ((error = _read_register(dev, MLX90393_REG_REF_TEMP, &DEV_REF_TEMP)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* check oversampling and digital filter configuration */
    if ((DEV_OSR_MAG == 0 && DEV_DIG_FILT == 0) ||
        (DEV_OSR_MAG == 0 && DEV_DIG_FILT == 1) ||
        (DEV_OSR_MAG == 1 && DEV_DIG_FILT == 0)) {
        _release(dev);
        DEBUG("[mlx90393] error: the configuration of oversampling and digital filter is not permitted\n");
        return MLX90393_ERROR;
    }
    /* magnetic sensor oversampling */
    if ((error = _write_register_bits(dev, MLX90393_REG_CONF2, MLX90393_MASK_OSR, DEV_OSR_MAG)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* magnetic sensor digital filter */
    uint16_t dig_filt_value_mask = DEV_DIG_FILT << MLX90393_SHIFT_DIG_FILT;
    if ((error = _write_register_bits(dev, MLX90393_REG_CONF2, MLX90393_MASK_DIG_FILT, dig_filt_value_mask)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* temperature oversampling */
    uint16_t temp_osr_value_mask = DEV_OSR_TEMP << MLX90393_SHIFT_OSR2;
    if ((error = _write_register_bits(dev, MLX90393_REG_CONF2, MLX90393_MASK_OSR2, temp_osr_value_mask)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* gain */
    uint16_t gain_value_mask = DEV_GAIN << MLX90393_SHIFT_GAIN;
    if ((error = _write_register_bits(dev, MLX90393_REG_CONF0, MLX90393_MASK_GAIN_SEL, gain_value_mask)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* resolution */
    uint16_t xyz_resolution_value_mask = (DEV_RESOLUTION << MLX90393_SHIFT_RES_Z) |
                                         (DEV_RESOLUTION << MLX90393_SHIFT_RES_Y) |
                                         (DEV_RESOLUTION << MLX90393_SHIFT_RES_X);
    if ((error = _write_register_bits(dev, MLX90393_REG_CONF2, MLX90393_MASK_RES_XYZ, xyz_resolution_value_mask)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* burst more */
    if (DEV_MODE == MLX90393_MODE_BURST) {
        /* set burst data rate */
        if ((error = _write_register_bits(dev, MLX90393_REG_CONF1, MLX90393_MASK_BDR, DEV_ODR)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        /* start burst mode */
        if ((error = _write_byte(dev, MLX90393_COMMAND_SB)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
    }
    /* wake up on change mode */
    else if (DEV_MODE == MLX90393_MODE_WAKE_UP_ON_CHANGE_ABSOLUTE || DEV_MODE == MLX90393_MODE_WAKE_UP_ON_CHANGE_RELATIVE) {
        if (!gpio_is_valid(DEV_INT_PIN)) {
            _release(dev);
            DEBUG("[mlx90393] error: No valid interrupt pin passed in params\n");
            return MLX90393_ERROR_NO_PIN;
        }
        /* set absolute or relative wake up on change mode */
        int wake_up_on_change_mode = 0;
        if (DEV_MODE == MLX90393_MODE_WAKE_UP_ON_CHANGE_RELATIVE) {
            wake_up_on_change_mode = 1;
        }
        wake_up_on_change_mode <<= MLX90393_SHIFT_WOC_MODE;
        if ((error = _write_register_bits(dev, MLX90393_REG_CONF1, MLX90393_MASK_WOC_DIFF, wake_up_on_change_mode))) {
            _release(dev);
            return error;
        }
        /* set burst data rate */
        if ((error = _write_register_bits(dev, MLX90393_REG_CONF1, MLX90393_MASK_BDR, DEV_ODR)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        /* set tresholds */
        int gain = _get_gain_factor(DEV_GAIN);
        uint16_t raw_xy_threshold = (1000 * dev->params.treshold.xy / (MLX90393_XY_SENS * (1 << DEV_RESOLUTION) * gain)) * 100;
        uint16_t raw_z_threshold = (1000 * dev->params.treshold.z / (MLX90393_Z_SENS * (1 << DEV_RESOLUTION) * gain)) * 100;
        uint16_t raw_temp_threshold = dev->params.treshold.temp * MLX90393_TEMP_RESOLUTION / 1000;

        if ((error = _write_register(dev, MLX90393_REG_WOXY_THRESHOLD, raw_xy_threshold))) {
            _release(dev);
            return error;
        }
        if ((error = _write_register(dev, MLX90393_REG_WOZ_THRESHOLD, raw_z_threshold))) {
            _release(dev);
            return error;
        }
        if ((error = _write_register(dev, MLX90393_REG_WOT_THRESHOLD, raw_temp_threshold))) {
            _release(dev);
            return error;
        }
        /* start wake up on change mode */
        if ((error = _write_byte(dev, MLX90393_COMMAND_SW)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        if((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
    }
    if (DEV_MODE == MLX90393_MODE_SINGLE_MEASUREMENT && !gpio_is_valid(DEV_INT_PIN)) {
        _calculate_conv_time(dev);
    }

    _release(dev);
    return MLX90393_SUCCESS;
}

static void _isr(void *lock)
{
    mutex_unlock(lock);
}

int mlx90393_read(mlx90393_t *dev, mlx90393_data_t *data) 
{
    assert(dev);
    assert(data);
    int error = 0;

    /* start single measurement if used */
    if (DEV_MODE == MLX90393_MODE_SINGLE_MEASUREMENT) {
        _acquire(dev);
        if ((error = _write_byte(dev, MLX90393_COMMAND_SM)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
        _release(dev);
    }
    uint8_t buffer[9];
    /* wait for interrupt if used */
    if (gpio_is_valid(DEV_INT_PIN)) {
        mutex_t lock = MUTEX_INIT_LOCKED;
        gpio_init_int(DEV_INT_PIN, GPIO_IN_PU, GPIO_RISING, _isr, &lock);
        mutex_lock(&lock);
        gpio_irq_disable(DEV_INT_PIN);
        if ((error = _read_measurement(dev, buffer)) != MLX90393_SUCCESS) {
            return error;
        }
    }
    else {
        /* sleep for conversion time in single measurement mode */
        if (DEV_MODE == MLX90393_MODE_SINGLE_MEASUREMENT) {
            ztimer_sleep(ZTIMER_MSEC, dev->conversion_time);
            if ((error = _read_measurement(dev, buffer)) != MLX90393_SUCCESS) {
                return error;
            }
        }
        /* polling in burst mode */
        else if(DEV_MODE ==MLX90393_MODE_BURST) {
            while (_read_measurement(dev, buffer) != MLX90393_SUCCESS) {
                ztimer_sleep(ZTIMER_MSEC, MLX90393_BM_READ_TIMEOUT);
            }
        }
    }

    /* convert read data according to Table 17 and 21 from Datasheet */
    int16_t raw_x, raw_y, raw_z;
    uint16_t raw_temp;
    raw_temp = (uint16_t)((buffer[1] << 8) | buffer[2]);
    raw_x = (int16_t)((buffer[3] << 8) | buffer[4]);
    raw_y = (int16_t)((buffer[5] << 8) | buffer[6]);
    raw_z = (int16_t)((buffer[7] << 8) | buffer[8]);
    
    if (DEV_RESOLUTION == MLX90393_RES_18) {
        raw_x -= 0x8000;
        raw_y -= 0x8000;
        raw_z -= 0x8000;
    }
    else if (DEV_RESOLUTION == MLX90393_RES_19) {
        raw_x -= 0x4000;
        raw_y -= 0x4000;
        raw_z -= 0x4000;
    }

    data->temp = _calculate_temp(raw_temp, DEV_REF_TEMP);
    int gain = _get_gain_factor(DEV_GAIN);
    data->x_axis = ((raw_x * gain) / 100) * MLX90393_XY_SENS * (1 << DEV_RESOLUTION) / 1000;
    data->y_axis = ((raw_y * gain) / 100) * MLX90393_XY_SENS * (1 << DEV_RESOLUTION) / 1000;
    data->z_axis = ((raw_z * gain) / 100) * MLX90393_Z_SENS * (1 << DEV_RESOLUTION) / 1000;

    return MLX90393_SUCCESS;
}

int mlx90393_stop_cont(mlx90393_t *dev)
{
    assert(dev);
    int error = 0;
    _acquire(dev);

    if ((error = _exit(dev)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }

    _release(dev);
    return MLX90393_SUCCESS;
}

int mlx90393_start_cont(mlx90393_t *dev)
{
    assert(dev);

    if (DEV_MODE == MLX90393_MODE_SINGLE_MEASUREMENT) {
        return MLX90393_ERROR;
    }

    int error = 0;
    _acquire(dev);

    if (DEV_MODE == MLX90393_MODE_BURST) {
        if ((error = _write_byte(dev, MLX90393_COMMAND_SB)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
    }
    else if (DEV_MODE == MLX90393_MODE_WAKE_UP_ON_CHANGE_RELATIVE || DEV_MODE == MLX90393_MODE_WAKE_UP_ON_CHANGE_ABSOLUTE) {
        if ((error = _write_byte(dev, MLX90393_COMMAND_SW)) != MLX90393_SUCCESS) {
            _release(dev);
            return error;
        }
    }
    if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    
    _release(dev);
    return MLX90393_SUCCESS;
}

#if MODULE_MLX90393_SPI

static int _init_bus(const mlx90393_t *dev)
{
    if (spi_init_cs(DEV_SPI, DEV_CS_PIN) != SPI_OK) {
        DEBUG("[mlx90393] error: unable to configure the chip select pin\n");
        return MLX90393_ERROR_SPI;
    }
    return MLX90393_SUCCESS;
}

static void _acquire(mlx90393_t *dev)
{
    spi_acquire(DEV_SPI, DEV_CS_PIN, SPI_MODE, DEV_CLK);
}

static void _release(mlx90393_t *dev)
{
    spi_release(DEV_SPI);
}

static int _write_byte(mlx90393_t *dev, uint8_t data)
{
    spi_transfer_byte(DEV_SPI, DEV_CS_PIN, false, data);
    return MLX90393_SUCCESS;
}

static int _read_byte(mlx90393_t *dev, uint8_t *buffer)
{
    *buffer = spi_transfer_byte(DEV_SPI, DEV_CS_PIN, false, 0x0);
    return MLX90393_SUCCESS;
}

static int _write_bytes(mlx90393_t *dev, void *data, size_t len)
{
    spi_transfer_bytes(DEV_SPI, DEV_CS_PIN, false, data, NULL, len);
    return MLX90393_SUCCESS;
}

static int _read_bytes(mlx90393_t *dev, void *buffer, size_t len)
{
    spi_transfer_bytes(DEV_SPI, DEV_CS_PIN, false, NULL, buffer, len);
    return MLX90393_SUCCESS;
}

#elif MODULE_MLX90393_I2C

static int _init_bus(const mlx90393_t *dev)
{
    (void) dev;
    return MLX90393_SUCCESS;
}

static void _acquire(mlx90393_t *dev)
{
    i2c_acquire(DEV_I2C);
}

static void _release(mlx90393_t *dev)
{
    i2c_release(DEV_I2C);
}

static int _write_byte(mlx90393_t *dev, uint8_t data)
{
    return i2c_write_byte(DEV_I2C, DEV_ADDR, data, 0) ? MLX90393_ERROR_I2C : MLX90393_SUCCESS;
}

static int _read_byte(mlx90393_t *dev, uint8_t *buffer)
{
    return i2c_read_byte(DEV_I2C, DEV_ADDR, buffer, 0) ? MLX90393_ERROR_I2C : MLX90393_SUCCESS;
}

static int _write_bytes(mlx90393_t *dev, void *data, size_t len)
{
    return i2c_write_bytes(DEV_I2C, DEV_ADDR, data, len, 0) ? MLX90393_ERROR_I2C : MLX90393_SUCCESS;
}

static int _read_bytes(mlx90393_t *dev, void *buffer, size_t len)
{
    return i2c_read_bytes(DEV_I2C, DEV_ADDR, buffer, len, 0) ? MLX90393_ERROR_I2C : MLX90393_SUCCESS;
}

#endif

static int _check_status_byte(mlx90393_t *dev) 
{
    uint8_t status;
    int error = 0;
    if ((error = _read_byte(dev, &status)) != MLX90393_SUCCESS) {
        return error;
    }
    return (status & MLX90393_STATUS_ERROR) ? MLX90393_ERROR : MLX90393_SUCCESS;
}

static int _write_register(mlx90393_t *dev, uint8_t addr, uint16_t value) 
{
    uint8_t buffer[4];
    buffer[0] = MLX90393_COMMAND_WR;
    buffer[1] = (uint8_t) (value >> 8);
    buffer[2] = (uint8_t) (value & 0xFF);
    buffer[3] = addr << 2;
    int error = 0;
    if ((error = _write_bytes(dev, buffer, 4)) != MLX90393_SUCCESS) {
        return error;
    }
    if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
        return error;
    }
    return MLX90393_SUCCESS;
}

static int _read_register(mlx90393_t *dev, uint8_t addr, uint16_t *value)
{
    uint8_t buffer_send[2];
    buffer_send[0] = MLX90393_COMMAND_RR;
    buffer_send[1] = addr << 2;
    int error = 0;
    if ((error = _write_bytes(dev, buffer_send, 2)) != MLX90393_SUCCESS) {
        return error;
    }
    uint8_t buffer_receive[3];
    if ((error = _read_bytes(dev, buffer_receive, 3)) != MLX90393_SUCCESS) {
        return error;
    }
    if (buffer_receive[0] & MLX90393_STATUS_ERROR) {
        return MLX90393_ERROR;
    }
    *value = (uint16_t)((buffer_receive[1] << 8) | buffer_receive[2]);

    return MLX90393_SUCCESS;
}

static int _write_register_bits(mlx90393_t *dev, uint8_t addr, uint16_t mask, uint16_t value) 
{
    uint16_t reg_value;
    int error = 0;
    if ((error = _read_register(dev, addr, &reg_value)) != MLX90393_SUCCESS) {
        return error;
    }
    reg_value &= ~mask;
    reg_value |= (mask & value);
    if ((error = _write_register(dev, addr, reg_value)) != MLX90393_SUCCESS) {
        return error;
    }
    return MLX90393_SUCCESS;
}

static int _calculate_temp(uint16_t raw_temp, uint16_t ref_temp)
{
    /* calculate temp in deci celsius (Application note MLX90393 temperature compensation - v4) */
    return (MLX90393_TEMP_OFFSET + (((raw_temp - ref_temp) * 1000) / MLX90393_TEMP_RESOLUTION));
}

static int _get_gain_factor(mlx90393_gain_t gain)
{
    switch (gain)
    {
    case MLX90393_GAIN_5X:
        return 500;
    case MLX90393_GAIN_4X:
        return 400;
    case MLX90393_GAIN_3X:
        return 300;
    case MLX90393_GAIN_2_5X:
        return 250;
    case MLX90393_GAIN_2X:
        return 200;
    case MLX90393_GAIN_1_67X:
        return 167;
    case MLX90393_GAIN_1_33X:
        return 133;
    case MLX90393_GAIN_1X:
        return 100;
    default:
        return -1;
    }
}

static int _reset(mlx90393_t *dev)
{
    int error = 0;
    if ((error = _write_byte(dev, MLX90393_COMMAND_RT)) != MLX90393_SUCCESS) {
        return error;
    }
    if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
        return error;
    }
    return MLX90393_SUCCESS;
}

static int _exit(mlx90393_t *dev) 
{
    int error = 0;
    if ((error = _write_byte(dev, MLX90393_COMMAND_EX)) != MLX90393_SUCCESS) {
        return error;
    }
    if ((error = _check_status_byte(dev)) != MLX90393_SUCCESS) {
        return error;
    }
    return MLX90393_SUCCESS;
}

static int _is_avaiable(mlx90393_t *dev) 
{
    int error = 0;
    if ((error = _write_register(dev, MLX90393_REG_CONN_TEST, CONN_TEST_DATA)) != MLX90393_SUCCESS) {
        return error;
    }
    uint16_t buffer = 0x00;
    if ((error = _read_register(dev, MLX90393_REG_CONN_TEST, &buffer)) != MLX90393_SUCCESS) {
        return error;
    }
    return buffer == CONN_TEST_DATA ? MLX90393_SUCCESS : MLX90393_ERROR_NOT_AVAILABLE;
}

static void _calculate_conv_time(mlx90393_t *dev)
{
    /* calculate single measurement conversion time in ms (Datasheet table 8: Timing specifications) */
    int conv_mag = 67 + 64 * powi(2, DEV_OSR_MAG) * (2 + powi(2, DEV_DIG_FILT));
    int conv_temp = 67 + 192 * powi(2, DEV_OSR_TEMP);
    dev->conversion_time = (MLX90393_T_STBY + MLX90393_T_ACTIVE + 3 * conv_mag + conv_temp + MLX90393_T_CONV_END) / 1000 + 1;
}

static int _read_measurement(mlx90393_t *dev, uint8_t *buffer)
{
    int error = 0;
    _acquire(dev);
    /* read measurement */
    if ((error = _write_byte(dev, MLX90393_COMMAND_RM)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    /* check status byte */
    if ((error = _read_bytes(dev, buffer, 9)) != MLX90393_SUCCESS) {
        _release(dev);
        return error;
    }
    if (buffer[0] & MLX90393_STATUS_ERROR) {
        DEBUG("Data could not be read out\n\r");
        _release(dev);
        return MLX90393_ERROR;
    }
    _release(dev);
    return MLX90393_SUCCESS;
}

