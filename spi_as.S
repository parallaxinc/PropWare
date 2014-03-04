/**
 * @file    spi_as.S
 */

/**
 * @brief   SPI routine for Parallax Propeller. Runs in master mode only.
 *
 * @project PropWare
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define ASM_OBJ_FILE
#include <PropWare.h>

/* NOTE: These definitions *MUST* match up with the SPI source file "spi.c" */
// Different fuctions supported by this GAS module
#define SPI_FUNC_SEND           0
#define SPI_FUNC_READ           1
#define SPI_FUNC_SEND_FAST      2
#define SPI_FUNC_READ_FAST      3
#define SPI_FUNC_READ_SECTOR    4
#define SPI_FUNC_SET_MODE       5
#define SPI_FUNC_SET_BITMODE    6
#define SPI_FUNC_SET_FREQ       7
#define SPI_FUNC_GET_FREQ       8

#define SPI_BITS_OFFSET         8

#define SPI_PHASE_BIT           BIT_0
#define SPI_POLARITY_BIT        BIT_1                   '' When set, clock idles high, When reset, clock idles low
#define SPI_BITMODE_BIT         BIT_2                   '' MSB_FIRST == HIGH, LSB_FIRST == LOW

#define SPI_FUNC_BITS           BYTE_0                  '' Interpret bits 7-0 as a function descriptor
#define SPI_BIT_COUNT_BITS      BYTE_1                  '' Interpret bits 15-8 as bit-count descriptor

#define SD_SECTOR_SIZE          512

                        .section spi_as.cog, "ax"
                        .compress off

                        org 0

                        or dira, testLEDs               '' DEBUG: LEDs used for debugging purposes, can be removed later
                        wrlong negOne, par              '' Inform parent cog that initialization has begun

                        // Begin by retreiving all parameters...
                        call #READ_CMD                  '' Read in the pin mask MOSI
                        mov mosi, mailbox
                        call #READ_CMD                  '' Read in the pin number for MOSI
                        mov mosiPinNum, mailbox
                        call #READ_CMD                  '' Read in the pin mask MISO
                        mov miso, mailbox
                        call #READ_CMD                  '' Read in the pin number for MISO
                        mov misoPinNum, mailbox
                        call #READ_CMD                  '' Read in the pin mask SCLK
                        mov sclk, mailbox

                        // Followed by setting MOSI & SCLK as outputs and MISO as input, Also set MOSI and MISO high
                        or dira, mosi
                        or dira, sclk
                        andn dira, miso

                        // Followed by preparing the counter module for fast reading/writing
                        // TODO: Do this...

/*** MAIN LOOP ***/
LOOP                    // Retrieve a command
                        call #READ_CMD
                        mov temp, mailbox
                        and temp, spiFuncBits           '' Mask away all bits but the function descriptor

                        // Using "jmp" instead of "call" because all functions return by "jmp #LOOP" instead of "ret"
                        // If command is "Send"
                        cmp temp, #SPI_FUNC_SEND wz
        if_z            jmp #SEND

                        // If command is "Read"
                        cmp temp, #SPI_FUNC_READ wz
        if_z            jmp #READ

                        // If command is "Send fast"
                        cmp temp, #SPI_FUNC_SEND_FAST wz
        if_z            jmp #SEND_fast

                        // If command is "Read fast"
                        cmp temp, #SPI_FUNC_READ_FAST wz
        if_z            jmp #READ_fast

                        // If command is "Read sector"
                        cmp temp, #SPI_FUNC_READ_SECTOR
        if_z            jmp #read_sector

                        // If command is "Set mode"
                        cmp temp, #SPI_FUNC_SET_MODE wz
        if_z            jmp #SET_MODE

                        // If command is "Set bitmode
                        cmp temp, #SPI_FUNC_SET_BITMODE wz
        if_z            jmp #SET_BITMODE

                        // If command is "Set clock"
                        cmp temp, #SPI_FUNC_SET_FREQ wz
        if_z            jmp #SET_FREQ

                        // If command is "Get clock"
                        cmp temp, #SPI_FUNC_GET_FREQ wz
        if_z            jmp #GET_FREQ

                        // Default: Return to loop
                        jmp #LOOP

/* FUNCTION: spi_set_mode() */
SET_MODE                // Set the current SPI polarity, If polarity high, initialize sclk high, else clear the bit
                        call #READ_DATA_CONFIRM                 '' Read in the mode

                        mov clkPhase, mailbox           '' Store the phase
                        and clkPhase, #SPI_PHASE_BIT    '' Clear the clock polarity bit

                        test mailbox, #BIT_1 wz         '' TODO: WHY IS "SPI_POLARITY_BIT" NOT WORKING!?!?!?!?!?!?
                        muxnz outa, sclk

                        jmp #LOOP

/* FUNCTION: spi_set_bit_mode() */
SET_BITMODE             // Set shifting bitmode (LSB or MSB first) of communication
                        call #READ_CMD                  '' Read in bitmode
                        mov bitmode, mailbox
                        jmp #LOOP

/* FUNCTION: spi_set_clock() */
SET_FREQ                call #READ_CMD                  '' Using READ_CMD instead of READ_DATA_CONFIRM because -1 is not a valid clock signal
                        mov clkDelay, mailbox
                        jmp #LOOP

/* FUNCTION: spi_get_clock() */
GET_FREQ                mov data, clkDelay
                        call #WRITE_DATA
                        jmp #LOOP

/* FUNCTION: spi_shift_out() */
SEND                    // Interpret the bit count and mode of output for this send command
                        mov bitCount, mailbox           '' Initialize 'bitCount' register
                        and bitCount, spiBitCountBits   '' Mask off all bits except the bit count
                        shr bitCount, #SPI_BITS_OFFSET  '' Shift bit count into the lsb

SEND_rd_data            call #READ_DATA
                        test mailbox, dataMask wz       '' Is BIT_31 cleared? (Implying data vs command)
        if_nz           jmp #SEND_rd_data               '' If not z, value is not data. Try again

                        mov data, mailbox               '' Initialize 'data' register

                        cmp bitmode, #SPI_BITMODE_BIT wz        '' Is bitmode MSB first or LSB first?
                        mov clock, cnt                  '' \__Prepare a register with the system counter for use in the *_CLOCK functions
                        add clock, clkDelay             '' /
        if_z            jmp #msb_first

                        // LSB_FIRST Initialization
                        mov loopIdx, negOne
                        sub bitCount, #1
                        mov clock, cnt                  '' \__Prepare a register with the system counter for use in the *_CLOCK functions
                        add clock, clkDelay             '' /
                        jmp #lsb_first

msb_first               // MSB_FIRST Routine
                        sub bitCount, #1 wz
                        mov temp, data
                        shr temp, bitCount
                        test temp, #BIT_0 wc
                        muxc outa, mosi
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
        if_nz           jmp #msb_first

                        // SEND complete, return to loop
                        jmp #SEND_complete

lsb_first               // LSB_FIRST Routine
                        add loopIdx, #1
                        mov temp, data
                        shr temp, loopIdx
                        test temp, #BIT_0 wc
                        muxc outa, mosi
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        cmp bitCount, loopIdx wz
        if_nz           jmp #lsb_first

                        // SEND complete, return to loop
SEND_complete           wrlong negOne, par              '' Indicate that send is complete and C-cog can continue execution
                        or outa, mosi                   '' Leave MOSI in the high state
                        jmp #LOOP

/* FUNCTION: spi_shift_in() */
READ                    // Interpret the bit count and mode of output for this read command
                        mov bitCount, mailbox           '' Initialize 'bitCount' register
                        and bitCount, spiBitCountBits   '' Mask off all bits except the bit count
                        shr bitCount, #SPI_BITS_OFFSET  '' Shift bit count into the lsb
                        mov loopIdx, bitCount           '' Create a second storage register for the bit count - used in LSB modes
                        mov data, #0                    '' Clear out the data register, ready for input

                        mov clock, cnt
                        add clock, clkDelay

                        // Determine clock phase and bit mode...
                        cmp bitmode, #SPI_BITMODE_BIT wz    '' First up is determining bitmode
        if_z            jmp #read_msb_first

                        cmp clkPhase, #SPI_PHASE_BIT wz     '' So it's LSB first, now determine CPHA
        if_z            call #lsb_cpha1
        if_nz           call #lsb_cpha0
                        jmp #finish_lsb_first

read_msb_first          cmp clkPhase, #SPI_PHASE_BIT wz     '' So it's MSB first, now determine CPHA
        if_z            call #msb_cpha1
        if_nz           call #msb_cpha0

finish_lsb_first        call #WRITE_DATA
                        jmp #LOOP

msb_cpha0               // Read in a value MSB-first with data valid before the clock
                        test miso, ina wc
                        muxc data, #BIT_0
                        shl data, #1
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        djnz bitCount, #msb_cpha0

                        shr data, #1
msb_cpha0_ret           ret

lsb_cpha0               // Read in a value LSB-first with data valid before the clock
                        test miso, ina wc
                        muxc data, dataMask             '' Would use BIT_31 here, but it's too large to fit in an operand. `dataMask` already contains BIT_31
                        shr data, #1 wc                 '' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        djnz loopIdx, #lsb_cpha0

                        rcl data, #1                    '' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
                        mov temp, #32
                        sub temp, bitCount
                        shr data, temp
lsb_cpha0_ret           ret

msb_cpha1               // Read in a value MSB-first with data valid after the clock
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        test miso, ina wc
                        muxc data, #BIT_0
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        shl data, #1
                        djnz bitCount, #msb_cpha1

                        shr data, #1
msb_cpha1_ret           ret

lsb_cpha1               // Read in a value LSB-first with data valid after the clock
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        test miso, ina wc
                        muxc data, dataMask             '' Would use BIT_31 here, but it's too large for a second operand. 'dataMask' already contains BIT_31
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        shr data, #1 wc                 '' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
                        djnz loopIdx, #lsb_cpha1

                        rcl data, #1                    '' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
                        mov temp, #32
                        sub temp, bitCount
                        shr data, temp
lsb_cpha1_ret           ret

/* FUNCTION: spi_shift_out_fast() */
SEND_fast               // Interpret the bit count and mode of output for this send command
                        mov bitCount, mailbox           '' Initialize 'bitCount' register
                        and bitCount, spiBitCountBits   '' Mask off all bits except the bit count
                        shr bitCount, #SPI_BITS_OFFSET  '' Shift bit count into the lsb

SEND_rd_data_fast       call #READ_DATA
                        test mailbox, dataMask wz       '' Is BIT_31 cleared? (Implying data vs command)
        if_nz           jmp #SEND_rd_data               '' If not z, value is not data. Try again

                        mov data, mailbox               '' Initialize 'data' register

                        mov clock, cnt                  '' \__Prepare a register with the system counter for use in the *_CLOCK functions
                        add clock, clkDelay             '' /
                        cmp bitmode, #SPI_BITMODE_BIT wz        '' Is bitmode MSB first or LSB first?
        if_z            jmp #msb_first

                        // LSB_FIRST Initialization
                        mov loopIdx, negOne
                        sub bitCount, #1
                        jmp #lsb_first

msb_first_fast          // MSB_FIRST Routine
                        sub bitCount, #1 wz
                        mov temp, data
                        shr temp, bitCount
                        test temp, #BIT_0 wc
                        muxc outa, mosi
                        xor outa, sclk
                        xor outa, sclk
        if_nz           jmp #msb_first

                        // SEND complete, return to loop
                        jmp #SEND_complete

lsb_first_fast          // LSB_FIRST Routine
                        add loopIdx, #1
                        mov temp, data
                        shr temp, loopIdx
                        test temp, #BIT_0 wc
                        muxc outa, mosi
                        xor outa, sclk
                        xor outa, sclk
                        cmp bitCount, loopIdx wz
        if_nz           jmp #lsb_first

                        // SEND complete, return to loop
SEND_complete_fast      wrlong negOne, par              '' Indicate that send is complete and C-cog can continue execution
                        or outa, mosi                   '' Leave MOSI in the high state
                        jmp #LOOP

/* FUNCTION: spi_shift_in_fast() */
READ_fast               // Interpret the bit count and mode of output for this read command
                        mov bitCount, mailbox           '' Initialize 'bitCount' register
                        and bitCount, spiBitCountBits   '' Mask off all bits except the bit count
                        shr bitCount, #SPI_BITS_OFFSET  '' Shift bit count into the lsb
                        mov loopIdx, bitCount           '' Create a second storage register for the bit count - used in LSB modes
                        mov data, #0                    '' Clear out the data register, ready for input

                        mov clock, cnt
                        add clock, clkDelay

                        // Determine clock phase and bit mode...
                        cmp bitmode, #SPI_BITMODE_BIT wz    '' First up is determining bitmode
        if_z            jmp #read_msb_first_fast

                        cmp clkPhase, #SPI_PHASE_BIT wz     '' So it's LSB first, now determine CPHA
        if_z            call #lsb_post_fast
        if_nz           call #lsb_pre_fast
                        jmp #finish_lsb_first_fast

read_msb_first_fast     cmp clkPhase, #SPI_PHASE_BIT wz     '' So it's MSB first, now determine CPHA
        if_z            call #msb_post_fast
        if_nz           call #msb_pre_fast

finish_lsb_first_fast   call #WRITE_DATA
                        jmp #LOOP

msb_pre_fast            // Read in a value MSB-first with data valid before the clock
                        test miso, ina wc
                        muxc data, #BIT_0
                        xor outa, sclk
                        shl data, #1
                        xor outa, sclk
                        djnz bitCount, #msb_pre_fast

                        shr data, #1
msb_pre_fast_ret        ret

lsb_pre_fast            // Read in a value LSB-first with data valid before the clock
                        test miso, ina wc
                        muxc data, dataMask             '' Would use BIT_31 here, but it's too large for a second operand. 'dataMask' already contains BIT_31
                        xor outa, sclk
                        shr data, #1 wc                 '' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
                        xor outa, sclk
                        djnz loopIdx, #lsb_pre_fast

                        rcl data, #1                    '' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
                        mov temp, #32
                        sub temp, bitCount
                        shr data, temp
lsb_pre_fast_ret        ret

msb_post_fast           // Read in a value MSB-first with data valid after the clock
                        xor outa, sclk
                        test miso, ina wc
                        muxc data, #BIT_0
                        xor outa, sclk
                        shl data, #1
                        djnz bitCount, #msb_post_fast

                        shr data, #1
msb_post_fast_ret       ret

lsb_post_fast           // Read in a value LSB-first with data valid after the clock
                        xor outa, sclk
                        test miso, ina wc
                        muxc data, dataMask             '' Would use BIT_31 here, but it's too large for a second operand. 'dataMask' already contains BIT_31
                        shr data, #1 wc                 '' wc flag is specified for the case of a 32-bit shift in - upon receiving the 32nd bit, the lsb will be shifted out of the register and must be saved.
                        xor outa, sclk
                        djnz loopIdx, #lsb_post_fast

                        rcl data, #1                    '' In the case of a 32-bit shift in, retreive the LSB *NOTE: 32-bit shift in is not currently supported for the same reason that a 32-bit shift out cannot happen
                        mov temp, #32
                        sub temp, bitCount
                        shr data, temp
lsb_post_fast_ret       ret

/* FUNCTION: SPIReadSDSector() */
read_sector             // Read an entire sector from the SD card as quickly as possible, write values to the mailbox
read_sector_addr        call #READ_DATA                 '' Read in hub address to store the data
                        cmp mailbox, negOne wz          '' Are we reading a valid parameter?
        if_z            jmp #read_sector_addr           '' If not valid, read in a new value
                        // Do not write -1 back to global mailbox until the entire read has completed

                        mov loopIdx, sdSectorSize       '' Load the index counter

                        // TODO TODO TODO: enable the counter module to do the clock for you!!!
beginSectorRead         mov bitCount, #8
/*                      // Bit 7
                        test miso, ina wc
                        muxc data, #BIT_7
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 6
                        test miso, ina wc
                        muxc data, #BIT_6
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 5
                        test miso, ina wc
                        muxc data, #BIT_5
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 4
                        test miso, ina wc
                        muxc data, #BIT_4
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 3
                        test miso, ina wc
                        muxc data, #BIT_3
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 2
                        test miso, ina wc
                        muxc data, #BIT_2
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 1
                        test miso, ina wc
                        muxc data, #BIT_1
                        xor outa, sclk
                        xor outa, sclk

                        // Bit 0
                        test miso, ina wc
                        muxc data, #BIT_0
                        xor outa, sclk
                        xor outa, sclk
*/
read_byte               test miso, ina wc
                        muxc data, #BIT_0
                        shl data, #1
                        call #POST_CLOCK
                        djnz bitCount, #read_byte

                        shr data, #1

                        // Shift the data one more time and write it to HUB
                        wrbyte data, mailbox
                        add mailbox, #1                 '' Increase the address to the next byte in HUB RAM

                        djnz loopIdx, #beginSectorRead  '' Continue looping for SD_SECTOR_SIZE bytes
                        wrlong negOne, par
                        jmp #LOOP

/* FUNCTION: SPIPostClock() */
POST_CLOCK              waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
POST_CLOCK_ret          ret

/* FUNCTION: SPIPreClock() */
PRE_CLOCK               xor outa, sclk
                        waitcnt clock, clkDelay
                        xor outa, sclk
                        waitcnt clock, clkDelay
PRE_CLOCK_ret           ret

/* FUNCTION: Loop reading in a value from hub-RAM and store in 'mailbox' ONLY if not -1. */
READ_CMD                rdlong mailbox, par             '' Wait for parameter to be passed in
                        cmp mailbox, negOne wz          '' Are we reading a valid parameter?
        if_z            jmp #READ_CMD                   '' If not valid, read in a new value
                        wrlong negOne, par              '' If value is valid, write -1 back to hub-RAM to indicate read completion
READ_CMD_ret            ret

/* Loop reading in a value from hub-RAM and store in 'mailbox' ONLY if BIT_31 is not set. */
READ_DATA               rdlong mailbox, par
                        test mailbox, dataMask wz       '' Is BIT_31 cleared? (Implying data vs command)
        if_nz           jmp #READ_DATA                  '' If not z, value is not data. Try again
READ_DATA_ret           ret

/* Loop reading in a value from hub-RAM and store in 'mailbox' ONLY if BIT_31 is not set. */
READ_DATA_CONFIRM       rdlong mailbox, par
                        test mailbox, dataMask wz       '' Is BIT_31 cleared? (Implying data vs command)
        if_nz           jmp #READ_DATA_CONFIRM          '' If not z, value is not data. Try again
                        wrlong negOne, par              '' Inform C cog that reading is complete
READ_DATA_CONFIRM_ret   ret

WRITE_DATA              wrlong data, par
write_loop              rdlong mailbox, par             '' \.
                        cmp mailbox, negOne wz          ''  |--> Hold GAS execution here until C has signaled read complete by writing -1
        if_nz           jmp #write_loop                 '' /
WRITE_DATA_ret          ret

/* Debugging purposes only, not accessible through C interface */
SEND_TO_LED             mov temp2, temp                 '' Save the original value
                        shl temp2, #16                  '' Shift the first byte into the LEDs
                        and temp2, testLEDs             '' Clear all bits not associated with the LEDs
                        mov outa, temp2                 '' Turn on the LEDs
SEND_TO_LED_ret         ret

/* Pre-Initialized Values */
negOne                  long    -1                      '' Used for comparison purposes
spiFuncBits             long    SPI_FUNC_BITS
spiBitCountBits         long    SPI_BIT_COUNT_BITS
dataMask                long    BIT_31
sdSectorSize            long    SD_SECTOR_SIZE

// DEBUG: The following set is used only for debugging purposes
testLEDs                long    BYTE_2
led16                   long    BIT_16
led17                   long    BIT_17
led18                   long    BIT_18
led19                   long    BIT_19
led20                   long    BIT_20
led21                   long    BIT_21
led22                   long    BIT_22
led23                   long    BIT_23
quarterSecond           long    20000000

/* Beginning of variables */
mailbox                 res     1                       '' Address in hub memory used for communication with another cog
temp                    res     1                       '' Working register
temp2                   res     1
loopIdx                 res     1                       '' Used when bitCount cannot be modified during a loop (LSB first modes)
clock                   res     1                       '' Used for clocking in and out with SCLK
bitmode                 res     1                       '' Store the current bitmode (LSB or MSB first)
clkPhase                res     1                       '' Store the current clock phase (CPHA)
bitCount                res     1                       '' Keep track of how many bits need to be sent/received - used for DJNZ loop
data                    res     1                       '' Working register, Data is written to and read from this register

mosi                    res     1                       '' Pin mask for MOSI pin
mosiPinNum              res     1                       '' Pin number for MOSI
miso                    res     1                       '' Pin mask for MISO pin
misoPinNum              res     1                       '' Pin number for MISO
sclk                    res     1                       '' Pin mask for SCLK pin
clkDelay                res     1                       '' Delay between clock ticks (Period / 2)

                        .compress default

/**
 * function to start the SPI code in its own COG
 * C interface is:
 *   int _SPIStartCog(void *arg)
 *
 * returns the number of the COG, or -1 if no COGs are left
 */
                        .text
                        .global __SPIStartCog
__SPIStartCog           mviw r7, #__load_start_spi_as_cog  '' linker magic for the start of the spi_as.cog section
                        shl r7, #2
                        or r7, #8                          '' 8 means first available cog
                        shl r0, #16                        '' assumes bottom two bits of r0 are 0, i.e. arg must be long aligned
                        or r0, r7
                        coginit r0 wc,wr
        if_b            neg r0, #1                         '' if C is set, return -1
                        // Temporary hack until fix for GCC is released
#ifdef __PROPELLER_CMM__
                        lret
#else
                        mov pc, lr
#endif
