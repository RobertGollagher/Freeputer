var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20171104+
  ------------------
  FREE:
  LAST SYMBOL: s000a
  ------------------

  FIXME Hit a roadblock: it appears a stack is necessary for practicality,
  rather than just relying on br/link alone; but if a stack is introduced
  then it probably makes more sense to have a stack machine instead
  of a register machine, so Plan A not Plan G would win.
*/


// Constants -----------------------------------------------------------------



// The sole forward reference ------------------------------------------------
jump(s0000) /*start*/

// Success -------------------------------------------------------------------

s0002: /*end:*/ b(0x00) halt


// Text Output Utilities -----------------------------------------------------
s0004: /*print_newline*/ a(0x0a) out link
// ---------------------------------------------------------------------------
// Print the uppercase alphabet ----------------------------------------------
s0006: /*print_alphabet*/
    a(0x41) b(0x01) r(0x19)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    link
// ---------------------------------------------------------------------------
// Hexadecimal print LSN -----------------------------------------------------
s0003: /*alpha*/ b(0x57) /*'a'*/ add out fromt link 
s0007: /*print_hex_nibble*/
  tot
  b(0x0fffffff) xor b(0x1c) shr
  b(0x09) jmpg(s0003) /*alpha*/
  b(0x30) /*'0'*/ add out fromt ret /*link*/ // Changing from link to ret
// ---------------------------------------------------------------------------



// FIXME stuck for 2 reasons: js xor not working for FLIP,
// and no way of returning as cannot do >1 level of link

s0008: /*print_hex_word*/
  r(0x07)
  s0009: /*loop_print_hex*/
    br(s0007) /*print_hex_nibble*/
    b(0x04) shl // FIXME disallow decimal immediates
    rpt(s0009) /*print_hex_word*/
    // FIXME no way of returning
    halt

// Secondary entry point -----------------------------------------------------

s0001: /*go:*/
  a(0x00000000) br(s0007) /*print_hex_nibble*/
  a(0x90000000) br(s0007) /*print_hex_nibble*/
  a(0xa0000000) br(s0007) /*print_hex_nibble*/
  a(0xf0000000) br(s0007) /*print_hex_nibble*/
  br(s0004) /*print_newline*/
  a(0x7654321a) br(s0008) /*print_hex_word*/ // FIXME cannot return
  br(s0004) /*print_newline*/
  br(s0006) /*print_alphabet*/
  jump(s0002) /*end*/

// Primary entry point -------------------------------------------------------

s000a: /*halting:*/ halt
s0000: /*start:*/



  a(0x300) /*zs*/ // Just some arbitrary stack
  b(0x200) /*zsp*/
  put

/*
  get
  b(0x02ff)
  jmpe(s000a) /*halting:*/
  // b(0x200) /*zsp*/
*/

  get
  a(0x11111111)
dpush
  push

  get
  a(0x22222222)
dpush
  push

  get
  a(0x33333333)
dpush
  push

  get
  a(0x44444444)
dpush
  push

  a(0x00)
  get
  pop

  get
  pop

  get
  pop

  get
  pop

  call(s0007) /*print_hex_nibble*/

dpop
dpop
dpop
dpop

  halt

  jump(s0001) /*go*/


// ---------------------------------------------------------------------------
`;
