#!/bin/bash
# ============================================================================
# Program:    romMake.sh
# Copyright © Robert Gollagher 2016
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20160314
# Updated:    20160314:2323
# Version:    pre-alpha-0.0.0.1 for FVM 1.1
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Instructions
# ============
#
# Running this script compiles a Freeputer program from the specified
# source file into binary 'rom.fp' and creates a C header file 'rom.h'
# which represents that binary program as a char array. The purpose of doing
# so is so that subequently you can include the compiled program
# into an FVM executable as described in 'fvm.m4'.
#
# Example:
#
#   ./romMake.sh source.fl
#
# Produces:
#
#   rom.fp
#   rom.h
#
# ============================================================================
if [ "$#" -ne 1 ]; then
  echo "You must specify a Freelang source file"
else
  ./flx.rb $1 rom.fp && xxd -i rom.fp > rom.h
  sed -i '1s/.*/const unsigned char prog[] PROGMEM = {/' rom.h
  sed -i '$s/rom_fp_len/prog_size/' rom.h
fi
