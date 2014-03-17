# Build the PropWare library with the extended memory model (XMM)
PRJ = PropWare_xmm

LIBNAME = $(PRJ)
OBJS = PropWare.o spi.o spi_as.o sd.o l3g.o mcp3000.o hd44780.o max6675.o \
		safeSpi.o
BOARD = QUICKSTART
MODEL = xmm
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
