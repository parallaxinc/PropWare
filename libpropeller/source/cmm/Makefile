# Build the Simple library with the compressed memory model (CMM)
PRJ = Libpropeller_cmm

LIBNAME = $(PRJ)
include ../libpropellerObjects.mk
MODEL = cmm
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

include ../libpropeller.mk
