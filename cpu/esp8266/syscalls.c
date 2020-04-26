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
 * @brief       Implementation of required system calls
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 *
 * @}
 */

#include "esp_attr.h"
#include "esp_common.h"
#include "sdk/sdk.h"
#include "syscalls.h"

#ifdef MODULE_ESP_IDF_HEAP
#include "esp_heap_caps.h"
#endif

#define ENABLE_DEBUG    (0)
#include "debug.h"

int64_t esp_timer_get_time(void)
{
    extern int32_t system_time_cycles;
    /* TODO return 32-bit time for the moment */
    return (((int64_t)system_time_cycles) << 32) + system_get_time();
}

#ifdef MODULE_ESP_IDF_HEAP

/* Platform specific function if module esp_idf_heap is used. */
void heap_stats(void)
{
    size_t _free = 0;
    size_t _alloc = 0;

    extern heap_region_t g_heap_region[HEAP_REGIONS_MAX];

    for (int i = 0; i < HEAP_REGIONS_MAX; i++) {
        _free += g_heap_region[i].free_bytes;
        _alloc += g_heap_region[i].total_size - g_heap_region[i].free_bytes;
    }

    ets_printf("heap: %u (used %u, free %u) [bytes]\n",
               _alloc + _free, _alloc, _free);
}

#else /* MODULE_ESP_IDF_HEAP */

extern uint8_t  _eheap;     /* end of heap (defined in ld script) */
extern uint8_t  _sheap;     /* start of heap (defined in ld script) */

/* Platform specific function if module esp_idf_heap is not used. */
size_t heap_caps_get_dram_free_size(void)
{
    return &_eheap - &_sheap;
}

#endif /* MODULE_ESP_IDF_HEAP */

/**
 * @name Other system functions
 */

int _rename_r(struct _reent *r, const char *from, const char *to)
{
    return 0;
}

uint32_t system_get_time(void)
{
    return phy_get_mactime();
}

uint32_t system_get_time_ms(void)
{
    return system_get_time() / USEC_PER_MSEC;
}

int32_t system_time_cycles = 0;

uint64_t system_get_time_64(void)
{
    return ((uint64_t)system_time_cycles << 32) + system_get_time();
}

void IRAM_ATTR syscalls_init_arch(void)
{
}
