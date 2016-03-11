# Program:    A rudimentary 'Makefile' for 'fvm.c' FVM implementation
# Copyright © Robert Gollagher 2015
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    20150913
# Updated:    20150913:2230
# Version:    0.1.0.0 alpha for FVM 1.0
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# [at your option] any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details. 
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ============================================================================
#
# To make on any platform use:
# 
#   make
# 
# However, that would result in very poor performance since it uses
# no optimizations whatsoever. Therefore it is better to use one of the below.
#
# To make for best performance on 32-bit x86 use:
# 
#   make x86
# 
# To make for best performance on ARM11 (Raspberry Pi) use:
# 
#   make pi
#
# To make for reasonably good performance on any platform use:
#
#   make any
#
# ============================================================================
all: fvm
fvm: fvm.c
	gcc -o fvm fvm.c
x86: fvm.c
	gcc -march=native -m32 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -o fvm fvm.c
pi: fvm.c
	gcc -march=armv6 -mfpu=vfp -mfloat-abi=hard -O3 -o fvm fvm.c
any: fvm.c
	gcc -O3 -o fvm fvm.c
