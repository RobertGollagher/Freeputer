/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.m4
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180503++
Version:    pre-alpha-0.0.0.2+ for FVM 2.0

This is an example program using the 'fvm2.c' virtual machine definition.

To build it do:

  m4 exampleProgram.m4 > exampleProgram.c

The resulting 'exampleProgram.c' should never be modified by hand.
It is currently included in 'fvm2.c' which you can then build by:

  make OBJ=fvm2  

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#define ulabels u0,u1,u2,u3;
#define slabels s0,s1,s2,s3;
// m4: define(`CONCAT',`$1$2$3')
// m4: define(`as',`define(`thismn',`$1')')
// m4: define(`export',CONCAT(thismn(),_,$1):)
#define module(name) { __label__ ulabels /*name is ignored*/
#define unit { __label__ slabels
#define endmod ; }
#define endun ; }
/* =========================================================================*/

jump(m0_x0) /*run.main*/

// ---------------------------------------------------------------------------
  as(m3)
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
  as(m1)
  //#define z1(xn) m3 ## _ ## xn /*uses(prn)*/
  module(foo)
    unit
      export(x0) /*prnIdent*/
        i(0x31)
        call(z1(x0)) /*prn.modName*/
        ret
    endun
  endmod

// ---------------------------------------------------------------------------
  as(m2)
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

  as(m0)
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
