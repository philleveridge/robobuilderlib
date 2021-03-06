###############################################################################
# Makefile for the project RobobuilderIR
###############################################################################

## General Flags
PROJECT = Robobuilderlib
MCU = atmega128
TARGET = Robobuilderlib.elf

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
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=RobobuilderIR.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o uart.o buffer.o motion.o rprintf.o adc.o ir.o accelerometer.o idle_mode.o experimental_mode.o charge_mode.o serialslave_mode.o wck.o experimental_bin_mode.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Robobuilderlib.hex Robobuilderlib.eep Robobuilderlib.lss size

## Compile
%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<
	
	
##Link
$(TARGET): $(OBJECTS)
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

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Robobuilderlib.elf dep/* Robobuilderlib.hex Robobuilderlib.eep Robobuilderlib.lss Robobuilderlib.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

