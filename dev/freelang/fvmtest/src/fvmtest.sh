#!/bin/bash
# ============================================================================
# Program:    fvmtest.sh
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150807
# Updated:    20150807:1619
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
# This script runs the fvmtest suite. For further information please
# see 'fvmtest.fl'. Assuming that the directory is which this script
# resides contains the necessary files as described in 'fvmtest.fl' then
# you can run the fvmtest suite simply by:
#
#        ./fvmtest.sh
#
# ============================================================================
reset && touch std.trc && rm std.trc && ./fvm16-16MB-sr-append < std.in.tst && cat std.trc

