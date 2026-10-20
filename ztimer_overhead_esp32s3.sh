CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_2=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @2MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_5=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @5MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_10=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @10MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_20=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @20MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_40=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @40MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_80=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @80MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_160=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @160MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn

CFLAGS='-DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ_240=1' make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 flash
echo "=== ESP32-S3 @240MHz ===" >> ztimer_overhead_s3.prn;
make BOARD=esp32s3-devkit -j8 -C tests/sys/ztimer_overhead PORT=/dev/ttyUSB2 test >> ztimer_overhead_s3.prn


