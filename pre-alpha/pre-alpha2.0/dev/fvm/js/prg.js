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
  LAST SYMBOL: s000d
  ------------------

*/

// The sole forward reference
jump(s0000) /*start*/


// Success
s0002: /*end:*/ b(0x00) halt

// ===========================================================================

// Leave only the most significant nibble
s000b: /*msn*/ b(0x0fffffff) xor ret


// Move the most significant nibble to the least significant nibble
s000c: /*msn_to_lsn*/ b(0x1c) shr ret


// Shift left by 1 nibble
s000d: /*shl_nibble*/ b(0x04) shl ret

// ===========================================================================

// Print a newline character
s0004: /*print_newline*/ a(0x0a) out ret


// Print the uppercase alphabet
s0006: /*print_alphabet*/
    a(0x41) b(0x01) r(0x19)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    ret


// Hexadecimal print MSN
s0003: /*alpha*/ b(0x57) /*'a'*/ add out tpop ret
s0007: /*print_hex_msn*/
  tpush
  call(s000c) /*msn_to_lsn*/
  b(0x09) jmpg(s0003) /*alpha*/
  b(0x30) /*'0'*/ add out tpop ret


// Hexadecimal print word
s0008: /*print_hex_word*/
  r(0x07)
  s0009: /*loop_print_hex*/
    call(s0007) /*print_hex_msn*/
    call(s000d) /*shl_nibble*/
    rpt(s0009) /*loop_print_hex*/
    ret

// ===========================================================================

s0001: /*go:*/
  a(0x7654321a) call(s0008) /*print_hex_word*/
//  call(s0004) /*print_newline*/
//  call(s0006) /*print_alphabet*/
  jump(s0002) /*end*/


s0000: /*start:*/
  jump(s0001) /*go*/
`;
