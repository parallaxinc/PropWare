CFLAGS = -Os

PRJ = SPI_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o spi.o spi_as.o PropWare.o
BOARD = QUICKSTART
MODEL = lmm

# Insert your own path here - it should be the same directory that contains "common.mk"
PROPWARE_PATH = /home/david/External/Kits/Embedded/Parallax/Library/PropWare

# Optionally, specify where your compiler is installed
PREFIX = /opt/parallax

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
