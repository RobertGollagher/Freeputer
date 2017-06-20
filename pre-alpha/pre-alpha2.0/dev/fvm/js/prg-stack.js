var prgSrc = `
(
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170617
  Updated:    20170619-1929+

  This is an experimental program for Freeputer 2 pre-alpha.
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

#def /RAMa 0s0001 0x100000 ( TODO remove this hard-coding )
#def /RAM00 0s0002 /RAMa 0s0001
#def /RAM01 0s0003 0x100001
#def /RAM02 0s0004 0x100002

( STANDARD BOILERPLATE

  The VM begins execution at cell 1 which here is 'jmp 0x000000'
  but the assembler will replace this 0x000000 with the value of 0s0000,
  which symbol is defined at the program entry point further below. )

fal --- jmp 0x00000

( START OF ATOMS )




( START OF MOLECULES )




( PROGRAM ENTRY POINT )

#def /start 0s0000 .

( Plan C experiments: )

  xD /RAM00 0s0002 ( xD = Destination or Accumulator address )
  xS /RAM01 0s0003 ( xS = Source or Operand address )
  xD! 0x000003
  xS! 0x000008
  x+ ---
  hal ---

(
  lit 0x000010
  fal ---
  lit /RAMa 0s0001
  fal ---
  sto ---
  lit /RAMa 0s0001
  fal ---
  ldo ---
  jmp 0f04
  lit 0x000003
  fal ---
  jmp 0f02
  jmp 0r03
  lit 0x000005
  fal ---
  lit 0x000008
  fal ---
  hal ---
)
`;
