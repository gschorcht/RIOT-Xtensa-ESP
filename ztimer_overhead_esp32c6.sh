CFLAGS='-DCONFIG_ESP32C6_DEFAULT_CPU_FREQ_MHZ_20=1' make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-C6 @20MHz ===" >> ztimer_overhead_esp32c6.prn;
make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32c6.prn

CFLAGS='-DCONFIG_ESP32C6_DEFAULT_CPU_FREQ_MHZ_40=1' make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-C6 @40MHz ===" >> ztimer_overhead_esp32c6.prn;
make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32c6.prn

CFLAGS='-DCONFIG_ESP32C6_DEFAULT_CPU_FREQ_MHZ_80=1' make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-C6 @80MHz ===" >> ztimer_overhead_esp32c6.prn;
make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32c6.prn

CFLAGS='-DCONFIG_ESP32C6_DEFAULT_CPU_FREQ_MHZ_160=1' make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-C6 @160MHz ===" >> ztimer_overhead_esp32c6.prn;
make BOARD=esp32c6-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32c6.prn

