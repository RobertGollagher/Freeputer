/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.fp2
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180508+
Version:    pre-alpha-0.0.0.8+ for FVM 2.0

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
// m4:  #x0...
// m4:  #u0...
// m4: 
// m4: 
// m4: 
/* =========================================================================*/
jump(m0_x0) /*run.main*/

// ---------------------------------------------------------------------------
  
  module(math)
    atom
      m4_x1/*add*/:
        add
        done
    endat
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  module(prn)
    unit
      m3_x0/*modName*/:
      u0/*modName*/:
        i(0x6d)
        out
        out
        i(0x0a)
        out
        done
    endun
    unit
      m3_x1/*prnIdent*/:
        i(0x33)
        doo(u0)/*modName*/
        done
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  #define z1(xn) m3_ ## xn /*prn*/
  module(foo)
    unit
      m1_x0/*prnIdent*/:
        i(0x31)
        doo(z1(x0))/*prn.modName*/
        done
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------
  
  #define z1(xn) m3_ ## xn /*prn*/
  module(bar)
    unit
      m2_x0/*prnIdent*/:
        i(0x32)
        doo(z1(x0))/*prn.modName*/
        done
    endun
  #include "endmod.c"

// ---------------------------------------------------------------------------

  
  module(run)
    unit
      m0_x0/*main*/:
        tron

        i(0x41424344)
        i(0)
        hold

        i(0x45464748)
        i(1)
        hold

        i(0)
        give
        outw

        i(1)
        give
        outw

        halt
    endun
  #include "endmod.c"
