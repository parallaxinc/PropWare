# Build the Simple library with the long memory model (LMM)
PRJ = Simple_cog

LIBNAME = $(PRJ)
include ../simpleObjects.mk
MODEL = cog
CFLAGS = -Os

# Insert your own path here - it should be the same directory that contains "common.mk"
PROPWARE_PATH ?= ..

# Optionally, specify where your compiler is installed
# Default = /opt/parallax
PROPGCC_PREFIX ?= /path/to/compiler/directory

include ../simple.mk
