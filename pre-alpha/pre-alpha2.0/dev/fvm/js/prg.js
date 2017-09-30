var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170930+
  ------------------
  FREE:
  LAST SYMBOL: s0009
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

s0002: /*end:*/ i(0x00) halt


// Text Output Utilities -----------------------------------------------------
s0004: /*print_newline*/ i(0x0a) fromb out link
// ---------------------------------------------------------------------------
// Print the uppercase alphabet ----------------------------------------------
s0006: /*print_alphabet*/
    i(0x19) fromb tor
    i(0x41) fromb
    i(0x01)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    link
// ---------------------------------------------------------------------------
// Hexadecimal print LSN -----------------------------------------------------
s0003: /*alpha*/ i(0x57) /*'a'*/ add out fromt link 
s0007: /*print_hex_nibble*/
  tot
  i(0x0fffffff) xor i(0x1c) shr
  i(0x09) jmpg(s0003) /*alpha*/
  i(0x30) /*'0'*/ add out fromt link
// ---------------------------------------------------------------------------



// FIXME stuck for 2 reasons: js xor not working for FLIP,
// and no way of returning as cannot do >1 level of link

s0008: /*print_hex_word*/
  tot
  i(0x07) fromb tor
  fromt
  s0009: /*loop_print_hex*/
    br(s0007) /*print_hex_nibble*/
    i(0x04) shl // FIXME disallow decimal immediates
    rpt(s0009) /*print_hex_word*/
    // FIXME no way of returning
    halt

// Secondary entry point -----------------------------------------------------

s0001: /*go:*/
  i(0x00) fromb i(0x1c) shl br(s0007) /*print_hex_nibble*/
  i(0x09) fromb i(0x1c) shl br(s0007) /*print_hex_nibble*/
  i(0x0a) fromb i(0x1c) shl br(s0007) /*print_hex_nibble*/
  i(0x0f) fromb i(0x1c) shl br(s0007) /*print_hex_nibble*/
  br(s0004) /*print_newline*/
  i(0x7654321a) fromb br(s0008) /*print_hex_word*/ // FIXME cannot return
  br(s0004) /*print_newline*/
  br(s0006) /*print_alphabet*/
  jump(s0002) /*end*/

// Primary entry point -------------------------------------------------------

s0000: /*start:*/
  jump(s0001) /*go*/


// ---------------------------------------------------------------------------
`;
