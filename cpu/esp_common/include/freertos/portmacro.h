/*
 * Copyright (C) 2019 Gunar Schorcht
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * FreeRTOS to RIOT-OS adaption module for source code compatibility
 */

#ifndef FREERTOS_PORTMACRO_H
#define FREERTOS_PORTMACRO_H

#ifndef DOXYGEN

#include "stdint.h"

#include "mutex.h"
#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif

#define portBASE_TYPE                   int
#define portUBASE_TYPE                  unsigned portBASE_TYPE

#define portMAX_DELAY                   0xFFFFFFFF

#define portMUX_TYPE                    mutex_t
#define portMUX_INITIALIZER_UNLOCKED    MUTEX_INIT

#define portYIELD_FROM_ISR              thread_yield_higher

#define portENTER_CRITICAL              vTaskEnterCritical
#define portEXIT_CRITICAL               vTaskExitCritical
#define portENTER_CRITICAL_ISR          vTaskEnterCritical
#define portEXIT_CRITICAL_ISR           vTaskExitCritical
#define portENTER_CRITICAL_NESTED       irq_disable
#define portEXIT_CRITICAL_NESTED        irq_restore

#ifdef MCU_ESP32

#define portNUM_PROCESSORS              2
#define xPortGetCoreID()                PRO_CPU_NUM

#else /* MCU_ESP32 */

#define portNUM_PROCESSORS              1
#define xPortGetCoreID()                PRO_CPU_NUM
#define xtbsp_clock_freq_hz()           80000000

extern const uint32_t g_esp_ticks_per_us;
extern unsigned _xt_tick_divisor;

#endif /* MCU_ESP32 */

extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);
extern void vTaskEnterCritical(portMUX_TYPE *mux);
extern void vTaskExitCritical(portMUX_TYPE *mux);

#ifdef __cplusplus
}
#endif

#endif /* DOXYGEN */
#endif /* FREERTOS_PORTMACRO_H */
