# @file     Makefile
#
# @author   David Zemon
# @project  SPI_Demo

PRJ = SPI_Demo

NAME = $(PRJ)
OBJS = $(PRJ).o
BOARD = QUICKSTART
MODEL = lmm
CFLAGS = -Os

# Insert your own path here - it should be the same directory that contains "common.mk"
PROPWARE_PATH ?= ../../..

# Optionally, specify where your compiler is installed
# Default = /opt/parallax
PROPGCC_PREFIX ?= /path/to/compiler/directory

all: $(NAME).elf

include $(PROPWARE_PATH)/common.mk
