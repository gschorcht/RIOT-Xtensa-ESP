/*
 * Copyright (C) 2019 Gunar Schorcht
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
 * @brief       ESP function required by the SDK
 *
 * This file is a collection of functions required by ESP8266 RTOS SDK.
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 */

#define ENABLE_DEBUG (0)
#include "debug.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "esp/xtensa_ops.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_libc.h"
#include "irq_arch.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "syscalls.h"

#include "xtensa/xtensa_api.h"

/* Just to satisfy the linker, lwIP from SDK is not used */
uint32_t LwipTimOutLim = 0;

#ifndef MODULE_LWIP_ETHERNET
const uint8_t ethbroadcast[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif

void IRAM_ATTR HDL_MAC_SIG_IN_LV1_ISR(void)
{
    extern unsigned int ets_soft_int_type;
    ets_soft_int_type = ETS_SOFT_INT_HDL_MAC;
    WSR(BIT(ETS_SOFT_INUM), interrupt);
}

void IRAM_ATTR PendSV(int req)
{
    extern unsigned int ets_soft_int_type;
    if (req == 1) {
        critical_enter();
        ets_soft_int_type = ETS_SOFT_INT_YIELD;
        WSR(BIT(ETS_SOFT_INUM), interrupt);
        critical_exit();
    } else if (req == 2) {
        ets_soft_int_type = ETS_SOFT_INT_HDL_MAC;
        WSR(BIT(ETS_SOFT_INUM), interrupt);
    }
}

void __attribute__((noreturn)) _esp_error_check_failed(esp_err_t rc, const char *file, int line, const char *function, const char *expression)
{
    printf("ESP_ERROR_CHECK failed: esp_err_t 0x%x at %p\n", rc, __builtin_return_address(0));
    printf("file: \"%s\" line %d\nfunc: %s\nexpression: %s\n", file, line, function, expression);
    abort();
}

void IRAM_ATTR _xt_isr_attach(uint8_t i, xt_handler func, void* arg)
{
    DEBUG("%s %d %p\n", __func__, i, func);
    xt_set_interrupt_handler(i, func, arg);
}


unsigned int IRAM_ATTR _xt_isr_unmask(unsigned int mask)
{
    DEBUG("%s %08x\n", __func__, mask);
    return xt_ints_on(mask);
}

unsigned int IRAM_ATTR _xt_isr_mask(unsigned int mask)
{
    DEBUG("%s %08x\n", __func__, mask);
    return xt_ints_off(mask);
}

void IRAM_ATTR _xt_clear_ints(uint32_t mask)
{
    DEBUG("%s %08x\n", __func__, mask);
    xt_set_intclear(mask);
}

void IRAM_ATTR _xt_set_xt_ccompare_val(void)
{
    /* to figure out whether it is called at all, not yet implemented */
    assert(0);
    DEBUG("%s\n", __func__);
}

/*
 * provided by: /path/to/esp-idf/component/log/log.c
 */
uint32_t IRAM_ATTR esp_log_timestamp(void)
{
    return system_get_time() / USEC_PER_MSEC;
}

typedef struct {
    const char *tag;
    unsigned level;
} esp_log_level_entry_t;

static esp_log_level_entry_t _log_levels[] = {
    { .tag = "wifi", .level = LOG_DEBUG },
    { .tag = "*", .level = LOG_DEBUG },
};

static char _printf_buf[PRINTF_BUFSIZ];

/*
 * provided by: /path/to/esp-idf/component/log/log.c
 */
void IRAM_ATTR esp_log_write(esp_log_level_t level,
                             const char* tag, const char* format, ...)
{
    /*
     * We use the log level set for the given tag instead of using
     * the given log level.
     */
    esp_log_level_t act_level = LOG_DEBUG;
    size_t i;
    for (i = 0; i < ARRAY_SIZE(_log_levels); i++) {
        if (strcmp(tag, _log_levels[i].tag) == 0) {
            act_level = _log_levels[i].level;
            break;
        }
    }

    /* If we didn't find an entry for the tag, we use the log level for "*" */
    if (i == ARRAY_SIZE(_log_levels)) {
        act_level = _log_levels[ARRAY_SIZE(_log_levels)-1].level;
    }

    /* Return if the log output has not the required level */    
    if ((unsigned)act_level > CONFIG_LOG_DEFAULT_LEVEL) {
        return;
    }

    va_list arglist;
    va_start(arglist, format);
    vsnprintf(_printf_buf, PRINTF_BUFSIZ, format, arglist);
    va_end(arglist);

    switch (act_level) {
        case LOG_NONE   : return;
        case LOG_ERROR  : LOG_TAG_ERROR  (tag, "%s\n", _printf_buf); break;
        case LOG_WARNING: LOG_TAG_WARNING(tag, "%s\n", _printf_buf); break;
        case LOG_INFO   : LOG_TAG_INFO   (tag, "%s\n", _printf_buf); break;
        case LOG_DEBUG  : LOG_TAG_DEBUG  (tag, "%s\n", _printf_buf); break;
        case LOG_ALL    : LOG_TAG_ALL    (tag, "%s\n", _printf_buf); break;
        default: break;
    }
}

#ifdef CONFIG_LOG_SET_LEVEL
/*
 * provided by: /path/to/esp-idf/component/log/log.c
 */
void esp_log_level_set(const char* tag, esp_log_level_t level)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(_log_levels); i++) {
        if (strcmp(tag, _log_levels[i].tag) == 0) {
            break;
        }
    }

    if (i == ARRAY_SIZE(_log_levels)) {
        LOG_DEBUG("Tag for setting log level not found\n");
        return;
    }

    _log_levels[i].level = level;
}
#endif
