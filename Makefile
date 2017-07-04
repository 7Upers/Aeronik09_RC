F_CPU = 16000000UL
CC = /usr/bin/avr-gcc
#-Wall enable all warnings
#-mcall-prologues convert code of functions (binnary will be smaller)
CFLAGS = -Os -Wall -mcall-prologues -mmcu=atmega328p
#some defines
CFLAGS += -DF_CPU=$(F_CPU)
OBJ2HEX = /usr/bin/avr-objcopy
UISP = /usr/bin/avrdude
FLASHER = arduino
PORT = /dev/ttyUSB0
SPEED = 57600
TARGET = main

#main
main.hex : $(TARGET).elf
	@echo 'converting'
	$(OBJ2HEX) -R .eeprom -O ihex $(TARGET).elf $(TARGET).hex

main.elf : $(TARGET).o lib/uart.o lib/ds18b20.o
	@echo 'linking'
	$(CC) $(CFLAGS) -o $(TARGET).elf $(TARGET).o lib/uart.o lib/ds18b20.o

main.o : $(TARGET).c
	@echo 'compilling'
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c

lib/uart.o : lib/uart.c lib/uart.h
	@echo 'uart'
	$(CC) $(CFLAGS) -c -o lib/uart.o lib/uart.c

lib/ds18b20.o : lib/ds18b20.c lib/ds18b20.h
	@echo 'ds18b20'
	$(CC) $(CFLAGS) -c -o lib/ds18b20.o lib/ds18b20.c

prog : $(TARGET).hex
	@echo 'flashing'
	$(UISP) -F -V -c $(FLASHER) -P $(PORT) -b $(SPEED) -p m328p -U flash:w:$(TARGET).hex:a

clean :
	@echo 'cleaning'
	rm -f *.hex *.elf *.o lib/*.o

.SILENT: fuse
fuse:
	@echo -e 'get fuse bits\nhfuse\nlfuse\nefuse'
	$(UISP) -F -V -c $(FLASHER) -P $(PORT) -b $(SPEED) -p m328p -U hfuse:r:-:h -U lfuse:r:-:h -U efuse:r:-:h 2>/dev/null
