# Copyright © 2017, Robert Gollagher.
# SPDX-License-Identifier: GPL-3.0+
#
# Program:    A rudimentary 'Makefile' for 'tvm.c' SRM implementation
# Author :    Robert Gollagher   robert.gollagher@freeputer.net
# Created:    2017
# Updated:    20170729
# Version:    pre-alpha-0.0.0.1
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
# To make on any Linux platform use:
# 
#   make
# 
# Some other options are provided below but are untested
# and, in some cases, incomplete.
#
# ============================================================================
CC=gcc
CPPC=g++
OBJ=fvm
SRCS=fvm.c
DEPS=fvm.c rom.h
all: fvm
fvm: $(DEPS)
	$(CC) -o $(OBJ) $(SRCS)
fvm32: $(DEPS)
	$(CC) -o $(OBJ) $(SRCS) -m32
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

