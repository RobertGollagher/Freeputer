var prgSrc = `
(
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170617
  Updated:    20170622-0719+

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
        - software reuse [especially of atoms]

  0x = hex, 0f = forward, 0r = reverse, 0s = symbol,
  0s0000 = start, / = token is a comment

  Note: the underlying VM implementation ['fvm2.js'] is very incomplete!
  Only a small subset of opcodes are currently functional.
  This program will change as VM development continues.

)

( STANDARD BOILERPLATE

  The VM begins execution at cell 1 which here is 'jmp 0x000000'
  but the assembler will replace this 0x000000 with the value of 0s0000,
  which symbol is defined at the program entry point further below. )
fal --- jmp 0x00000

( DATA SPACE )

  #def /v5 0s0005 0x000100
  #def /v6 0s0006 0x000101

( SOME RANDOM PADDING )

  nop ---
  nop ---
  nop ---

( PSEUDO FUNCTIONS )

  #def /f1ret 0s0007 .
    fal ---
    #def /f1 0s0008 .
    dst@ 0s0007 ( FIXME )
    jst ---

( PROGRAM ENTRY POINT )

#def /start 0s0000 .

  ( returning is not so simple here )
  ( tentatively I think we do not have a solution here, Plan B is better )

  jmp /f1 0s0008

(
  res 0x222222
  dst 0s0005
  sav ---
  res 0x000010
  src@ 0s0005
  lod ---
  lit 0x010000
  add ---
)
  #def /end 0s0009 .
  hal ---

`;
