# Introduction #

_A functional BASIC interpreter running on the Robobuilder RBC unit, for those people who would like to try programming - but are daunted by the complexity of writing in C - and yet want more than Action builder, download and try it out !!_

This custom firmware for Robobuilder gives it the ability to create and store simple programs in an elementary BASIC language. The software is stable beta release. **_Always download latest version firmware_**

This BASIC is a simple integer version based on Dartmouth Basic (see wiki http://en.wikipedia.org/wiki/BASIC). But with some additional features to support robotics on the Robobuilder RBC robots.

See [Getting started](GettingStarted.md) wiki page.

For an in depth look at features  see [Robobuild Basic manual 2.3](http://code.google.com/p/robobuilderlib/downloads/detail?name=Robobuilder%20Basic%20V2.3.pdf) for more details. And for example programs see :
  * [SimpleDemos](SimpleDemos.md) or
  * [BasicDemos](BasicDemos.md) or
  * [AdvancedDemos](AdvancedDemos.md)

## Feature Summary ##
  1. A-Z 32 bit signed integer variables
  1. IF THEN ELSE
  1. GOTO & GOSUB/RETURN
  1. [FOR NEXT](ForNext.md) Loops
  1. PRINT
  1. END
  1. [Simple](InputMode.md) editor with Immediate mode so commands can be run directly
  1. Debug mode to allow trace of programs
  1. Full maths expression `+ * - /` and including MOD AND OR
  1. [Maths functions](MathsFunctions.md) such as SIN and COS
  1. Developer environment with VT100 terminal support (Windows/Mono - see [BasicClient](BasicClient.md)
  1. Special features including access to Servo (Read/ Write), sensors [PSD](SimpleDemos#10)_OUT_-_show_PSD_sensor_output_as_a_bar.md) and [Accelerometer](SimpleDemos#6)_read_accelerometer_values.md)
  1. Lists (A-Z) (and [Matrix commands](MatrixCommands.md))
  1. [Built in motions](BuiltinMotion.md) (19 including football kick)
  1. Built in FFT - [FFT demo](sample.md)  can access sound samples and process in realtime
  1. Built in Neural network calculation see [demo](neuron1.md)
  1. Event handling [ON GOSUB](SimpleDemos#22)_ON_event_base_routines.md)
  1. [I2C communication](SimpleDemos#17)_I2C_comm_example(s).md)
  1. Offset feature supports different robot configs
  1. [Hardware access](SimpleDemos#20)_Reading_PF1_and_PF2.md) to LED and Buttons
  1. Cross platform support for [LinuxBuild](LinuxBuild.md), Windows and [Omnima](Omnima.md) (openwrt)
    1. Includes extended commands for image processing [see demo](ImageDemos.md)
    1. floating point arithmetic
    1. Asynchronous move routing (PLY/REC)

See wiki page [for language specification](LanguageSpec.md) details

## Recent updates ##

Recent updates
  1. New list functions $REV and $SORT
  1. $input read a number kbd or ir controller
  1. SET BASE option for lists
  1. LET improvement A`[`x`]`=6
  1. LIST improvement - sub-lists  A`[`x,y`]`
  1. FOR/NEXT improvements :  BY/IN
  1. Matrix commands updates
  1. Input mode '=' get last typed
  1. MOVE options PLAY
  1. y/Y option
  1. B - breakpoints
  1. STAND - no need for argument
  1. cmdline option STAND on Linux/Omnima load stand.txt
  1. DCMP mode  (type M)!
  1. Chargemode - improved output
  1. Linux/omnima - more floating point functions SIN/COS ACOS TAN, LOG,EXP, etc
  1. floating point matrices and Backprop example
  1. Wiki updates
  1. new demo [hexapawn!](Hexapawn.md)
fixes
  1. convolve matrix
  1. expression operator precedence bug on mod/%
  1. trap divide by 0 on mod
  1. fix unitary '-' i.e. -2--2 = 0
  1. Basicclient MONO fix on download slowed Vt100 terminal