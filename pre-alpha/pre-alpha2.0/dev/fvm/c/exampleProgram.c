/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.fp2
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180511+
Version:    pre-alpha-0.0.0.13+ for FVM 2.0

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

Template for modules:

// ---------------------------------------------------------------------------
  as()
  module()
    unit
      PUB(,):
        done
    endun
  endmod

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
// m4: // Copyright © 2018, Robert Gollagher.
// SPDX-License-Identifier: GPL-3.0+
//
// Program:    fpx.m4
// Author :    Robert Gollagher   robert.gollagher@freeputer.net
// Created:    20180503
// Updated:    20180510+
// Version:    pre-alpha-0.0.0.12+ for FVM 2.0
//
// FIXME macro definitions are conflicting with comments, e.g. As
//
// m4: 
// m4: 
// m4: 
// m4:  #x0...
// m4:  #u0...
// m4:  #s0...
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 
// m4: 

tron
jjump(m0_x0)/*main.run*/

// ---------------------------------------------------------------------------
   /*m1*/
  module(st)
  // st: Stack Operations.
    atom
      m1_x1/*nip*/:
      // (n1 n2)(n1)
        swap
        drop
        done
    enda
    atom
      m1_x2/*drop2*/:
      // (n1 n2)()
        drop
        drop
        done
    enda
  #include "endmod.c"
// ---------------------------------------------------------------------------
   /*m3*/
  module(co)
  // co: Constants.
    atom
      m3_x1/*nan*/:
        // ()(nan)
        i(0x0)
        flip
        done
    enda
    atom
      m3_x2/*true*/:
        // ()(0x1)
        i(0x1)
        done
    enda
    atom
      m3_x3/*false*/:
        // ()(0x0)
        i(0x0)
        done
    enda
  #include "endmod.c"

// ---------------------------------------------------------------------------
   /*m2*/
  #define z1(xn) m3_ ## xn /*co*/
  module(ch)
  // ch: Character Operations.
  //  These act on whole words.
  //  These treat NaN As a character.
    atom
      m2_x1/*eq*/:
        // (n1 n2)(bool) equality
        jjmpe(z1(x2))/*true*/
        jjump(z1(x3))/*false*/
    enda
  #include "endmod.c"

// ---------------------------------------------------------------------------
   /*m0*/
  #define z1(xn) m1_ ## xn /*st*/
  #define z2(xn) m2_ ## xn /*ch*/
  #define z3(xn) m3_ ## xn /*co*/
  module(main)
    unit
      m0_x0/*run*/:
        d(z3(x1))/*nan*/
        d(z3(x1))/*nan*/
        d(z2(x1))/*eq*/
        halt
    endu
  #include "endmod.c"
