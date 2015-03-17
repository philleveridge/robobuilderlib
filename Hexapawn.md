# Introduction #

<img src='http://robobuilderlib.googlecode.com/files/IMG_3218.2.JPG' width='350'>


wikipedia <a href='http://en.wikipedia.org/wiki/Hexapawn'>http://en.wikipedia.org/wiki/Hexapawn</a>
<pre><code>"Gardner specifically constructed it as a game with a small game tree,<br>
 in order to demonstrate how it could be played by a heuristic AI <br>
implemented by a mechanical computer."<br>
</code></pre>

Inspired by <a href='http://www.atomclub.nl/sites/Fpga/www.howell1964.freeserve.co.uk/Acorn/Atom/amb/AtomMagic.htm'>http://www.atomclub.nl/sites/Fpga/www.howell1964.freeserve.co.uk/Acorn/Atom/amb/AtomMagic.htm</a>

Demos new $input and set @a<code>[</code>x<code>]</code>=y<br>
<br>
Source: <a href='http://code.google.com/p/robobuilderlib/source/browse/trunk/basic/examples/hexpawn.rbas'>http://code.google.com/p/robobuilderlib/source/browse/trunk/basic/examples/hexpawn.rbas</a>

<pre><code>: r<br>
Run Program <br>
C C C    0 1 2<br>
         3 4 5<br>
H H H    6 7 8<br>
Input your move <br>
? 63<br>
My move 14<br>
C   C    0 1 2<br>
H C      3 4 5<br>
  H H    6 7 8<br>
Input your move <br>
? <br>
</code></pre>


<h1>Details</h1>

<h2>initialise</h2>

<pre><code><br>
c=1 'computer<br>
h=2 'human<br>
'stored positions<br>
p=100<br>
list s=$zeros(p)<br>
</code></pre>

<h2>display board</h2>

<pre><code><br>
newg:<br>
'clear board<br>
list b=9,c,c,c,0,0,0,h,h,h<br>
main:<br>
t=h<br>
for i=0 to 8<br>
out (B[i]=0)*32+(B[i]=c)*C+(B[i]=h)*H<br>
out 32<br>
if i=2 then<br>
print "   0 1 2"<br>
endif<br>
if i=5 then<br>
print "   3 4 5"<br>
endif<br>
if i=8 then<br>
print "   6 7 8"<br>
endif<br>
next i<br>
</code></pre>


<h2>read user move</h2>
This use new $input feature to read move.<br>
<pre><code><br>
'Check if legal human move available - if not finished<br>
for m=10 to 87<br>
gosub cm<br>
if e=0 then ym<br>
next m<br>
goto yl<br>
ym:<br>
print "Input your move "<br>
let m=$input<br>
if m=0 then yl<br>
gosub cm<br>
if e&lt;&gt;0 then ym<br>
gosub mp<br>
</code></pre>

<h2>The robot make its move</h2>

Code the board and then search for position in possible list<br>
<br>
<pre><code><br>
print "My move ";<br>
t=c<br>
x=B[0]+(4*B[3])+(16*B[6])+64<br>
y=B[1]+(4*B[4])+(16*B[7])+64<br>
z=B[2]+(4*B[5])+(16*B[8])+64<br>
v=0<br>
mm:<br>
u=0<br>
if S[v]&gt;128 then mend<br>
if S[v]=0 then newe<br>
if S[v]=x and S[v+1]=y and S[v+2]=z then sm<br>
u=1<br>
if S[v]=z and S[v+1]=y and S[v+2]=x then sm<br>
v=v+2<br>
mend:<br>
v=v+1<br>
if v&lt;p then mm<br>
print "v=";v<br>
end<br>
</code></pre>

<h2>add new entry</h2>
Adds code position to store<br>
<pre><code><br>
newe:<br>
S[v]=x<br>
S[v+1]=y<br>
S[v+2]=z<br>
S[v+3]=0<br>
w=v+3<br>
for m=0 to 78<br>
if (m mod 10)&lt;9 then<br>
gosub cm<br>
if e=0 then<br>
set S[w]=m+128<br>
w=w+1<br>
set S[w] = 0<br>
endif<br>
endif<br>
next m<br>
</code></pre>

<h2>select move</h2>
<blockquote>select move, if u=0 use as stored, u=1 mean switch rows<br>
<pre><code><br>
sm:<br>
v=v+3<br>
if S[v]&lt;128 then delm<br>
m=S[v]-128<br>
<br>
if u=0 then sme<br>
<br>
if m/10&lt;3 then<br>
m=m+60<br>
goto sm2<br>
endif<br>
if m/10&gt;5 then<br>
m=m-60<br>
endif<br>
sm2:<br>
if m%10&lt;3 then<br>
m=m+6<br>
goto sme<br>
endif<br>
if m%10&gt;5 then<br>
m=m-6<br>
endif<br>
sme:<br>
print m,"u=";u<br>
gosub mp<br>
l=v<br>
goto main<br>
</code></pre></blockquote>

<h2>delete moves</h2>
Remove move from store because we lost - no use<br>
<pre><code><br>
delm:<br>
for v=l to p-1<br>
set S[v]=S[v+1]<br>
next v<br>
</code></pre>

<h2>win or loose - its playing the game that's important</h2>
<pre><code><br>
yw:<br>
print "??"<br>
print "you win!"<br>
goto ag<br>
yl:<br>
print "you loose - haha"<br>
ag:<br>
print "Another game? press 1"<br>
wait key<br>
k=$kir<br>
if k=`1 or k=12 then newg<br>
end<br>
</code></pre>

<h2>check move</h2>
Set return value in e, e=0 good, e=1 bad<br>
<pre><code><br>
cm:<br>
e=1<br>
f=m/10<br>
g=m mod 10<br>
if (B[f]&lt;&gt;T) or (B[G]=t) then<br>
return<br>
endif<br>
if B[G]=0 then<br>
'move - must be straight<br>
if ((t=c) and (g=f+3)) or ((t=h) and (g=f-3)) then good<br>
else<br>
'take - must be diagonal<br>
if ((t=c) and ((g=f+4) or (g=g+2)) and ((g/3-f/3)=1)) then good<br>
if ((t=h) and ((g=f-4) or (g=f-2)) and ((f/3-g/3)=1)) then good<br>
endif<br>
return<br>
good:<br>
e=0<br>
return<br>
</code></pre>

<h2>move piece</h2>
Move in variable m. 63 means move from location 6 to location 3<br>
<pre><code><br>
mp:<br>
B[m/10]=0<br>
B[m%10]=t<br>
return<br>
</code></pre>