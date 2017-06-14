var prgSrc = `( 0x = hex, 0s = symbol, 0f = forward )
#def lit 0x01
#def jmp 0x03
#def nop 0xfd
#def hal 0xff
#def --- 0x000000
lit 0x000003
#def /foo 0s000001 0x000008
#def /bar 0s000002 /foo 0s000001   ( a back ref )
lit /bar 0s000002                  ( a back ref )
jmp /green 0f000004
nop ---
nop ---
/red jmp /end 0f000004
/green jmp /red 0r000001
nop ---
nop ---
/end hal ---
`;
