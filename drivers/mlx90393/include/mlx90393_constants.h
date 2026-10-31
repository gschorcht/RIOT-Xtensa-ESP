/*
 * Copyright (C) 2023 mi6527ri
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
 * @brief       MLX90393 internal addresses, registers and constants
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 */

#ifndef MLX90393_CONSTANTS_H
#define MLX90393_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Register address map
 */
#define MLX90393_REG_CONF0          0x00    /**< Configuration register 0 */
#define MLX90393_REG_CONF1          0x01    /**< Configuration register 1 */
#define MLX90393_REG_CONF2          0x02    /**< Configuration register 2 */
#define MLX90393_REG_SENS_TC        0x03    /**< Sensitivity drift compensation factor */
#define MLX90393_REG_OFFSET_X       0x04    /**< Offset for x axis */
#define MLX90393_REG_OFFSET_Y       0x05    /**< Offset for y axis */
#define MLX90393_REG_OFFSET_Z       0x06    /**< Offset for z axis */
#define MLX90393_REG_WOXY_THRESHOLD 0x07    /**< Wake up on change threshold for x and y axis */
#define MLX90393_REG_WOZ_THRESHOLD  0x08    /**< Wake up on change threshold for z axis */
#define MLX90393_REG_WOT_THRESHOLD  0x09    /**< Wake up on change threshold for temp */
#define MLX90393_REG_CONN_TEST      0x0A    /**< Free available register used for
                                                 connectivity test */
#define MLX90393_REG_REF_TEMP       0x24    /**< Reference temperature */

/**
 * @brief   Configuration parameter bit masks
 */
#define MLX90393_MASK_BDR           0x003F  /**< burst data rate */
#define MLX90393_MASK_WOC_DIFF      0x1000  /**< wake up on change mode (relative or absolute) */
#define MLX90393_MASK_TCMP_EN       0x0400  /**< enable temperature compensation */
#define MLX90393_MASK_GAIN_SEL      0x0070  /**< analog chain gain */
#define MLX90393_MASK_RES_XYZ       0x07E0  /**< xyz resolution */
#define MLX90393_MASK_COMM_MODE     0x6000  /**< set allowed bus communication (I2C or SPI) */
#define MLX90393_MASK_OSR           0x0003  /**< oversampling ratio magnetic sensor */
#define MLX90393_MASK_OSR2          0x1800  /**< oversampling ratio temperature */
#define MLX90393_MASK_DIG_FILT      0x001C  /**< digital filter magnetic sensor */

/**
 * @brief   Number of left shifts in configuration registers
 */
#define MLX90393_SHIFT_OSR2         11      /**< oversampling ratio temperature */
#define MLX90393_SHIFT_DIG_FILT     2       /**< digital filter magnetic sensor */
#define MLX90393_SHIFT_GAIN         4       /**< analog chain gain */
#define MLX90393_SHIFT_RES_X        5       /**< x resolution */
#define MLX90393_SHIFT_RES_Y        7       /**< y resolution */
#define MLX90393_SHIFT_RES_Z        9       /**< z resolution */
#define MLX90393_SHIFT_WOC_MODE     12      /**< wake-up on change mode */

/**
 * @brief   Command Set
 */
#define MLX90393_COMMAND_SB         0x1F    /**< start burst mode */
#define MLX90393_COMMAND_SW         0x2F    /**< start wake up on change mode */
#define MLX90393_COMMAND_SM         0x3F    /**< start single measurement mode */
#define MLX90393_COMMAND_RM         0x4F    /**< read measurement */
#define MLX90393_COMMAND_RR         0x50    /**< read register */
#define MLX90393_COMMAND_WR         0x60    /**< write register */
#define MLX90393_COMMAND_EX         0x80    /**< exit mode */
#define MLX90393_COMMAND_HR         0xD0    /**< memory recall */
#define MLX90393_COMMAND_HS         0xE0    /**< memory store */
#define MLX90393_COMMAND_RT         0xF0    /**< reset */

/**
 * @brief   Status byte bit map
 */
#define MLX90393_STATUS_RESET       0x04    /**< reset bit */
#define MLX90393_STATUS_ERROR       0x10    /**< error bit */

/**
 * @brief   Timeout durations in ms
 */
#define MLX90393_COMMAND_EX_TIMEOUT     1    /**< Timeout after exit command */
#define MLX90393_COMMAND_RT_TIMEOUT     2    /**< Timeout after reset command */

/**
 * @brief   Temperature conversion constants
 */
#define MLX90393_TEMP_OFFSET        3500    /**< Temperature offset in centi celsius */
#define MLX90393_TEMP_RESOLUTION    452     /**< Temperature sensor resolution
                                                 (45.2 * 10 for avoiding float values) */

/**
 * @brief   Magnetic flux conversion constants
 */
#define MLX90393_XY_SENS            150     /**< xy sensitivity in nT/LSB */
#define MLX90393_Z_SENS             242     /**< z sensitivity in nT/LSB */

/**
 * @brief   Timing constants
 */
#define MLX90393_T_STBY             264     /**< Time from IDLE to STBY in us */
#define MLX90393_T_ACTIVE           432     /**< Time from STBY to ACTIVE in us */
#define MLX90393_T_CONV_END         120     /**< Time to end analog active mode in us */

#ifdef __cplusplus
}
#endif

#endif /* MLX90393_CONSTANTS_H */
/** @} */
