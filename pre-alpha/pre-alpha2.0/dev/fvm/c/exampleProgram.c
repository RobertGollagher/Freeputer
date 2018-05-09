/*
Copyright © 2018, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    exampleProgram.fp2
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20180503
Updated:    20180509+
Version:    pre-alpha-0.0.0.11+ for FVM 2.0

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
// m4: // Copyright © 2018, Robert Gollagher.
// SPDX-License-Identifier: GPL-3.0+
// 
// Program:    fpx.m4
// Author :    Robert Gollagher   robert.gollagher@freeputer.net
// Created:    20180503
// Updated:    20180509+
// Version:    pre-alpha-0.0.0.11+ for FVM 2.0
//
// m4: 
// m4: 
// m4:  #x0...
// m4:  #u0...
// m4: 
// m4: 
// m4: 
// m4: 



jump(m0_x0)/*main.run*/

// ---------------------------------------------------------------------------
  
  module(main)
    unit
      m0_x0/*run*/:
        tron
        noop
        halt
    endun
  #include "endmod.c"
