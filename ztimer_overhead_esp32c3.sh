CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_2=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @2MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_5=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @5MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_10=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @10MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_20=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @20MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_40=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @40MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_80=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @80MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn

CFLAGS='-DCONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ_160=1' make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 flash
echo "=== ESP32-C3 @160MHz ===" >> ztimer_overhead_c3.prn;
make BOARD=esp32c3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB1 test >> ztimer_overhead_c3.prn


