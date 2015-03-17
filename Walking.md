# Introduction #

This wiki covers different walking methods
  * Built in
  * Pre defined
  * Generated on the fly
  * With balance control

## Play built in motion ##

Simplest program - simply walk until obstacle
  * Line 10 Plays built in motion "Forward"
  * Line 20 print the value of distance sensor
  * Line 30 If nothing close then repeat

```vb

10 RUN 8
20 PRINT $psd
30 IF $psd>20 THEN 10```

## Continuous Walking Pre defined motion ##

  * Line 5  STAND - instructs the robot to take up basic posture.
  * line 15 causes the program to wait for a key press - you could alternatively make it wait for IR remote by switch $KBD for $IR.
  * Line 20 set A delay between each MOVE
  * Line 35 move @{...},1,A moves servos from their current position to the new position, as specified in the list @{...}.
    * First number is length of list i.e. 16. There are two other parameters,
    * '1' is the number of increments to take
    * 'A' is a variable containing 25, the total length of time i.e. 25ms.
  * Line 125 GOTO will cause the program to loop

```vb

5 STAND 16
10 PRINT "press any key start"
15 LET K=$kbd
20 LET A=25
30 PRINT "R";
35 MOVE @{16,123,156,212,80,108,126,73,40,150,141,68,44,40,138,208,195},1,A
40 MOVE @{16,130,165,201,81,115,134,81,31,147,149,72,44,40,145,209,201},1,A
45 MOVE @{16,132,171,197,83,117,137,86,28,148,152,78,43,41,154,209,206},1,A
50 MOVE @{16,132,175,195,87,117,139,91,27,152,154,87,43,43,164,209,211},1,A
55 MOVE @{16,132,178,197,91,117,137,95,28,157,152,97,43,48,172,209,213},1,A
60 MOVE @{16,130,179,201,95,115,134,96,31,161,149,105,43,53,179,210,214},1,A
65 MOVE @{16,127,178,206,98,112,130,95,35,166,145,111,42,57,182,210,214},1,A
70 MOVE @{16,124,175,212,100,109,127,92,40,170,142,113,42,59,183,210,214},1,A
80 PRINT "L";
85 MOVE @{16,120,172,217,102,105,123,88,46,170,138,111,42,57,182,210,214},1,A
90 MOVE @{16,116,167,221,103,101,120,83,51,169,135,106,43,53,179,210,214},1,A
95 MOVE @{16,113,162,224,102,98,118,77,55,167,133,97,43,48,173,209,213},1,A
100 MOVE @{16,111,157,225,98,96,118,73,57,163,133,87,43,43,164,209,211},1,A
105 MOVE @{16,113,153,224,93,98,118,70,55,159,133,79,43,41,154,209,206},1,A
110 MOVE @{16,116,152,221,89,101,120,69,51,155,135,72,44,40,146,209,201},1,A
115 MOVE @{16,120,153,217,84,105,123,70,46,152,138,69,44,40,140,208,197},1,A
120 MOVE @{16,123,156,212,80,108,126,73,40,150,141,68,44,40,138,208,195},1,A
125 GOTO 30```


## Generating a walking gait ##

A more advanced example [Generate](Generate.md) can be seen above.

In one of the examples above I show continuous walking by sending out a sequence of predefined servo moves. But its possible to generate a walking gate mathematically using sine waves. The position of each servo can be defined as

```
Position = Offset + Amplitude * SIN (angle + phase)
```

As the angle goes through 360 degrees the servo position will vary about the offset by +/- the amplitude during each walking cycle.

To program this in BASIC needs the function $SIN(angle) will returns a sine value between -32768 and +32768 for angle values between 0 and 255. So to use the I create the following code:

```
FOR X=0 to 15
   LET P = B + (A*$SIN((X + C)*16)/32)/1024
NEXT X

    P = Position
    X = Angle   (0->15)
    A = amplitude
    B = offset
    C = phase   (0->15)
```

To enable each servo to have a different value of A, B and C I put those values into three indexed lists as follows

```
LIST A=16, 10, 13, 15,11, 10, 10,13,15, 11, 10,22, 1, 9, 22,  1,  9
LIST B=16,122,166,210,92,107,129,83,42,159,144,91,43,50,161,209,205
LIST C=16,  0,  2,  8, 5,  0, 15, 2, 8,  6, 15, 5,11, 5,  5,  5,  5
```

A contains a 16 element list containing the amplitudes of the sine waves. To access a list element use `@A[Y]` where Y is a value between 0 and 15. `@A[0]` will point to the value 10 (not 16 which is just the length and not stored).

So putting this all together the following code will generate a continuous walking motion,:

  * Line 5-15 Setup the parameter values to generate sine wave
  * Line 30   calculates the SIN wave value for each servo in turn.
  * Line 55   the calculation is sent to the servo. The program is fast enough to run a smooth walking gate.
  * Line 75   Detects IR or keyboard press (or you turn it off!)

```vb

5  LIST A=16,10,13,15,11,10,10,13,15,11,10,22,1,9,22,1,9
10 LIST B=16,122,166,210,92,107,129,83,42,159,144,91,43,50,161,209,205
15 LIST C=16,0,2,8,5,0,15,2,8,6,15,5,11,5,5,5,5
20 FOR X=0 TO 15
25 FOR Y=0 TO 15
30 LET P=@B[Y]+@A[Y]*$SIN((X+@C[y])*16)/32768
50 PRINT P;":";
55 SERVO y=p
60 NEXT Y
65 PRINT ""
70 NEXT X
75 IF $KIR<0 THEN 20```

## Basic auto-balance ##

Lets look first at balance control

  * Line 10 causes the robot takes up Basic pose.
  * Line 30 make it wait until keyboard or IR input.
  * Line 40 set up list of offset values. Lines 50 maps the output of the z axis of the accelerometer ($z) to a range using the $find function.
  * Line 60 then translates that into a displacement to apply to the servo. L
  * Line 90 and 100 it updates the current position of the servo $servo(x) with the offset o.
  * Line 120 cause the program to loop until either keyboard or IR remote is pressed ($kir) is received.

```vb

10 STAND 16
20 Print "Press any key"
30 if $kir<0 then 30
40 list m=7,-4,-2,-1,0,1,2,4
50 Let d=$find($z, @{7,-15,-10,-5,0,5,10,15}
60 let o=@m[d]
70 wait 10
80 print $z;"-";d;"-";o
90 servo 10=$servo(10)+o
100 servo 13=$servo(13)-o
110 wait 10
120 if $kir<0 then 40```

## Continuous walk with balance control ##

Now combine using Offset and EVents. See above for how to use ON based events to add balance to other motions
Continuous walk with a 1s interrupt to check balance and modify OFFSET array.

Requires Basic IDE to load

```vb

'Continuous forward walk
LIST m=7,-4,-2,-1,0,1,2,4
LIST r=7,-15,-10,-5,0,5,10,15
LIST o=$zeros(16)
OFFSET
STAND 16
PRINT "press any key start"
A=25
B=25

check:
IF $KIR<0 THEN check
g=$Z
ON TIME 1000 GOSUB timer
ON KEY GOSUB keytest
Loop:
PRINT "R";
MOVE @{16,123,156,212,80,108,126,73,40,150,141,68,44,40,138,208,195},1,A
MOVE @{16,130,165,201,81,115,134,81,31,147,149,72,44,40,145,209,201},1,A
MOVE @{16,132,171,197,83,117,137,86,28,148,152,78,43,41,154,209,206},1,A
MOVE @{16,132,175,195,87,117,139,91,27,152,154,87,43,43,164,209,211},1,A
MOVE @{16,132,178,197,91,117,137,95,28,157,152,97,43,48,172,209,213},1,A
MOVE @{16,130,179,201,95,115,134,96,31,161,149,105,43,53,179,210,214},1,A
MOVE @{16,127,178,206,98,112,130,95,35,166,145,111,42,57,182,210,214},1,A
MOVE @{16,124,175,212,100,109,127,92,40,170,142,113,42,59,183,210,214},1,A
PRINT "L";
MOVE @{16,120,172,217,102,105,123,88,46,170,138,111,42,57,182,210,214},1,B
MOVE @{16,116,167,221,103,101,120,83,51,169,135,106,43,53,179,210,214},1,B
MOVE @{16,113,162,224,102,98,118,77,55,167,133,97,43,48,173,209,213},1,B
MOVE @{16,111,157,225,98,96,118,73,57,163,133,87,43,43,164,209,211},1,B
MOVE @{16,113,153,224,93,98,118,70,55,159,133,79,43,41,154,209,206},1,B
MOVE @{16,116,152,221,89,101,120,69,51,155,135,72,44,40,146,209,201},1,B
MOVE @{16,120,153,217,84,105,123,70,46,152,138,69,44,40,140,208,197},1,B
MOVE @{16,123,156,212,80,108,126,73,40,150,141,68,44,40,138,208,195},1,B
PRINT

'face down (z<-50)
IF $Z<-50 THEN
PRINT "FACE DOWN"
GOTO finish
ENDIF
'face up (z>50)
IF $Z>50 THEN
PRINT "FACE UP"
GOTO finish
ENDIF
'upright  (y>50)
IF $Y>50 THEN
PRINT "UPRIGHT"
ENDIF
GOTO Loop
'
' some key check
'
keytest:
LET k=$event
IF k=7 or k=`q THEN finish
RETURN
'
'check balance
'
timer:
z=g-$Z
d=$FIND(z,@R)
o=@m[d]
'PRINT "D=";d;" Z=";z;" O=";o
SET @o,10,o
SET @o,13,-o
OFFSET @o
RETURN
finish:
PRINT "Done = ";k
STAND 16```