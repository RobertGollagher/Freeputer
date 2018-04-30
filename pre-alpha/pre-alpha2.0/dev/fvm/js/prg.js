var prgSrc = `
/*
  Copyright 2018, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20180430+

  NOTES:

  - Currently changing for QMISC experiment
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
  - TODO Consider if I/O trap rather than branch on failure could be viable
  - TODO Consider flag- or stack-based strategy rather than traps
  - See 'fvma.js' for further caveats

*/

// ---------------------------------------------------------------------------

m{ mod(m1) /*MODULE:forward*/ jump(m0.x0) /*run*/ }m

m{ mod(m4) /*MODULE:sk*/ // Stack synthesis from QMISC instructions

  // ( -- ) init
  u{ x1: i(0x200) fromb i(0x200) /*dsp*/ put ret }u

  // ( n -- ) push
  u{ x2: i(0x200) /*dsp*/ decm puti ret }u

  // ( -- n ) pop
  u{ x3: i(0x200) /*dsp*/ geti incm ret }u

}m

// ---------------------------------------------------------------------------

m{ mod(m0) /*run*/

  u{
    s0:
      fail
    x0:
      call(m4.x1) /*sk.init*/
      
      /* Using push, pop (3) */     
        // i(0x12345678) fromb
        // i(0x200) push pop

      /* Calling push, pop (10) */
        // i(0x12345678) fromb
        // call(m4.x2) /*sk.push*/
        // call(m4.x3) /*sk.pop*/

      /* Inlining (6) */
        i(0x12345678) fromb
      tron
        i(0x200) decm puti
        i(0x77777777) fromb
        i(0x200) geti incm
      halt
  }u

}m

`;
