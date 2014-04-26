# Build the Simple library with the extended memory model - single (XMM-Single)
PRJ = Simple_xmm-single

LIBNAME = $(PRJ)
include ../simpleObjects.mk
MODEL = xmm-single
CFLAGS = -Os

# Insert your own path here - it should be the same directory that contains "common.mk"
ifndef PROPWARE_PATH
	PROPWARE_PATH=..
endif

# Optionally, specify where your compiler is installed
ifndef PROPGCC_PREFIX
	# Default = /opt/parallax
	PROPGCC_PREFIX = /path/to/compiler/directory
endif

include ../simple.mk
