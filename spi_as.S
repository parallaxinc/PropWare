/* File:    spi_as.S
 *
 * Author:  David Zemon
 */

/* Description: SPI routine for Parallax Propeller. Runs in master mode only.
 *
 */

#include <bit_expansion.h>

/* NOTE: These definitions *MUST* match up with the SPI source file "spi.c" */
// Different fuctions supported by this GAS module
#define SPI_FUNC_SEND			0
#define	SPI_FUNC_READ			1
#define SPI_FUNC_CLK			2

#define SPI_BITS_OFFSET			8
#define SPI_MODE_OFFSET			16

#define SPI_FUNC_BITS			BYTE_0					'' Interpret bits 7-0 as a function descriptor
#define SPI_BIT_COUNT_BITS		BYTE_1					'' Interpret bits 15-8 as bit-count descriptor
#define SPI_MODE_BITS			BYTE_2					'' Interpret bits 23-16 as mode descriptor

#define SPI_MSB_PRE				(0 << SPI_MODE_OFFSET)
#define SPI_LSB_PRE				(1 << SPI_MODE_OFFSET)
#define SPI_MSB_POST			(2 << SPI_MODE_OFFSET)
#define SPI_LSB_POST			(3 << SPI_MODE_OFFSET)
#define SPI_LSB_FIRST			(4 << SPI_MODE_OFFSET)
#define	SPI_MSB_FIRST			(5 << SPI_MODE_OFFSET)

						.section spi_as.cog, "ax"
						.cog_ram


						or dira, testLEDs				'' DEBUG: LEDs used for debugging purposes, can be removed later
						// Begin by retreiving all parameters...
entry					call #READ_CMD					'' Read in the pin mask MOSI
						mov mosi, mailbox
						call #READ_CMD					'' Read in the pin number for MOSI
						mov mosiPinNum, mailbox
						call #READ_CMD					'' Read in the pin mask MISO
						mov miso, mailbox
						call #READ_CMD					'' Read in the pin number for MISO
						mov misoPinNum, mailbox
						call #READ_CMD					'' Read in the pin mask SCLK
						mov sclk, mailbox
						call #READ_CMD					'' Read in the clock delay
						mov clkDelay, mailbox
						call #READ_CMD					'' Read in the polarity
						andn outa, mailbox				'' \__ If polarity high, initialize sclk high, else clear the bit
						or outa, mailbox				'' /

						// Followed by setting MOSI & SCLK as outputs and MISO as input; Also set MOSI and MISO high
						or dira, mosi
						or dira, sclk
						andn dira, miso

/*** MAIN LOOP ***/
LOOP					// Retrieve a command
						call #READ_CMD
						mov temp, mailbox
						and temp, spiFuncBits			'' Mask away all bits but the function descriptor

						// Using "jmp" instead of "call" because all functions return by "jmp #LOOP" instead of "ret"
						// If command is "Send"
						cmp temp, #SPI_FUNC_SEND wz
		if_z			jmp #SEND

						// If command is "Read", call function
						cmp temp, #SPI_FUNC_READ wz
		if_z			jmp #READ

						// If command is "Set clock", call function
						cmp temp, #SPI_FUNC_CLK wz
		if_z			jmp #SET_CLOCK

						// Default: Return to loop
						jmp #LOOP

/* FUNCTION: SPIShiftOut() */
SEND					// Interpret the bit count and mode of output for this send command
						mov bitCount, mailbox			'' Initialize 'bitCount' register
						and bitCount, spiBitCountBits	'' Mask off all bits except the bit count
						shr bitCount, #SPI_BITS_OFFSET	'' Shift bit count into the lsb
						mov mode, mailbox				'' Initialize 'mode' register
						and mode, spiModeBits			'' Mask off all bits except the mode

SEND_rd_data			rdlong mailbox, par
						test mailbox, dataMask wz		'' Is BIT_31 cleared? (Implying data vs command)
		if_nz			jmp #SEND_rd_data				'' If not z, value is not data. Try again

						mov data, mailbox				'' Initialize 'data' register

						mov clock, cnt					'' \__Prepare a register with the system counter for use in the *_CLOCK functions
						add clock, clkDelay				'' /
						cmp mode, spiMSBFirst wz		'' Is mode MSB first or LSB first?
		if_z			jmp #msb_first

						// LSB_FIRST Initialization
						mov loopIdx, negOne
						sub bitCount, #1
						jmp #lsb_first

msb_first				// MSB_FIRST Routine
						sub bitCount, #1 wz
						mov temp, data
						shr temp, bitCount
						test temp, #BIT_0 wc
						muxc outa, mosi
						call #POST_CLOCK
		if_nz			jmp #msb_first

						// SEND complete, return to loop
						jmp #SEND_complete

lsb_first				// LSB_FIRST Routine
						add loopIdx, #1
						mov temp, data
						shr temp, loopIdx
						test temp, #BIT_0 wc
						muxc outa, mosi
						call #POST_CLOCK
						cmp bitCount, loopIdx wz
		if_nz			jmp #lsb_first

						// SEND complete, return to loop
SEND_complete			wrlong negOne, par				'' Indicate that send is complete and C-cog can continue execution
						jmp #LOOP

/* FUNCTION: SPIShiftIn() */
READ					// Interpret the bit count and mode of output for this read command
						mov bitCount, mailbox			'' Initialize 'bitCount' register
						and bitCount, spiBitCountBits	'' Mask off all bits except the bit count
						shr bitCount, #SPI_BITS_OFFSET	'' Shift bit count into the lsb
						mov loopIdx, bitCount			'' Create a second storage register for the bit count - used in LSB modes
						mov mode, mailbox				'' Initialize 'mode' register
						and mode, spiModeBits			'' Mask off all bits except the mode
						mov data, #0					'' Clear our the data register, ready for input

						mov clock, cnt
						add clock, clkDelay

						cmp mode, spiMSBPre wz
		if_z			call #msb_pre
						cmp mode, spiLSBPre wz
		if_z			call #lsb_pre
						cmp mode, spiMSBPost wz
		if_z			call #msb_post
						cmp mode, spiLSBPost wz
		if_z			call #lsb_post

						call #WRITE_DATA
						jmp #LOOP

msb_pre					// Read in a value MSB-first with data valid before the clock
						test miso, ina wc
						muxc data, #BIT_0
						shl data, #1
						call #POST_CLOCK
						djnz bitCount, #msb_pre

						shr data, #1
msb_pre_ret				ret

lsb_pre					// Read in a value LSB-first with data valid before the clock
						test miso, ina wc
						muxc data, dataMask				'' Would use BIT_31 here, but it's too large for a second operand. 'dataMask' already contains BIT_31
						shr data, #1 wc					'' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
						call #POST_CLOCK
						djnz loopIdx, #lsb_pre

						rcl data, #1					'' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
						mov temp, #32
						sub temp, bitCount
						shr data, temp
lsb_pre_ret				ret

msb_post				// Read in a value MSB-first with data valid after the clock
						call #PRE_CLOCK
						test miso, ina wc
						muxc data, #BIT_0
						shl data, #1
						djnz bitCount, #msb_post

						shr data, #1
msb_post_ret			ret

lsb_post				// Read in a value LSB-first with data valid after the clock
						call #PRE_CLOCK
						test miso, ina wc
						muxc data, dataMask				'' Would use BIT_31 here, but it's too large for a second operand. 'dataMask' already contains BIT_31
						shr data, #1 wc					'' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
						djnz loopIdx, #lsb_post

						rcl data, #1					'' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
						mov temp, #32
						sub temp, bitCount
						shr data, temp
lsb_post_ret			ret

SET_CLOCK				call #READ_CMD					'' Using READ_CMD instead of READ_DATA because -1 is not a valid clock signal
						mov clkDelay, mailbox
						jmp #LOOP

/* FUNCTION: SPIPostClock() */
POST_CLOCK				waitcnt clock, clkDelay
						xor outa, sclk
						waitcnt clock, clkDelay
						xor outa, sclk
POST_CLOCK_ret			ret

/* FUNCTION: SPIPreClock() */
PRE_CLOCK				xor outa, sclk
						waitcnt clock, clkDelay
						xor outa, sclk
						waitcnt clock, clkDelay
PRE_CLOCK_ret			ret

/* FUNCTION: Loop reading in a value from hub-RAM and store in 'mailbox' ONLY if not -1. */
READ_CMD				rdlong mailbox, par				'' Wait for parameter to be passed in
						cmp mailbox, negOne wz			'' Are we reading a valid parameter?
		if_z			jmp #READ_CMD					'' If not valid, read in a new value
						wrlong negOne, par				'' If value is valid, write -1 back to hub-RAM to indicate read completion
READ_CMD_ret			ret

/* Loop reading in a value from hub-RAM and store in 'mailbox' ONLY if BIT_31 is not set. */
READ_DATA				rdlong mailbox, par
						test mailbox, dataMask wz		'' Is BIT_31 cleared? (Implying data vs command)
		if_nz			jmp #READ_DATA					'' If not z, value is not data. Try again
						wrlong negOne, par				'' Inform C cog that reading is complete
READ_DATA_ret			ret

WRITE_DATA				wrlong data, par
write_loop				rdlong mailbox, par				'' \.
						cmp mailbox, negOne wz			''  |--> Hold GAS execution here until C has signaled read complete by writing -1
		if_nz			jmp #write_loop					'' /
WRITE_DATA_ret			ret

/* Pre-Initialized Values */
negOne					long	-1						'' Used for comparison purposes
spiFuncBits				long	SPI_FUNC_BITS
spiBitCountBits			long	SPI_BIT_COUNT_BITS
spiModeBits				long	SPI_MODE_BITS
dataMask				long	BIT_31
spiMSBPre				long	SPI_MSB_PRE
spiLSBPre				long	SPI_LSB_PRE
spiMSBPost				long	SPI_MSB_POST
spiLSBPost				long	SPI_LSB_POST
spiLSBFirst				long	SPI_LSB_FIRST
spiMSBFirst				long	SPI_MSB_FIRST

// DEBUG: The following set is used only for debugging purposes
testLEDs				long	BYTE_2
led16					long	BIT_16
led17					long	BIT_17
led18					long	BIT_18
led19					long	BIT_19
led20					long	BIT_20
led21					long	BIT_21
led22					long	BIT_22
led23					long	BIT_23

/* Beginning of variables */
mailbox					res		1						'' Address in hub memory used for communication with another cog
temp					res		1						'' Working register
loopIdx					res		1						'' Used when bitCount cannot be modified during a loop (LSB first modes)
clock					res		1						'' Used for clocking in and out with SCLK
mode					res		1						'' Store for the original command passed in - used for referencing the mode and bit count
bitCount				res		1						'' Keep track of how many bits need to be sent/received - used for DJNZ loop
data					res		1						'' Data is written to and read from this register

mosi					res		1						'' Pin mask for MOSI pin
mosiPinNum				res		1						'' Pin number for MOSI
miso					res		1						'' Pin mask for MISO pin
misoPinNum				res		1						'' Pin number for MISO
sclk					res		1						'' Pin mask for SCLK pin
clkDelay				res		1						'' Delay between clock ticks (Period / 2)
