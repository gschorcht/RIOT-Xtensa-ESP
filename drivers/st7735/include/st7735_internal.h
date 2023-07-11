/*
 * Copyright (C) 2018 Koen Zandberg
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
 *
 * @}
 */

#ifndef ST7735_INTERNAL_H
#define ST7735_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lcd_internal.h"

/**
 * @name LCD commands
 *
 * Additional LCD commands not defined in LCD driver
 * @{
 */
#define LCD_CMD_TEOFF           0x34    /**< Tearing Effect Line Off */
#define LCD_CMD_TEON            0x35    /**< Tearing Effect Line On */
#define LCD_CMD_COLMOD          0x3A    /**< Interface Pixel Format Set */
/** @} */

/**
 * @name LCD ST7735 commands
 *
 * LCD commands extension available for ST7735 LCD controllers
 * @{
 */
#define LCD_CMD_INVCTR          0xb4    /**< Display Inversion Control (ST7735) */
#define LCD_CMD_PWCTRL3         0xc2    /**< Power control 3 */
#define LCD_CMD_PWCTRL4         0xc3    /**< Power control 4 */
#define LCD_CMD_PWCTRL5         0xc4    /**< Power control 5 */
#define LCD_CMD_PWCTRL6         0xfc    /**< Power control 6 */
/** @} */

/**
 * @name LCD ST7789 commands
 *
 * LCD commands extension available for ST7789 LCD controllers
 * @{
 */
#define LCD_CMD_RAMWRC          0x3c    /**< Memory Write Continue */
#define LCD_CMD_RAMRDC          0x3e    /**< Memory Read Continue */
#define LCD_CMD_PORCTRL         0xb2    /**< Porch Control */
#define LCD_CMD_GCTRL           0xb7    /**< Gate Control */
#define LCD_CMD_VCOMS           0xbb    /**< VCOM Setting */
#define LCD_CMD_LCMCTRL         0xc0    /**< LCM Control */
#define LCD_CMD_VDVVRHEN        0xc2    /**< VDV and VRH Command Enable */
#define LCD_CMD_VRHS            0xc3    /**< VRH Set */
#define LCD_CMD_VDVS            0xc4    /**< VDV Set */
#define LCD_CMD_VCMOFSET        0xc4    /**< VCOM Offset Set */
#define LCD_CMD_FRCTRL2         0xc6    /**< Frame Rate Control in Normal Mode */
#define LCD_CMD_PWCTRL1X        0xd0    /**< Power Control 1 */
/** @} */

/**
 * @name LCD ST7796 commands
 *
 * LCD commands extension available for ST7796 LCD controllers
 * @{
 */
#define LCD_CMD_VCMPCTL         0xc5    /**< VCOM Control */
#define LCD_CMD_VCM             0xc6    /**< VCOM Offset */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ST7735_INTERNAL_H */
