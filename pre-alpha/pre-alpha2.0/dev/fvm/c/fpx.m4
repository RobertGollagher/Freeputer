// Copyright © 2018, Robert Gollagher.
// SPDX-License-Identifier: GPL-3.0+
// 
// Program:    fpx.m4
// Author :    Robert Gollagher   robert.gollagher@freeputer.net
// Created:    20180503
// Updated:    20180509+
// Version:    pre-alpha-0.0.0.11+ for FVM 2.0
//
// m4: define(`CONCAT',`$1$2$3')
// m4: define(`as',`define(`thismn',`$1')')
// m4: define(`pub',CONCAT(thismn(),_,$1)/*$2*/) #x0...
// m4: define(`loc',$1/*$2*/) #u0...
// m4: define(`use',`#define $1(xn) $2_ #// xn /*$3*/')
// m4: define(`endmod',`#include "endmod.c"')
// m4: define(`do',`d($1)/*$2*/')
// m4: define(`launch',`jump(m0_x0)/*main.run*/')

