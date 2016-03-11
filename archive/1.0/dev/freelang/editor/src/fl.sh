#!/bin/bash
# ============================================================================
# Program:    fl.sh
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150708
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
# Run this script to run the FreeLine text editor.
# In the same directory as this script there must be:
#
#   * fvm16-16MB = a (suitably sized) VM executable for running the editor
#   * rom.fp     = a rom containing the (previously compiled) FreeLine editor
#                 program that the fvm executable will run for you to use
#   * std.blk = "standard block" (the standard block device used for storage;
#                 text is saved to and loaded from here)
#   * std.imp = "standard import" (a standard data import stream;
#                 text can be imported from here).
#
# Before starting the editor, set your terminal window size to
#   80 columns x 24 rows (or one of the larger sizes supported by the
#   editor, namely 80x43, 132x24 or 132x43) and do NOT change the
#   size of your terminal window while the editor is running.
#   In the editor, do NOT set the editor to a screen mode
#   any larger than that of your terminal window!
#
# To run the editor simply issue the command:
#
#   ./fl.sh
#
# There is no choice of file to edit. See the in-program help for further
# information. When you save your text it is simply saved into 'std.blk'.
# The next time you run the editor that text will be retrieved. If you wish
# to work on a different text, shut down the editor then copy 'std.blk'
# to a safe location and replace it with a new zero-filled 'std.blk'
# of the same total size; as a convenience you can do this by
# copying the supplied 'std.blk.empty' to 'std.blk'. Alternatively,
# assuming that the original size of your 'std.blk' was 16777216 bytes,
# you can recreate a suitable zero-filled 'std.blk' file by:
#     head -c 16777216 /dev/zero > std.blk
# To resume work on an older text, simply swap the 'std.blk' files
# appropriately while the editor is shut down, then restart the editor.
#
# The editor NEVER automatically saves text. All changes are temporary
# until you command the editor to save your text. Therefore if you make
# changes that you decide you no longer wish to keep, simply do not save.
# Exiting the editor does NOT cause the text to be saved.
#
# FreeLine is currently a demonstration program not a mature editor.
# Although it might seem strange that there is no choice of file to edit,
# there are advantages to this. Most importantly, the Freeputer VM
# does not require a file system. It is therefore appropriate that there
# should be a simple text editor that itself requires no file system.
# This accords with the principle of modular not monolithic design.
# This module edits text from a single location. Some other module
# in future could move data to and from that location, and such
# data does not necessarily have to live in a file system.
#
# The Freeputer VM is designed to be easy to port to bare metal,
# without requiring any underlying operating system, and thus
# provides extreme portability of applications. FreeLine is
# an editor well suited to such bare-metal deployment;
# it could be accessed from a remote terminal.
#
# You will also notice, in the same directory as this 'fl.sh' script,
# that the following two files will exist after running the editor:
#
#   * std.exp = "standard export" (this is automatically created by the VM;
#                 it represents a standard data export stream to which
#                 the editor can export text).
#   * std.trc = "standard trace" (this is automatically created by the VM;
#                 it represents a standard stream for tracing and debugging
#                 output but is not used by the editor).
#
# See also the notes in 'flc.sh' for further helpful and
# relevant information regarding the platform, including information
# regarding how to create the necessary components mentioned above.
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

# Switch the terminal into raw mode and disable local echo.
# This is necessary for correct function of the editor.
stty raw -echo

# Start up our local FVM instance (the local 'fvm16-16MB' executable).
# It will run the editor present in its local 'rom.fp' file then shut down
# when the user chooses to exit the editor.
./fvm16-16MB

# Return the terminal to normal mode.
# Otherwise it would behave very strangely.
echo
stty sane
echo

