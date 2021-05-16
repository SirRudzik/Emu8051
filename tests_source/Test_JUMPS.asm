org 0H
AJMP kokos0
ret0: AJMP kokos1
ret1:AJMP kokos2
ret2:AJMP kokos3
ret3:AJMP kokos4
ret4:AJMP kokos5
ret5:AJMP kokos6
ret6:AJMP kokos7

org 0AAH
kokos0:
mov a,@#1H
AJMP ret0
org 1AAH
kokos1:
mov a,@#2H
AJMP ret1
org 2AAH
kokos2:
mov a,@#3H
AJMP ret2
org 3AAH
kokos3:
mov a,@#4H
AJMP ret3
org 4AAH
kokos4:
mov a,@#5H
AJMP ret4
org 5AAH
kokos5:
mov a,@#6H
AJMP ret5
org 6AAH
kokos6:
mov a,@#7H
AJMP ret6
org 7AAH
kokos7:
mov a,@#8H
LJMP kokos8

org 7FFFH
kokos8:
MOV DPTR,#0AFFFH
JMP @A+DPTR

org 0AFF0H
krotki:
jmp jump

org 0AFFFH
sjmp krotki
jump:
mov a,#01010101B
END
