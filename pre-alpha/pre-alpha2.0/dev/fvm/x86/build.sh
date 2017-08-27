#! /bin/bash

# Assemble before linking
as -o qmisc.o qmisc.s --32

# For linking with gcc uncomment next line:
#gcc -o qmisc qmisc.o #-m32

# For linking with ld uncomment next line:
ld -o qmisc qmisc.o -m elf_i386

# For disassembly uncomment next line:
objdump -d qmisc > qmisc.dasm

