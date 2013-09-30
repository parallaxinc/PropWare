CFLAGS = -Os

PRJ = SPI_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o spi.o spi_as.o
BOARD = QUICKSTART
MODEL = lmm

# Insert your own path here - it should be the same directory that contains "common.mk"
PROPWARE_PATH = /home/david/PropWare

# Optionally, specify where your compiler is installed
PREFIX = /opt/parallax

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
