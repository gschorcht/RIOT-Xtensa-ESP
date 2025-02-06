MODULE = esp_idf_heap

# source files to be compiled for this module
#ESP32_SDK_SRC = \
#  components/heap/heap_caps.c \
#  components/heap/heap_caps_init.c \
#  components/heap/heap_task_info.c \
#  components/heap/heap_trace_standalone.c \
#  components/heap/multi_heap.c \
#  components/heap/multi_heap_poisoning.c \
#  components/heap/port/$(CPU_FAM)/memory_layout.c \
#  components/heap/port/memory_layout_utils.c \
  #

ESP32_SDK_SRC += components/esp_mm/heap_align_hw.c
ESP32_SDK_SRC += components/esp_mm/esp_cache.c

ESP32_SDK_SRC += components/heap/heap_caps.c
ESP32_SDK_SRC += components/heap/heap_caps_base.c
ESP32_SDK_SRC += components/heap/heap_caps_init.c
#ESP32_SDK_SRC += components/heap/heap_task_info.c
#ESP32_SDK_SRC += components/heap/heap_trace_standalone.c
ESP32_SDK_SRC += components/heap/multi_heap.c
#ESP32_SDK_SRC += components/heap/multi_heap_poisoning.c
ESP32_SDK_SRC += components/heap/port/$(CPU_FAM)/memory_layout.c
ESP32_SDK_SRC += components/heap/port/memory_layout_utils.c

# We don't use ESP-IDF TLSF at all to prevent colissions. It doesn't seem to do
# something special with the exception of heap poisoning which isn't configured
# by default.
# ifeq (,$(filter tlsf,$(USEPKG)))
#   ESP32_SDK_SRC += components/heap/heap_tlsf.c
# endif

# additional include pathes required by this module
INCLUDES += -I$(ESP32_SDK_DIR)/components/esp_rom/include
INCLUDES += -I$(ESP32_SDK_DIR)/components/esp_system/include
INCLUDES += -I$(ESP32_SDK_DIR)/components/heap/tlsf

include $(RIOTBASE)/Makefile.base

ESP32_SDK_BIN = $(BINDIR)/$(MODULE)

include ../esp_idf.mk
include ../esp_idf_cflags.mk

# set by default
CFLAGS += -DMULTI_HEAP_FREERTOS
CFLAGS += -Wno-incompatible-pointer-types
