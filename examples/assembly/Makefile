#=============================================================================#
# ARM makefile
#
# author: Freddie Chopin, http://www.freddiechopin.info/
# last change: 2012-01-08
#
# this makefile is based strongly on many examples found in the network
#=============================================================================#

#=============================================================================#
# toolchain configuration
#=============================================================================#

TOOLCHAIN = arm-none-eabi-

CC = $(TOOLCHAIN)gcc
AS = $(TOOLCHAIN)gcc -x assembler-with-cpp
OBJCOPY = $(TOOLCHAIN)objcopy
OBJDUMP = $(TOOLCHAIN)objdump
SIZE = $(TOOLCHAIN)size
RM = rm -f

#=============================================================================#
# project configuration
#=============================================================================#

# project name
PROJECT = assembly

# core type
CORE = cortex-m0

# linker script
LD_SCRIPT = gcc.ld

# output folder (absolute or relative path, leave empty for in-tree compilation)
OUT_DIR = bin

# C definitions
C_DEFS = -DCORE_M0

# ASM definitions
AS_DEFS = -D__STARTUP_CLEAR_BSS -DCORE_M0

# include directories (absolute or relative paths to additional folders with
# headers, current folder is always included)
INC_DIRS = ../../lpc_chip_11cxx_lib/inc inc/

# library directories (absolute or relative paths to additional folders with
# libraries)
LIB_DIRS = 

# libraries (additional libraries for linking, e.g. "-lm -lsome_name" to link
# math library libm.a and libsome_name.a)
LIBS =

# additional directories with source files (absolute or relative paths to
# folders with source files, current folder is always included)
SRCS_DIRS = ../../lpc_chip_11cxx_lib/src src/

# extension of C files
C_EXT = c

# wildcard for C source files (all files with C_EXT extension found in current
# folder and SRCS_DIRS folders will be compiled and linked)
C_SRCS = $(wildcard $(patsubst %, %/*.$(C_EXT), . $(SRCS_DIRS)))

# extension of ASM files
AS_EXT = S

# wildcard for ASM source files (all files with AS_EXT extension found in
# current folder and SRCS_DIRS folders will be compiled and linked)
AS_SRCS = $(wildcard $(patsubst %, %/*.$(AS_EXT), . $(SRCS_DIRS)))

# optimization flags ("-O0" - no optimization, "-O1" - optimize, "-O2" -
# optimize even more, "-Os" - optimize for size or "-O3" - optimize yet more) 
OPTIMIZATION = -O2

# set to 1 to optimize size by removing unused code and data during link phase
REMOVE_UNUSED = 1

# define warning options here
C_WARNINGS = -Wall -Wstrict-prototypes -Wextra

# C language standard ("c89" / "iso9899:1990", "iso9899:199409",
# "c99" / "iso9899:1999", "gnu89" - default, "gnu99")
C_STD = gnu89

#=============================================================================#
# Write Configuration
#=============================================================================#

COMPORT = $(word 1, $(wildcard /dev/tty.usbserial-*) $(wildcard /dev/ttyUSB*))
BAUDRATE = 115200
CLOCK_OSC = 0

#=============================================================================#
# Lint Configuration
#=============================================================================#

MAX_LINE_SIZE = 140

#=============================================================================#
# set the VPATH according to SRCS_DIRS *Very Important*
#=============================================================================#

VPATH = $(SRCS_DIRS)

#=============================================================================#
# when using output folder, append trailing slash to its name
#=============================================================================#

ifeq ($(strip $(OUT_DIR)), )
	OUT_DIR_F =
else
	OUT_DIR_F = $(strip $(OUT_DIR))/
endif

#=============================================================================#
# various compilation flags
#=============================================================================#

# core flags
CORE_FLAGS = -mcpu=$(CORE) -mthumb

# flags for C compiler
C_FLAGS = -fdiagnostics-color=always -std=$(C_STD) -g -ggdb3 -fverbose-asm -Wa,-ahlms=$(OUT_DIR_F)$(notdir $(<:.$(C_EXT)=.lst)) -DUART_BAUD=$(BAUDRATE)
#			add diagnostic colors		c standard	debug(?) extra comments	

# flags for assembler
AS_FLAGS = -g -ggdb3 -Wa,-amhls=$(OUT_DIR_F)$(notdir $(<:.$(AS_EXT)=.lst))

# flags for linker
LD_FLAGS = -T$(LD_SCRIPT) -g -nostartfiles -Wl,-Map=$(OUT_DIR_F)$(PROJECT).map,--cref

# flags for lint
LINT_FLAGS = -rc LONG_LINE=$(MAX_LINE_SIZE)

# process option for removing unused code
ifeq ($(REMOVE_UNUSED), 1)
	# enable garbage collection of unused sections
	LD_FLAGS += -Wl,--gc-sections
	# put functions and data into their own sections
	OPTIMIZATION += -ffunction-sections -fdata-sections
endif

#=============================================================================#
# do some formatting
#=============================================================================#

C_OBJS = $(addprefix $(OUT_DIR_F), $(notdir $(C_SRCS:.$(C_EXT)=.o)))
AS_OBJS = $(addprefix $(OUT_DIR_F), $(notdir $(AS_SRCS:.$(AS_EXT)=.o)))
T_OBJS = $(AS_OBJS) $(C_OBJS) $(USER_OBJS)
OBJS = $(subst bin/startup_LPC11xx.o,, $(T_OBJS))
INC_DIRS_F = -I. $(patsubst %, -I%, $(INC_DIRS))
LIB_DIRS_F = $(patsubst %, -L%, $(LIB_DIRS))

ELF = $(OUT_DIR_F)$(PROJECT).elf
HEX = $(OUT_DIR_F)$(PROJECT).hex
BIN = $(OUT_DIR_F)$(PROJECT).bin
LSS = $(OUT_DIR_F)$(PROJECT).lss
DMP = $(OUT_DIR_F)$(PROJECT).dmp

# format final flags for tools, request dependancies for C and asm
C_FLAGS_F = $(CORE_FLAGS) $(OPTIMIZATION) $(C_WARNINGS) $(C_FLAGS) $(C_DEFS) -MD -MP -MF $(OUT_DIR_F)$(@F:.o=.d) $(INC_DIRS_F)
AS_FLAGS_F = $(CORE_FLAGS) $(AS_FLAGS) $(AS_DEFS) -MD -MP -MF $(OUT_DIR_F)$(@F:.o=.d) $(INC_DIRS_F)
LD_FLAGS_F = $(CORE_FLAGS) $(LD_FLAGS) $(LIB_DIRS_F)

#contents of output directory
GENERATED = $(wildcard $(patsubst %, $(OUT_DIR_F)*.%, bin d dmp elf hex lss lst map o))

#=============================================================================#
# make all
#=============================================================================#

all : make_output_dir $(ELF) $(LSS) $(DMP) $(HEX) $(BIN) print_size

# make object files dependent on Makefile
$(OBJS) : Makefile
# make .elf file dependent on linker script
$(ELF) : $(LD_SCRIPT)

#-----------------------------------------------------------------------------#
# linking - objects -> elf
#-----------------------------------------------------------------------------#

$(ELF) : $(OBJS)
	@echo 'Linking target: $(ELF)'
	$(CC) $(LD_FLAGS_F) $(OBJS) $(LIBS) -o $@
	@echo ' '

#-----------------------------------------------------------------------------#
# compiling - C source -> objects
#-----------------------------------------------------------------------------#

$(OUT_DIR_F)%.o : %.$(C_EXT)
	@echo 'Compiling file: $<'
	$(CC) -c $(C_FLAGS_F) $< -o $@
	@echo ' '

#-----------------------------------------------------------------------------#
# assembling - ASM source -> objects
#-----------------------------------------------------------------------------#

$(OUT_DIR_F)%.o : %.$(AS_EXT)
	@echo 'Assembling file: $<'
	$(AS) -c $(AS_FLAGS_F) $< -o $@
	@echo ' '

#-----------------------------------------------------------------------------#
# memory images - elf -> hex, elf -> bin
#-----------------------------------------------------------------------------#

$(HEX) : $(ELF)
	@echo 'Creating IHEX image: $(HEX)'
	$(OBJCOPY) -O ihex $< $@
	@echo ' '

$(BIN) : $(ELF)
	@echo 'Creating binary image: $(BIN)'
	$(OBJCOPY) -O binary $< $@
	@echo ' '

#-----------------------------------------------------------------------------#
# memory dump - elf -> dmp
#-----------------------------------------------------------------------------#

$(DMP) : $(ELF)
	@echo 'Creating memory dump: $(DMP)'
	$(OBJDUMP) -x --syms $< > $@
	@echo ' '

#-----------------------------------------------------------------------------#
# extended listing - elf -> lss
#-----------------------------------------------------------------------------#

$(LSS) : $(ELF)
	@echo 'Creating extended listing: $(LSS)'
	$(OBJDUMP) -S $< > $@
	@echo ' '

#-----------------------------------------------------------------------------#
# print the size of the objects and the .elf file
#-----------------------------------------------------------------------------#

print_size :
	@echo 'Size of modules:'
	$(SIZE) -B -t --common $(OBJS) $(USER_OBJS)
	@echo ' '
	@echo 'Size of target .elf file:'
	$(SIZE) -B $(ELF)
	@echo ' '

#-----------------------------------------------------------------------------#
# create the desired output directory
#-----------------------------------------------------------------------------#

make_output_dir :
	$(shell mkdir $(OUT_DIR_F) 2>/dev/null)

#-----------------------------------------------------------------------------#
# Write to flash of chip
#-----------------------------------------------------------------------------#

writeflash: all
	@echo "Writing to" $(COMPORT)
	lpc21isp -NXPARM -control $(HEX) $(COMPORT) $(BAUDRATE) $(CLOCK_OSC)

#-----------------------------------------------------------------------------#
# Open up in picocom
#-----------------------------------------------------------------------------#

com:
	@echo "Opening" $(COMPORT)
	lpc21isp -NXPARM -control -termonly $(HEX) $(COMPORT) $(BAUDRATE) $(CLOCK_OSC)

#=============================================================================#
# make clean
#=============================================================================#

clean:
ifeq ($(strip $(OUT_DIR_F)), )
	@echo 'Removing all generated output files'
else
	@echo 'Removing all generated output files from output directory: $(OUT_DIR_F)'
endif
ifneq ($(strip $(GENERATED)), )
	$(RM) $(GENERATED)
else
	@echo 'Nothing to remove...'
endif

#=============================================================================#
# global exports
#=============================================================================#

.PHONY: all clean dependents

.SECONDARY:

# include dependancy files
-include $(DEPS)
