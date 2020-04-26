/*
 * Copyright (C) 2019 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * FreeRTOS to RIOT-OS adaption module for source code compatibility
 */

#ifndef DOXYGEN

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "esp_attr.h"
#include "irq.h"
#include "rom/ets_sys.h"
#include "sdk_conf.h"

#include "freertos/FreeRTOS.h"
#include "xtensa/xtensa_rtos.h"

/* cached number of cycles per tick */
unsigned _xt_tick_divisor = xtbsp_clock_freq_hz() / XT_TICK_PER_SEC;

extern void vTaskEnterCritical( portMUX_TYPE *mux );
extern void vTaskExitCritical( portMUX_TYPE *mux );

void vPortEnterCritical(void)
{
    vTaskEnterCritical(0);
}

void vPortExitCritical(void)
{
    vTaskExitCritical(0);
}

#define INT_ENA_WDEV        0x3ff20c18
#define WDEV_TSF0_REACH_INT (BIT(27))

extern char NMIIrqIsOn;
extern uint32_t WDEV_INTEREST_EVENT;

void ets_nmi_lock(void)
{
    REG_WRITE(INT_ENA_WDEV, 0);
    for (unsigned i = 0; i < 10; i++) { }
    REG_WRITE(INT_ENA_WDEV, WDEV_TSF0_REACH_INT);
}

void ets_nmi_unlock(void)
{
    REG_WRITE(INT_ENA_WDEV, WDEV_INTEREST_EVENT);
}

void IRAM_ATTR vPortETSIntrLock(void)
{
    if (NMIIrqIsOn == 0) {
        vPortEnterCritical();
        REG_WRITE(INT_ENA_WDEV, 0);
        for (unsigned i = 0; i < 10; i++) { }
        REG_WRITE(INT_ENA_WDEV, WDEV_TSF0_REACH_INT);
    }
}

void IRAM_ATTR vPortETSIntrUnlock(void)
{
    if (NMIIrqIsOn == 0) {
        REG_WRITE(INT_ENA_WDEV, WDEV_INTEREST_EVENT);
        vPortExitCritical();
    }
}

void ResetCcountVal(unsigned int cnt_val)
{
    __asm__ volatile("wsr a2, ccount");
}

#endif /* DOXYGEN */
