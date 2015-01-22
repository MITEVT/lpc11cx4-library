NXP_BASE=../..

include $(NXP_BASE)/makefile.conf
NAME=main

STARTUP_DEFS=-D__STARTUP_CLEAR_BSS -D__START=main -DRAM_MODE=1

# Need following option for LTO as LTO will treat retarget functions as
# unused without following option
CFLAGS+=-fno-builtin

LDSCRIPTS=-L. -L $(NXP_BASE)/ldscripts -T gcc.ld

# Driver code
INC_DRIVER= -I$(NXP_BASE)/Drivers/inc/CANopen/ -I$(NXP_BASE)/Drivers/src/CANopen/

INC = $(INC_DRIVER) $(INC_DEVICE)

LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

TARGET=main-CM0

all: $(TARGET).hex

%.hex: %.axf
	arm-none-eabi-objcopy -O ihex $^ $@;

$(NAME)-$(CORE).axf: $(NAME).c $(STARTUP) $(SYSTEM)
	$(CC) $^ $(CFLAGS) $(INC) $(LFLAGS) -o $@

clean:
	rm -f *.axf *.map *.o *.hex
