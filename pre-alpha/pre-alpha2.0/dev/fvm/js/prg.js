var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171205+
  ------------------
  FREE: s2 s5
  LAST SYMBOL: s8
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - s0 is the only forward reference the assembler allows.

  ISSUES:

  +/+ Third space needed (pm, dm, rom) such as for strings or von Neumann
  +/+ Some concept of modules or namespaces is needed
  +/+ Might be best to drop C compatibility
  +/- Might be best to adopt FW32

*/
s1: jump(s0)

s6: fail
s4: ret
// ( n -- ) Print no more than n characters from stdin
s8: cpush s7: in(s4) out(s6) rpt(s7) jump(s4)

// ( -- ) Print 'A' plus no more than 4 characters from stdin
s3: lit(0x41) out(s6) lit(0x4) call(s8) ret

s0: call(s3) halt
`;
