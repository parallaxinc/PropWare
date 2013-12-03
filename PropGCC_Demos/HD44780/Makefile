CFLAGS = -Os

PRJ = HD44780_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o hd44780.o PropWare.o
BOARD = QUICKSTART
MODEL = lmm

# Insert your own path here - it should be the same directory that contains "common.mk"
ifndef PROPWARE_PATH
	PROPWARE_PATH = /path/to/PropWare
endif

# Optionally, specify where your compiler is installed
ifndef PROPGCC_PREFIX
	PROPGCC_PREFIX = /path/to/compiler/directory  # Default = /opt/parallax
endif

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk