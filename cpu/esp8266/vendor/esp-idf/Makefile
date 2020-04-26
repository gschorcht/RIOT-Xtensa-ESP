MODULE=esp_idf

DIRS += esp8266/source
DIRS += nvs_flash/src
DIRS += spi_flash/src
DIRS += util/src
DIRS += wpa_supplicant

ifneq (, $(filter esp_idf_heap, $(USEMODULE)))
  DIRS += heap
endif

include $(RIOTBASE)/Makefile.base

INCLUDES += -I$(RIOTCPU)/$(CPU)/vendor/esp-idf/log/include

