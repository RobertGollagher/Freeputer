var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as 'prg.c')
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171216+
  ------------------
  FREE: 
  LAST SYMBOL: g6
  ------------------

  NOTES:

  - This is written in a C-compatible assembly language.
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm2.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - g0 is the only forward reference the assembler allows.
  - g symbols are global.
  - s symbols are local to a unit.
  - Units begin with {unit and end with }
  - The C-preprocessor would replace {unit with { __label__ s0, s1 ...
  - Currently adding {mod and m1...
  - Will add u1...

  ISSUES:

  +/+ Third space needed (pm, dm, rom) such as for strings or von Neumann
  +/+ Module system needed (add m sybols, u symbols)

*/
{mod /*forward*/ m2
  // imports m0.g0 /*simpleProgram.run*/ as g0
  {unit
    jump(g0) /*run*/
  }
}

{mod /*incs*/ m3
  {unit
    // ( n1 -- n2 ) doIncs
    // Increment n1 times to give the number of increments n2.
    // This is only to test that the VM is working correctly. 
    g1: cpush lit(0x0) s0: inc rpt(s0) ret }

  {unit
    // ( -- 0x100000 ) doManyIncs
    // Do 1,048,576 increments to test VM performance.
    // Temporarily disable tracing while doing so.
    // See browser console for timer output.
    g2: lit(0x100000) troff call(m3.g1) /*doIncs*/ tron ret }
}

{mod /*io*/ m4
  {unit
    s0: fail
    // ( n -- ) send
    // Output n to stdout or fail if not possible.
    g3: out(s0) ret
  }

  {unit
    // ( -- ) sendA
    // Output 'A' to stdout FIXME remove m3 self references
    g5: lit(0x41) call(m4.g3) /*send*/ ret
  }

  {unit
    s0: fail
    // ( n -- ) nInOut
    // Output to stdout no more than n characters from stdin.
    g6: cpush s1: in(s0) call(m4.g3) /*send*/ rpt(s1) ret
  }

  {unit
    // ( -- ) max9InOut
    // Output to stdout no more than 9 characters from stdin.
    g4:  lit(0x9) call(m4.g6) ret
  }
}

{mod /*simpleProgram*/ m1
  {unit
    // ( -- n ) run
    g99: lit(0x3) call(m3.g1) /*doIncs*/ call(m4.g5) /*sendA*/ halt
  }
}

g0: lit(0x3) call(m3.g1) /*doIncs*/ call(m4.g5) /*sendA*/ halt

`;
