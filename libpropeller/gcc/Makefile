#
# The following parameters can be defined:
# ROOT: the path to the Propeller GCC folder (propgcc, most likely)
# MODEL: the memory model to use.
# DEFINEFLAGS: ... ???

# If Unit tests are being run, the #define UNIT_TEST flag is set.

ifndef MODEL
MODEL = cmm
endif

#If no root given, make best guess
ifndef ROOT
ROOT = ../../
#ROOT=$(shell root=$$(pwd); while [ "$$(basename $$root)" != "libpropeller" ]; do root=$$(dirname $$root); done; echo $$root"/")
endif

ifndef DEFINEFLAGS
DEFINEFLAGS =
endif

ASM=$(wildcard *.S)

UNITYPATH = ${ROOT}libpropeller/unity_tools/
BOARDCONFIGURATIONPATH = ${ROOT}libpropeller/board/

INCLUDE = -I${UNITYPATH} -I${BOARDCONFIGURATIONPATH} -I../ -I ../../ -I${ROOT}

#-Wa,-alh,-L  == output to STDOUT the generated assembly, with C interspersed

DEBUGFLAGS = -Wall -fverbose-asm #-Wa,-alhm,-L#-pedantic #-fstack-usage #-save-temps #-ftime-report -fmem-report #-Q

CXX = /opt/parallax/bin/propeller-elf-g++
CLD = propeller-elf-ld
#-Wall

INLINEFLAGS = #-fno-default-inline

OPTIMIZEFLAGS = -pipe -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-rtti -m32bit-doubles -mno-fcache -fpermissive
CXXFLAGS = -m${MODEL} ${INCLUDE} ${DEBUGFLAGS} ${DEFINEFLAGS} ${OPTIMIZEFLAGS} ${INLINEFLAGS} -std=gnu++0x -fno-strict-aliasing 
CLDFLAGS = -Wl,--gc-sections #-Wl,--print-gc-sections #-Wl,--verbose #-Wl,--script=${GCCUTILITIESPATH}main.ld 

Unit:
	ruby ${UNITYPATH}generate_test_runner.rb
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -o main.elf -D UNIT_TEST -D SINGLE_TRANSLATION_UNIT ${UNITYPATH}unity.cpp *.generate.cpp ${ASM}
	rm -f *.generate.*

