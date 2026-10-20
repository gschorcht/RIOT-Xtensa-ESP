CFLAGS='-DCONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ_16=1' make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-H2 @16MHz ===" >> ztimer_overhead_esp32h2.prn;
make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32h2.prn

CFLAGS='-DCONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ_32=1' make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-H2 @32MHz ===" >> ztimer_overhead_esp32h2.prn;
make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32h2.prn

CFLAGS='-DCONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ_48=1' make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-H2 @48MHz ===" >> ztimer_overhead_esp32h2.prn;
make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32h2.prn

CFLAGS='-DCONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ_64=1' make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-H2 @64MHz ===" >> ztimer_overhead_esp32h2.prn;
make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32h2.prn

CFLAGS='-DCONFIG_ESP32H2_DEFAULT_CPU_FREQ_MHZ_96=1' make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 flash
echo "=== ESP32-H2 @96MHz ===" >> ztimer_overhead_esp32h2.prn;
make BOARD=esp32h2-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB0 test >> ztimer_overhead_esp32h2.prn

