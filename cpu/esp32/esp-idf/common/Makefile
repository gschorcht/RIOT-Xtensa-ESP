MODULE = esp_idf_common

# source files required from ESP-IDF in any case, regardless of additional modules
#ESP32_SDK_SRC = \
#  components/bootloader_support/bootloader_flash/src/bootloader_flash_config_$(CPU_FAM).c \
#  components/bootloader_support/bootloader_flash/src/bootloader_flash.c \
#  components/bootloader_support/src/bootloader_efuse.c \
#  components/bootloader_support/src/bootloader_mem.c \
#  components/bootloader_support/src/bootloader_random_$(CPU_FAM).c \
#  components/esp_hw_support/esp_clk.c \
#  components/esp_hw_support/periph_ctrl.c \
#  components/esp_hw_support/esp_gpio_reserve.c \
#  components/esp_hw_support/port/$(CPU_FAM)/rtc_clk.c \
#  components/esp_hw_support/port/$(CPU_FAM)/rtc_clk_init.c \
#  components/esp_hw_support/port/$(CPU_FAM)/rtc_init.c \
#  components/esp_hw_support/port/$(CPU_FAM)/rtc_sleep.c \
#  components/esp_hw_support/port/$(CPU_FAM)/rtc_time.c \
#  components/esp_hw_support/regi2c_ctrl.c \
#  components/esp_hw_support/port/$(CPU_FAM)/sar_periph_ctrl.c \
#  components/esp_hw_support/sleep_gpio.c \
#  components/esp_hw_support/sleep_event.c \
#  components/esp_hw_support/sleep_modes.c \
#  components/esp_hw_support/sleep_modem.c \
#  components/esp_mm/esp_mmu_map.c \
#  components/esp_mm/port/$(CPU_FAM)/ext_mem_layout.c \
#  components/esp_pm/pm_impl.c \
#  components/esp_rom/patches/esp_rom_sys.c \
#  components/esp_rom/patches/esp_rom_uart.c \
#  components/esp_system/esp_err.c \
#  components/esp_system/esp_system.c \
#  components/esp_system/port/cpu_start.c \
#  components/esp_system/port/soc/$(CPU_FAM)/cache_err_int.c \
#  components/esp_system/port/soc/$(CPU_FAM)/clk.c \
#  components/esp_system/port/soc/$(CPU_FAM)/reset_reason.c \
#  components/esp_system/port/esp_system_chip.c \
#  components/esp_system/system_time.c \
#  components/esp_timer/src/esp_timer.c \
#  components/esp_timer/src/esp_timer_impl_common.c \
#  components/esp_timer/src/esp_timer_init.c \
#  components/esp_timer/src/system_time.c \
#  components/hal/efuse_hal.c \
#  components/hal/$(CPU_FAM)/efuse_hal.c \
#  components/hal/mpu_hal.c \
#  components/hal/timer_hal.c \
#  components/hal/uart_hal.c \
#  components/hal/wdt_hal_iram.c \
#  components/newlib/port/esp_time_impl.c \
#  components/spi_flash/cache_utils.c \
#  components/spi_flash/flash_ops.c \
  #

ESP32_SDK_SRC += components/bootloader_support/bootloader_flash/src/bootloader_flash_config_$(CPU_FAM).c
ESP32_SDK_SRC += components/bootloader_support/bootloader_flash/src/bootloader_flash.c
ESP32_SDK_SRC += components/bootloader_support/src/bootloader_common.c
ESP32_SDK_SRC += components/bootloader_support/src/bootloader_efuse.c
ESP32_SDK_SRC += components/bootloader_support/src/bootloader_mem.c
ESP32_SDK_SRC += components/bootloader_support/src/bootloader_random_$(CPU_FAM).c
ESP32_SDK_SRC += components/esp_app_format/esp_app_desc.c
ESP32_SDK_SRC += components/esp_hw_support/clk_ctrl_os.c
ESP32_SDK_SRC += components/esp_hw_support/esp_clk.c
ESP32_SDK_SRC += components/esp_hw_support/esp_gpio_reserve.c
ESP32_SDK_SRC += components/esp_hw_support/periph_ctrl.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/esp_clk_tree.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/rtc_clk.c
#ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/rtc_clk_init.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/rtc_init.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/rtc_sleep.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/rtc_time.c
ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/sar_periph_ctrl.c
ESP32_SDK_SRC += components/esp_hw_support/port/esp_clk_tree_common.c
ESP32_SDK_SRC += components/esp_hw_support/regi2c_ctrl.c
ESP32_SDK_SRC += components/esp_hw_support/sleep_gpio.c
ESP32_SDK_SRC += components/esp_hw_support/sleep_event.c
ESP32_SDK_SRC += components/esp_hw_support/sleep_modes.c
ESP32_SDK_SRC += components/esp_hw_support/sleep_modem.c
ESP32_SDK_SRC += components/esp_mm/esp_mmu_map.c
ESP32_SDK_SRC += components/esp_mm/port/$(CPU_FAM)/ext_mem_layout.c
#ESP32_SDK_SRC += components/esp_pm/pm_impl.c
ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_sys.c
ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_uart.c
ESP32_SDK_SRC += components/esp_system/esp_err.c
#ESP32_SDK_SRC += components/esp_system/esp_system.c
ESP32_SDK_SRC += components/esp_system/port/cpu_start.c
ESP32_SDK_SRC += components/esp_system/port/soc/$(CPU_FAM)/cache_err_int.c
ESP32_SDK_SRC += components/esp_system/port/soc/$(CPU_FAM)/clk.c
#ESP32_SDK_SRC += components/esp_system/port/soc/$(CPU_FAM)/reset_reason.c
ESP32_SDK_SRC += components/esp_system/port/esp_system_chip.c
#ESP32_SDK_SRC += components/esp_system/system_time.c
ESP32_SDK_SRC += components/esp_timer/src/esp_timer.c
ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_common.c
ESP32_SDK_SRC += components/esp_timer/src/esp_timer_init.c
ESP32_SDK_SRC += components/esp_timer/src/system_time.c
ESP32_SDK_SRC += components/hal/efuse_hal.c
ESP32_SDK_SRC += components/hal/mmu_hal.c
ESP32_SDK_SRC += components/hal/$(CPU_FAM)/clk_tree_hal.c
ESP32_SDK_SRC += components/hal/$(CPU_FAM)/efuse_hal.c
#ESP32_SDK_SRC += components/hal/mpu_hal.c
#ESP32_SDK_SRC += components/hal/timer_hal.c
#ESP32_SDK_SRC += components/hal/uart_hal.c
ESP32_SDK_SRC += components/hal/wdt_hal_iram.c
ESP32_SDK_SRC += components/newlib/port/esp_time_impl.c
ESP32_SDK_SRC += components/spi_flash/cache_utils.c
ESP32_SDK_SRC += components/spi_flash/flash_ops.c
#ESP32_SDK_SRC += components/esp_system/startup_funcs.c

ifneq (,$(filter esp_spi_ram,$(USEMODULE)))
  INCLUDES += -I$(ESP32_SDK_DIR)/components/esp_psram/include
endif

ifneq (,$(filter periph_adc periph_dac,$(USEMODULE)))
#  ESP32_SDK_SRC += components/hal/rtc_io_hal.c
endif

# TODO separate module
ifneq (,$(filter periph_can,$(USEMODULE)))
  ESP32_SDK_SRC += components/hal/twai_hal.c
  ESP32_SDK_SRC += components/hal/twai_hal_iram.c
endif

# TODO separate module
ifneq (,$(filter periph_dac,$(USEMODULE)))
#  ESP32_SDK_SRC += components/driver/dac_common.c
#  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/dac_periph.c
endif

# TODO separate module
ifneq (,$(filter periph_hwrng,$(USEMODULE)))
  ESP32_SDK_SRC += components/esp_hw_support/hw_random.c
endif

# TODO separate module
ifneq (,$(filter periph_i2c%,$(USEMODULE)))
  ESP32_SDK_SRC += components/driver/i2c/i2c.c
  ESP32_SDK_SRC += components/hal/i2c_hal.c
  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/i2c_periph.c
endif

# TODO separate module
ifneq (,$(filter periph_pwm%,$(USEMODULE)))
#  ESP32_SDK_SRC += components/hal/ledc_hal.c
#  ESP32_SDK_SRC += components/hal/ledc_hal_iram.c
#  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/ledc_periph.c
endif

# TODO separate module
ifneq (,$(filter periph_spi,$(USEMODULE)))
#  ESP32_SDK_SRC += components/hal/spi_hal.c
  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/spi_periph.c
endif

ifneq (,$(filter xtensa%,$(TARGET_ARCH)))
#  ESP32_SDK_SRC += components/esp_hw_support/rtc_wdt.c
  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/rtc_io_periph.c
endif

ifneq (,$(filter riscv%,$(TARGET_ARCH)))
  ESP32_SDK_SRC += components/riscv/interrupt.c
  ESP32_SDK_SRC += components/riscv/interrupt_intc.c
  ESP32_SDK_ASMSRC += components/freertos/FreeRTOS-Kernel/portable/riscv/portasm.S
  ESP32_SDK_ASMSRC += components/riscv/vectors.S
  ESP32_SDK_ASMSRC += components/riscv/vectors_intc.S
endif

ifeq (esp32,$(CPU_FAM))
#  # ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_frc_legacy.c
  ESP32_SDK_SRC += components/esp_mm/cache_esp32.c
  ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_lac.c
  ESP32_SDK_SRC += components/hal/esp32/cache_hal_esp32.c
endif

ifeq (esp32c3,$(CPU_FAM))
#  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/cpu_util_$(CPU_FAM).c
  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/systimer.c
  ESP32_SDK_SRC += components/esp_hw_support/sleep_console.c
#  ESP32_SDK_SRC += components/esp_hw_support/sleep_retention.c
  ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_systimer.c
#  ESP32_SDK_SRC += components/hal/$(CPU_FAM)/rtc_cntl_hal.c
  ESP32_SDK_SRC += components/hal/cache_hal.c
  ESP32_SDK_SRC += components/hal/systimer_hal.c
endif

ifeq (esp32s2,$(CPU_FAM))
  ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_regi2c_$(CPU_FAM).c
  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/systimer.c
  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/memprot.c
#  ESP32_SDK_SRC += components/esp_hw_support/sleep_retention.c
  ESP32_SDK_SRC += components/esp_system/port/brownout.c
  ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_systimer.c
#  ESP32_SDK_SRC += components/hal/$(CPU_FAM)/brownout_hal.c
  ESP32_SDK_SRC += components/hal/$(CPU_FAM)/touch_sensor_hal.c
  ESP32_SDK_SRC += components/hal/brownout_hal.c
  ESP32_SDK_SRC += components/hal/cache_hal.c
  ESP32_SDK_SRC += components/hal/systimer_hal.c
endif

ifeq (esp32s3,$(CPU_FAM))
#  ESP32_SDK_SRC += components/esp_hw_support/sleep_retention.c
  ESP32_SDK_SRC += components/esp_hw_support/mspi_timing_tuning.c
  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/mspi_timing_config.c
  ESP32_SDK_SRC += components/esp_hw_support/port/$(CPU_FAM)/systimer.c
  ESP32_SDK_SRC += components/esp_hw_support/sleep_console.c
  ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_cache_esp32s2_esp32s3.c
  ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_efuse.c
  ESP32_SDK_SRC += components/esp_timer/src/esp_timer_impl_systimer.c
#  ESP32_SDK_SRC += components/hal/$(CPU_FAM)/rtc_cntl_hal.c
  ESP32_SDK_SRC += components/hal/$(CPU_FAM)/touch_sensor_hal.c
  ESP32_SDK_SRC += components/hal/cache_hal.c
  ESP32_SDK_SRC += components/hal/systimer_hal.c
#  ESP32_SDK_SRC += components/hal/systimer_hal.c
#  ESP32_SDK_SRC += components/spi_flash/spi_flash_timing_tuning.c
  ESP32_SDK_SRC += components/spi_flash/$(CPU_FAM)/spi_flash_oct_flash_init.c
#  ESP32_SDK_SRC += components/spi_flash/$(CPU_FAM)/spi_timing_config.c
endif

include $(RIOTBASE)/Makefile.base

ESP32_SDK_BIN = $(BINDIR)/$(MODULE)

include ../esp_idf.mk
include ../esp_idf_cflags.mk
