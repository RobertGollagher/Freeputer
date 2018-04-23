var prgSrc = `
/*
  Copyright 2018, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20180423+

  NOTES:

  - This little language is only ad hoc, what matters is the VM design.
  - This is written in an assembly language which aims to be C-compatible.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - This assembler is very simplistic (see 'fvma.js') and uses little memory.
  - m0.x0 is the only forward reference the assembler allows
  - x symbols are exported from a module (e.g. x0 in m1 is m1.x0)
  - u symbols are local to a module (u0..uff)
  - s symbols are local to a unit (s0..sff)
  - Units begin with u{ and end with }u
  - Modules begin with m{ and end with }m
  - Modules are named by mod(m1)
  - The C-preprocessor would replace u{ with { __label__ s0, s1 ...
  - TODO Enforce use of u{ keyword prior to any locals
  - See 'fvma.js' for further caveats

*/

// ---------------------------------------------------------------------------

m{ mod(m1) /*MODULE:forward*/ jump(m0.x0) /*run*/ }m

m{ mod(m2) /*MODULE:incs*/

  // ( n1 -- n2 ) doIncs
  // Increment n1 times to give the number of increments n2.
  // This is only to test that the VM is working correctly. 
  u{ x1: cpush lit(0x0) s0: inc rpt(s0) ret }u

  // ( -- 0x100000 ) doManyIncs
  // Do 1,048,576 increments to test VM performance.
  // Temporarily disable tracing while doing so.
  // See browser console for timer output.
  u{ x2: lit(0x100000) troff call(x1) /*doIncs*/ tron ret }u

}m

m{ mod(m3) /*MODULE:io*/

  // ( n -- ) send
  // Output n to stdout or fail if not possible.
  u{ s0: fail x1: out(s0) ret }u

  // ( -- ) sendA
  // Output 'A' to stdout
  u{ x2: lit(0x41) call(x1) /*send*/ ret }u

  // ( n -- ) nInOut
  // Output to stdout no more than n characters from stdin.
  u{ s0: fail u1: cpush s1: in(s0) call(x1) /*send*/ rpt(s1) ret }u

  // ( -- ) max9InOut
  // Output to stdout no more than 9 characters from stdin.
  u{ x3: lit(0x9) call(u1) /*nInOut*/ ret }u

}m

// ---------------------------------------------------------------------------

m{ mod(m0) /*run*/

  u{
    x0: 

      // DEMONSTRATING THE USE OF STDHOLD:
      // =================================
      // You can comment the next 5 lines out after running this once,
      // and if your browser (Firefox or Chrome) supports local storage
      // the values will still be correctly retrieved below.
      // See browser settings advice on fvmui.html.

      // Store A,B,C,D,E characters to the first 5 words of stdhold:

      lit(0x41) lit(0x0) hold
      lit(0x42) lit(0x1) hold
      lit(0x43) lit(0x2) hold
      lit(0x44) lit(0x3) hold
      lit(0x45) lit(0x4) hold // hold is now A,B,C,D,E

      // Load the first 5 words of stdhold:

      lit(0x4) give
      lit(0x3) give
      lit(0x2) give
      lit(0x1) give
      lit(0x0) give // data stack now A,B,C,D,E

      // Print these 5 elements:

      lit(0x5) cpush s0: call(m3.x1) /*io.send*/ rpt(s0)

      // Design note: maybe it would be easier just to trap and use catch
      // rather than branching on failure? Has several advantages for
      // simplicity although rather drastic.

      halt
  }u

}m

`;
