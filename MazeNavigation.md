# Introduction #

This navigates around a space using the PSD sensor to identify obstacles and remembering where its been in an array which it displays by outputting as it goes
> Source : [maze.rbas](http://code.google.com/p/robobuilderlib/source/browse/trunk/basic/examples/maze.rbas)

# Details #

```vb

'Example maze program
'Based on Robobuilder C tutorial

const dely 1000
STAND 16
PRINT "Maze Program"

LIST A=6,`.,`^,`>,`V,`<,`*

LET v=8    'matrix size v x v
LET c=0    'initial state
LET x=v/2  'current location x coord (mid of matrix)
LET y=v/2  'current location y coord (mid of matrix)
LET t=1    'point forward
GOSUB matrix

loop:
t=(t-1) MOD 4
t=t+1
SET (v*x)+y,t

GOSUB show

d=$PSD
LIGHTS d-10
PRINT "PSD= ";D

k=$KBD

IF k=`q THEN
PRINT "done"
END
ENDIF

IF d < 20 THEN
GOSUB backz
GOTO loop
ENDIF

IF (c=0) AND (d<30) THEN
GOSUB leftturn90
LET t=t+3
LET c=1
GOTO loop
ENDIF

IF (c=1) AND (d<30) THEN
GOSUB leftturn180
LET t=t+2
LET c=2
GOTO loop
ENDIF

IF (c=1) AND (d<30) THEN
GOSUB rightturn90
LET t=t+1
LET c=0
GOTO loop
ENDIF

LET c=0

GOSUB forwardz

t=(t-1) MOD 4
SET (v*x)+y,5   'empty space

IF t=0 THEN
y=y-1
ENDIF

IF t=1 THEN
x=x+1
ENDIF

IF t=2 THEN
y=y+1
ENDIF

IF t=3 THEN
x=x-1
ENDIF

t=t+1

x=$RANGE(x,0,v-1)
y=$RANGE(y,0,v-1)
GOTO loop

backz:
PRINT "BACKUP"
RUN 9
WAIT 1000
RETURN

leftturn90:
PRINT "LT90"
RUN 4
RUN 4
RUN 4
WAIT dely
RETURN

leftturn180:
PRINT "180"
RUN 4
RUN 4
RUN 4
RUN 4
RUN 4
WAIT dely
RETURN

rightturn90:
PRINT "RT90"
RUN 5
RUN 5
RUN 5
RUN 5
WAIT dely
RETURN

forwardz:
PRINT "Forward"
RUN 8
RUN 8
RUN 8
WAIT dely
RETURN

show:
FOR J=0 TO v-1
FOR I=0 TO v-1
M=@![I*v+J]
OUT @A[M] `
`   NEXT I
PRINT
NEXT J
PRINT
RETURN

matrix:
'create an empty matrix
DELETE *           'deletes all entries
LIST !=$zeros(v*v) 'set as zero
RETURN

```

# Output #

```
........
........
........
........
....^...
........
........
........

PSD= 0

```