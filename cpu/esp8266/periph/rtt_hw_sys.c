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
#include "periph/rtt.h"

#include "esp_common.h"
#include "esp/common_macros.h"
#include "esp/dport_regs.h"
#include "esp/rtc_regs.h"
#include "esp_sleep.h"
#include "irq_arch.h"
#include "log.h"
#include "rtt_arch.h"
#include "sdk/sdk.h"
#include "syscalls.h"
#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/* variables used to save counters during sleep or reboot */
static uint32_t RTC_BSS_ATTR _rtc_counter_saved;
static uint32_t RTC_BSS_ATTR _sys_counter_saved;

/* the offset of the system time to the RTC time in microseconds */
static uint32_t _sys_counter_offset;

extern uint32_t pm_rtc_clock_cali_proc(void);
extern uint32_t rtc_clk_to_us(uint32_t rtc_cycles, uint32_t period);

/* system timer used for the counter */
xtimer_t _sys_timer;

uint32_t _rtc_get_counter(void)
{
    return RTC.COUNTER;
}

static void _sys_init(void)
{
}

static void _sys_poweron(void)
{
}

static void _sys_poweroff(void)
{
    xtimer_remove(&_sys_timer);
}

static uint32_t _sys_get_counter(void)
{
    uint32_t _sys_time = system_get_time();
    DEBUG("%s sys_time=%u sys_offset=%u @sys=%u\n", __func__,
          _sys_time, _sys_counter_offset, _sys_time + _sys_counter_offset);
    return _sys_time + _sys_counter_offset;
}

static void _sys_set_alarm(uint32_t alarm_us, rtt_cb_t cb, void *arg)
{
    /* compute the time difference for the alarm in microseconds */
    uint32_t _sys_time = _sys_get_counter();
    uint32_t _rtt_diff = alarm_us - _sys_time;

    DEBUG("%s alarm=%u sys_diff=%u @sys=%u\n", __func__,
          alarm_us, _rtt_diff, _sys_time);

    /* set the timer */
    _sys_timer.callback = cb;
    _sys_timer.arg = arg;
    
    xtimer_set(&_sys_timer, _rtt_diff);
}

static void _sys_clear_alarm(void)
{
    /* reset the timer */
    _sys_timer.callback = NULL;
    _sys_timer.arg = NULL;
    
    xtimer_remove(&_sys_timer);
}

static void _sys_save_counter(void)
{
    critical_enter();

    /* save counters for synchronization after wakeup or reboot */
    _rtc_counter_saved = _rtc_get_counter();
    _sys_counter_saved = system_get_time() + _sys_counter_offset;

    critical_exit();

    DEBUG("%s rtc_time_saved=%u sys_time_saved=%u\n", __func__,
          _rtc_counter_saved, _sys_counter_saved);
}

static void _sys_restore_counter(bool in_init)
{
    critical_enter();

    /* synchronize RTC counter and the 32-bit microsecond system timer */
    uint32_t _rtc_diff = _rtc_get_counter() - _rtc_counter_saved;

    _sys_counter_offset += rtc_clk_to_us(_rtc_diff, pm_rtc_clock_cali_proc());
    _sys_counter_offset += (in_init) ? _sys_counter_saved : 0;

    critical_exit();

    DEBUG("%s rtc_counter_saved=%u "
          "sys_counter_saved=%u sys_counter_offset=%u\n", __func__,
          _rtc_counter_saved, _sys_counter_saved, _sys_counter_offset);
}

const rtt_hw_driver_t _rtt_hw_sys_driver = {
        .init = _sys_init,
        .get_counter = _sys_get_counter,
        .set_alarm = _sys_set_alarm,
        .clear_alarm = _sys_clear_alarm,
        .poweron = _sys_poweron,
        .poweroff = _sys_poweroff,
        .save_counter = _sys_save_counter,
        .restore_counter = _sys_restore_counter,
};
