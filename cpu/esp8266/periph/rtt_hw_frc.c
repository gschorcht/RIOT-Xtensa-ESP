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
#include "log.h"
#include "periph/rtt.h"

#include "esp_common.h"
#include "esp/common_macros.h"
#include "esp/dport_regs.h"
#include "esp/rtc_regs.h"
#include "irq_arch.h"
#include "rtt_arch.h"
#include "sdk/sdk.h"
#include "syscalls.h"
#include "timex.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define FRC_CLK_DIV_256         2   /* divider for the 80 MHz AHB clock */
#define FRC_FREQUENCY           (80000000UL >> 8)
#define FRC_TICK_TO_COUNT(tck)  ((uint32_t)((uint64_t)(tck) * RTT_FREQUENCY / FRC_FREQUENCY))
#define FRC_COUNT_TO_TICK(cnt)  ((uint32_t)((uint64_t)(cnt) * FRC_FREQUENCY / RTT_FREQUENCY))
#define FRC_US_TO_TICK(us)      ((uint32_t)((uint64_t)(us) * FRC_FREQUENCY / 1000000UL))
#define FRC_OVERFLOW            (FRC_COUNT_TO_TICK(1ULL << RTT_COUNTER_SIZE))

/**
 * FRC2 is a 32-bit countup timer, triggers interrupt when reaches alarm value.
 */
typedef struct {
    uint32_t load;
    uint32_t count;
    union {
        struct {
            uint32_t intr_hold : 1;
            uint32_t reserved1 : 1;
            uint32_t clk_div   : 2;
            uint32_t reserved2 : 2;
            uint32_t reload    : 1;
            uint32_t enable    : 1;
            uint32_t intr_sta  : 1;
            uint32_t reserved3 : 23;
        };
        uint32_t val;
    } ctrl;
    union {
        struct {
            uint32_t clear    : 1;
            uint32_t reserved1: 31;
        };
        uint32_t val;
    } intr;
    uint32_t alarm;
} frc2_struct_t;

/*
 * linker script esp8266.peripherals.ld will make sure this points to the
 * hardware register address
 */
extern volatile frc2_struct_t frc2;

typedef struct {
    uint32_t alarm_set;     /**< alarm set at interface */
    rtt_cb_t alarm_cb;      /**< alarm callback */
    void *alarm_arg;        /**< argument for alarm callback */
    uint32_t active;        /**< active alarm */
} _frc_alarm_t;

static _frc_alarm_t _frc_alarm;

/* variables used to save counters during sleep or reboot */
static uint32_t RTC_BSS_ATTR _rtc_counter_saved;
static uint32_t RTC_BSS_ATTR _frc_counter_saved;

extern uint32_t pm_rtc_clock_cali_proc(void);
extern uint32_t rtc_clk_to_us(uint32_t rtc_cycles, uint32_t period);

static void IRAM _frc_isr(void *arg);

uint32_t _rtc_get_counter(void)
{
    return RTC.COUNTER;
}

static void _frc_init(void)
{
    DEBUG("%s frc_saved=%u rtc_saved=%u @rtc=%u @sys_time=%u\n", __func__,
          _frc_counter_saved, _rtc_counter_saved, RTC.COUNTER,
          system_get_time());

    frc2.ctrl.clk_div = FRC_CLK_DIV_256;
    frc2.ctrl.reload = 0;
    frc2.ctrl.intr_hold = 0;
    frc2.ctrl.enable = 1;
    frc2.alarm = FRC_OVERFLOW;
    _frc_alarm.active = 0;
}

static void _frc_poweron(void)
{
    /* power on simply reactivates the FRC2 counter */
    frc2.ctrl.enable = 1;

    /* enable the interrupt */
    ets_isr_attach(ETS_FRC2_INUM, _frc_isr, NULL);
    ets_isr_unmask(BIT(ETS_FRC2_INUM));
    DPORT.INT_ENABLE |= DPORT_INT_ENABLE_FRC2;
}

static void _frc_poweroff(void)
{
    /* power off simply deactivates the FRC2 counter */
    frc2.ctrl.enable = 0;

    /* disable the interrupt */
    ets_isr_mask(BIT(ETS_FRC2_INUM));
    DPORT.INT_ENABLE &= ~DPORT_INT_ENABLE_FRC2;
}

static uint32_t _frc_get_counter(void)
{
    uint32_t ticks = frc2.count;
    DEBUG("%s frc_ticks=%u frc_count=%u @sys_time=%u\n", __func__,
          ticks, FRC_TICK_TO_COUNT(ticks), system_get_time());
    return FRC_TICK_TO_COUNT(ticks);
}

static void _update_alarm(uint32_t counter)
{
    if ((_frc_alarm.alarm_cb && (_frc_alarm.alarm_set > counter))) {
        _frc_alarm.active = _frc_alarm.alarm_set;
        frc2.alarm = _frc_alarm.active;
    }
    else {
        _frc_alarm.active = 0;
        frc2.alarm = FRC_OVERFLOW;
    }
}

static void _frc_set_alarm(uint32_t alarm, rtt_cb_t cb, void *arg)
{
    assert(alarm <= RTT_MAX_VALUE);

    /* save current counter value */
    uint32_t _frc_counter = frc2.count;

    _frc_alarm.alarm_set = FRC_COUNT_TO_TICK(alarm) % FRC_OVERFLOW;
    _frc_alarm.alarm_cb = cb;
    _frc_alarm.alarm_arg = arg;

    _update_alarm(_frc_counter);

    DEBUG("%s alarm=%u frc_alarm=%u frc_alarm_set=%u @frc=%u @sys_time=%u\n",
          __func__, alarm,
          _frc_alarm.alarm_set, frc2.alarm, _frc_counter, system_get_time());
}

static void _frc_clear_alarm(void)
{
    /* reset the alarm configuration for interrupt handling */
    _frc_alarm.alarm_set = 0;
    _frc_alarm.alarm_cb = NULL;
    _frc_alarm.alarm_arg = NULL;
}

static void _frc_save_counter(void)
{
    critical_enter();

    /* save counters before going to sleep or reboot */
    _frc_counter_saved = frc2.count;
    _rtc_counter_saved = RTC.COUNTER;

    critical_exit();

    DEBUG("%s rtc_saved=%u frc_saved=%u\n", __func__,
          _rtc_counter_saved, _frc_counter_saved);
}

static void _frc_restore_counter(bool in_init)
{
    critical_enter();

    /* synchronize RTC counter and the 32-bit microsecond system timer */
    uint32_t _rtc_counter = RTC.COUNTER;
    uint32_t _rtc_diff = _rtc_counter - _rtc_counter_saved;
    uint32_t _diff_us = rtc_clk_to_us(_rtc_diff, pm_rtc_clock_cali_proc());
    uint32_t _frc_diff = FRC_US_TO_TICK(_diff_us);

    frc2.load = (_frc_counter_saved + _frc_diff) % FRC_OVERFLOW;

    critical_exit();

    DEBUG("%s rtc_saved=%u rtc_diff=%u @rtc=%u diff_us=%u "
          "frc_saved=%u frc_diff=%u\n", __func__,
          _rtc_counter_saved, _rtc_diff, _rtc_counter, _diff_us,
          _frc_counter_saved, _frc_diff);
}

static void IRAM _frc_isr(void *arg)
{
    uint32_t counter = frc2.count % FRC_OVERFLOW; /* save current counter value */

    DEBUG("%s %u\n", __func__, counter);

    if (_frc_alarm.active == 0) {
        DEBUG("%s overflow %u\n", __func__, counter);
        frc2.load = counter;
    }

    if ((_frc_alarm.active == _frc_alarm.alarm_set) && _frc_alarm.alarm_cb) {
        DEBUG("%s alarm %u\n", __func__, counter);

        rtt_cb_t alarm_cb = _frc_alarm.alarm_cb;
        void *alarm_arg = _frc_alarm.alarm_arg;

        /* clear the alarm first */
        _frc_alarm.alarm_cb = NULL;
        _frc_alarm.alarm_arg = NULL;

        /* call the alarm handler afterwards if callback was defined */
        alarm_cb(alarm_arg);
    }

    _update_alarm(counter);
}

const rtt_hw_driver_t _rtt_hw_frc_driver = {
        .init = _frc_init,
        .get_counter = _frc_get_counter,
        .set_alarm = _frc_set_alarm,
        .clear_alarm = _frc_clear_alarm,
        .poweron = _frc_poweron,
        .poweroff = _frc_poweroff,
        .save_counter = _frc_save_counter,
        .restore_counter = _frc_restore_counter,
};
