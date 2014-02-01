GCC ?= g++

HEADERS := image.h haar.h stdio-wrapper.h

all: build

build: vj

image.o: image.c $(HEADERS)
	$(GCC) -o $@ -c $<

stdio.o: stdio-wrapper.c $(HEADERS)
	$(GCC) -o $@ -c $<

main.o: main.cpp $(HEADERS)
	$(GCC) -o $@ -c $<

haar.o: haar.cpp $(HEADERS)
	$(GCC) -o $@ -c $<

rectangles.o: rectangles.cpp $(HEADERS)
	$(GCC) -o $@ -c $<

vj: main.o haar.o image.o stdio-wrapper.o rectangles.o
	ar rvs vj.a *.o


run: build
	./vj

clean:
	rm -f vj.a *.o Output.pgm
