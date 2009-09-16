SDCCLIB=C:\Program\ Files\SDCC\lib\pic
LKR=C:\Program\ Files\gputils\lkr
SIMDIR=C:\Program\ Files\gpsim\bin

PRJ=cylon_v2
CC=sdcc
CPU=16f648A

OBJECTS=main.o cylon.o interupt.o sound.o

LIB_INCLUDES=-I"C:\Program Files\SDCC\lib"  -I"C:\Program Files\SDCC\lib\pic" -w -r 
LIB=libsdcc.lib pic$(CPU).lib 


CFLAGS=-V -mpic14 -p$(CPU)
#CLFAGS=-mpic14 -p16f648A --no-peep --nogcse --noinvariant --noinduction  --no-pcode-opt

.c.o:
	$(CC) $(CFLAGS)  -c $<
	
$(PRJ).hex : $(OBJECTS)
	gplink $(LIB_INCLUDES) -o $(PRJ).hex $(OBJECTS) $(LIB)

clean:
	rm *.o *.asm *.lst *.adb *.cod
	
sim:
	$(SIMDIR)\gpsim -s $(PRJ).cod -c netlist.stc

