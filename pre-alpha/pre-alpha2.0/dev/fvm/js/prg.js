var prgSrc = `( TODO: consider forward refs )
#def lit 0x01
#def hal 0xff
lit 0x000003
lit 0x000005
#def foo .    ( a label )
#def bar foo  ( a back ref )
lit bar       ( a back ref )
hal 0x000000
`;
