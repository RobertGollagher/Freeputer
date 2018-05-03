/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180503++
Version:    pre-alpha-0.0.0.1+ for FVM 2.0

This is an example program using the 'fvm2.c' virtual machine definition.

Currently it is included and compiled by 'fvm2.c'.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/

jump(m0_x0) /*run.main*/

// ---------------------------------------------------------------------------
  #define export(xn) m3 ## _ ## xn: /*as(m3)*/
  module(prn)
    unit
      export(x0) /*modName*/
      u0:
        i(0x6d)
        out
        out
        i(0x0a)
        out
        ret
    endun
    unit
      export(x1) /*prnIdent*/
        i(0x33)
        call(u0) /*modName*/
        ret
    endun
  endmod

// ---------------------------------------------------------------------------
  #define export(xn) m1 ## _ ## xn: /*as(m1)*/
  #define z1(xn) m3 ## _ ## xn /*uses(prn)*/
  module(foo)
    unit
      export(x0) /*prnIdent*/
        i(0x31)
        call(z1(x0)) /*prn.modName*/
        ret
    endun
  endmod

// ---------------------------------------------------------------------------
  #define export(xn) m2 ## _ ## xn: /*as(m2)*/
  #define z1(xn) m3 ## _ ## xn /*uses(prn)*/
  module(bar)
    unit
      export(x0) /*prnIdent*/
        i(0x32)
        call(z1(x0)) /*prn.modName*/
        ret
    endun
  endmod

// ---------------------------------------------------------------------------

  #define export(xn) m0 ## _ ## xn: /*as(m0)*/
  #define z1(xn) m1 ## _ ## xn /*uses(foo)*/
  #define z2(xn) m2 ## _ ## xn /*uses(bar)*/
  #define z3(xn) m3 ## _ ## xn /*uses(prn)*/
  module(run)
    unit
      export(x0) /*main*/
        call(z1(x0)) /*foo.prnIdent*/
        call(z2(x0)) /*bar.prnIdent*/
        call(z3(x1)) /*prn.prnIdent*/
        halt
    endun
  endmod
