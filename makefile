#name of the produced executable
PRG		= weather
SRC_DIR		= src
OBJ_DIR		= obj
SRC		= $(wildcard $(SRC_DIR)/*.c)
OBJ		= $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) 

#name of the target mcu
MCU_TARGET     = atmega328p
OPTIMIZE       = -Os
#DF_CPU is the cpu speed of the target. 
DEFS           = -DF_CPU=16000000L
INC            = -Iinclude/ -I/usr/avr/include/ -I/home/brandon/usr/include/ArduinoCore-avr/cores/arduino -I/home/brandon/usr/include/ArduinoCore-avr/variants/mega/
LIBS           =

TARGET_DIR     = bld

# You should not have to change anything below here.

CC             = avr-gcc

CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(INC)
LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

#all: $(PRG).elf lst text eeprom
all: $(PRG).elf lst text

listen:
	picocom -b 19200 /dev/ttyACM0

log:
	@echo "Forcing reset"
	python scripts/reset.py "/dev/ttyACM0"
	@echo "Logging to data.csv"
	@echo "ctrl-a then ctrl-q to quit."
	picocom --quiet -b 19200 /dev/ttyACM0 > data.csv

stats:
	avr-size -C --mcu=atmega328 $(PRG).elf 

upload:
	avrdude -v -p atmega328p -c arduino -P /dev/ttyACM0 -b 115200 -D -U flash:w:$(PRG).hex:i


$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
	rm -rf obj/*.o

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@

# Every thing below here is used by avr-libc's build system and can be ignored
# by the casual user.

JPEGFILES               = stdiodemo-setup.jpg

JPEG2PNM                = jpegtopnm
PNM2EPS                 = pnmtops
JPEGRESOLUTION          = 180
EXTRA_CLEAN_FILES       = *.hex *.bin *.srec *.eps

dox: ${JPEGFILES:.jpg=.eps}

%.eps: %.jpg
	$(JPEG2PNM) $< |\
	$(PNM2EPS) -noturn -dpi $(JPEGRESOLUTION) -equalpixels \
	> $@
