/*
 * Copyright (C) 2019 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_esp8266
 * @ingroup     drivers_periph_pm
 * @{
 *
 * @file
 * @brief       Implementation of power management functions
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 * @}
 */

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "periph_cpu.h"
#include "periph/rtt.h"
#include "gpio_arch.h"
#include "rtt_arch.h"

#include "sdk/sdk.h"
#include "syscalls.h"

extern void rtt_save_counter(void);
extern void rtt_restore_counter(void);
extern void rtt_handle_pending_irq(void);
extern uint32_t rtt_get_next_irq(void);

static uint32_t last_system_time = 0;
static uint32_t new_system_time;
extern int32_t system_time_cycles;

/* used by different components to store the wakeup reason from light sleep */
uint32_t pm_wakeup_reason;

/* sleep source type corresponds to the wake-up cause type */
#define esp_sleep_wakeup_cause_t esp_sleep_source_t

static inline esp_sleep_wakeup_cause_t pm_get_wakeup_cause(void)
{
    return pm_wakeup_reason;
}

/* function that is required by pm_set if esp_now and esp_wifi are not used */
esp_err_t __attribute__((weak)) esp_wifi_start(void)
{
    return ESP_OK;
}

/* function that is required by pm_set if esp_now and esp_wifi are not used */
esp_err_t __attribute__((weak)) esp_wifi_stop(void)
{
    return ESP_OK;
}

static inline void pm_set_lowest_normal(void)
{
    /* reset system watchdog timer */
    system_wdt_feed();

    new_system_time = system_get_time();
    if (new_system_time < last_system_time) {
        /* overflow of 32-bit system timer occured */
        system_time_cycles++;
    }
    last_system_time = new_system_time;

#ifndef MODULE_ESP_QEMU
    /* passive wait for interrupt to leave lowest power mode */
    __asm__ volatile ("waiti 0");

    /* reset system watchdog timer */
    system_wdt_feed();
#endif
}

void IRAM_ATTR pm_off(void)
{
    DEBUG("%s\n", __func__);

    if (IS_USED(MODULE_ESP_WIFI_ANY)) {
        /* stop WiFi if necessary */
        esp_wifi_stop();
    }

    /* enter hibernate mode without any enabled wake-up sources */
    esp_deep_sleep(0);
}

void pm_reboot(void)
{
    DEBUG("%s\n", __func__);

    if (IS_USED(MODULE_ESP_WIFI_ANY)) {
        /* stop WiFi if necessary */
        esp_wifi_stop();
    }

    if (IS_USED(MODULE_PERIPH_RTT)) {
        /* save counters */
        rtt_save_counter();
    }

    /* restart */
    esp_restart();
}

#ifndef MODULE_PM_LAYERED

void pm_set_lowest(void)
{
    pm_set_lowest_normal();
}

#else /* MODULE_PM_LAYERED */

void pm_set(unsigned mode)
{
    if (mode == ESP_PM_MODEM_SLEEP) {
        pm_set_lowest_normal();
        return;
    }
    /* default wake-up reason */
    pm_wakeup_reason = ESP_SLEEP_WAKEUP_TIMER;

    DEBUG ("%s enter to power mode %d @%u\n", __func__, mode, system_get_time());

    /* flush stdout */
    fflush(stdout);

    /* first disable all wake-up sources */
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    /* Prepare the RTT timer if an RTT alarm is set to wake up. */
    uint32_t t_sleep = rtt_pm_sleep_enter(mode);

    /* Prepare GPIOs as wakeup source */
    gpio_pm_sleep_enter(mode);

    if (mode == ESP_PM_DEEP_SLEEP) {
        esp_deep_sleep(t_sleep);
        /* waking up from deep-sleep leads to a DEEPSLEEP_RESET */
        UNREACHABLE();
    }
    else if (mode == ESP_PM_LIGHT_SLEEP) {
        if (IS_USED(MODULE_ESP_WIFI_ANY)) {
            /* stop WiFi if necessary */
            esp_wifi_stop();
        }

        esp_light_sleep_start();

        esp_sleep_wakeup_cause_t wakeup_reason = pm_get_wakeup_cause();
        gpio_pm_sleep_exit(wakeup_reason);
        /* call the RTT alarm handler if an RTT alarm was set */
        rtt_pm_sleep_exit(wakeup_reason);

        DEBUG ("%s exit from power mode %d @%u with reason %d\n", __func__,
               mode, system_get_time(), wakeup_reason);

        /* restart WiFi if necessary */
        if (IS_USED(MODULE_ESP_WIFI_ANY) && (esp_wifi_start() != ESP_OK)) {
            LOG_ERROR("esp_wifi_start failed\n");
        }
    }
}

#endif /* MODULE_PM_LAYERED */
