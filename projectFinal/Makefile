# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:piano_tiles.elf

#additional rules for files
piano_tiles.elf: ${COMMON_OBJECTS} piano_tiles.o wdt_handler.o buzzer.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd

load: piano_tiles.elf
	msp430loader.sh $^

clean:
	rm -f *.o *.elf
	
buzzer. o: buzzer.h
