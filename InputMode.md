# Introduction #

Input mode (aka immediate mode) allows for programs to be input but also commands to be run instantly.

To enter input mode type 'i' to the command prompt ':'. You will then be prompted for input with a '>'.  To exit input type a period "." as the first character of the line. Also '=' will recall the last line typed.

# Details #

To execute a command imnediately simply enter it without a line number
```
: i
Enter Program '.' to Finish
> PRINT "Hello"
Hello
```

Most commands can be run including LET commands to set variables and also SERVO commands to read and servos etc, For example enter "RUN 17" and the robot will wave to you !

### Entering a program ###

Entering the same command with a line number causes it to be stored
```
> 10 PRINT "Hello"
> 20 PRINT "world"
> 

2 lines entered, [35/3072] Bytes
: l
List Program 
10 PRINT "Hello"
20 PRINT "world"

```

## Insert a new line ##

A new line can now be inserted between the lines 10 and 20 by adding one with any number inbetween i.e 15

```
: i
Enter Program '.' to Finish
> 15 PRINT "robot"
Insert line
> 

1 lines entered, [51/3072] Bytes
: l
List Program 
10 PRINT "Hello"
15 PRINT "robot"
20 PRINT "world"
: r
Run Program 
Hello
robot
world
End
Elapsed Time 00:00-000

```

### Edit a line ###

To edit an existing line simply re-enter using the same number
```
: i
Enter Program '.' to Finish
> 15 PRINT "Robobuilder"
Edit line
> 

1 lines entered, [73/3072] Bytes
: l
List Program 
10 PRINT "Hello"
15 PRINT "Robobuilder"
20 PRINT "world"
: r
Run Program 
Hello
Robobuilder
world
End
Elapsed Time 00:00-000
: 
```

### Delete a line ###

To delete a line simply enter its line number with other content
```
: i
Enter Program '.' to Finish
> 15
Delete - 15
> 

0 lines entered, [73/3072] Bytes
: l
List Program 
10 PRINT "Hello"
20 PRINT "world"
: r
Run Program 
Hello
world
End
Elapsed Time 00:00-000
: 
```