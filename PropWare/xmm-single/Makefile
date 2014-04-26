# Build the PropWare library with the extended memory model single (XMM-Single)
PRJ = PropWare_xmm-single

LIBNAME = $(PRJ)
BOARD = QUICKSTART
MODEL = xmm-single
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

include ../PropWareObjects.mk
include ../../common.mk
