.section ".rodata.orbit_module","a"
.balign 4
.global orbit_module_info
orbit_module_info:
    .short 0
    .byte 6,1
    .asciz "PS3GameOrbitService"
    .space 8
    .long orbit_toc
    .long orbit_exports,orbit_exports_end
    .long 0,0
.balign 4
orbit_exports:
    .byte 0x1c,0
    .short 0
    .short 0x8000
    .short 2
    .short 0,0
    .long 0
    .long orbit_nids,orbit_function_stubs,0
orbit_exports_end:
orbit_nids:
    .long 0xab779874,0xbc9a0086
orbit_function_stubs:
    .long orbit_service_stop+16,orbit_service_start+16
.section ".note.GNU-stack","",@progbits
