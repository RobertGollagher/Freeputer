# Copyright © 2017, Robert Gollagher.
# SPDX-License-Identifier: GPL-3.0+
#
# Program:    A rudimentary 'Makefile' for 'tvm.c' SRM implementation
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    2017
# Updated:    20180513
# Version:    pre-alpha-0.0.0.4
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
# WARNING: This is pre-alpha software and as such may well be incomplete,
# unstable and unreliable. It is considered to be suitable only for
# experimentation and nothing more.
# 
# =============================================================================
#
# To make 'foo.c' on any Linux platform use:
# 
#   make OBJ=foo
#
# To make on any Linux platform with decent performance use:
# 
#   make good OBJ=foo
# 
# Some other options are provided below but are untested
# and, in some cases, incomplete.
#
# ============================================================================
CC=gcc
CPPC=g++
OBJ?=fvm2
SRCS=$(OBJ).c
DEPS=$(OBJ).c
all: $(OBJ)
tvm: $(DEPS)
	$(CC) -o $(OBJ) $(SRCS)
tvm32: $(DEPS)
	$(CC) -o $(OBJ) $(SRCS) -m32
good: $(DEPS)
	$(CC) -o $(OBJ) $(SRCS) -O3
fast: $(DEPS)
	$(CC) -march=native -mfpmath=sse -Ofast -flto -funroll-loops -o $(OBJ) $(SRCS)
cpp: $(DEPS)
	$(CPPC) -o $(OBJ) $(SRCS)
x86: $(DEPS)
	$(CC) -march=native -m32 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -o $(OBJ) $(SRCS)
x86cpp: $(DEPS)
	$(CPPC) -march=native -m32 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -o $(OBJ) $(SRCS)
pi: $(DEPS)
	$(CC) -march=armv6 -mfpu=vfp -mfloat-abi=hard -O3 -o $(OBJ) $(SRCS)
any: $(DEPS)
	$(CC) -O3 -o $(OBJ) $(SRCS)
anycpp: $(DEPS)
	$(CPPC) -O3 -o $(OBJ) $(SRCS)

