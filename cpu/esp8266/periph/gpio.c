/*
 * Copyright (C) 2019 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_esp8266
 * @ingroup     drivers_periph_gpio
 * @{
 *
 * @file
 * @brief       Low-level GPIO driver implementation for ESP8266
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @}
 */

#define ENABLE_DEBUG    (0)
#include "debug.h"

#include <stdbool.h>

#include "log.h"
#include "periph/gpio.h"    /* RIOT gpio.h */

#include "esp8266/eagle_soc.h"
#include "esp8266/gpio_register.h"
#include "rom/ets_sys.h"

#include "sdk/ets.h"
#include "esp/gpio_regs.h"
#include "esp/iomux_regs.h"
#include "esp/rtc_regs.h"

#include "esp_common.h"
#include "esp_sleep.h"
#include "gpio_arch.h"
#include "irq_arch.h"
#include "syscalls.h"

#define GPIO_CONF(i)    (*(gpio_conf_reg_t *)(&GPIO.CONF[i]))

/* GPIO pin config register struct for easier access to the register fields */
typedef struct {
    uint32_t source:        1;
    uint32_t reserved1:     1;
    uint32_t driver:        1;
    uint32_t reserved2:     4;
    uint32_t int_type:      3;
    uint32_t wakeup_enable: 1;
    uint32_t reserved3:     21;
} gpio_conf_reg_t;

/*
 * IOMUX to GPIO mapping
 * source https://www.espressif.com/sites/default/files/documentation/0d-esp8266_pin_list_release_15-11-2014.xlsx
 */

const uint8_t _gpio_to_iomux[] = { 12, 5, 13, 4, 14, 15, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3 };
const uint8_t _iomux_to_gpio[] = { 12, 13, 14, 15, 3, 1, 6, 7, 8, 9, 10, 11, 0, 2, 4, 5 };

gpio_pin_usage_t _gpio_pin_usage [GPIO_PIN_NUMOF] =
{
   _GPIO,   /* gpio0 */

   _UART,   /* gpio1  UART0 RxD */
   _GPIO,   /* gpio2 */
   _UART,   /* gpio3  UART0 TxD */
   _GPIO,   /* gpio4 */
   _GPIO,   /* gpio5 */
   _SPIF,   /* gpio6  SPI flash CLK */
   _SPIF,   /* gpio7  SPI flash MISO */
   _SPIF,   /* gpio8  SPI flash MOSI */
   #if defined(FLASH_MODE_QIO) || defined(FLASH_MODE_QOUT)
   _SPIF,   /* gpio9  SPI flash HD (qio/qout flash mode) */
   _SPIF,   /* gpio10 SPI flash WP (qio/qout flash mode) */
   #else
   _GPIO,   /* gpio9  can be used as GPIO if not needed for flash */
   _GPIO,   /* gpio10 can be used as GPIO if not needed for flash */
   #endif
   _SPIF,   /* gpio11 SPI flash CS */
   _GPIO,   /* gpio12 */
   _GPIO,   /* gpio13 */
   _GPIO,   /* gpio14 */
   _GPIO,   /* gpio15 */
   _GPIO    /* gpio16 */
};

/* String representation of usage types */
const char* _gpio_pin_usage_str[] =
{
    "GPIO", "I2C", "PWM", "SPI", "SPI Flash", "UART", "N/A"
};

int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    DEBUG("%s: %d %d\n", __func__, pin, mode);

    CHECK_PARAM_RET(pin < GPIO_PIN_NUMOF, -1);

    /* check whether pin can be used as GPIO or is used in anyway else */
    switch (_gpio_pin_usage[pin]) {
        case _I2C:  LOG_ERROR("GPIO%d is used as I2C signal.\n", pin); return -1;
        case _PWM:  LOG_ERROR("GPIO%d is used as PWM output.\n", pin); return -1;
        case _SPI:  LOG_ERROR("GPIO%d is used as SPI interface.\n", pin); return -1;
        case _SPIF: LOG_ERROR("GPIO%d is used as SPI flash.\n", pin); return -1;
        case _UART: LOG_ERROR("GPIO%d is used as UART interface.\n", pin); return -1;
        default: break;
    }

    /* GPIO16 requires separate handling */
    if (pin == GPIO16) {
        RTC.GPIO_CFG[3] = (RTC.GPIO_CFG[3] & 0xffffffbc) | BIT(0);
        RTC.GPIO_CONF = RTC.GPIO_CONF & ~RTC_GPIO_CONF_OUT_ENABLE;

        switch (mode) {
            case GPIO_OUT:
                RTC.GPIO_ENABLE = RTC.GPIO_OUT | RTC_GPIO_CONF_OUT_ENABLE;
                break;

            case GPIO_OD:
                LOG_ERROR("GPIO mode GPIO_OD is not supported for GPIO16.\n");
                return -1;

            case GPIO_OD_PU:
                LOG_ERROR("GPIO mode GPIO_OD_PU is not supported for GPIO16.\n");
                return -1;

            case GPIO_IN:
                RTC.GPIO_ENABLE = RTC.GPIO_OUT & ~RTC_GPIO_CONF_OUT_ENABLE;
                RTC.GPIO_CFG[3] &= ~RTC_GPIO_CFG3_PIN_PULLUP;
                break;

            case GPIO_IN_PU:
                LOG_ERROR("GPIO mode GPIO_IN_PU is not supported for GPIO16.\n");
                return -1;

            case GPIO_IN_PD:
                LOG_ERROR("GPIO mode GPIO_IN_PD is not supported for GPIO16.\n");
                return -1;

            default:
                LOG_ERROR("Invalid GPIO mode for GPIO%d.\n", pin);
                return -1;
        }
        return 0;
    }

    uint8_t  iomux      = _gpio_to_iomux [pin];
    uint32_t iomux_conf = (iomux > 11) ? IOMUX_FUNC(0) : IOMUX_FUNC(3);

    switch (mode) {

        case GPIO_OUT:   iomux_conf |= IOMUX_PIN_OUTPUT_ENABLE;
                         iomux_conf |= IOMUX_PIN_OUTPUT_ENABLE_SLEEP;
                         GPIO.CONF[pin] &= ~GPIO_CONF_OPEN_DRAIN;
                         GPIO.ENABLE_OUT_SET = BIT(pin);
                         break;

        case GPIO_OD_PU: iomux_conf |= IOMUX_PIN_PULLUP;
                         iomux_conf |= IOMUX_PIN_PULLUP_SLEEP;
        case GPIO_OD:    iomux_conf |= IOMUX_PIN_OUTPUT_ENABLE;
                         iomux_conf |= IOMUX_PIN_OUTPUT_ENABLE_SLEEP;
                         GPIO.CONF[pin] |= GPIO_CONF_OPEN_DRAIN;
                         GPIO.ENABLE_OUT_SET = BIT(pin);
                         break;

        case GPIO_IN_PU: iomux_conf |= IOMUX_PIN_PULLUP;
                         iomux_conf |= IOMUX_PIN_PULLUP_SLEEP;
        case GPIO_IN:    GPIO.CONF[pin] |= GPIO_CONF_OPEN_DRAIN;
                         GPIO.ENABLE_OUT_CLEAR = BIT(pin);
                         break;

        case GPIO_IN_PD: LOG_ERROR("GPIO mode GPIO_IN_PD is not supported.\n");
                         return -1;

        default: LOG_ERROR("Invalid GPIO mode for GPIO%d.\n", pin);
    }

    IOMUX.PIN[iomux] = iomux_conf;

    return 0;
}

#ifdef MODULE_PERIPH_GPIO_IRQ
static gpio_isr_ctx_t gpio_isr_ctx_table [GPIO_PIN_NUMOF] = { };
static bool gpio_int_enabled_table [GPIO_PIN_NUMOF] = { };
extern uint32_t pm_wakeup_reason;

void IRAM gpio_int_handler (void* arg)
{
    irq_isr_enter();

    for (int i = 0; i < GPIO_PIN_NUMOF; i++) {

        uint32_t mask = BIT(i);

        if (GPIO.STATUS & mask) {
            pm_wakeup_reason = ESP_SLEEP_WAKEUP_GPIO;
            GPIO.STATUS_CLEAR = mask;
            if (gpio_int_enabled_table[i] && (GPIO.CONF[i] & GPIO_PIN_INT_TYPE_MASK)) {
                gpio_isr_ctx_table[i].cb (gpio_isr_ctx_table[i].arg);
            }
        }
        mask = mask << 1;
    }

    irq_isr_exit();
}

int gpio_init_int(gpio_t pin, gpio_mode_t mode, gpio_flank_t flank,
                  gpio_cb_t cb, void *arg)
{
    if (gpio_init(pin, mode)) {
        return -1;
    }

    if (pin == GPIO16) {
        /* GPIO16 requires separate handling */
        LOG_ERROR("GPIO16 cannot generate interrupts.\n");
        return -1;
    }
    gpio_isr_ctx_table[pin].cb  = cb;
    gpio_isr_ctx_table[pin].arg = arg;

    GPIO.CONF[pin] = SET_FIELD(GPIO.CONF[pin], GPIO_CONF_INTTYPE, flank);
    if (flank != GPIO_NONE) {
        GPIO_CONF(pin).int_type = flank;
        GPIO_CONF(pin).wakeup_enable = 1;
        gpio_int_enabled_table[pin] = (gpio_isr_ctx_table[pin].cb != NULL);
        ets_isr_attach(ETS_GPIO_INUM, gpio_int_handler, 0);
        ets_isr_unmask((1 << ETS_GPIO_INUM));
    }

    return 0;
}

void gpio_irq_enable (gpio_t pin)
{
    CHECK_PARAM(pin < GPIO_PIN_NUMOF);

    gpio_int_enabled_table [pin] = true;
}

void gpio_irq_disable (gpio_t pin)
{
    CHECK_PARAM(pin < GPIO_PIN_NUMOF);

    gpio_int_enabled_table [pin] = false;
}
#endif /* MODULE_PERIPH_GPIO_IRQ */

int IRAM_ATTR gpio_read (gpio_t pin)
{
    CHECK_PARAM_RET(pin < GPIO_PIN_NUMOF, -1);

    if (pin == GPIO16) {
        /* GPIO16 requires separate handling */
        return RTC.GPIO_IN & BIT(0);
    }

    return (GPIO.IN & BIT(pin)) ? 1 : 0;
}

void IRAM_ATTR gpio_write (gpio_t pin, int value)
{
    DEBUG("%s: %d %d\n", __func__, pin, value);

    CHECK_PARAM(pin < GPIO_PIN_NUMOF);

    if (pin == GPIO16) {
        /* GPIO16 requires separate handling */
        RTC.GPIO_OUT = (RTC.GPIO_OUT & ~BIT(0)) | (value ? 1 : 0);
        return;
    }

    if (value) {
        GPIO.OUT_SET = BIT(pin) & GPIO_OUT_PIN_MASK;
    }
    else {
        GPIO.OUT_CLEAR = BIT(pin) & GPIO_OUT_PIN_MASK;
    }
}

void IRAM_ATTR gpio_set (gpio_t pin)
{
    gpio_write (pin, 1);
}

void IRAM_ATTR gpio_clear (gpio_t pin)
{
    gpio_write (pin, 0);
}

void IRAM_ATTR gpio_toggle (gpio_t pin)
{
    DEBUG("%s: %d\n", __func__, pin);

    CHECK_PARAM(pin < GPIO_PIN_NUMOF);

    if (pin == GPIO16) {
        /* GPIO16 requires separate handling */
        RTC.GPIO_OUT = (RTC.GPIO_OUT & ~BIT(0)) | ((RTC.GPIO_IN & BIT(0)) ? 0 : 1);
        return;
    }

    GPIO.OUT ^= BIT(pin);
}

int gpio_set_pin_usage(gpio_t pin, gpio_pin_usage_t usage)
{
    CHECK_PARAM_RET(pin < GPIO_PIN_NUMOF, -1);
    _gpio_pin_usage [pin] = usage;
    return 0;
}

gpio_pin_usage_t gpio_get_pin_usage (gpio_t pin)
{
    return (pin < GPIO_PIN_NUMOF) ? _gpio_pin_usage[pin] : _NOT_EXIST;
}

const char* gpio_get_pin_usage_str(gpio_t pin)
{
    return _gpio_pin_usage_str[_gpio_pin_usage[((pin < GPIO_PIN_NUMOF) ? pin : _NOT_EXIST)]];
}

#if MODULE_PERIPH_GPIO_IRQ
static uint32_t gpio_int_saved_type[GPIO_PIN_NUMOF];
#endif

void gpio_pm_sleep_enter(unsigned mode)
{
#if MODULE_PERIPH_GPIO_IRQ
    if (mode == ESP_PM_LIGHT_SLEEP) {
        extern esp_err_t esp_sleep_enable_gpio_wakeup(void);
        esp_sleep_enable_gpio_wakeup();

        for (unsigned i = 0; i < GPIO_PIN_NUMOF; i++) {
            if (i == GPIO16) {
                continue;
            }

            if (gpio_int_enabled_table[i] && GPIO_CONF(i).int_type) {
                gpio_int_saved_type[i] = GPIO_CONF(i).int_type;
                switch (GPIO_CONF(i).int_type) {
                    case GPIO_FALLING:
                        GPIO_CONF(i).int_type = GPIO_LOW;
                        DEBUG("%s gpio=%u GPIO_LOW\n", __func__, i);
                        break;
                    case GPIO_RISING:
                        GPIO_CONF(i).int_type = GPIO_HIGH;
                        DEBUG("%s gpio=%u GPIO_HIGH\n", __func__, i);
                        break;
                    case GPIO_BOTH:
                        DEBUG("%s gpio=%u GPIO_BOTH not supported\n",
                               __func__, i);
                        break;
                    default:
                        break;
                }
            }
        }
    }
#endif
}

void gpio_pm_sleep_exit(uint32_t cause)
{
    (void)cause;
#if MODULE_PERIPH_GPIO_IRQ
    DEBUG("%s\n", __func__);
    for (unsigned i = 0; i < GPIO_PIN_NUMOF; i++) {
        if (i == GPIO16) {
            continue;
        }
        if (gpio_int_enabled_table[i]) {
            GPIO_CONF(i).int_type = gpio_int_saved_type[i];
        }
    }
#endif
}
