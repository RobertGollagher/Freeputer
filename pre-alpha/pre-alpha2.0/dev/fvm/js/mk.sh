#!/bin/bash
# ============================================================================
# Program:    mk.sh
# Copyright © Robert Gollagher 2017
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150713
# Updated:    20170513-1704
# Version:    pre-alpha-0.0.0.0 for FVM 2.0
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
# Run this script to make a file called 'prgBase64.js' which declares
# a JavaScript variable named prgBase64 and defines its value as a base64
# string representing a compiled Freelang program.
#
# Specify the Freelang source file to be compiled:
#
#   ./mk.sh source.fl
#
# For this to work, 'flx.rb' (or a symbolic link to it) must exist
# locally. The usual information produced by 'flx.rb' will also be produced,
# namely the files 'debug.flx' and 'map.info'.
#
# The generated 'prgBase64.js' can be used by 'fvmui.html'.
#
# ============================================================================
if [ -z "$1" ] ; then
    echo "Must specify Freelang source to be compiled, such as: ./mk.sh source.fl"
    exit 1
fi
./flx.rb $1 rom.fp &&
echo "// Base64-encoded Freeputer program bytecode" > prgBase64.js &&
echo -n "var prgBase64 = \"" >> prgBase64.js &&
base64 -w 0 rom.fp >> prgBase64.js &&
echo "\"" >> prgBase64.js

