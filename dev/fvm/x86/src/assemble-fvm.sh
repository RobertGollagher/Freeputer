#!/bin/bash
# ============================================================================
# Program:    assemble-fvm.sh
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150707
# Updated:    20150807:1715
# Version:    1.0.0.0 for FVM 1.0
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
# Run this script to assemble the FVM from its x86 assembly language source.
# The source file 'fvm.s' must be in the same directory as this script.
#
# The command:
#
#   ./assemble-fvm.sh
#
# Will produce:
#
#     build/fvm              = the VM executable (ready for deployment)
#     build/fvm.o            = object file (you can discard this)
#
# After assembling the FVM executable you may wish to rename it to some name
# that helps you to identify its sizing, version or other attributes.
# For example, you might rename 'fvm' to 'fvm16MB' or similar.
# You would then modify any start-up scripts appropriately.
#
# ----------------------------------------------------------------------------
# Requirements:
#
#   This bash script is intended to be run on 32-bit Linux.
#   You must have as (the GNU assembler) and ld (the GNU linker) installed;
#   in other words, you must have GNU Binutils installed.
#
# Details:
#
#   This script has been tested and is known to work correctly on:
#     * 32-bit Debian GNU/Linux 7.8 using
#     * GNU assembler (GNU Binutils for Debian) 2.22
#     * GNU ld (GNU Binutils for Debian) 2.22
#   As found in:
#     Package: binutils
#     Version: 2.22-8+deb7u2
# ============================================================================

# To assemble and link for production:
# ====================================
 as -o build/fvm.o fvm.s          # Assemble
 ld -o build/fvm build/fvm.o      # Link
 strip build/fvm                  # Strip symbols

# To assemble and link for debugging the FVM executable itself:
# =============================================================
# as -gstabs -o build/fvm.o fvm.s            # Assemble for debugging
# ld -o build/fvm build/fvm.o                # Link
# objdump -Dslx build/fvm > build/fvm.dasm   # Produce objdump file for analysis
# readelf -a build/fvm > build/fvm.readelf   # Produce readelf file for analysis

