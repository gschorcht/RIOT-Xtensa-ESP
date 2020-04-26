/*
 * Copyright (C) 2020 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_esp8266
 * @{
 *
 * @file
 * @brief       Architecture specific RTT functions for ESP8266
 *
 * Due to the inaccuracy of the ESP8266 RTC counter clocked by an RC
 * oscillator and the fact that it is not possible to generate interrupts
 * with the RTC, the RTT counter is emulated by CPU timers in the
 * active state of the CPU. The RTC counter is only used in sleep mode
 * or during a reboot. For this purpose the CPU timer is stored in the
 * RTC memory when entering a sleep mode or a restart. When leaving the
 * sleep mode or after a restart, it is updated by the RTC counter.
 *
 * The emulated RTT counter implements a 27-bit RTT counter with a frequency
 * of 32.768 kHz using either
 *
 * - the 32-bit CPU FRC2 hardware counter with a frequency of 312.500 kHz or
 * - the 32-bit microsecond @ref xtimer module and the microsecond system time.
 *
 * The FRC2 CPU counter is used whenever possible. Because the FRC2 CPU
 * counter is occupied by the WiFi hardware driver for WiFi power management,
 * the @ref xtimer module has to be used by emulated RTT when the WiFi
 * interface is enabled by the `esp_wifi` and `esp_now` modules.
 * However, since the @ref xtimer module is needed for the `esp_wifi` or
 * `esp_now` modules anyway, the xtimer module is also available for
 * emulating the RTT.
 *
 * The emulated RTT counter uses a hardware abstraction layer that is
 * defined by a driver interface of the type #rtt_hw_driver_t, which
 * generally provides a 27-bit RTC counter with a frequency of 32.678 kHz and
 * without set feature. This way the RTT implementation always sees a
 * 27-bit counter with a frequency of 32.678 kHz regardless of which
 * implementation is actually used.
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 */

#ifndef RTT_ARCH_H
#define RTT_ARCH_H

#include "periph/rtt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   RTT hardware abstraction layer driver
 */
typedef struct {
    /**
     * @brief       Init the current hardware counter
     */
    void (*init)(void);

    /**
     * @brief       Get the current hardware counter value
     * @return      32-bit counter value with a frequency of 1 MHz
     */
    uint32_t (*get_counter)(void);

    /**
     * @brief       Set the hardware alarm
     * @param[in]   alarm   alarm time in microseconds
     * @param[in]   cb      function called on alarm interrupt
     * @param[in]   arg     argument used as parameter for the @p cb function
     */
    void (*set_alarm)(uint32_t alarm_us, rtt_cb_t cb, void *arg);

    /**
     * @brief       Clear the hardware alarm
     */
    void (*clear_alarm)(void);

    /**
     * @brief       Save the counter value before sleep or reboot if necessary
     */
    void (*save_counter)(void);

    /**
     * @brief       Restore the counter value before sleep or reboot
     * @param[in]   in_init true if function is called after deep sleep or
     *              reboot, false otherwise
     */
    void (*restore_counter)(bool in_init);

    /**
     * @brief       Enable the RTT hardware counter
     */
    void (*poweron)(void);

    /**
     * @brief       Disable the RTT hardware counter
     */
    void (*poweroff)(void);

} rtt_hw_driver_t;

/**
 * @brief   Called before the power management enters a light or deep sleep mode
 * @param   mode    sleep mode that is entered
 * @return          time to sleep in us
 */
uint32_t rtt_pm_sleep_enter(unsigned mode);

/**
 * @brief   Called after the power management left light sleep mode
 * @param   cause   wake-up cause
 */
void rtt_pm_sleep_exit(uint32_t cause);

#ifdef __cplusplus
}
#endif

#endif /* RTT_ARCH_H */
/** @} */
