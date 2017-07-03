var prgSrc = `
(
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170617
  Updated:    20170701:2336+

  This is an experimental program for Freeputer 2 pre-alpha for Plan C.
  This program is being changed frequently.
  It demonstrates:

    - a simple assembly language for a tiny one-pass assembler ['fvma.js']
    - the only forward reference allowed is the program entry point 0s0000
    - the assembler does not otherwise support forward references at all
    - symbols have format 0s0001 and are stored as 0x0001 [lightweight]
    - the assembler allows relative forwards but use them sparingly
    - comment tokens are used by convention to aid understanding
        and could also be used by verification tools in future
    - this approach encourages:
        - human intelligence rather than compiler intelligence
        - modular design rather than monolithic design
        - software reuse

  0x = hex, 0f = forward, 0r = reverse, 0s = symbol,
  0s0000 = start, / = token is a comment

  Note: the underlying VM implementation ['fvm2.js'] is very incomplete!
  Only a small subset of opcodes are currently functional.
  This program will change as VM development continues.

)

( PRIOR TO PROGRAM ENTRY POINT )

  fal -- --

( PROGRAM ENTRY POINT: JUMPS TO /start )

( the assembler will rewrite this jmp to the address of /start )
( jmp r0 0x0000 )

  jmp -- --

( PROGRAM START )

  #def /start 0s0000 .

  hal -- --

`;
