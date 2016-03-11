#!/bin/bash
# ============================================================================
# Program:    fld.sh
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150707
# Updated:    20150807:1543
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
# Run this script to use the self-hosted decompiler to decompile
# a Freeputer program. In the same directory as this script there must be:
#
#   * fvm16-0kB = a (suitably sized) VM executable for running the decompiler
#   * rom.fp    = a rom containing the (previously compiled) decompiler
#                 that the fvm executable will run in order
#                 to decompile your program
#   * std.blk = "standard block" (although not used by the decompiler,
#                 this is required by the VM; it represents the
#                 standard block device used for storage).
#   * std.imp = "standard import" (although not used by the decompiler,
#                 this is required by the VM; it represents a
#                 standard data import stream).           
#
# For example, if your compiled program is 'work/rom.fp' then:
#
#   ./fld.sh work/rom.fp
#
# Will decompile your program to 'std.trc' (see below).
#
# You will then notice, in the same directory as this 'fld.sh' script,
# that the following two files will now also exist:
#
#   * std.exp = "standard export" (although not used by the decompiler,
#                 this is automatically created by the VM; it represents a
#                 standard data export stream).
#   * std.trc = "standard trace" (this is automatically created by the VM;
#                 it represents a standard stream for tracing and debugging
#                 output and contains the decompiler output).
#
# All output from the self-hosted decompiler is to be found in the file
# 'std.trc' ("standard trace") which is located in the same directory as
# this 'fld.sh' script. It provides a useful decompiled view
# of your program in plain text format.
#
# ----------------------------------------------------------------------------
# Requirements:
#
#   This bash script is intended to be run on 32-bit Linux.
#
# Details:
#
#   This script has been tested and is known to work correctly on:
#     * 32-bit Debian GNU/Linux 7.8
# ============================================================================

# Start up our local FVM instance (the local 'fvm16-0kB' executable). It
# will run the decompiler present in its local 'rom.fp' file then shut down.
# Pipe input from the compiled program to stdin of the FVM.
# All decompiler output will be found in 'std.trc'.
./fvm16-0kB < $1

