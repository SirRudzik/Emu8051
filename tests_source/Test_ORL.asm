mov a,#01010101B
mov 10H,#00110011B
ORL 10H,a
mov 11H,#01010101B
ClR A
ORL 11H,#00110011B
mov a,#01010101B
orl a,#00110011B
mov 12H,#01010101B
mov a,#00110011B
orl a,12H
mov 13H,#01010101B
mov 14H,#01010101B
mov r0,#13H
mov r1,#14H
mov a,#00110011B
ORL A,@R0
mov a,#00110011B
orl A,@R1
mov r0,#01010101B
mov r1,#01010101B
mov r2,#01010101B
mov r3,#01010101B
mov r4,#01010101B
mov r5,#01010101B
mov r6,#01010101B
mov r7,#01010101B
mov a,#00110011B
ORL A,r0
mov a,#00110011B
ORL A,r1
mov a,#00110011B
ORL A,r2
mov a,#00110011B
ORL A,r3
mov a,#00110011B
ORL A,r4
mov a,#00110011B
ORL A,r5
mov a,#00110011B
ORL A,r6
mov a,#00110011B
ORL A,r7
