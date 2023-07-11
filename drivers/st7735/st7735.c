/*
 * Copyright (C) 2018 Koen Zandberg
 *               2021 Francisco Molina
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_st7735
 * @{
 *
 * @file
 * @brief       Device driver implementation for the st7735 display controller
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @author      Francisco Molina <francois-xavier.molina@inria.fr>
 *
 * @}
 */

#include <assert.h>
#include <string.h>
#include "byteorder.h"
#include "periph/spi.h"
#include "ztimer.h"
#include "kernel_defines.h"

#include "st7735.h"
#include "st7735_internal.h"
#include "lcd.h"
#include "lcd_internal.h"

#define ENABLE_DEBUG 1
#include "debug.h"

#if MODULE_ST7789

#if CONFIG_ST7789_CUSTOM_CONFIG
/* avdd in mV with 200 mV increments: 6600 = 6.6V
 * Datasheet page 289:
 *
 *      y = 0                       for avdd < 6400         (< 6.4V)
 *      y = (avdd - 6400) / 200     for avdd = 6400 to 6800 (6.4 to 6.8V)
 *      y = 2                       for avdd > 6800         (> 6.8V)
 *
 * default value after reset is 6.8 V (0x02)
 */
static inline uint8_t _st7789_calc_avdd(int16_t avdd)
{
    assert((avdd >= 6400) && (avdd <= 6800));
    assert((avdd % 200) == 0);

    return (avdd - 6400) / 200;
}

/* avcl in mV with 200 mV increments: -4800 = -4.8V
 * Datasheet page 289:
 *
 *      y = 0                           for avcl < -5000          (< -5.0V)
 *      y = 3 - ((avcl + 5000) / 200)   for avcl = -5000 to -4400 (-5.0V to -4.4V)
 *      y = 3                           for avcl > -4400          (> -4.4V)
 *
 * default value after reset is -4.8 V (0x02)
 */
static inline uint8_t _st7789_calc_avcl(int16_t avcl)
{
    assert((avcl >= -5000) && (avcl <= -4400));
    assert((avcl % 200) == 0);

    return 3 - ((avcl + 5000) / 200);
}

/* vcom in mV with 25 mV increments: 1325 = 1.325V
 * Datasheet page 270:
 *
 *      y = 0                   for vcom < 100         (< 0.1V)
 *      y = (vcom - 100) / 25   for vcom = 100 to 1675 (0.1V to 1.675V)
 *      y = 63                  for vcom > 1675        (> 1.675V)
 *
 * default value after reset is 0.9 V (0x20)
 */
static inline uint8_t _st7789_calc_vcom(int16_t vcom)
{
    assert((vcom >= 100) && (vcom <= 1675));
    assert((vcom % 25) == 0);

    return (vcom - 100) / 25;
}

/* vol in mV with 25 mV increments: 100 = 0.1V
 * Datasheet page 279:
 *
 *      y = 0                   for vol < -800        (< -0.8V)
 *      y = (vol + 800) / 25    for vol = -800 to 775 (-0.8V to 0.775V)
 *      y = 63                  for vol > 0.775       (> 0.775V)
 *
 * default value after reset is 0 V (0x00)
 */
static inline uint8_t _st7789_calc_vcom_offset_vdv(int16_t vol)
{
    assert((vol >= -800) && (vol <= 775));
    assert((vol % 25) == 0);

    return (vol + 800) / 25;
}

/* vrh in mV with 50 mV increments: 4800 = 4.8V
 * Datasheet page 277:
 *
 *      y = 0                   for vrh < 3550         (< 3.55V)
 *      y = (vrh - 3550) / 50   for vrh = 3550 to 5500 (3.55V to 5.5V)
 *      y = 39                  for vrh > 5500         (> -4.4V)
 *
 * default value after reset is 4.1 V (0x0b)
 */
static inline uint8_t _st7789_calc_vrh(int16_t vrh)
{
    assert((vrh >= 3550) && (vrh <= 5500));
    assert((vrh % 50) == 0);

    return (vrh - 3550) / 50;
}
#endif /* CONFIG_ST7789_CUSTOM_CONFIG */

#elif MODULE_ST7796

#if CONFIG_ST7796_CUSTOM_CONFIG
/* avdd in mV with 200 mV increments: 6600 = 6.6V
 * Datasheet page 223:
 *
 *      y = 0                       for avdd < 6200         (< 6.2V)
 *      y = (avdd - 6200) / 200     for avdd = 6200 to 6800 (6.2 to 6.8V)
 *      y = 3                       for avdd > 6800         (> 6.8V)
 *
 * default value after reset is 6.6 V (0x02)
 */
static inline uint8_t _st7796_calc_avdd(int16_t avdd)
{
    assert((avdd >= 6200) && (avdd <= 6800));
    assert((avdd % 200) == 0);

    return (avdd - 6200) / 200;
}

/* avcl in mV with 200 mV increments: -4800 = -4.8V
 * Datasheet page 223:
 *
 *      y = 0                           for avcl < -5000          (< -5.0V)
 *      y = 3 - ((avcl + 5000) / 200)   for avcl = -5000 to -4400 (-5.0V to -4.4V)
 *      y = 3                           for avcl > -4400          (> -4.4V)
 *
 * default value after reset is -4.4 V (0x00)
 */
static inline uint8_t _st7796_calc_avcl(int16_t avcl)
{
    assert((avcl >= -5000) && (avcl <= -4400));
    assert((avcl % 200) == 0);

    return 3 - ((avcl + 5000) / 200);
}

/* vcom in mV with 25 mV increments: 1325 = 1.325V
 * Datasheet page 227:
 *
 *      y = 0                   for vcom < 300         (< 0.3V)
 *      y = (vcom - 300) / 25   for vcom = 100 to 1875 (0.3V to 1.875V)
 *      y = 63                  for vcom > 1685        (> 1.875V)
 *
 * default value after reset is 1.0V (0x1c)
 */
static inline uint8_t _st7796_calc_vcom(int16_t vcom)
{
    assert((vcom >= 300) && (vcom <= 1875));
    assert((vcom % 25) == 0);

    return (vcom - 300) / 25;
}

/* vol in mV with 25 mV increments: 100 = 0.1V
 * Datasheet page 229:
 *
 *      y = 0                   for vol < -800        (< -0.8V)
 *      y = (vol + 800) / 25    for vol = -800 to 775 (-0.8V to 0.775V)
 *      y = 63                  for vol > 0.775       (> 0.775V)
 *
 * default value after reset is 0 V (0x00)
 */
static inline uint8_t _st7796_calc_vcom_offset(int16_t off)
{
    assert((off >= -800) && (off <= 775));
    assert((off % 25) == 0);

    return (off < 0) ? 32 + ((off + 800) / 25) : off / 25;
}

/* vrh in mV with 50 mV increments: 4800 = 4.8V
 * Datasheet page 224:
 *
 *      y = 0                   for vrh < 3550         (< 3.55V)
 *      y = (vrh - 3550) / 50   for vrh = 3550 to 5500 (3.55V to 5.5V)
 *      y = 39                  for vrh > 5500         (> -4.4V)
 *
 * default value after reset is 4.5 V (0x13)
 */
static inline uint8_t _st7796_calc_vrh(int16_t vrh)
{
    assert((vrh >= 3550) && (vrh <= 5500));
    assert((vrh % 50) == 0);

    return (vrh - 3550) / 50;
}
#endif /* CONFIG_ST7796_CUSTOM_CONFIG */

#else /* MODULE_ST7735 */

#if CONFIG_ST7735_CUSTOM_CONFIG
/* avdd in mV with in 100 mV steps: 4600 = 4.6V
 * Datasheet page 130:
 *
 *      y = 0                           for avdd < 4500         (< 4.5V)
 *      y = (5100 - avdd) / 100         for avdd = 4500 to 5100 (4.5V to 5.1V)
 *      y = 6                           for avdd > 5100         (> 5.1V)
 *
 * default value after reset is 4.9 V (0x04)
 */
static inline uint8_t _st7735_calc_avdd(uint16_t avdd)
{
    assert((avdd >= 4500) && (avdd <= 5100));
    assert((avdd % 100) == 0);

    return (5100 - avdd) / 100;
}

/* gvdd in mV with 50 mV increments: 4650 = 4.65V
 * Datasheet page 130:
 *
 *      y = 31                          for gddv < 3150         (< 3.15V)
 *      y = 31 - ((gddv - 3150) / 50)   for gddv = 3150 to 4700 (3.15V to 4.7V)
 *      y = 0                           for gddv > 4700         (> 4.7V)
 *
 * default value after reset is 4.6 V (0x02)
 */
static inline uint8_t _st7735_calc_gvdd(uint16_t gvdd)
{
    assert((gvdd >= 3150) && (gvdd <= 4700));
    assert((gvdd % 50) == 0);

    return 31 - ((gvdd - 3150) / 50);
}

/* gvcl in mV with 50 mV increments: -4650 = -4.65V
 * Datasheet page 130:
 *
 *      y = 0                           for gdcl < -4700          (< -4.7V)
 *      y = 31 - (-3150 - gvcl) / 50)   for gddv = -4700 to -3150 (-4.7V to -3.15V)
 *      y = 31                          for gddv > -3150          (> -3.15V)
 *
 * default value after reset is -4.6 V (0x02)
 */
static inline uint8_t _st7735_calc_gvcl(int16_t gvcl)
{
    assert((gvcl >= -4700) && (gvcl <= -3150));
    assert((gvcl % 50) == 0);

    return 31 - ((-3150 - gvcl) / 50);
}

/* vcom in mV with 25 mV increments: -625 = -0.625V
 * Datasheet page 140:
 *
 *      y = 63                          for vcom < -2000         (> 2V)
 *      y = 63 - ((2000 + vcom) / 25)   for vcom = -2000 to -425 (-2V to -0.425V)
 *      y = 0                           for vcom > -425          (< -0.425V)
 *
 * default value after reset is 4.9 V (4)
 */
static inline uint8_t _st7735_calc_vcom(int16_t vcom)
{
    assert((vcom >= -2000) && (vcom <= 425));
    assert((vcom % 25) == 0);

    return 63 - ((2000 + vcom) / 25);
}

/* vgh in mV with 100 mV increments: 11200 = 11.2V
 * vgl in mV with 2500 mV increments: 12500 = 12.5V
 * Datasheet page 132
 */
static inline uint8_t _st7735_calc_vghl(uint16_t vgh, int16_t vgl, uint16_t avdd)
{
    assert((vgh >= 10000) && (vgh <= 15000));
    assert((vgl >= -13000) && (vgl <= 7500));
    assert((vgh >= ((avdd * 2) + 2100)) && (vgh <= ((3 * avdd) + 2400)));

    uint16_t bt = vgh / avdd;
    uint16_t h25 = 0;
    assert((bt == 2) || (bt == 3)); /* bt must be either 2 or 3 */

    if ((vgh - (bt * avdd)) > 2100) {
        /* if there remains an offset of at least 2.1V, use VGH25 */
        h25 = ((vgh - (bt * avdd)) - 2100) / 100;
        assert(h25 <= 3);
    }

    bt -= 2;    /* convert (3 * AVDD) to 01b and (2 * AVDD) to 00b */

    if (bt && h25) {
        /* represents 3 * AVDD + VGH25 */
        bt++;
    }
    else {
        h25 = 3;
    }

    uint16_t sel = (vgl < -12500) ? 3 : 2 - ((vgl + 12500) / 2500);

    return (h25 << 6) + (sel << 2) + bt;
}
#endif /* CONFIG_ST7735_CUSTOM_CONFIG */
#endif

static int _init(lcd_t *dev, const lcd_params_t *params)
{
    if (IS_USED(MODULE_ST7789)) {
        assert(params->lines <= 320);
    }
    else {
        assert(params->lines <= 162);
    }

    uint8_t command_params[5] = { 0 };

    gpio_init(dev->params->dcx_pin, GPIO_OUT);
    int res = spi_init_cs(dev->params->spi, dev->params->cs_pin);

    /* Soft Reset, requires 120 ms if in Sleep In mode */
    lcd_ll_write_cmd(dev, LCD_CMD_SWRESET, NULL, 0);
    ztimer_sleep(ZTIMER_MSEC, 120);

    /* Sleep Out command to leave Sleep In state after reset, requires 120 ms */
    lcd_ll_write_cmd(dev, LCD_CMD_SLPOUT, NULL, 0);
    ztimer_sleep(ZTIMER_MSEC, 120);

#if MODULE_ST7789
    DEBUG("ST7789 used ...\n");

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* LCMCTRL (C0h): LCM Control (== reset defaults) */
    command_params[0] = 0x2c;   /* XOR RGB, MX and MH setting in command 36h */
    lcd_ll_write_cmd(dev, LCD_CMD_LCMCTRL, command_params, 1);
    DEBUG("LCMCTRL (C0h) %02x\n", command_params[0]);

    /* VDVVRHEN (C2h): VDV and VRH Command Enable (== reset defaults) */
    command_params[0] = 0x01;   /* CMDEN=1 (VDV and VDH command write enable */
    command_params[1] = 0xff;
    lcd_ll_write_cmd(dev, LCD_CMD_VDVVRHEN, command_params, 2);
    DEBUG("VDVVRHEN (C2h) %02x %02x\n", command_params[0], command_params[1]);

#endif /* no need to write reset defaults, just for documentation purpose */

#if CONFIG_ST7789_CUSTOM_CONFIG

    /* VCOMS (BBh): VCOM Setting */
    command_params[0] = _st7789_calc_vcom(CONFIG_ST7789_VCOM);
    lcd_ll_write_cmd(dev, LCD_CMD_VCOMS, command_params, 1);
    DEBUG("VCOMS (BBh) %02x\n", command_params[0]);

    /* VRHS (C3h): VRH Set */
    command_params[0] = _st7789_calc_vrh(CONFIG_ST7789_VRH);
    lcd_ll_write_cmd(dev, LCD_CMD_VRHS, command_params, 1);
    DEBUG("VRHS (C3h) %02x\n", command_params[0]);

    /* VDVS (C4h): VDV Set */
    command_params[0] = _st7789_calc_vcom_offset_vdv(CONFIG_ST7789_VDV);
    lcd_ll_write_cmd(dev, LCD_CMD_VDVS, command_params, 1);
    DEBUG("VDVS (C4h) %02x\n", command_params[0]);

    /* VCMOFSET (C5h): VCOM Offset Set */
    command_params[0] = _st7789_calc_vcom_offset_vdv(CONFIG_ST7789_VCOM_OFFSET);
    lcd_ll_write_cmd(dev, LCD_CMD_VCMOFSET, command_params, 1);
    DEBUG("VCMOFSET (C5h) %02x\n", command_params[0]);

    /* PWCTRL1 (D0h): Power Control 1 */
    command_params[0] = 0xa4;
    command_params[1] = (_st7789_calc_avdd(CONFIG_ST7789_AVDD) << 6) |
                        (_st7789_calc_avcl(CONFIG_ST7789_AVCL) << 4) | 0x01;
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1X, command_params, 2);
    DEBUG("PWCTRL1 (D0h): %02x %02x\n", command_params[0], command_params[1]);

#else /* CONFIG_ST7789_CUSTOM_CONFIG */

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* VCOMS (BBh): VCOM Setting (== reset defaults) */
    command_params[0] = 0x20;   /* VCOM=0.9V */
    lcd_ll_write_cmd(dev, LCD_CMD_VCOMS, command_params, 1);

    /* VRHS (C3h): VRH Set (== reset defaults) */
    command_params[0] = 0x0b;   /* VRH=4.1V */
    lcd_ll_write_cmd(dev, LCD_CMD_VRHS, command_params, 1);

    /* VDVS (C4h): VDV Set  (== reset defaults)*/
    command_params[0] = 0x20;   /* VDV=0V */
    lcd_ll_write_cmd(dev, LCD_CMD_VDVS, command_params, 1);

    /* VCMOFSET (C5h): VCOM Offset Set (== reset defaults) */
    command_params[0] = 0x20;   /* VCOMFS=0V */
    lcd_ll_write_cmd(dev, LCD_CMD_VCMOFSET, command_params, 1);

    /* PWCTRL1 (D0h): Power Control 1 (== reset defaults) */
    command_params[0] = 0xa4;
    command_params[1] = 0xa1;   /* AVDD=6.8V, AVCL=4.8V, VDS=2.3 */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1X, command_params, 2);

#endif /* no need to write reset defaults, just for documentation purpose */

#endif /* CONFIG_ST7789_CUSTOM_CONFIG */

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* FRCTRL2 (C6h): Frame Rate Control in Normal Mode */
    command_params[0] = 0x0f; /* == reset default (60 Hz) */
    lcd_ll_write_cmd(dev, LCD_CMD_FRCTRL2, command_params, 1);
    DEBUG("FRCTRL2 (C6h) %02x\n", command_params[0]);

    /* PORCTRL (B2h): Porch Setting */
    command_params[0] = 0x0c; /* == reset defaults */
    command_params[1] = 0x0c;
    command_params[2] = 0x00;
    command_params[3] = 0x33;
    command_params[4] = 0x33;
    lcd_ll_write_cmd(dev, LCD_CMD_PORCTRL, command_params, 5);

    /* GCTRL (B7h): Gate Control */
    command_params[0] = 0x35; /* == reset defaults */
    lcd_ll_write_cmd(dev, LCD_CMD_GCTRL, command_params, 1);

#endif /* no need to write reset defaults, just for documentation purpose */

    /* VGAMCTRL (E0h): Positive Voltage Gamma Control */
    {
        static const uint8_t gamma_pos[] = {
            0xd0, 0x08, 0x11, 0x08, 0x0c, 0x15, 0x39,
            0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2d
        };
        lcd_ll_write_cmd(dev, LCD_CMD_PGAMCTRL, gamma_pos, sizeof(gamma_pos));
    }
    /* NVGAMCTRL (E1h): Negative Voltage Gamma Control */
    {
        static const uint8_t gamma_neg[] = {
            0xd0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39,
            0x44, 0x51, 0x0b, 0x16, 0x14, 0x2f, 0x32
        };
        lcd_ll_write_cmd(dev, LCD_CMD_NGAMCTRL, gamma_neg, sizeof(gamma_neg));
    }

#elif MODULE_ST7796
    DEBUG("ST7796 used ...\n");

#if CONFIG_ST7796_CUSTOM_CONFIG

    /* PWR1 (c0h): Power Control 1 */
    command_params[0] = (_st7796_calc_avdd(CONFIG_ST7796_AVDD) << 6) |
                        (_st7796_calc_avcl(CONFIG_ST7796_AVCL) << 4);
    command_params[1] = 0x25;   /* use reset default, TODO VGH and VGL config */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1, command_params, 2);
    DEBUG("PWR1 (c0h): %02x %02x\n", command_params[0], command_params[1]);

    /* PWR2 (C1h): Power Control 2 */
    command_params[0] = _st7796_calc_vrh(CONFIG_ST7796_VRH);
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL2, command_params, 1);
    DEBUG("PWR2 (C1h) %02x\n", command_params[0]);

    /* VCMPCTL (C5h): VCOM Control */
    command_params[0] = _st7796_calc_vcom(CONFIG_ST7796_VCOM);
    lcd_ll_write_cmd(dev, LCD_CMD_VCMPCTL, command_params, 1);
    DEBUG("VCMPCTL (C5h) %02x\n", command_params[0]);

    /* VCM Offset (C6h): Vcom Offset Register */
    command_params[0] = _st7796_calc_vcom_offset(CONFIG_ST7796_VCOM_OFFSET);
    lcd_ll_write_cmd(dev, LCD_CMD_VCM, command_params, 1);
    DEBUG("VCM (C6h) %02x\n", command_params[0]);

#else /* CONFIG_ST7796_CUSTOM_CONFIG */

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* PWR1 (c0h): Power Control 1 (== reset defaults) */
    command_params[0] = 0x80;   /* AVDD=6.6V, AVCL=-4.4 */
    command_params[1] = 0x25;   /* use reset default, TODO VGH and VGL config */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1, command_params, 2);
    DEBUG("PWR1 (c0h): %02x %02x\n", command_params[0], command_params[1]);

    /* PWR2 (C1h): Power Control 2 (== reset defaults) */
    command_params[0] = 0x0b;   /* VRH=4.1V */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL2, command_params, 1);
    DEBUG("PWR2 (C1h) %02x\n", command_params[0]);

    /* VCMPCTL (C5h): VCOM Control (== reset defaults) */
    command_params[0] = 0x1c;   /* VCOM=1.0V */
    lcd_ll_write_cmd(dev, LCD_CMD_VCMPCTL, command_params, 1);
    DEBUG("VCMPCTL (C5h) %02x\n", command_params[0]);

    /* VCM Offset (C6h): Vcom Offset Register (== reset defaults) */
    command_params[0] = 0x00;   /* VCOM Offset=0V (VMF_REG=0) */
    lcd_ll_write_cmd(dev, LCD_CMD_VCM, command_params, 1);
    DEBUG("VCM (C6h) %02x\n", command_params[0]);

#endif /* no need to write reset defaults, just for documentation purpose */

#endif /* CONFIG_ST7796_CUSTOM_CONFIG */

    /* VGAMCTRL (E0h): Positive Voltage Gamma Control */
    {
        static const uint8_t gamma_pos[] = {
            0xf0, 0x09, 0x0b, 0x06, 0x04, 0x15, 0x2f,
            0x54, 0x42, 0x3c, 0x17, 0x14, 0x18, 0x1b,
        };
        lcd_ll_write_cmd(dev, LCD_CMD_PGAMCTRL, gamma_pos, sizeof(gamma_pos));
    }
    /* NVGAMCTRL (E1h): Negative Voltage Gamma Control */
    {
        static const uint8_t gamma_neg[] = {
            0xe0, 0x09, 0x0b, 0x06, 0x04, 0x03, 0x2b,
            0x43, 0x42, 0x3b, 0x16, 0x14, 0x17, 0x1b
        };
        lcd_ll_write_cmd(dev, LCD_CMD_NGAMCTRL, gamma_neg, sizeof(gamma_neg));
    }

#else
    /* ST7735R initialization sequence */
    DEBUG("ST7735 used ...\n");

    /* INVCTR (B4h): Display Inversion Control */
    command_params[0] = 0x07;   /* NLA=1, NLB=1, NLC=1 Line inversion in all modes */
    lcd_ll_write_cmd(dev, LCD_CMD_INVCTR, command_params, 1);

#if CONFIG_ST7735_CUSTOM_CONFIG

    /* PWCTR1 (C0h): Power Control 1 */
    command_params[0] = (_st7735_calc_avdd(CONFIG_ST7735_AVDD) << 5) |
                        _st7735_calc_gvdd(CONFIG_ST7735_GVDD);
    command_params[1] = _st7735_calc_gvcl(CONFIG_ST7735_GVCL);
    command_params[2] = 0x84;   /* AUTO mode */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1, command_params, 3);
    DEBUG("PWCTRL1 (C0h): %02x %02x %02x\n",
          command_params[0], command_params[1], command_params[2]);

    /* PWCTR2 (C1h): Power Control 2 (== reset defaults) */
    command_params[0] = _st7735_calc_vghl(CONFIG_ST7735_VGH, CONFIG_ST7735_VGL,
                                          CONFIG_ST7735_AVDD);
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL2, command_params, 1);
    DEBUG("PWCTRL2 (C1h): %02x\n", command_params[0]);

    /* VMCTR1 (C5h): VCOM Control 1 */
    command_params[0] = _st7735_calc_vcom(CONFIG_ST7735_VCOM);
    lcd_ll_write_cmd(dev, LCD_CMD_VMCTRL1, command_params, 1);
    DEBUG("VMCTR1 (C5h): %02x\n", command_params[0]);

#else /* CONFIG_ST7735_CUSTOM_CONFIG */

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* PWCTR1 (C0h): Power Control 1 (== reset defaults) */
    command_params[0] = 0x82;   /* AVDD=4.9V, GVDD=4.6V */
    command_params[1] = 0x02;   /* GVCL=-4.6V */
    command_params[2] = 0x84;   /* AUTO mode */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL1, command_params, 3);

    /* PWCTR2 (C1h): Power Control 2 (== reset defaults) */
    command_params[0] = 0xc5;   /* VGH=3*AVDD, VGL=-10V */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL2, command_params, 1);

    /* VMCTR1 (C5h): VCOM Control 1 (== reset defaults) */
    command_params[0] = 0x04;   /* VCOM=-0.525V */
    lcd_ll_write_cmd(dev, LCD_CMD_VMCTRL1, command_params, 1);

#endif /* no need to write reset defaults, just for documentation purpose */

#endif /* CONFIG_ST7735_CUSTOM_CONFIG */

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* PWCTR3 (C2h): Power Control 3 Normal Mode (== reset defaults) */
    command_params[0] = 0x0a;   /* AP=Medium Low, SAP=Small */
    command_params[0] = 0x00;   /* DCA=BCLK/1 BCLK/1 BCLK/1 BCLK/1 BCLK/1 */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL3, command_params, 2);

    /* PWCTR4 (C3h): Power Control 4 Idle Mode (== reset defaults) */
    command_params[0] = 0x8a;   /* AP=Medium Low, SAP=Small */
    command_params[1] = 0x2e;   /* DCA=BCLK/2 BCLK/1 BCLK/2 BCLK/4 BCLK/2 */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL4, command_params, 2);

    /* PWCTR5 (C4h): Power Control 5 Partial Mode (== reset defaults) */
    command_params[0] = 0x8a;   /* AP=Medium Low, SAP=Small */
    command_params[1] = 0xaa;   /* DCA=BCLK/2 BCLK/2 BCLK/2 BCLK/2 BCLK/2 */
    lcd_ll_write_cmd(dev, LCD_CMD_PWCTRL5, command_params, 2);

    /* FRMCTRL1 (B1H): Frame Control 1 in Normal mode (== reset defaults) */
    /* Frame rate = fosc/((RNTA * 2 + 40) x (lines + FPA + BPA)) with fosc = 624 kHz */
    command_params[0] = 0x01;   /* RNTA=1 */
    command_params[1] = 0x2c;   /* FPA (Front Porch) = 44 lines */
    command_params[2] = 0x2d;   /* BPA (Back Porch) = 45 lines */
    lcd_ll_write_cmd(dev, LCD_CMD_FRAMECTL1, command_params, 3);

    /* FRMCTRL2 (B2H): Frame Control 2 in Idle mode (== reset defaults) */
    /* Frame rate = fosc/((RNTB * 2 + 40) x (lines + FPB + BPB)) with fosc = 624 kHz */
    command_params[0] = 0x01;   /* RNTB=1 */
    command_params[1] = 0x2c;   /* FPB (Front Porch) = 44 lines */
    command_params[2] = 0x2d;   /* BPB (Back Porch) = 45 lines */
    lcd_ll_write_cmd(dev, LCD_CMD_FRAMECTL2, command_params, 3);

    /* FRMCTRL3 (B3H): Frame Control 3 in Partal mode (== reset defaults) */
    /* Frame rate = fosc/((RNTC * 2 + 40) x (lines + FPC + BPC)) with fosc = 624 kHz */
    command_params[0] = 0x01;   /* RNTC=1 */
    command_params[1] = 0x2c;   /* FPC (Front Porch) = 44 lines */
    command_params[2] = 0x2d;   /* BPC (Back Porch) = 45 lines */
    command_params[0] = 0x01;   /* RNTD=1 */
    command_params[1] = 0x2c;   /* FPD (Front Porch) = 44 lines */
    command_params[2] = 0x2d;   /* BPD (Back Porch) = 45 lines */
    lcd_ll_write_cmd(dev, LCD_CMD_FRAMECTL3, command_params, 6);

#endif /* no need to write reset defaults, just for documentation purpose */

    /* GMCTRP1 (E0h): Gamma +polarity Correction Characteristics Setting */
    {
        static const uint8_t gamma_pos[] = {
            0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
            0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10
        };
        _write_cmd(dev, LCD_CMD_PGAMCTRL, gamma_pos,
                   sizeof(gamma_pos));
    }
    /* GMCTRN1 (E1h): Gamma -polarity Correction Characteristics Setting */
    {
        static const uint8_t gamma_neg[] = {
            0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
            0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10
        };
        _write_cmd(dev, LCD_CMD_NGAMCTRL, gamma_neg,
                   sizeof(gamma_neg));
    }

#endif

#if 0 /* no need to write reset defaults, just for documentation purpose */

    /* GAMSET (26h): Gamma Set */
    command_params[0] = 0x01; /* == reset defaults */
    lcd_ll_write_cmd(dev, LCD_CMD_GAMSET, command_params, 1);

    /* TEON (35h): Tearing Effect Line ON (== reset defaults) */
    command_params[0] = 0x00;   /* TEM=0 (only V-Blanking) */
    lcd_ll_write_cmd(dev, LCD_CMD_VCOMS, command_params, 1);

#endif /* no need to write reset defaults, just for documentation purpose */

    /* COLMOD (3Ah): Interface Pixel Format */
    command_params[0] = 0x055; /* 16 bit mode RGB & Control */
    lcd_ll_write_cmd(dev, LCD_CMD_COLMOD, command_params, 1);

    /* MADCTL (36h): Memory Data Access Control */
    command_params[0] = dev->params->rotation;
    command_params[0] |= dev->params->rgb ? 0 : LCD_MADCTL_BGR;
    lcd_ll_write_cmd(dev, LCD_CMD_MADCTL, command_params, 1);

    /* enable Inversion if configured, reset default is off */
    if (dev->params->inverted) {
        /* INVON (21h): Display Inversion On */
        lcd_ll_write_cmd(dev, LCD_CMD_DINVON, NULL, 0);
    }

    /* Sleep out (turn off sleep mode) */
    _write_cmd(dev, LCD_CMD_SLPOUT, NULL, 0);

    /* Normal display mode on */
    _write_cmd(dev, LCD_CMD_NORON, NULL, 0);
    ztimer_sleep(ZTIMER_MSEC, 1);

    /* Display on */
    _write_cmd(dev, LCD_CMD_DISPON, NULL, 0);
    spi_release(dev->params->spi);

    return 0;
}

static void _set_area(const lcd_t *dev, uint16_t x1, uint16_t x2,
                      uint16_t y1, uint16_t y2)
{
    be_uint16_t params[2];

    x1 += dev->params->offset_x;
    x2 += dev->params->offset_x;
    y1 += dev->params->offset_y;
    y2 += dev->params->offset_y;

    params[0] = byteorder_htons(x1);
    params[1] = byteorder_htons(x2);

    _write_cmd(dev, LCD_CMD_CASET, (uint8_t *)params,
               sizeof(params));
    params[0] = byteorder_htons(y1);
    params[1] = byteorder_htons(y2);
    _write_cmd(dev, LCD_CMD_PASET, (uint8_t *)params,
               sizeof(params));
}

const lcd_driver_t lcd_st7735_driver = {
    .init = _init,
    .set_area = _set_area,
};
