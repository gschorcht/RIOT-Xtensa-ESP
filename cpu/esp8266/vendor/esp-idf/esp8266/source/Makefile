MODULE=esp_idf_esp8266

include $(RIOTBASE)/Makefile.base

INCLUDES += -I$(RIOTCPU)/$(CPU)/vendor/esp-idf/wpa_supplicant/include
INCLUDES += -I$(RIOTCPU)/$(CPU)/vendor/esp-idf/wpa_supplicant/port/include
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/esp8266/source
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/util/include
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/wpa_supplicant/include

CFLAGS += -Wno-strict-prototypes
CFLAGS += -Wno-old-style-definition
CFLAGS += -fcommon
