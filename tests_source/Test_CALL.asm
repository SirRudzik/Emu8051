org 0H
ACALL kokos0
ACALL kokos1
ACALL kokos2
ACALL kokos3
ACALL kokos4
ACALL kokos5
ACALL kokos6
ACALL kokos7
LCALL kokos8
mov a,#01010101B


org 0AAH
kokos0:
mov a,@#1H
ret
org 1AAH
kokos1:
mov a,@#2H
ret
org 2AAH
kokos2:
mov a,@#3H
ret
org 3AAH
kokos3:
mov a,@#4H
ret
org 4AAH
kokos4:
mov a,@#5H
ret
org 5AAH
kokos5:
mov a,@#6H
ret
org 6AAH
kokos6:
mov a,@#7H
ret
org 7AAH
kokos7:
mov a,@#8H
ret
org 0AFFH
kokos8:
mov a,@#9H
ret
end
