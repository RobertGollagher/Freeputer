var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170923+
  ------------------
  LAST SYMBOL: s0004
  ------------------
*/
#define /*X*/ s0002 0x03
#define /*Y*/ s0003 0x05
#define /*REPEATS*/ s0004 0x02

i(s0002) /*X*/ add i(s0003) /*Y*/ add

i(s0004) /*REPEATS*/ fromb tor nop

s0001: /*loop:*/
  nop rpt(s0001) /*loop*/ xor halt



  /* FIXME The above rpt(s0001) is not being assembled correctly. */

`;
