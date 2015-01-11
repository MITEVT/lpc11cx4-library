include makefile.conf
NAME=blink

STARTUP_DEFS=-D__STARTUP_CLEAR_BSS -D__START=main

# Need following option for LTO as LTO will treat retarget functions as
# unused without following option
CFLAGS+=-fno-builtin

LDSCRIPTS=-L. -L$(BASE)/ldscripts -T gcc.ld

INC = -I$(BASE)/CMSIS/CM0/DeviceSupport/NXP/LPC11xx -I$(BASE)/CMSIS/CM0/CoreSupport

LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

$(NAME)-$(CORE).axf: $(NAME).c $(STARTUP)
	$(CC) $^ $(CFLAGS) $(INC) $(LFLAGS) -o $@

clean:
	rm -f *.axf *.map *.o
