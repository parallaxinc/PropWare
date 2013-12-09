CFLAGS = -Os

PRJ = MAX6675_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o
BOARD = QUICKSTART
MODEL = lmm

# Insert your own path here - it should be the same directory that contains "common.mk"
ifndef PROPWARE_PATH
	PROPWARE_PATH = ../../..
endif

# Optionally, specify where your compiler is installed
ifndef PROPGCC_PREFIX
	# Default = /opt/parallax
	PROPGCC_PREFIX = /path/to/compiler/directory
endif

dummy := $(shell test -d Debug || mkdir -p Debug)

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
