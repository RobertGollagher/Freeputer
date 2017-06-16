var prgSrc = `fal --- jmp /start 0x00000e
/put1 lit 0x000001
  frt ---
  ret ---
/put2and2 /replaced jmp 0x000017
  fal ---
  fal ---
/put4 lit 0x000004
  frt ---
  ret ---
/put3 lit 0x000003
  frt ---
  ret ---
/put1to4 cal /put1 0x000002
  frt ---
  cal /put2and2 0x000005
  frt ---
  cal /put3 0x00000b
  frt ---
  cal /put4 0x000008
  frt ---
  hal ---
/put2and2 lit 0x000002
  frt ---
  lit 0x000002
  frt ---
  ret ---
/start cal /put1to4 0x00000e ( =========================== )
  frt ---
  hal ---




( Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+ )
`;
