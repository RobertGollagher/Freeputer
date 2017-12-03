var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171203+
  ------------------
  FREE: 
  LAST SYMBOL: s0008
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - s0000 is the only forward reference the assembler allows.

*/
s0001:
  jump(s0000)

s0002:
  halt

s0006:
  fail

s0004:
  jump(s0002)
// ( n -- ) Print no more than n characters from stdin then halt.
s0008:
  cpush
  s0007:
    in(s0004)
    out(s0006)
    rpt(s0007)
  jump(s0004)

s0003:
  // Print 'A'
  lit(0x41) out(s0006)
  // Print no more than 4 characters from stdin then halt.
  lit(0x4) jump(s0008)

s0000:
  jump(s0003)
`;
