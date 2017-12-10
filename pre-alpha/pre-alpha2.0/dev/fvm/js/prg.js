var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171210+
  ------------------
  FREE: 
  LAST SYMBOL: g6
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - s0 is the only forward reference the assembler allows.

  ISSUES:

  +/+ Third space needed (pm, dm, rom) such as for strings or von Neumann
  +/+ Some concept of modules or namespaces would be nice:
        trying this now with s (local) vs g (global) symbols experiment

*/
jump(g0) /*main*/


// ( n1 -- n2 ) doIncs
// Increment n1 times to give the number of increments n2.
// This is only to test that the VM is working correctly. 
    g1: cpush lit(0x0) s1: inc rpt(s1) ret


// ( -- 0x100000 ) doManyIncs
// Do 1,048,576 increments to test VM performance.
// Temporarily disable tracing while doing so.
// See browser console for timer output.
    g2: lit(0x100000) troff call(g1) /*doIncs*/ tron ret


    s1: fail
// ( n -- ) send
// Output n to stdout or fail if not possible.
    g3: out(s1) ret


// ( -- ) sendA
// Output 'A' to stdout
    g5: lit(0x41) call(g3) /*send*/ ret


    s1: fail
// ( n -- ) nInOut
// Output to stdout no more than n characters from stdin.
    g6: cpush s2: in(s1) call(g3) /*send*/ rpt(s2) ret


// ( -- ) max9InOut
// Output to stdout no more than 9 characters from stdin.
    g4:  lit(0x9) call(g6) ret


// ( -- ) main
g0: lit(0x3) call(g1) /*doIncs*/ halt
`;
