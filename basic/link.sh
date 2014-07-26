#
FILES="avrlibdefs.h \
avrlibtypes.h \
basic.c \
cmap.c \
cmap.h \
commands.c \
edit.c \
edit.h \
e-motion.h \
express.c \
express.h \
fix_fft.c \
functions.c \
functions.h \
HunoBasic.h \
ir.c \
jpeg.c \
linux.c \
linux.h \
linux-remote.c \
lists.c \
lists.h \
macro.h \
main.c \
main.h \
fmatrix.c \
fnetwork.c \
matrix.c \
network.c \
trex.c \
trex.h \
TRexpp.h \
oexpress.c \
oexpress.h \
ocells.c  \
ocells.h  \
odict.c  \
odict.h \
ofunction.c  \
ofunction.h \
oobj.c \
oobj.h \
oimg.c \
oimg.h \
ostack.c \
ostack.h \
ostring.c \
ostring.h \
rbmread.c \
rbmread.h \
opflow.c \
opflow.h \
mem.h \
mem.c \
oparticles.c \
oparticles.h \
harris.h \
harris.c \
kmeans.h \
kmeans.c \
fmatrix.h"

SFILE="class.txt  image.c  info.txt   Face.pgm   haar.h    image.h     stdio-wrapper.c haar.cpp  main.cpp  Makefile   rectangles.cpp  stdio-wrapper.h"


DIR="/home/phil/Desktop/basic"

mkdir -p $DIR/vj_cpp

for i in $SFILE
do
echo $DIR/vj_cpp/$i
rm vj_cpp/$i
ln -s "$DIR/vj_cpp/$i" vj_cpp/$i
done



for i in $FILES
do
echo $DIR/$i
rm $i
ln -s "$DIR/$i" $i
done

rm Makefile
ln -s "$DIR/Makefile-rbasic-src" Makefile

cd ..
rm Makefile
ln -s "$DIR/Makefile-rbasic" Makefile
