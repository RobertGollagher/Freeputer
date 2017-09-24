var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js (also known as prg.c)
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170924+
  ------------------
  FREE: s0003-4
  LAST SYMBOL: s0007
  ------------------
*/


// Constants -----------------------------------------------------------------



// The sole forward reference ------------------------------------------------

jump(s0000) /*start*/


// Success -------------------------------------------------------------------

s0002: /*end:*/
  i(0) halt


// Print the alphabet --------------------------------------------------------

s0006: /*print_alphabet*/
    i(0x19) fromb tor
    i(0x41) fromb
    i(1)
  s0005: /*loop_alpha:*/
    out add rpt(s0005) /*loop_alpha*/
    link


// Hexadecimal word print ----------------------------------------------------

s0007: /*print_hex*/
  // TODO NEXT   
  link


// Secondary entry point -----------------------------------------------------

s0001: /*go:*/
  br(s0006) /*print_alphabet*/
  jump(s0002) /*end*/


// Primary entry point -------------------------------------------------------

s0000: /*start:*/
  jump(s0001) /*go*/


// ---------------------------------------------------------------------------
`;
