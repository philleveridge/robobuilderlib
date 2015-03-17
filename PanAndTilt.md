# Introduction #

This demonstrates simple manual control of pan and tilt web cam.

The servos added are ID 22 and 24.

Commands are  :
  1. Up    'w'
  1. Down  'z'
  1. Left  'a'
  1. Right 's'
  1. Bow   'b
  1. Shake 'n'
  1. Quit  'q'

<a href='http://www.youtube.com/watch?feature=player_embedded&v=P3hqA1ygYA8' target='_blank'><img src='http://img.youtube.com/vi/P3hqA1ygYA8/0.jpg' width='425' height=344 /></a>

# Details #

```vb

'Cam test program
print "type: w,a,s,z,q"
loop:
P = SERVO(22)
Q = SERVO(24)
G = 5
H = 5
PRINT "P=",P,"Q=",Q
LET K =  $KBD
PRINT K

if k=`q then done

if k=`b then bow
if k=`n then shake
'Pan
IF  K=`s THEN
LET P =  P+H
ENDIF
IF  K=`a THEN
LET P =  P-H
ENDIF
'Tilt
IF  K=`w THEN
LET Q =  Q+G
ENDIF
IF  K=`z THEN
LET Q =  Q-G
ENDIF

'update servos
SERVO 22=P
SERVO 24=Q

'load webcam
! IMAGE LOAD 32
! IMAGE SHOW 6
GOTO loop

bow:
SERVO 22=70
SERVO 24=127
wait 500
servo 24=90
wait 1000
servo 24=127
goto loop

shake:
SERVO 22=70
SERVO 24=127
wait 500
servo 22=35
wait 500
SERVO 22=70
wait 500
servo 22=115
wait 500
SERVO 22=70
wait 500
goto loop

done:
servo 22=@
servo 24=@
end
```