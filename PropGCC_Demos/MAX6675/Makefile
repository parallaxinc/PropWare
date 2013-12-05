CFLAGS = -Os

PRJ = MAX6675_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o hd44780.o spi.o spi_as.o PropWare.o max6675.o
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

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
