DEVICE = atmega328p
MCU    = atmega328p
F_CPU  = 16000000
TARGET = manchester

CC      = avr-gcc
OBJCOPY = avr-objcopy

INCLUDES = -I./ -I/usr/lib/avr/include

CFLAGS   += -std=c99
CFLAGS   += -Wall
CFLAGS   += -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS   += -Os
CFLAGS   += -Wold-style-definition
CFLAGS   += $(INCLUDES)

LDFLAGS  = -Wl,-gc-sections -Wl,-relax

OBJECT_FILES = main.o usart.o manchester.o

.PHONY: Release program fuses all size clean doc

Release: all
all: $(TARGET).hex

clean:
	rm -rf *.o *.hex *.obj

doc:
	doxygen

###############################################################################
program: $(TARGET).hex
	avrdude -u -c avrispmkII -p $(DEVICE) -U flash:w:"$(TARGET).hex":a

# fuses from https://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p&LOW=DE&HIGH=DA&EXTENDED=FD&LOCKBIT=FF
fuses:
	avrdude -u -c avrispmkII -p $(DEVICE) -U lfuse:w:0xDE:m -U  hfuse:w:0xDA:m -U efuse:w:0xFD:m -U lock:w:0xFF:m

###############################################################################

%.hex: %.obj
	avr-size -C --mcu=$(MCU) $(TARGET).obj
	$(OBJCOPY) -R .eeprom -O ihex $< $@

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@
