# Introduction #

Uses a FFT to split sound received.

```
SAMPLE - get the data into a special LIST @!
FFT    - does the FFT putting result into @!
```

Set variable F before running program to either show frequemcy diagram, or use it to move arms in time with sound.

Program will stop if it "hears" no sound during 5s sample window

# Details #

```vb

'demo Sample sound test
STAND $NS  'attention !
LET F=0    'switch display modes

Loop:
print "Listen"

SAMPLE 4,5000,1,64    'min level 4, 5 secs, 1ms, 64 samples
IF $NE=0 THEN Done    'No element returned stop
FFT @!,1000           'FFT sound sample

IF f THEN             'either
Gosub ShowFFt      'display the FFT
ELSE                  'or
Gosub moveservos   'Move servos
ENDIF

wait 50

IF $kir<0 THEN Loop   'loop if nothing pressed
F=1-F                 'else switch display mode
GOTO Loop             'and loop

Done:
print "Finished"
stand $NS
END
```

This shows the FFT as a bar chart

```vb

Showfft:
GOSUB clscreen

select 1,*      'remove DC component from FFT - cell 0
LET n=$MAX(@!)

FOR e=0 TO 30
print e+1;" ";
v=(@![e]*50)/n
out 45,v
out 80
print
NEXT e

LET t=1+$IMAX(@!)
PRINT "Pk=";n;"[";t;"] Hz=";t*16
RETURN
```

This clears screen for each run.
```vb

clscreen:
OUT 27 'esc
OUT 91 '[
OUT 50 '2
OUT 74 'J
RETURN
```

This moves the servos depending on amplitude of two frequencies
```vb

moveservos:
GOSUB clscreen
n=$MAX(@!)     'Find the peak value

d = @![16]     'Approx 256 +/- 8Hz
d = (50*d)/n
p=d

e = @![21]     'Approx 336 +/- 8Hz
e = (50*e)/n
q=e

out 45,p
PRINT "P=";p
out 45,q
PRINT "Q=";q

SERVO 11= 48+p
SERVO 14=205-q
RETURN
```

# Output #

```
--------------------------P=26
------------------------Q=24
Listen
User Break on line 14
Elapsed Time 00:27-668
: 

```