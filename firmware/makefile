DEVICE = atmega328p
MCU    = atmega328p
F_CPU  = 16000000
TARGET = manchester

CC      = avr-gcc
OBJCOPY = avr-objcopy

INCLUDES = -I./ -I/usr/lib/avr/include
CFLAGS   = -std=c99 -g -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(INCLUDES) -Os
LDFLAGS  = -Wl,-gc-sections -Wl,-relax

OBJECT_FILES = main.o usart.o manchester.o

.PHONY: Release program fuses all size clean doc

Release: all
all: $(TARGET).hex

clean:
	rm -rf *.o *.hex *.obj

%.hex: %.obj
	avr-size -C --mcu=$(MCU) $(TARGET).obj
	$(OBJCOPY) -R .eeprom -O ihex $< $@

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@
