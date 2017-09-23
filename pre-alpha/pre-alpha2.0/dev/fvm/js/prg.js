var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170923+
  ------------------
  LAST SYMBOL: s0006
  ------------------
*/
#define /*X*/ s0002 0x03
#define /*Y*/ s0003 0x05
#define /*REPEATS*/ s0004 0x02

jump(s0000) /*start:*/

s0006: /*foo:*/
  i(s0002) /*X*/ add 
  i(s0003) /*Y*/ add
  i(s0004) /*REPEATS*/ fromb tor

s0001: /*loop:*/
  rpt(s0001) /*loop*/

s0005: /*end:*/
  i(0) halt

s0000: /*start:*/
  jump(s0006) /*foo:*/

`;
