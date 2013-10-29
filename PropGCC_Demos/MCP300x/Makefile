CFLAGS = -Os

PRJ = MCP300x_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o mcp300x.o spi.o spi_as.o PropWare.o
BOARD = QUICKSTART
MODEL = lmm

# Insert your own path here - it should be the same directory that contains "common.mk"
PROPWARE_PATH = /home/david/External/Kits/Embedded/Parallax/Library/PropWare

# Optionally, specify where your compiler is installed
PREFIX = /home/david/reusable/SimpleIDE-0-9-40/parallax

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
