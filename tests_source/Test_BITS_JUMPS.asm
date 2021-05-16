clr c
JC kokos
JNC kokos
NOP
NOP
NOP
NOP
kokos:
setb c
jnc kokos2
jc kokos2
NOP
NOP
NOP
NOP
kokos2:
clr P0.0
JB P0.0,kokos3
JNB P0.0,kokos3
NOP
NOP
NOP
NOP
kokos3:
setb P0.0
JNB P0.0,kokos4
JB P0.0,kokos4
NOP
NOP
NOP
NOP
kokos4:
clr P1.0
JBC P1.0,kokos5
setb P1.0
JBC P1.0,kokos5
NOP
NOP
NOP
NOP
kokos5:
mov a,01010101B