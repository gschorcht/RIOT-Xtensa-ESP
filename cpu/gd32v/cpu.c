/*
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
/**
 * @ingroup         cpu_gd32v
 * @{
 *
 * @file
 * @brief           GD32V CPU initialization
 *
 * @author          Koen Zandberg <koen@bergzand.net>
 */
#include "kernel_init.h"
#include "stdio_uart.h"
#include "periph/init.h"
#include "irq_arch.h"
#include "periph_cpu.h"
#include "periph_conf.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#undef MCAUSE_CAUSE /* redefined in NMSIS header */
#include "core_feature_base.h"

extern void __libc_init_array(void);

void cpu_init(void)
{
    gd32vf103_clock_init();
    /* enable PMU required for pm_layered */
    periph_clk_en(APB1, RCU_APB1EN_PMUEN_Msk);
    /* Common RISC-V initialization */
    riscv_init();
    early_init();
    periph_init();
}

void sched_arch_idle(void)
{
#if 0 // #ifdef MODULE_PM_LAYERED
    void pm_set_lowest(void);
    pm_set_lowest();
#else
    __WFI();
#endif
    __enable_irq();
    /* read/write memory barrier */
    __RWMB();
    __disable_irq();
}
