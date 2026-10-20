CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_2=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @2MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_5=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @5MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_10=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @10MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_20=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @20MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_40=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @40MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_80=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @80MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_160=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @160MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn

CFLAGS='-DCONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ_240=1' make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-S2 @240MHz ===" >> ztimer_overhead_esp32s2.prn;
make BOARD=esp32s2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32s2.prn


