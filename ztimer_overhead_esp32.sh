CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_2=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @2MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_5=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @5MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_10=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @10MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_20=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @20MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_40=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @40MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_80=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @80MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_160=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @160MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn

CFLAGS='-DCONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ_240=1' make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32 @240MHz ===" >> ztimer_overhead_esp32.prn.prn;
make BOARD=esp32-wroom-32 -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32.prn.prn


