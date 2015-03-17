# Introduction #

A windows based editor and compiler to simplify working with Robobuilder Basic programs. The Basic firmware can work directly with all that is needed being a terminal program, however the client does make transferring programs from PC to Basic much simpler, and enables storage of programs and increase readability of programs.

The upper window is a standard text editor. Note it auto-generates line number and allows the use of labels (unlike the firmware version).

The bottom windows shows the output of a compile. This can be transfered to the robot by connecting  via a serial link - use the red button (edit COM5 to be the port you use).

The windows is then an interactive console on to the robot. Press z (to put robot in download mode) and then select download from the menu. The program will automatically compile if not already been performed.

## Key advantages of client ##

  * enable programs to be stored on PC
  * download  and transferred to basic firmware on robot
  * auto generates line numbers (int increments of 5)
  * allows comments to be added (using ' )
  * allows labels
  * provides context sensitive colour coding
  * allows multi-line if/then/else/endif statements
  * allows const definition
  * Simple functions and procedures
  * Create image filter maps
  * Load binary files and reverse compile
  * basic VT100 terminal with com port selection
  * will check firmware version running on robot

## Examples program ##

Here's an example using example : accelm.rbas

http://robobuilderlib.googlecode.com/svn/trunk/basic/examples/accelm.rbas

<a href='http://www.youtube.com/watch?feature=player_embedded&v=jGPBwTbgT6o' target='_blank'><img src='http://img.youtube.com/vi/jGPBwTbgT6o/0.jpg' width='425' height=344 /></a>

## working with RBasic.exe ##

When a program is compiled a file bindata.txt is created - and this file can then be loaded into basic.exe using 'z' option. It will also be automatically loaded into basic when it starts.

## Details ##

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:14:15.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:14:15.png)

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2019:58:13.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2019:58:13.png)

"output" window after program has compiled

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:11:30.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:11:30.png)

VT100 window - connected to Robobuilder

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-15%2022:16:19.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-15%2022:16:19.png)

"image window" after selecting a jpg asnd using "load web" button

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:12:25.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:12:25.png)

"Filtered image"

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:12:54.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:12:54.png)

"downscaling to 32x32 of filtered image

![http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:13:19.png](http://robobuilderlib.googlecode.com/svn/trunk/basic/BasicClient/Screenshot%20from%202012-09-07%2020:13:19.png)