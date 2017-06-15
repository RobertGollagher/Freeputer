var prgSrc = `nop --- nop --- ( 0x = hex, 0f = forward, 0r = reverse, / = token is a comment )
/start lit 0x000003
  lit /bar 0x000008                 ( literal )
  jmp /green 0f000004               ( forward )
  nop ---
  nop ---
/red jmp /end 0f000004              ( forward )
/green jmp /red 0r000001            ( reverse )
  nop ---
  nop ---
/end hal ---

`;
