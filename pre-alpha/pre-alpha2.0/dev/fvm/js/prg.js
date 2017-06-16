var prgSrc = `///startProgram jmp /handleFailedProgram 0p000003 lit 0x000001
frt ---
cal /put2 0p000001
frt ---
hal ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
///put2 lit 0x000002
frt ---
cal /put3 0p000002
frt ---
ret 0f000001
fal ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
///put3 lit 0x000003
frt ---
frt ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
///handleFailedProgram lit 0x0000ff
fal ---
hal ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
(
/// nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
nop ---
)
( Copyright 2017, Robert Gollagher.
  SPDX-License-Identifier: GPL-3.0+ )
`;
