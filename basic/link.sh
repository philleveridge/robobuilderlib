#
FILES="avrlibdefs.h \
avrlibtypes.h \
basic.c \
cmap.c \
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
oexpress.c"

DIR="/home/phil/Desktop/basic"

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
