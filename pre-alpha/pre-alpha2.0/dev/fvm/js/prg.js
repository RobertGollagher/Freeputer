var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170923+
  ------------------
  LAST SYMBOL: s0003
  ------------------
*/

#define /*FOO*/ s0003 0xff

jump(s0000) /*start:*/

s0001: /*go:*/

s0002: /*end:*/
  i(0) halt

s0000: /*start:*/
  jump(s0001) /*go:*/

`;
