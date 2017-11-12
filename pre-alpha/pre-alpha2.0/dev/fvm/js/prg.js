var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171112+
  ------------------
  FREE:
  LAST SYMBOL: s0005
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM 2.0 (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - s0000 is the only forward reference the assembler allows.

*/
s0001: /*forward*/
  jump(s0000) /*start*/

s0002: /*halt*/
  halt

s0004: /*recover*/
  0x10 jump(s0002) /*halt*/

s0003: /*go*/
  0x1 0x2 0x3
  0x3 cpush
  s0005: /*droploop*/
    drop catch(s0004) /*recover*/
    rpt(s0005) /*droploop*/
  jump(s0002) /*halt*/

s0000: /*start*/
  jump(s0003) /*go*/
`;
