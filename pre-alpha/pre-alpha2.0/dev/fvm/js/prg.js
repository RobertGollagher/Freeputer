var prgSrc = `
(
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170617
  Updated:    20170617-1410+

  This is an experimental program for Freeputer 2 pre-alpha.
  This program is being changed frequently.
  It demonstrates:

    - a simple assembly language for a tiny one-pass assembler ['fvma.js']
    - the only forward reference allowed is the program entry point 0s000000
    - the assembler does not otherwise support forward references at all
    - the assembler allows relative forwards but use them sparingly
    - comment tokens are used by convention to aid understanding
        and could also be used by verification tools in future
    - this approach encourages:
        - human intelligence rather than compiler intelligence
        - modular design rather than monolithic design
        - software reuse [especially of atoms]

  0x = hex, 0f = forward, 0r = reverse, 0s = symbol,
  0s000000 = start, / = token is a comment

  Note: the underlying VM implementation ['fvm2.js'] is very incomplete!
  Only a small subset of opcodes are currently functional.
  This program will change as VM development continues.

)

( STANDARD BOILERPLATE

  The VM begins execution at cell 1 which here is 'jmp 0x000000'
  but the assembler will replace this 0x000000 with the value of 0s000000,
  which symbol is defined at the program entry point further below. )

fal --- jmp 0x000000

( START OF ATOMS )

#def /put1 0s000001 .
  lit 0x000001
  frt ---
  ret ---

#def /put2 0s000002 .
  lit 0x000002
  frt ---
  ret ---

#def /put3 0s000004 .
  lit 0x000003
  frt ---
  ret ---

#def /put4 0s000003 .
  lit 0x000004
  frt ---
  ret ---


( START OF MOLECULES )

#def /put2twice 0s000006 .
  cal /put2 0s000002
  frt ---
  cal /put2 0s000002
  frt ---
  ret ---


( PROGRAM ENTRY POINT )

#def /start 0s000000 .
  cal /put4 0s000003
  frt ---
  cal /put3 0s000004
  frt ---
  cal /put2twice 0s000002
  frt ---
  cal /put1 0s000001
  frt ---
  hal ---

`;
