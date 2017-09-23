var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170923+
  ------------------
  LAST SYMBOL: s0007
  ------------------
*/

#define /*FIRST_LETTER*/ s0003 0x41
#define /*ALPHABET_SIZE*/ s0004 0x1a

// The sole forward reference
jump(s0000) /*start*/

// Success
s0002: /*end:*/
  i(0) halt

// Print the alphabet
s0006: /*print_alphabet*/
    i(s0004) /*ALPHABET_SIZE*/ dec fromb tor
    i(s0003) /*FIRST_LETTER*/ fromb
    i(1)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    link

// Hexadecimal word print
s0007: /*print_hex*/
  // TODO NEXT   
  link

// Secondary entry point
s0001: /*go:*/
  br(s0006) /*print_alphabet*/
  jump(s0002) /*end*/

// Primary entry point
s0000: /*start:*/
  jump(s0001) /*go*/

`;
