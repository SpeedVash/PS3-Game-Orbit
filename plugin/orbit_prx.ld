OUTPUT_FORMAT("elf64-powerpc")
OUTPUT_ARCH(powerpc:common64)
ENTRY(orbit_module_info)
PHDRS {
  text PT_LOAD FLAGS(0x400005);
  data PT_LOAD FLAGS(0x600006);
}
SECTIONS {
  . = 0;
  .text : { *(.text .text.*) } :text
  .rodata : { *(.rodata .rodata.*) } :text
  .opd : ALIGN(8) { *(.opd) } :text
  . = ALIGN(0x10000);
  .data : { *(.data .data.*) } :data
  .got : ALIGN(8) { *(.got .got.*) } :data
  .toc : ALIGN(8) { *(.toc .toc.* .toc1) } :data
  orbit_toc = ADDR(.got) + 0x8000;
  PROVIDE(.TOC. = orbit_toc);
  .bss (NOLOAD) : ALIGN(128) { *(.bss .bss.* COMMON) } :data
  /DISCARD/ : { *(.comment .note.* .eh_frame .eh_frame_hdr .gcc_except_table .gnu.attributes) }
}
