#!/bin/bash
# ============================================================================
# Copyright © 2017, Robert Gollagher.
# SPDX-License-Identifier: GPL-3.0+
#
# Program:    mk.sh
# Copyright © Robert Gollagher 2017
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150713
# Updated:    20170513-1710
# Version:    pre-alpha-0.0.0.2 for FVM 2.0
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
echo "\";" >> prgBase64.js

