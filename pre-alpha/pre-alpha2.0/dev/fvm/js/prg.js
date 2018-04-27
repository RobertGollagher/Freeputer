var prgSrc = `
/*
  Copyright 2018, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20180428+

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
  - TODO Consider if I/O trap rather than branch on failure could be viable
  - TODO Consider flag- or stack-based strategy rather than traps
  - See 'fvma.js' for further caveats

*/

// ---------------------------------------------------------------------------

m{ mod(m1) /*MODULE:forward*/ jump(m0.x0) /*run*/ }m

m{ mod(m2) /*MODULE:incs*/

  // ( n1 -- n2 ) doIncs
  // Increment n1 times to give the number of increments n2.
  // This is only to test that the VM is working correctly. 
  u{ x1: cpush i(0x0) s0: inc rpt(s0) ret }u

  // ( -- 0x100000 ) doManyIncs
  // Do 1,048,576 increments to test VM performance.
  // Temporarily disable tracing while doing so.
  // See browser console for timer output.
  u{ x2: i(0x100000) troff call(x1) /*doIncs*/ tron ret }u

}m

m{ mod(m3) /*MODULE:io*/

  // ( n -- ) send
  // Output n to stdout or fail if not possible.
  u{ s0: fail x1: out(s0) ret }u

  // ( -- ) sendA
  // Output 'A' to stdout
  u{ x2: i(0x41) call(x1) /*send*/ ret }u

  // ( n -- ) nInOut
  // Output to stdout no more than n characters from stdin.
  u{ s0: fail u1: cpush s1: in(s0) call(x1) /*send*/ rpt(s1) ret }u

  // ( -- ) max9InOut
  // Output to stdout no more than 9 characters from stdin.
  u{ x3: i(0x9) call(u1) /*nInOut*/ ret }u

  // ( n -- ) nInOutFast
  // Output to stdout no more than n characters from stdin, quickly.
  u{ s0: fail x4: cpush s1: in(s0) out(s0) rpt(s1) ret }u

  // ( n -- ) inOutAll
  // Output to stdout all available characters from stdin, then return.
  u{ s0: ret s1: fail x5: in(s0) out(s1) jump(x5) ret }u

}m

// ---------------------------------------------------------------------------

m{ mod(m0) /*run*/

  u{
    s0:
      fail
    x0:
      tron
      i(0x48) tpush
      i(0x47) tpush
      i(0x46) tpush
      i(0x45) tpush
      i(0x44) tpush
      i(0x43) tpush
      i(0x42) tpush
      i(0x41) tpush

      i(0x1) tpeek out(s0)
      i(0x2) tpeek out(s0)
      i(0x3) tpeek out(s0)
      i(0x4) tpeek out(s0)
      i(0x5) tpeek out(s0)
      i(0x6) tpeek out(s0)
      i(0x7) tpeek out(s0)
      i(0x8) tpeek out(s0)

      i(0x49) i(0x1) tpoke
      i(0x4a) i(0x2) tpoke
      i(0x4b) i(0x3) tpoke
      i(0x4c) i(0x4) tpoke
      i(0x4d) i(0x5) tpoke
      i(0x4e) i(0x6) tpoke
      i(0x4f) i(0x7) tpoke
      i(0x50) i(0x8) tpoke

      i(0x1) tpeek out(s0)
      i(0x2) tpeek out(s0)
      i(0x3) tpeek out(s0)
      i(0x4) tpeek out(s0)
      i(0x5) tpeek out(s0)
      i(0x6) tpeek out(s0)
      i(0x7) tpeek out(s0)
      i(0x8) tpeek out(s0)

      halt
  }u

}m

`;
