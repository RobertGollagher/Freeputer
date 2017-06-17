var prgSrc = `fal --- jmp 0x00000e
#def /put1 0s000001 . lit 0x000001
  frt ---
  ret ---
#def /put2and2 0s000002 . lit 0x0fffff      ( a bug )
  frt ---
  ret ---
#def /put4 0s000003 . lit 0x000004
  frt ---
  ret ---
#def /put3 0s000004 . lit 0x000003
  frt ---
  ret ---
#def /Molecule 0s000005 . cal /put4 0s000003
  frt ---
  cal /put3 0s000004
  frt ---
  cal /put2and2 0s000002
  frt ---
  cal /put1 0s000001
  frt ---
  hal ---
  nop ---
  nop ---
  nop ---
  nop ---
  nop ---

( Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+ )
`;
