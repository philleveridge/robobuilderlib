
HEADERS := 

CC = gcc
GCC = g++

all: build

build: oflow


OpticalFlow.o: OpticalFlow.cpp $(HEADERS)
	$(GCC) -o $@ -c $<

oflow:	OpticalFlow.o
	$(AR) rvs oflow.a *.o

clean:
	rm -f oflow.a *.o 
