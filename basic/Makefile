###############################################################################
# Makefile for the BASIC project 
###############################################################################
#
#

## General Flags
PROJECT = Basic
MCU = atmega128
TARGET = Basic.elf

## Attempt to tell what platform we're on, to pick the right avr-gcc
ifdef SHELL
	# looks like Unix (inc. Mac) or Linux
	CC = avr-gcc
else
	# must be Windows
	CC = avr-gcc.exe
endif

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON) -D AVR 
CFLAGS += -Wall -gdwarf-2 -Os -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

REV = $(shell svnversion -n .)
CFLAGS += -D VER='"Basic v=$(REV)\r\nCommands: i r l c z q s V R F $\r\n"'

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o uart.o dcmp.o buffer.o rprintf.o adc.o ir.o accelerometer.o basic.o edit.o wckmotion.o fix_fft.o express.o functions.o lists.o matrix.o commands.o network.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Basic.hex Basic.eep Basic.lss size

## Compile
%.o: %.c
#	$(CC) $(INCLUDES) $(CFLAGS) -DPSD_SENSOR -c  $<
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	
	
##Link
$(TARGET): $(OBJECTS)
	echo "REVISION = " $(REV)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}
	#@avr-size ${TARGET}

install: ${TARGET}
	@echo
	@echo "cp Basic.hex Basic_$(REV).hex"
	@java -jar /home/phil/Desktop/RBCLoader/RBCLoader.jar Basic.hex 


## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Basic.elf dep/* Basic.hex Basic.eep Basic.lss Basic.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

