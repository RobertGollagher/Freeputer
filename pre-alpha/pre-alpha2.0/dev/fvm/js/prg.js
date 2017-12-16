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
  - This is a demonstration program for FVM2 pre-alpha (see 'fvm1.js').
  - The assembler is very simplistic (see 'fvma.js') and uses little memory.
  - x0 is the only forward reference the assembler allows.
  - x symbols are exported from a module (e.g. x0 in m1 is m1.x0)
  - u symbols are local to a module (u0..uff)
  - s symbols are local to a unit (s0..sff)
  - Units begin with unit and have no explicit end
  - Modules begin with {module and end with end}
  - The C-preprocessor would replace unit with { __label__ s0, s1 ...
  - TODO NEXT Will add u1... for non-exported units
  - See 'fvma.js' for further caveats

  ISSUES:

  +/+ Third space needed (pm, dm, rom) such as for strings or von Neumann
*/

{module /*forward*/ m1
  unit
    jump(x0) /*run*/

end}

{module /*incs*/ m2
  unit
    // ( n1 -- n2 ) doIncs
    // Increment n1 times to give the number of increments n2.
    // This is only to test that the VM is working correctly. 
    x1: cpush lit(0x0) s0: inc rpt(s0) ret

  unit
    // ( -- 0x100000 ) doManyIncs
    // Do 1,048,576 increments to test VM performance.
    // Temporarily disable tracing while doing so.
    // See browser console for timer output.
    x2: lit(0x100000) troff call(m2.x1) /*doIncs*/ tron ret
end}

{module /*io*/ m3
  unit
    s0: fail
    // ( n -- ) send
    // Output n to stdout or fail if not possible.
    u1: out(s0) ret

  unit
    // ( -- ) sendA
    // Output 'A' to stdout FIXME remove m2 self references
    x1: lit(0x41) call(u1) /*send*/ ret

  unit
    s0: fail
    // ( n -- ) nInOut
    // Output to stdout no more than n characters from stdin.
    u2: cpush s1: in(s0) call(u1) /*send*/ rpt(s1) ret

  unit
    // ( -- ) max9InOut
    // Output to stdout no more than 9 characters from stdin.
    x2:  lit(0x9) call(u2) ret

end}

{module /*simpleProgram*/ m0
  unit
    // ( -- n ) run
    x0: lit(0x3) call(m2.x1) /*doIncs*/ call(m3.x1) /*sendA*/ halt

end}

`;
