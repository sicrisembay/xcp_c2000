;****************************************************************************
;* boot.asm
;*
;* This is a simple long branch to initial boot routine (c_int00) for
;* c28xx C++ programs.  This is the initial entry point and located
;* in "codestart" section specified in the linker file.
;*
;****************************************************************************
    .global _c_int00
    .sect "app_codestart"
code_start:
    LB _c_int00


; This is workaround until bootloader is ready
    .sect "boot_codestart"
boot_code_start:
    LB 0x337C00

