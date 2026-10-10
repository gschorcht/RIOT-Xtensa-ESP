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
 * @brief       MLX90393 adaption to the RIOT actuator/sensor interface
 *
 * @author      Michael Ristau <michael.ristau@fh-erfurt.de>
 *
 * @}
 */

#include "saul.h"
#include "mlx90393.h"

static int read(const void *dev, phydat_t *res)
{
    mlx90393_data_t data;

    if (mlx90393_read((mlx90393_t*) dev, &data) == 0) {
        res->val[0] = (int16_t)(data.x_axis / 10);
        res->val[1] = (int16_t)(data.y_axis / 10);
        res->val[2] = (int16_t)(data.z_axis / 10);
        res->unit = UNIT_T;
        res->scale = -5;
        return 1;
    }
    return -ECANCELED;
}

const saul_driver_t mlx90393_saul_driver = {
    .read = read,
    .write = saul_write_notsup,
    .type = SAUL_SENSE_MAG,
};
