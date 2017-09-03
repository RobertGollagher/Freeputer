#! /bin/bash
# To build 'qmisc' from 'qmisc.s' do: ./build.sh qmisc

# Assemble before linking
as -o $1.o $1.s --32

# For linking with gcc uncomment next line:
#gcc -static -o $1 $1.o -m32

# For linking with ld uncomment next line:

ld -o $1 $1.o -m elf_i386

# For disassembly uncomment next line:
objdump -d $1 > $1.dasm

