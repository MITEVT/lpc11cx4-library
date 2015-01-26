NXP_BASE=../..

###
# Directory Structure
BINDIR=bin
SRCDIR=src
BUILDDIR=build

DRIVERS= 	gpio_11xx_2.c \

DRIVERS_SRC=$(addprefix $(CHIP_SRC)/, $(DRIVERS)) 

ASOURCES=$(shell find -L $(SRCDIR) -name '*.S')
ASOURCES+=$(STARTUP)
CSOURCESI=$(shell find -L $(SRCDIR) -name '*.c')
CSOURCES=$(CSOURCESI) $(DRIVERS_SRC)

OBJECTS=$(ASOURCES:%.S=%.o)
OBJECTS+=$(CSOURCES:%.c=%.o)



BINELF=outp.elf
BINHEX=outp.hex

include $(NXP_BASE)/makefile.conf

STARTUP_DEFS=-D__STARTUP_CLEAR_BSS -D__START=main -DRAM_MODE=1


# Need following option for LTO as LTO will treat retarget functions as
# unused without following option
CFLAGS+=-fno-builtin -DCORE_M0

LDSCRIPTS=-L. -L $(NXP_BASE)/ldscripts -T gcc.ld

INC = $(CHIP_INC)

DEFINE= -DCORE_M0
CFLAGS= -c $(MCFLAGS) $(INC) $(DEFINE) $(USE_NOHOST)

LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)


# System code
SYSTEM=sysinit.c

TARGET=$(NAME)-CM0

all: release

release: $(BINDIR)/$(BINHEX)

$(BINDIR)/$(BINHEX): $(BINDIR)/$(BINELF)
	$(CP) -O ihex $< $@
	@echo "Objcopy from ELF to IHEX complete!\n"

$(BINDIR)/$(BINELF): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!\n"
	$(SIZE) $(BINDIR)/$(BINELF)

%.o: %.c $(STARTUP)
	$(CC) $(CFLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

%.o: %.S
	$(CC) $(CFLAGS) $< -o $@
	@echo "Assambled "$<"!\n"

clean:
	rm -f $(OBJECTS) $(BINDIR)/$(BINELF) $(BINDIR)/$(BINHEX) $(BINDIR)/output.map
