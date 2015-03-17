# Introduction #

Image processing assumes a webcam is streaming jpeg images to a file called "test.jpg" using a streamer such as mjpg\_streamer.

In the /etc/init.d/mjpg\_streamer I have added line

```
service_start /usr/bin/mjpg_streamer --input "input_uvc.so \
                --device $device --fps $fps --resolution $resolution" \
                --output "output_file.so -d 1000 -c /root/test.sh -f /tmp"
```

This calls a file test.sh every 1s (1000ms)
The name of the image file is passed as $1 and then test.jpg simply renames it, places it in correct directory and then signal basic using USR1 to tell it the image is ready.

Ive now added a simple locking mechanism such that the !IMAGE LOAD command (in Basic) will wait for the lock to become free. This is too prevent it trying to read from the jpeg file whilst being updated. The lock mechanism is  simple directory so that it can be controlled by shell script used by mjpeg streamer

```
root@OpenWrt:~# cat /root/test.sh
#
DIR=/root
echo $1
#START critical section
while ! $(mkdir /tmp/x.lock 2>/dev/null)
do
     echo 'a'
done
mv $1 $DIR/test.jpg
rmdir /tmp/x.lock
#END critical section

cp -f $DIR/test.jpg /www/test.jpg
kill -s USR1 $(cat $DIR/PID)
```

The PID file is created when rbasic runs and contain the process ID to simplify the signal generation

# A few simple demos #

This demo loads an image as 2x2 array and then control a servo (20) which control the left / right position of the webcam.

### Demo 1 - Track bright object ###

<a href='http://www.youtube.com/watch?feature=player_embedded&v=WQdsdZTa1j8' target='_blank'><img src='http://img.youtube.com/vi/WQdsdZTa1j8/0.jpg' width='425' height=344 /></a>

```vb

'Find brightest light
LIST M=12,0,0,1,-1,1,-1,0,-1,1,-1,1,0
LOOP:
!IMAGE LOAD 2   '2x2 matrix
!IMAGE SHOW 6   'show values
GOSUB DONET     'Process network
WAIT 1000
GOTO LOOP
DONET:
LIST !=@!.@M    'Add weights & threshhold
NETWORK 4,2,11,2,0,0
P=$SERVO(20)
IF @![4]=1 then
print "nuron 1 fired"   'turn left
P=P-2
ENDIF
IF @![5]=1 then
print "nuron 2 fired"   'turn right
P=P+2
ENDIF
SERVO 20=P
RETURN```

### Demo 2 - Use the IMAGE signal to trigger process ###
Rather than looping you can control loading the image using ON IMAGE extension

```vb

' Demo of basic Image process
Print "Load image and turn head servo (20) to brightest source"
LIST M=12,0,0,1,-1,1,-1,0,-1,1,-1,1,0
ON IMAGE GOSUB ImProc

Loop:
WAIT 50
GOTO Loop

ImProc:
PRINT "load image"
t=$tick
!IMAGE LOAD 2        'load 2x2 image (grey scale)
!IMAGE SHOW 6        'show values
LIST !=@!.@M         'Add weights & threshhold
NETWORK 4,2,3,2,0,0  'Do network
P=$SERVO(20)
IF @![4]=1 THEN
print "nuron 1 fired"
P=P-2
ENDIF
IF @![5]=1 THEN
print "nuron 2 fired"
P=P+2
ENDIF
SERVO 20=P
PRINT "t=";$tick-t
RETURN```

### Demo 3 - Filter processing in a Loop with a WAIT ###

<a href='http://www.youtube.com/watch?feature=player_embedded&v=YgzakRelVaM' target='_blank'><img src='http://img.youtube.com/vi/YgzakRelVaM/0.jpg' width='425' height=344 /></a>

The above uses the IMAGE FILT command with WAIT IMAGE

Alternatively use e a colour filter is applied using code based on CMVision2.0 library. This will filter the image and detect regions of same colour

```vb

'set up filter
!IMAGE COLO "orange";20;30;60;2
!IMAGE THRE CID;120; 175; 40; 70; 30; 40
Loop:
!WAIT IMAGE          'wait for image available
!IMAGE RAW           'load
!IMAGE PROC 10       'process as 10x10 image
!IMAGE SHOW 6        'show process output
!IMAGE REG 1         'return region in @! for colour id 1
if $ne=0 then loop   'if no region found $NE will be zero
PRINT @!:8           'show region info
goto Loop```

Example output given an image with an "orange ball" in it
<pre>
Run Program<br>
Add color orange 20,30,60 >2<br>
Add threshold [1], (120,175) (40,70) (30,40)<br>
0   0   0   0   0   0   0   0   0   0<br>
0   0   0   0   1   0   0   0   0   0<br>
0   0   0   0   0   0   0   0   0   0<br>
0   0   0   0   0   0   0   0   0   0<br>
0   0   0   0   1   1   1   0   0   0<br>
0   0   0   0   1   1   1   0   0   0<br>
0   0   0   1   1   1   1   0   0   0<br>
0   0   0   0   1   0   0   0   0   0<br>
0   0   0   0   0   0   0   0   0   0<br>
0   0   0   0   0   0   0   0   0   0<br>
0) orange : n=1 [20 30 60] Area>=2<br>
x1=3,y1=4,x2=6,y2=7</pre>