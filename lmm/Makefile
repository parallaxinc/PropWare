# Build all source code files within PropWare for easy testing of compilation errors
PRJ = PropWare

LIBNAME = $(PRJ)
OBJS = PropWare.o spi.o spi_as.o sd.o l3g.o mcp300x.o hd44780.o
BOARD = QUICKSTART
MODEL = lmm
CFLAGS = -Os

# Insert your own path here - it should be the same directory that contains "common.mk"
ifndef PROPWARE_PATH
	PROPWARE_PATH = ..
endif

# Optionally, specify where your compiler is installed
ifndef PROPGCC_PREFIX
	# Default = /opt/parallax
	PROPGCC_PREFIX = /path/to/compiler/directory
endif

include $(PROPWARE_PATH)/common.mk
