#!/bin/bash
# ============================================================================
# Program:    flc.sh
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150707
# Updated:    20150807:1545
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
# Run this script to use the self-hosted Freelang compiler to compile
# a Freelang program. In the same directory as this script there must be:
#
#   * fvm16-0MB = a (suitably sized) VM executable for running the compiler
#   * rom.fp    = a rom containing the (previously compiled) compiler
#                 that the fvm executable will run in order
#                 to compile your program
#   * std.blk   = "standard block" (although not used by the compiler,
#                 this is required by the VM; it represents the
#                 standard block device used for storage).
#   * std.imp   = "standard import" (although not used by the compiler,
#                 this is required by the VM; it represents a
#                 standard data import stream).           
#
# For example, if your source code is 'work/source.fl' then:
#
#   ./flc.sh work/source.fl work/rom.fp
#
# Will produce:
#
#     work/rom.fp     = a rom containing your newly compiled program
#
# You will also notice, in the same directory as this 'flc.sh' script,
# that the following two files will now also exist:
#
#   * std.exp = "standard export" (although not used by the compiler,
#                 this is automatically created by the VM; it represents a
#                 standard data export stream).
#   * std.trc = "standard trace" (this is automatically created by the VM;
#                 it represents a standard stream for tracing and debugging
#                 output and is extensively used by the compiler).
#
# All error messages and debugging output from flc, the self-hosted compiler,
# are to be found in the file 'std.trc' ("standard trace") which is located
# in the same directory as this 'flc.sh' script. If compiling succeeds,
# a detailed map of the newly compiled program will be found in 'std.trc';
# this map is invaluable when debugging Freelang programs.
#
# Notes:
#
#   1. The term "suitably sized" does not refer to the actual size of
#      the 'fvm' executable itself; it refers to the amount of RAM, ROM and
#      other resources that the VM instance will support at runtime. These
#      must be sufficient to run the compiler. For details see 'flc.fl'
#      (the Freelang source code for the self-hosted compiler)
#      and 'fvm.s' (the x86 assembly source for the VM).
#
#      This example 'flc.sh' script uses an fvm executable arbitrarily
#      called 'fvm16-0MB' which happens to have 16 MB of RAM, 16 MB of ROM,
#      a standard block of no capacity and no memory mapped devices.
#
#   2. The 'rom.fp' file containing the compiler must have been produced
#      either in this same manner (that is, compiled by an instance of the
#      self-hosted compiler) or cross-compiled on some other platform
#      (such as by using the Ruby Freelang cross compiler on Linux, 'flx.rb').
#
#   3. For your convenience, an 'fvm16-0MB' executable and a 'rom.fp'
#      containing the compiled compiler have been provided and are present
#      in the directory in which this 'flc.sh' script resides.
#      These are sufficient for compiling programs of moderate size,
#      such as compiling 'flc.fl', the self-hosted compiler itself.
#
#      IMPORTANT: You will find 'dev/freelang/compiler/bin/rom.fp'
#      (or similar) in the Freeputer distribution. This contains the compiled
#      self-hosted Freelang compiler, flc. In the event of accidental deletion
#      of the 'rom.fp' in your compiler directory, you can simply copy this
#      'dev/freelang/compiler/bin/rom.fp' file to your compiler directory.
#      Alternatively, you can use the flx cross compiler found in
#      'dev/xcompiler/ruby/flx.rb' to compile a new 'rom.fp'
#      from the 'flc.fl' source code.
#
#   4. An appropriate 'std.blk' file has been provided for your convenience.
#      It was created as a file of exactly zero size (no capacity at all)
#      by the following command: touch std.blk
#
#   5. The 'std.imp' file has been provided for your convenience.
#      It is simply an empty file and was created by the
#      following command: touch std.imp
#
#   6. To actually run your newly compiled program, which is now
#      present in the newly created 'work/rom.fp' file, copy that 'rom.fp'
#      file into a directory containing a suitably sized FVM executable
#      capable of running your program (along with a 'std.blk' file
#      and a 'std.imp' file that are appropriate for running your program).
#      The sizing of the FVM instance to which you deploy your compiled
#      program is unrelated to the size of the FVM instance used to
#      compile your program and depends on your program's nature.
#
#      The following discussion assumes that the FVM executable on which
#      you intend to run your newly compiled program is called 'fvm'.
#
#      Depending on the nature of your program, it may or may not be
#      appropriate to run it simply by issuing the command: ./fvm
#      Such an approach may suffice for simple programs.
#
#      Often you may wish to pipe input and output to your program
#      at runtime such as: ./fvm < myInputFile > myOutputFile
#      See this 'flc.sh' script for an example of this.
#
#      If your program involves terminal interaction then you may
#      need to place your terminal emulator in raw mode or the like.
#      See the FreeLine text editor ('fl.fl') for an example of this.
#
#      In future, some FVM instances may provide sophisticated devices such
#      as a graphical user interface (that is, as a memory-mapped device,
#      a block device or a stream device). If your program required the
#      use of such devices you should then deploy it only on an FVM instance
#      which provided them. It is, however, strongly recommended that the
#      design of applications should be modular not monolithic;
#      thus one FVM instance, lacking any such devices, could communicate
#      with another FVM instance, representing one such device,
#      to accomplish such ends in a modular manner.
#
#      Lastly, remember that Freeputer is designed to be so portable as to be
#      highly suitable for bare metal use without requiring any underlying
#      operating system. An FVM instance deployed on bare metal would then
#      be preconfigured to provide the few mandatory devices necessary to
#      run Freeputer programs (stdin, stdout, stdimp, stdexp, stdtrc, stdblk)
#      and would be sized appropriately for the hardware and for its
#      intended use depending on your program's requirements.
#
#   7. See also the comments in 'flx.rb' for further helpful and
#      relevant information regarding the platform.
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

# Start up our local FVM instance (the local 'fvm16-0MB' executable).
# It will run the compiler present in its local 'rom.fp' file then shut down.
# Pipe input from the specified source file to stdin of the FVM.
# Assuming you pipe output from stdout of the FVM to 'work/rom.fp' then
# the compiled program will then be in 'work/rom.fp'.
./fvm16-0MB < $1 > $2

