var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170924+
  ------------------
  FREE:
  LAST SYMBOL: s0007
  ------------------
*/


// Constants -----------------------------------------------------------------



// The sole forward reference ------------------------------------------------

jump(s0000) /*start*/


// Success -------------------------------------------------------------------

s0002: /*end:*/ i(0) halt


// Print newline -------------------------------------------------------------

s0004: /*print_newline*/ i(0x0a) fromb out link


// Print the uppercase alphabet ----------------------------------------------

s0006: /*print_alphabet*/
    i(0x19) fromb tor
    i(0x41) fromb
    i(1)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    link

// Hexadecimal nibble print --------------------------------------------------

s0003: /*alpha*/ i(0x57) add out fromt link 
s0007: /*print_hex_nibble*/
  tot
  i(0x0000000f) and
  i(0x09) jmpg(s0003) /*alpha*/
  i(0x30) add out fromt link

// Secondary entry point -----------------------------------------------------

s0001: /*go:*/
  i(0x00) fromb br(s0007) /*print_hex_nibble*/
  i(0x09) fromb br(s0007) /*print_hex_nibble*/
  i(0x0a) fromb br(s0007) /*print_hex_nibble*/
  i(0x0f) fromb br(s0007) /*print_hex_nibble*/
  br(s0004) /*print_newline*/
  i(0x09abcdef) fromb br(s0007) /*print_hex_nibble*/
  br(s0004) /*print_newline*/
  br(s0006) /*print_alphabet*/
  jump(s0002) /*end*/


// Primary entry point -------------------------------------------------------

s0000: /*start:*/
  jump(s0001) /*go*/


// ---------------------------------------------------------------------------
`;
