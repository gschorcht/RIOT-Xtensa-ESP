/*
 * Copyright (C) 2020 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_esp8266
 * @ingroup     drivers_periph_rtt
 * @{
 *
 * @file
 * @brief       Low-level RTT driver implementation for ESP8266
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 *
 * @}
 */

#include "cpu.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "irq_arch.h"
#include "log.h"
#include "periph/rtt.h"
#include "rtt_arch.h"
#include "syscalls.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

/* contains the values as given at the interface */
typedef struct {
    uint32_t alarm;         /**< alarm value as set at the interface */
    rtt_cb_t alarm_cb;      /**< alarm callback */
    rtt_cb_t overflow_cb;   /**< overflow callback */
    void *alarm_arg;        /**< argument for alarm callback */
    void *overflow_arg;     /**< argument for overflow callback */
    uint32_t alarm_active;  /**< alarm that is currently active */
    bool alarm_set;         /**< indicates whether an alarm is active */
    bool wakeup;            /**< indicates whether next alarm is a wake-up */
} rtt_counter_t;

static rtt_counter_t rtt_counter;

static uint32_t RTC_BSS_ATTR _rtt_offset;

extern uint32_t pm_rtc_clock_cali_proc(void);
extern uint32_t rtc_clk_to_us(uint32_t rtc_cycles, uint32_t period);

/* forward declaration of functions */
void rtt_restore_counter(bool sys_time);
static void _rtt_update_hw_alarm(void);
static void IRAM_ATTR _rtt_isr(void *arg);

/* forward declarations of driver functions */
uint32_t _rtc_get_counter(void);

/* declaration possible of hardware counter drivers */
extern const rtt_hw_driver_t _rtt_hw_sys_driver;
extern const rtt_hw_driver_t _rtt_hw_frc_driver;

/* used hardware driver */
#ifdef MODULE_ESP_WIFI_ANY
static const rtt_hw_driver_t *_rtt_hw = &_rtt_hw_sys_driver;
#else
static const rtt_hw_driver_t *_rtt_hw = &_rtt_hw_frc_driver;
#endif

void rtt_init(void)
{
    DEBUG("%s rtt_offset=%u @rtc=%u @sys_time=%u\n", __func__,
          _rtt_offset, _rtc_get_counter(), system_get_time());

    /* init the hardware counter if necessary */
    _rtt_hw->init();

    /* restore counter from RTC after deep sleep or reboot */
    rtt_restore_counter(true);

    /* clear alarm settings */
    rtt_clear_alarm();
    rtt_clear_overflow_cb();

    /* power on the module and enable interrupts */
    rtt_poweron();

}

void rtt_poweron(void)
{
    _rtt_hw->poweron();
}

void rtt_poweroff(void)
{
    _rtt_hw->poweroff();
}

void rtt_set_overflow_cb(rtt_cb_t cb, void *arg)
{
    /* there is no overflow interrupt, we emulate */
    rtt_counter.overflow_cb = cb;
    rtt_counter.overflow_arg = arg;

    _rtt_update_hw_alarm();
}

void rtt_clear_overflow_cb(void)
{
    /* there is no overflow interrupt, we emulate */
    rtt_counter.overflow_cb = NULL;
    rtt_counter.overflow_arg = NULL;

    _rtt_update_hw_alarm();
}

uint32_t rtt_get_counter(void)
{
    uint32_t counter = _rtt_hw->get_counter() + _rtt_offset;
    DEBUG("%s counter=%u @sys_time=%u\n", __func__, counter, system_get_time());
    return counter;
}

void rtt_set_counter(uint32_t counter)
{
    uint32_t _rtt_current = _rtt_hw->get_counter();
    _rtt_offset = counter - _rtt_current;

    DEBUG("%s set=%u rtt_offset=%u @rtt=%u\n",
          __func__, counter, _rtt_offset, _rtt_current);

    _rtt_update_hw_alarm();
}

void rtt_set_alarm(uint32_t alarm, rtt_cb_t cb, void *arg)
{
    uint32_t counter = rtt_get_counter();
    rtt_counter.alarm = alarm;
    rtt_counter.alarm_cb = cb;
    rtt_counter.alarm_arg = arg;

    DEBUG("%s alarm=%u @rtt=%u\n", __func__, alarm, counter);

    _rtt_update_hw_alarm();
}

void rtt_clear_alarm(void)
{
    /* clear the alarm */
    rtt_counter.alarm = 0;
    rtt_counter.alarm_cb = NULL;
    rtt_counter.alarm_arg = NULL;

    DEBUG("%s @rtt=%u\n", __func__, _rtt_hw->get_counter());

    _rtt_update_hw_alarm();
}

uint32_t rtt_get_alarm(void)
{
    return rtt_counter.alarm;
}

void rtt_save_counter(void)
{
    _rtt_hw->save_counter();
}

void rtt_restore_counter(bool in_init)
{
    _rtt_hw->restore_counter(in_init);
}

uint32_t rtt_pm_sleep_enter(unsigned mode)
{
    rtt_save_counter();

    if (!rtt_counter.alarm_set) {
        return 0;
    }

    uint32_t counter = rtt_get_counter();
    uint64_t t_diff = RTT_TICKS_TO_US(rtt_counter.alarm_active - counter);

    DEBUG("%s rtt_alarm=%u @rtt=%u t_diff=%llu\n", __func__,
          rtt_counter.alarm_active, counter, t_diff);

    if (t_diff) {
        rtt_counter.wakeup = true;
        esp_sleep_enable_timer_wakeup(t_diff);
    }
    else {
        rtt_counter.wakeup = false;
    }

    return t_diff;
}

void rtt_pm_sleep_exit(uint32_t cause)
{
    rtt_restore_counter(false);

    if (cause == ESP_SLEEP_WAKEUP_TIMER) {
        _rtt_isr(NULL);
    }
}

static void _rtt_update_hw_alarm(void)
{
    if (rtt_counter.alarm_cb && ((rtt_counter.alarm > rtt_get_counter()) ||
                                 (rtt_counter.overflow_cb == NULL))) {
        /* alarm is the next event if either the alarm is greater than the
           current counter value or the overflow callback is not set. */
        rtt_counter.alarm_active = rtt_counter.alarm;
        rtt_counter.alarm_set = true;
        _rtt_hw->set_alarm(rtt_counter.alarm - _rtt_offset, _rtt_isr, NULL);
        DEBUG("%s alarm=%u alarm_hw=%u\n", __func__,
              rtt_counter.alarm, rtt_counter.alarm - _rtt_offset);
    }
    else if (rtt_counter.overflow_cb) {
        /* otherwise the overflow is the next event if its callback is set */
        rtt_counter.alarm_active = 0;
        rtt_counter.alarm_set = true;
        _rtt_hw->set_alarm(0 - _rtt_offset, _rtt_isr, NULL);
        DEBUG("%s alarm=0 alarm_hw=%u\n", __func__, 0 - _rtt_offset);
    }
    else {
        rtt_counter.alarm_set = false;
        _rtt_hw->clear_alarm();
        DEBUG("%s no alarm\n", __func__);
    }
}

static void IRAM_ATTR _rtt_isr(void *arg)
{
    DEBUG("%s\n", __func__);

    uint32_t alarm = rtt_counter.alarm_active;

    if (rtt_counter.wakeup) {
        rtt_counter.wakeup = false;
        DEBUG("%s wakeup alarm alarm=%u rtt_alarm=%u @rtt=%u\n",
              __func__, alarm, rtt_counter.alarm_active, rtt_get_counter());
    }

    if ((alarm == rtt_counter.alarm) && rtt_counter.alarm_cb) {
        DEBUG("%s alarm\n", __func__);
        rtt_cb_t alarm_cb = rtt_counter.alarm_cb;
        void * alarm_arg = rtt_counter.alarm_arg;
        /* clear the alarm first, includes setting next alarm to overflow */
        rtt_clear_alarm();
        /* call the alarm handler afterwards if a callback is set */
        if (alarm_cb) {
            alarm_cb(alarm_arg);
        }
    }

    if (alarm == 0) {
        DEBUG("%s overflow\n", __func__);
        /* set next alarm which is either an alarm if configured or overflow */
        _rtt_update_hw_alarm();
        /* call the overflow handler if set */
        if (rtt_counter.overflow_cb) {
            rtt_counter.overflow_cb(rtt_counter.overflow_arg);
        }
    }

   DEBUG("%s next rtt=%u\n", __func__, rtt_counter.alarm_active);
}
