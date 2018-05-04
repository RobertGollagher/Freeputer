/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.fp2
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180504+
Version:    pre-alpha-0.0.0.6+ for FVM 2.0

This is an example program using the 'fvm2.c' virtual machine definition.

This source is preprocessed with sed followed by m4 to produce the C source.

Thus to build it do:

  sed -r 's/([z][0-9a-f]+)\.([x][0-9a-f]+)/\1(\2)/g' \
  <exampleProgram.fp2 >exampleProgram.m4 \
  && m4 -d exampleProgram.m4 > exampleProgram.c

Or simply:

  ./build.sh exampleProgram

The resulting 'exampleProgram.c' should never be modified by hand.
It is currently included in 'fvm2.c' which you can then build and run by:

  make good OBJ=fvm2
  time ./fvm2; echo $?

Or to build and run simply:

  ./go.sh exampleProgram

If you wish to examine the output of the C preprocessor you can do:

  gcc -E fvm2.c

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
/* =========================================================================*/

jump(m0_x0) /*run.main*/

// ---------------------------------------------------------------------------
  
  module(math)
    atom(add)
      m4_x1:
        add
        ret
    endat
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  module(prn)
    unit(modName)
      m3_x0: u0:
        i(0x6d)
        out
        out
        i(0x0a)
        out
        ret
    endun
    unit(prnIdent)
      m3_x1:
        i(0x33)
        call(u0) /*modName*/
        ret
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  #define z1(xn) m3_ ## xn /*prn*/
  module(foo)
    unit(prnIdent)
      m1_x0:
        i(0x31)
        call(z1(x0)) /*prn.modName*/
        ret
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  #define z1(xn) m3_ ## xn /*prn*/
  module(bar)
    unit(prnIdent)
      m2_x0:
        i(0x32)
        call(z1(x0)) /*prn.modName*/
        ret
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------

  
  #define z1(xn) m1_ ## xn /*foo*/
  #define z2(xn) m2_ ## xn /*bar*/
  #define z3(xn) m3_ ## xn /*prn*/
  #define z4(xn) m4_ ## xn /*math*/
  module(run)
    unit(main)
      m0_x0:
        call(z1(x0)) /*foo.prnIdent*/ // Should print "m1\n"
        call(z2(x0)) /*bar.prnIdent*/ // Should print "m2\n"
        call(z3(x1)) /*prn.prnIdent*/ // Should print "m3\n"

        i(0x40)
        i(1)
        call(z4(x1)) /*math.add*/
        out
        i(0xa)
        out // Should print "A\n"

        tron

        i(0x41424344)
        i(0)
        hold
        i(0)
        give

        outw
        i(0xa)
        out // Should print "DCBA\n" (note: all is little endian, for now)

        inw // If we read "1234\n"...
        outw
        i(0xa)
        out // ...then this should print "1234\n"

        halt
    endun
  #include "endmod.c"
