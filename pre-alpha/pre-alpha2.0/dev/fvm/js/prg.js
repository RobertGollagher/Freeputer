var prgSrc = `
/*
  Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+

  Program:    prg.js
  Author :    Robert Gollagher   robert.gollagher@freeputer.net
  Created:    20170911
  Updated:    20170915+

  This source file will contain the QMISC program to be assembled.
  The QMISC assembler is not yet implemented.
*/

/* Without rpt: */
i(8) add i(11) add 

i(2) fromb i(256) put

/*LOOP:*/
i(256) get i(1) sub

i(0) jmpe(18) i(256) put

jump(8) xor


/*DONE:*/
halt



/* With rpt:
i(8) add i(11) add i(2) fromb tor rpt(7) xor halt
*/

`;
