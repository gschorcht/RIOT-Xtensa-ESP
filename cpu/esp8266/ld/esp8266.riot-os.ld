/**
 * This linker script is a combined and modified version of esp8266.ld and
 * esp8266.common.ld from ESP8266-RTOS-SDK.
 */

ENTRY(call_start_cpu)

SECTIONS
{
  /* RTC .data section holds RTC data/rodata (RTC_DATA_ATTR, RTC_RODATA_ATTR) */
  .rtc.data :
  {
    _rtc_data_start = ABSOLUTE(.);

    *(.rtc.data .rtc.data.* .rtc.rodata .rtc.rodata.*)

    _sbackup_data = ABSOLUTE(.);
    *(.backup.data)
    _ebackup_data = ABSOLUTE(.);

    _rtc_data_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* RTC .bss holds RTC bss */
  .rtc.bss (NOLOAD) :
  {
    _rtc_bss_start = ABSOLUTE(.);
    *(.rtc.bss)

    _sbackup_bss = ABSOLUTE(.);
    *(.backup.bss)
    _ebackup_bss = ABSOLUTE(.);

    _rtc_bss_end = ABSOLUTE(.);
  } > rtc_data_seg

  /* RTC .noinit data that are not initialized at power up (RTC_NOINIT_ATTR) */
  .rtc_noinit (NOLOAD):
  {
    . = ALIGN(4);
    _rtc_noinit_start = ABSOLUTE(.);

    *(.rtc_noinit .rtc_noinit.*)

    . = ALIGN(4) ;
    _rtc_noinit_end = ABSOLUTE(.);
  } > rtc_data_seg

  ASSERT(((_rtc_noinit_end - ORIGIN(rtc_data_seg)) <= LENGTH(rtc_data_seg)),
        "RTC segment data does not fit.")

  .iram0.vectors : /* IRAM */
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    _iram_start = ABSOLUTE(.);
    _init_start = ABSOLUTE(.);

    KEEP(*(.SystemInfoVector.text));
    . = 0x10;
    KEEP(*(.DebugExceptionVector.text));
    . = 0x20;
    KEEP(*(.NMIExceptionVector.text));
    . = 0x30;
    KEEP(*(.KernelExceptionVector.text));
    . = 0x50;
    KEEP(*(.UserExceptionVector.text));
    . = 0x70;
    KEEP(*(.DoubleExceptionVector.text));
    . = 0x90;
    KEEP(*(.UserExceptionTrampoline.text));

    *(.*Vector.literal)
    *(.UserEnter.literal);
    *(.UserEnter.text);
    . = ALIGN (16);
    *(.entry.text)
    *(.init.literal)
    *(.init)

    _init_end = ABSOLUTE(.);
  } > iram0_0_seg

  .iram0.text :
  {
    /* Code marked as runnning out of IRAM, normal code should be in flash */
    _iram_text_start = ABSOLUTE(.);

    /* functions that are marked as .iram1.* section (IRAM_ATTR) */
    *(.iram1 .iram1.*)

    /* SDK libraries that expect their .text sections in IRAM */
    *libphy.a:(.literal .literal.* .text .text.*)
    *libpp.a:(.literal .literal.* .text .text.*)

    *gdbstub.a:*(.literal .text .literal.* .text.*)
    *(.stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)

    *esp_idf_heap.a:*(.literal .text .literal.* .text.*)
    *esp_idf_esp8266.a:esp_wifi.o(.literal .literal.* .text .text.*)
    *esp_idf_esp8266.a:ets_printf.o(.literal .text .literal.* .text.*)
    *esp_idf_spi_flash.a:spi_flash_raw.o(.literal .text .literal.* .text.*)

    *core.a:sched.o(.literal .text .literal.* .text.*)
    *freertos.a:*(.literal .text .literal.* .text.*)
    *xtimer.a:*(.literal .text .literal.* .text.*)

    /* Xtensa basic functionality written in assembler should be placed in iram */
    *xtensa.a:*(.literal .text .literal.* .text.*)

    /* libgcc functions required for debugging have to be in IRAM */
    *libgcc.a:unwind-dw2.o(.literal .text .literal.* .text.*)

    /* libgcc integer functions also need to be in .text */
    /* some are called before flash is mapped and also for performance) */
    /*
    *libgcc.a:*i3.o(.literal .text .literal.* .text.*)
    *libgcc.a:*mulsf3.o(.literal .text .literal.* .text.*)
    *libgcc.a:*divsf3.o(.literal .text .literal.* .text.*)
    *libgcc.a:*fixsfsi.o(.literal .text .literal.* .text.*)
    *libgcc.a:*cmpdf2.o(.literal .text .literal.* .text.*)
    *libgcc.a:*modsi3.o(.literal .text .literal.* .text.*)
    */

    /* libc also in IRAM */
    /*
    *libc.a:*malloc.o(.literal .text .literal.* .text.*)
    *libc.a:*mallocr.o(.literal .text .literal.* .text.*)
    *libc.a:*freer.o(.literal .text .literal.* .text.*)
    *libc.a:*memcpy.o(.literal .text .literal.* .text.*)
    *libc.a:*memchr.o(.literal .text .literal.* .text.*)
    *libc.a:*memset.o(.literal .text .literal.* .text.*)
    *libc.a:*memcmp.o(.literal .text .literal.* .text.*)
    *libc.a:*memmove.o(.literal .text .literal.* .text.*)
    *libc.a:*rand.o(.literal .text .literal.* .text.*)
    *libc.a:*bzero.o(.literal .text .literal.* .text.*)
    *libc.a:*lock.o(.literal .text .literal.* .text.*)

    *libc.a:*printf.o(.literal .text .literal.* .text.*)
    *libc.a:*findfp.o(.literal .text .literal.* .text.*)
    *libc.a:*fputwc.o(.literal .text .literal.* .text.*)
    *libc.a:*stdio.o(.literal .text .literal.* .text.*)
    *libc.a:*vfprintf.o(.literal .text .literal.* .text.*)
    *libc.a:*wsetup.o(literal .text .literal.* .text.*)
    *libc.a:*dtoa.o(literal .text .literal.* .text.*)
    */

    enc28j60.a:*(.literal .text .literal.* .text.*)

    _iram_text_end = ABSOLUTE(.);
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >iram0_0_seg

  .iram0.bss :
  {
    . = ALIGN (4);
    /* Data placed in IRAM */
    _iram_bss_start = ABSOLUTE(.);

    *libcore.a:(.bss .bss.* COMMON)
    *lwip_core.a:(.bss .bss.* COMMON)

    . = ALIGN (4);
    _iram_bss_end = ABSOLUTE(.);
    _iram_end = ABSOLUTE(.);
  } > iram0_0_seg

  ASSERT(((_iram_end - ORIGIN(iram0_0_seg)) <= LENGTH(iram0_0_seg)),
          "IRAM0 segment data does not fit.")

  .dram0.data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.gnu.linkonce.d.*)
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    *(.jcr)
    *(.dram0 .dram0.*)
    *(.data .data.* .dram1 .dram1.*)

    /* Place only necessary .rodata in DRAM */
    *libpp.a:(.rodata.* .rodata)
    *esp_idf_spi_flash.a:spi_flash_raw.o(.rodata.* .rodata)
    *esp_idf.a:(.rodata.* .rodata)
    *log.a:(.rodata.* .rodata)

    *libc.a:*.o(.rodata .rodata.*)
    *core.a:*(.rodata .rodata.*)
    *cpu.a:*(.rodata .rodata.*)
    *log.a:(.rodata .rodata.*)

    _data_end = ABSOLUTE(.);
  } > dram0_0_seg

  /**
   * This section holds data that should not be initialized at power up.
   * The section located in Internal SRAM memory region. The macro _NOINIT
   * can be used as attribute to place data into this section.
   * See the esp_attr.h file for more information.
   */
  .noinit (NOLOAD):
  {
    . = ALIGN(4);
    _noinit_start = ABSOLUTE(.);
    *(.noinit .noinit.*)
    . = ALIGN(4) ;
    _noinit_end = ABSOLUTE(.);
  } > dram0_0_seg

  /* Shared RAM */
  .dram0.bss (NOLOAD) :
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);

    *(EXCLUDE_FILE(*libcore.a) .bss .bss.* COMMON)
    *(EXCLUDE_FILE(*lwip_core.a) .bss .bss.* COMMON)

    *(.bss .bss.* COMMON)
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.share.mem)
    *(.gnu.linkonce.b.*)

    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _sheap = ABSOLUTE(.);
  } > dram0_0_seg

  ASSERT(((_bss_end - ORIGIN(dram0_0_seg)) <= LENGTH(dram0_0_seg)),
          "DRAM segment data does not fit.")

  . = 0x3FFFFFF0;
  _eheap = ABSOLUTE(.);

  .flash.text :
  {
    _stext = .;
    _text_start = ABSOLUTE(.);

    /* For ESP8266 library function */
    *(.irom0.literal .irom0.text)
    *(.irom.literal .irom.text .irom.text.literal)
    *(.text2 .text2.* .literal2 .literal2.*)

    /* place everything else in iram0_2_seg (cached ROM) */
    *(.text .text.* .literal .literal.*)

    *(.stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.irom0.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.fini.literal)
    *(.fini)
    *(.gnu.version)
    _text_end = ABSOLUTE(.);
    _etext = .;

    /**
     * Similar to _iram_start, this symbol goes here so it is
     * resolved by addr2line in preference to the first symbol in
     * the flash.text segment.
     */
    _flash_cache_start = ABSOLUTE(0);
  } > iram0_2_seg

  /* If the code size is smaller than 64 kB, align to next 64 kB page */
  . = MAX(ABSOLUTE(.), 0x40220000);

  .flash.rodata ALIGN(4) :
  {
    _rodata_start = ABSOLUTE(.);

    /**
     * Insert 8 bytes data to make realy rodata section's link address offset
     * to be 0x8, esptool will remove these data and add real segment header
     */
    . = 0x8;

    /* Should be the first. App version info. DO NOT PUT ANYTHING BEFORE IT! */
    *(.rodata_desc .rodata_desc.*)
    /* Should be the second. Custom app version info. DO NOT PUT ANYTHING BEFORE IT! */
    *(.rodata_custom_desc .rodata_custom_desc.*)
    /* For ESP8266 library function */
    *(.rodata2 .rodata2.*)

    *(EXCLUDE_FILE(esp_idf_spi_flash.a:spi_flash_raw.o) .rodata.* .rodata)
    *(EXCLUDE_FILE(esp_idf.a) .rodata.* .rodata)
    *(EXCLUDE_FILE(log.a).rodata.* .rodata)

    *(.irom1.text) /* catch stray ICACHE_RODATA_ATTR */
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE_ = ABSOLUTE(.);
    *(.xt_except_table)
    *(.gcc_except_table .gcc_except_table.*)
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    . = (. + 3) & ~ 3;
    __eh_frame = ABSOLUTE(.);
    KEEP(*(.eh_frame))
    . = (. + 7) & ~ 3;
    /*  C++ constructor and destructor tables */
    __init_array_start = ABSOLUTE(.);
    KEEP (*(EXCLUDE_FILE (*crtend.* *crtbegin.*) .ctors .ctors.*))
    __init_array_end = ABSOLUTE(.);
    KEEP (*crtbegin.*(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.*) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS_ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    /* Addresses of memory regions reserved via SOC_RESERVE_MEMORY_REGION() */
    soc_reserved_memory_region_start = ABSOLUTE(.);
    KEEP (*(.reserved_memory_address))
    soc_reserved_memory_region_end = ABSOLUTE(.);
    _rodata_end = ABSOLUTE(.);
    /* Literals are also RO data. */
    _lit4_start = ABSOLUTE(.);
    *(*.lit4)
    *(.lit4.*)
    *(.gnu.linkonce.lit4.*)
    _lit4_end = ABSOLUTE(.);
    . = ALIGN(4);
    _thread_local_start = ABSOLUTE(.);
    *(.tdata)
    *(.tdata.*)
    *(.tbss)
    *(.tbss.*)
    _thread_local_end = ABSOLUTE(.);
    . = ALIGN(4);
  } > iram0_2_seg
}
