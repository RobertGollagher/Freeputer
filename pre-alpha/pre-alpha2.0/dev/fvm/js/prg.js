var prgSrc = `
/*
  Copyright 2018, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20180515+

  NOTES:

  - This little language is only ad hoc, what matters is the VM design.
  - This is written in an assembly language which aims to be C-compatible.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - This assembler does not yet support the same full syntax seen in 'fpx.m4'.
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
  - TODO Consider C-like call syntax as no forward refs...
  - See 'fvma.js' for further caveats

*/

// ---------------------------------------------------------------------------
as(m1)
module
  jump(m0.x0) /*main.run*/
endm

// ---------------------------------------------------------------------------
as(m0) /*main*/
module
  unit
    pri(s0):
      fail
    PUB(x0):
      tron
      i(0x41)
      out(s0)
      halt
  endu
endm

`;
