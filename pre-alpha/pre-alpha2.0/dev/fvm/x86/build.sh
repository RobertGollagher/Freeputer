#! /bin/bash
as -o qmisc.o qmisc.s --32
ld -o qmisc qmisc.o -m elf_i386
