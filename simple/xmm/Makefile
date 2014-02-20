# Build the Simple library with the extended memory model (XMM)
PRJ = Simple_xmm

LIBNAME = $(PRJ)
OBJS = serial_rxtx.o \
    getDec.o \
    print.o \
    putStr.o \
    serial_open.o \
    fdserial.o \
    getBin.o \
    libservoAux.o \
    printNumber.o \
    libvgatext.o \
    doscanf.o \
    getHex.o \
    wavplayer.o \
    simplei2c.o \
    simpleterm_close.o \
    writeBinDigits.o \
    libservo.o \
    libfdserial.o \
    adcDCpropab.o \
    libadcDCpropab.o \
    dprint.o \
    ping.o \
    floatToString.o \
    servoAux.o \
    sscan.o \
    putChar.o \
    libdacctr.o \
    stringToFloat.o \
    writeHexDigits.o \
    libwavplayer.o \
    serial_close.o \
    dacctr.o \
    putFloat.o \
    writeFloatPrecision.o \
    mx2125.o \
    mstimer.o \
    writeLine.o \
    adcDCVpropab.o \
    libadcACpropab.o \
    getFloat.o \
    gets.o \
    vga.o \
    putDec.o \
    putHex.o \
    putBin.o \
    getStr.o \
    putLine.o \
    dscan.o \
    libcompass3d.o \
    getChar.o \
    writeFloat.o \
    writeDec.o \
    vgatext.o \
    sprint.o \
    libmstimer.o \
    writeBin.o \
    scan.o \
    compass3d.o \
    libsimpletools.o \
    mx2125_accel.o \
    mx2125_rotate.o \
    simpleterm.o \
    adcACpropab.o \
    writeChar.o \
    libmx2125.o \
    libsimpletext.o \
    libservodiffdrive.o \
    writeStr.o \
    libping.o \
    safe_gets.o \
    dosprint.o \
    servodiffdrive.o \
    writeDecDigits.o \
    putln.o \
    writeHex.o \
    libsimplei2c.o \
    scanf_getl.o \
    mx2125_tilt.o \
    scanf_getf.o \
    writeStrDigits.o \
    fdserial_utils.o \
    servo.o
MODEL = xmm
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
