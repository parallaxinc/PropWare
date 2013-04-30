# #########################################################
# This makefile fragment builds LMM/XMM/XMMC demo programs
#
# To use it, define:
#  PROPLIB to be the path to this directory
#  NAME to be the name of project
#       - this is used to create the final program $(NAME).elf
#  OBJS to be the object files needed for the project
#  MODEL to lmm, xmm, or xmmc
#  CFLAGS to be desired CFLAGS
#
#  Then set up a default "all" target (normally this will be
#    all: $(NAME).elf
#  and finally
#    include $(PROPLIB)/demo.mk
#
# Copyright (c) 2011 Parallax Inc.
# All rights MIT licensed
# #########################################################

# *********************************************************
# Modification by David Zemon...
#
# !!! NOTE !!!
# All C source files must have an accompanying header file
# in the same directory as the source.
# Without this mod, changes in header files would be ignored
# until the source file was modified (and therefore rebuilt)
#
# *********************************************************

# #########################################################
# Variable Definitions
# #########################################################
# where we installed the propeller binaries and libraries
PREFIX = /opt/parallax

# libgcc directory
LIBGCC = $(PREFIX)/lib/gcc/propeller-elf/4.6.1

ifndef MODEL
MODEL=lmm
endif

ifndef BOARD
BOARD=$(PROPELLER_LOAD_BOARD)
endif

ifneq ($(BOARD),)
BOARDFLAG=-b$(BOARD)
endif

CFLAGS_NO_MODEL := -Wextra $(CFLAGS)
CFLAGS += -m$(MODEL)
CXXFLAGS += $(CFLAGS)
LDFLAGS += -m$(MODEL) -fno-exceptions -fno-rtti
INC += -I/mnt/win-7/Users/Public/Kits/Embedded/Parallax/Library -I/opt/parallax/include 

ifneq ($(LDSCRIPT),)
LDFLAGS += -T $(LDSCRIPT)
endif

# basic gnu tools
CC = propeller-elf-gcc
CXX = propeller-elf-g++
LD = propeller-elf-ld
AS = propeller-elf-as
AR = propeller-elf-ar
OBJCOPY = propeller-elf-objcopy
LOADER = propeller-load

# BSTC program
BSTC=bstc
SPINDIR=.

# #########################################################
# Build Commands
# #########################################################
ifneq ($(NAME),)
$(NAME).elf: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: PropGCC Linker'
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
endif

ifneq ($(LIBNAME),)
lib$(LIBNAME).a: $(OBJS)
	$(AR) rs $@ $(OBJS)
endif

%.o: ../%.c ../%.h
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Compiler'
	$(CC) $(INC) $(CFLAGS) -o $@ -c $<
	@echo 'Finished building: $<'
	@echo ' '
	
%.o: ../%.cpp ../%.h
	@echo 'Building file: $<'
	@echo 'Invoking: PropG++ Compiler'
	$(CC) $(INC) $(CXXFLAGS) -o $@ -c $<
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.s
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Assembler'
	$(CC) $(INC) -o $@ -c $<
	@echo 'Finished building: $<'
	@echo ' '
	
%.o: ../%.S
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Assembler'
	$(CC) $(INC) -o $@ -c $<
	@echo 'Finished building: $<'
	@echo ' '

#
# a .cog program is an object file that contains code intended to
# run in a COG separate from the main program; i.e., it's a COG
# driver that the linker will place in the .text section.
#
%.cog: ../%.c ../%.h
	$(CC) $(INC) $(CFLAGS_NO_MODEL) -mcog -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

%.cog: ../%.cogc ../%.h
	$(CC) $(INC) $(CFLAGS_NO_MODEL) -mcog -xc -r -o $@ $<
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@

#
# a .ecog program is an object file that contains code intended to
# run in a COG separate from the main program; i.e., it's a COG
# driver that the linker will place in the .drivers section which
# gets loaded to high EEPROM space above 0x8000.
#
%.ecog: ../%.c ../%.h
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Compiler'
	$(CC) $(INC) $(CFLAGS_NO_MODEL) -mcog -r -o $@ $<
	@echo 'Renaming: ".text" section'
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@
	@echo 'Finished building: $<'
	@echo ' '

%.ecog: ../%.ecogc ../%.h
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Compiler'
	$(CC) $(INC) $(CFLAGS_NO_MODEL) -mcog -xc -r -o $@ $<
	@echo 'Renaming: ".text" section'
	$(OBJCOPY) --localize-text --rename-section .text=$@ $@
	@echo 'Finished building: $<'
	@echo ' '

%.binary: ../%.elf
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Loader'
	$(LOADER) -s $<
	@echo 'Finished building: $<'
	@echo ' '

%.dat: $(SPINDIR)/%.spin
	@echo 'Building file: $<'
	@echo 'Invoking: bstc'
	$(BSTC) -Ox -c -o $(basename $@) $<
	@echo 'Finished building: $<'
	@echo ' '

%_firmware.o: ../%.dat
	@echo 'Building file: $<'
	@echo 'Invoking: PropGCC Object Copy'
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	@echo 'Finished building: $<'
	@echo ' '

clean:
	rm -f *.o *.elf *.a *.cog *.ecog *.binary

# #########################################################
# how to run
# #########################################################
run: $(NAME).elf
	$(LOADER) $(BOARDFLAG) $(NAME).elf -r -t
