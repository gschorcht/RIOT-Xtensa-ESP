MODULE = esp_idf_spi_flash

# source files to be compiled for this module
#ESP32_SDK_SRC = \
#  components/bootloader_support/src/bootloader_common.c \
#  components/driver/spi_common.c \
#  components/spi_flash/$(CPU_FAM)/flash_ops_$(CPU_FAM).c \
#  components/spi_flash/$(CPU_FAM)/spi_flash_rom_patch.c \
#  components/spi_flash/partition.c \
  #

# source files to be compiled for this module
#ESP32_SDK_SRC = \
#  components/bootloader_support/src/bootloader_common.c \
#  components/esp_rom/patches/esp_rom_spiflash.c \
#  components/spi_flash/cache_utils.c \
#  components/hal/mmu_hal.c \
  #

ESP32_SDK_SRC += components/esp_rom/patches/esp_rom_spiflash.c

ESP32_SDK_SRC += components/spi_flash/esp_flash_api.c

# ---

ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_drivers.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_generic.c
ESP32_SDK_SRC += components/spi_flash/memspi_host_driver.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_os_func_noos.c
ESP32_SDK_SRC += components/hal/spi_flash_hal.c
ESP32_SDK_SRC += components/hal/spi_flash_hal_iram.c
ESP32_SDK_SRC += components/hal/spi_flash_encrypt_hal_iram.c

ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_boya.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_th.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_mxic.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_gd.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_issi.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_mxic_opi.c
ESP32_SDK_SRC += components/spi_flash/spi_flash_chip_winbond.c

ESP32_SDK_SRC += components/spi_flash/flash_mmap.c
ESP32_SDK_SRC += components/spi_flash/esp_flash_spi_init.c

ifneq (,$(filter esp32,$(CPU_FAM)))
endif

ifneq (,$(filter esp32s2,$(CPU_FAM)))
  ESP32_SDK_SRC += components/hal/spi_flash_hal_gpspi.c
endif

ifneq (,$(filter esp32s3,$(CPU_FAM)))
  ESP32_SDK_SRC += components/hal/spi_flash_hal_gpspi.c
endif

ifneq (,$(filter esp32c3,$(CPU_FAM)))
  ESP32_SDK_SRC += components/hal/spi_flash_hal_gpspi.c
endif

# ---

ifeq (,$(filter periph_spi,$(USEMODULE)))
  # no need to compile it here if it is already compiled for periph_spi
  ESP32_SDK_SRC += components/soc/$(CPU_FAM)/spi_periph.c
endif

ifneq (,$(filter esp32,$(CPU_FAM)))
#  ESP32_SDK_SRC += components/spi_flash/esp_flash_spi_init.c
endif

# additional include pathes required by this module
INCLUDES += -I$(ESP32_SDK_DIR)/components/app_update/include
#INCLUDES += -I$(ESP32_SDK_DIR)/components/bootloader_support/include_bootloader
INCLUDES += -I$(ESP32_SDK_DIR)/components/esp_rom/$(CPU_FAM)
INCLUDES += -I$(ESP32_SDK_DIR)/components/spi_flash/include/spi_flash

ifeq (,$(filter esp_spi_ram,$(USEMODULE)))
  INCLUDES += -I$(ESP32_SDK_DIR)/components/esp_psram/include
endif

include $(RIOTBASE)/Makefile.base

ESP32_SDK_BIN = $(BINDIR)/$(MODULE)

include ../esp_idf.mk
include ../esp_idf_cflags.mk
