var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171209+
  ------------------
  FREE: s6
  LAST SYMBOL: s14
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - s0 is the only forward reference the assembler allows.

  ISSUES:

  +/+ Third space needed (pm, dm, rom) such as for strings or von Neumann
  +/+ Some concept of modules or namespaces would be nice
  +/- Possibly consider dropping C compatibility
  +/- Possibly consider adopting FW32

*/
s1: jump(s0) /*main*/


// ( n1 -- n2 ) doIncs
// Increment n1 times to give the number of increments n2.
// This is only to test that the VM is working correctly. 
    s2: cpush lit(0x0) s5: inc rpt(s5) ret


// ( -- 0x100000 ) doManyIncs
// Do 1,048,576 increments to test VM performance.
// Temporarily disable tracing while doing so.
// See browser console for timer output.
    s14: lit(0x100000) troff call(s2) /*doIncs*/ tron ret


    s12: fail
// ( n -- ) send
// Output n to stdout or fail if not possible.
    s13: out(s12) ret


    s11: fail
// ( -- ) sendA
// Output 'A' to stdout
    s10: lit(0x41) call(s13) /*send*/ ret


    s4: fail
// ( n -- ) nInOut
// Output to stdout no more than n characters from stdin.
    s8: cpush s7: in(s4) call(s13) /*send*/ rpt(s7) ret


    s9: fail
// ( -- ) max9InOut
// Output to stdout no more than 9 characters from stdin.
    s3:  lit(0x9) call(s8) ret


// ( -- ) main
s0: call(s3) /*max9InOut*/ halt
`;
