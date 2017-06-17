var prgSrc = `/Molecule fal --- nop ---
cal /put4 0x000016
frt ---
cal /put3 0x000019
frt ---
cal /put2 0x000013
frt ---
cal /put1 0x000010
frt ---
hal ---
nop ---
nop ---
nop ---
nop ---
nop ---
/Atom /put1 lit 0x000001
  frt ---
  ret ---
/Atom /put2and2 lit 0x0fffff      ( a bug )
  frt ---
  ret ---
/Atom /put4 lit 0x000004
  frt ---
  ret ---
/Atom /put3 lit 0x000003
  frt ---
  ret ---

( Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+ )
`;
