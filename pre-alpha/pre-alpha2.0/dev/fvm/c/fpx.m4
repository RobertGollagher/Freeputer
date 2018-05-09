// Copyright © 2018, Robert Gollagher.
// SPDX-License-Identifier: GPL-3.0+
//
// Program:    fpx.m4
// Author :    Robert Gollagher   robert.gollagher@freeputer.net
// Created:    20180503
// Updated:    20180509+
// Version:    pre-alpha-0.0.0.11+ for FVM 2.0
//
// FIXME macro definitions are conflicting with comments, e.g. As
//
// m4: define(`CONCAT',`$1$2$3')
// m4: define(`as',`define(`thismn',`$1') /*thismn()*/')
// m4: define(`myname',/*thismn()*/)
// m4: define(`pub',CONCAT(thismn(),_,$1)/*$2*/) #x0...
// m4: define(`loc',$1/*$2*/) #u0...
// m4: define(`pri',$1/*$2*/) #s0...
// m4: define(`use',`#define $1(xn) $2_ ## xn /*$3*/')
// m4: define(`endmod',`#include "endmod.c"')
// m4: define(`do',`d($1)/*$2*/')
// m4: define(`jump',`jjump($1)/*$2*/')
// m4: define(`jnan',`jjnan($1)/*$2*/')
// m4: define(`jann',`jjann($1)/*$2*/')
// m4: define(`jorn',`jjorn($1)/*$2*/')
// m4: define(`jnne',`jjnne($1)/*$2*/')
// m4: define(`launch',`jjump(m0_x0)/*main.run*/')
