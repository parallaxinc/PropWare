/* File:    sd.c
 * 
 * Author:  David Zemon
 */

// Includes
#include <sd.h>

// SPI config
#define SD_SPI_INIT_FREQ		200000					// Run SD initialization at 200 kHz
#define SD_SPI_FINAL_FREQ		1800000					// Speed clock to 1.8 MHz after initialization
#define SD_SPI_POLARITY			SPI_POLARITY_LOW		// SD cards like low polarity
#define SD_SPI_MODE_OUT			SPI_MSB_FIRST
#define SD_SPI_MODE_IN			SPI_MSB_PRE
#define SD_SPI_BYTE_IN_SZ		1

// Misc. SD Definitions
#define SD_DATA_BLOCK_SIZE		512
#define SD_WIGGLE_ROOM			10000
#define SD_RESPONSE_TIMEOUT		CLKFREQ/10			// Wait 0.1 seconds for a response before timing out
// SD Commands
#define SD_CMD_IDLE				0x40 + 0			// Send card into idle state
#define	SD_CMD_SDHC				0x40 + 8			// Set SD card version (1 or 2) and voltage level range
#define SD_CMD_RD_CSD			0x40 + 9			// Request "Card Specific Data" block contents
#define SD_CMD_RD_CID			0x40 + 10			// Request "Card Identification" block contents
#define SD_CMD_RD_BLOCK			0x40 + 17			// Request data block
#define SD_CMD_READ_OCR			0x40 + 58			// Request "Operating Conditions Register" contents
#define SD_CMD_APP				0x40 + 55			// Inform card that following instruction is application specific
#define SD_CMD_WR_OP			0x40 + 41			// Send operating conditions for SDC
// SD Arguments
#define SD_CMD_VOLT_ARG			0x000001AA
#define SD_ARG_LEN				5

// SD CRCs
#define SD_CRC_IDLE				0x95
#define SD_CRC_SDHC				0x87
#define SD_CRC_ACMD				0x77
#define SD_CRC_OTHER			0x01

// SD Responses
#define SD_RESPONSE_IDLE		0x01
#define SD_RESPONSE_ACTIVE		0x00
#define SD_DATA_START_ID		0xFE
#define SD_RESPONSE_LEN_R1		1
#define SD_RESPONSE_LEN_R3		5
#define	SD_RESPONSE_LEN_R7		5

uint32 g_cs;
#ifdef SD_DEBUG
uint8 g_sd_invalidResponse;
#endif


/***********************************
 *** Private Function Prototypes ***
 ***********************************/
/* @Brief: Send a command and argument over SPI to the SD card
 *
 * @param    command       6-bit value representing the command sent to the SD card
 *
 * @return
 */
uint8 SDSendCommand (const uint8 cmd, const uint32 arg, const uint8 crc);

/* Brief: Receive response and data from SD card over SPI
 *
 * @param	bytes		Number of bytes to receive
 * @param	*data		Location in memory with enough space to store 'bytes' bytes of data
 *
 * @return		Returns 0 for success, else error code
 */
uint8 SDGetResponse (const uint8 numBytes, uint8 *dat);

/* @Brief: Receive data from SD card via SPI
 *
 * @param	bytes		Number of bytes to receive
 * @param	*data		Location in memory with enough space to store 'bytes' bytes of data
 * @param	address		Address for the SD card from which data should be read
 *
 * @return		Returns 0 for success, else error code
 */
uint8 SDReadBlock (uint16 bytes, uint8 *dat);

/* @Brief: Read 512-byte data block from SD card
 *
 * @param	address		Block address to read from SD card
 * @param	*dat		Location in chip memory to store data block
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDReadDataBlock (uint32 address, uint8 *dat);

/****************************
 *** Function Definitions ***
 ****************************/
uint8 SDStart (const uint32 mosi, const uint32 miso, const uint32 sclk, const uint32 cs) {
	uint16 i, err;
	uint8 response[32];
	response[0] = (uint8) -1; // Set CS for output and initialize high
	g_cs = cs;
	GPIODirModeSet(cs, GPIO_DIR_OUT);
	GPIOPinSet(cs);

	// Start SPI module
	if (err = SPIStart(mosi, miso, sclk, SD_SPI_INIT_FREQ, SD_SPI_POLARITY))
		SDError(err);

	for (i = 0; i < 10; ++i) {
		waitcnt(CLKFREQ/2 + CNT);
		// Send at least 72 clock cycles to enable the SD card
		GPIOPinSet(cs);
		for (i = 0; i < 5; ++i)
			SPIShiftOut(16, -1, SD_SPI_MODE_OUT);

		GPIOPinClear(cs);
		// Send SD into idle state, retrieve a response and ensure it is the "idle" response
		if (err = SDSendCommand(SD_CMD_IDLE, 0, SD_CRC_IDLE))
			SDError(err);
		SDGetResponse(SD_RESPONSE_LEN_R1, response);
		if (SD_RESPONSE_IDLE == response[0])
			i = 10;
	}
	if (SD_RESPONSE_IDLE != response[0])
		SDError(SD_INVALID_INIT, response[0]);

#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("Sending CMD8...\n");
#endif

	// Set voltage to 3.3V and ensure response is R7
	if (err = SDSendCommand(SD_CMD_SDHC, SD_CMD_VOLT_ARG, SD_CRC_SDHC))
		SDError(err);
	if (err = SDGetResponse(SD_RESPONSE_LEN_R7, response))
		SDError(err);
	if ((SD_RESPONSE_IDLE != response[0]) || (0x01 != response[3])
			|| (0xAA != response[4]))
		SDError(SD_INVALID_INIT, response[0]);

#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("CMD8 succeeded. Requesting operating conditions...\n");
#endif

	// Request operating conditions register and ensure response begins with R1
	if (err = SDSendCommand(SD_CMD_READ_OCR, 0, SD_CRC_OTHER))
		SDError(err);
	if (err = SDGetResponse(SD_RESPONSE_LEN_R3, response))
		SDError(err);
#if (defined SD_VERBOSE && defined SD_DEBUG)
	SDPrintHexBlock(response, SD_RESPONSE_LEN_R3);
#endif
	if (SD_RESPONSE_IDLE != response[0])
		SDError(SD_INVALID_INIT, response[0]);

#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("OCR read successfully. Sending into active state...\n");
#endif
	for (i = 0; i < 8; ++i) {
		if (err = SDSendCommand(SD_CMD_APP, 0, SD_CRC_OTHER))
			SDError(err);
		if (err = SDGetResponse(1, response))
			SDError(err);
		if (err = SDSendCommand(SD_CMD_WR_OP, BIT_30, SD_CRC_OTHER))
			SDError(err);
		SDGetResponse(1, response);
		if (SD_RESPONSE_ACTIVE == response[0])
			break;
	}
	if (SD_RESPONSE_ACTIVE != response[0])
		SDError(SD_INVALID_RESPONSE, response[0]);
#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("Activated!\n");
#endif

	// Initialization nearly complete, increase clock
	SPISetClock(SD_SPI_FINAL_FREQ);

// If debugging requested, print to the screen CSD and CID registers from SD card
#ifdef SD_DEBUG
#ifdef SD_VERBOSE
	__simple_printf("Requesting CSD...\n");
#endif
	if (err = SDSendCommand(SD_CMD_RD_CSD, 0, SD_CRC_OTHER))
	SDError(err);
	if (err = SDReadBlock(16, response))
	SDError(err);
#ifdef SD_VERBOSE
	__simple_printf("CSD Contents:\n");
	SDPrintHexBlock(response, 16);
#endif

#ifdef SD_VERBOSE
	__simple_printf("Requesting CID...\n");
#endif
	if (err = SDSendCommand(SD_CMD_RD_CID, 0, SD_CRC_OTHER))
	SDError(err);
	if (err = SDReadBlock(16, response))
	SDError(err);
#ifdef SD_VERBOSE
	__simple_printf("CID Contents:\n");
	SDPrintHexBlock(response, 16);
#endif
#endif

	GPIOPinSet(g_cs);

	// Initialization complete
	return 0;
}

uint8 SDMount (void) {
	uint8 buf[512];
	uint32 r = 0, start, sectorspercluster, reserved, rootentries, sectors;

	/*SDReadDataBlock(0, buf);
	if (brlong(buf + 0x36) == constant('F' + ('A' << 8) + ('T' << 16) + ('1' << 24))
			&& buf[0x3a] == '6') {
		return 1;
	}
	if (brlong(buf + 0x52) == constant('F' + ('A' << 8) + ('T' << 16) + ('3' << 24))
			&& buf[0x56] == '2') {
		return 2;
	}*/

	return 0;
}

uint8 SDSendCommand (const uint8 cmd, const uint32 arg, const uint8 crc) {
	uint8 err;

	// Send out the command
	if (err = SPIShiftOut(8, cmd, SD_SPI_MODE_OUT))
		return err;

	// Send argument
	if (err = SPIShiftOut(16, (arg >> 16), SD_SPI_MODE_OUT))
		return err;
	if (err = SPIShiftOut(16, arg & WORD_0, SD_SPI_MODE_OUT))
		return err;

	// Send sixth byte - CRC
	if (err = SPIShiftOut(8, crc, SD_SPI_MODE_OUT))
		return err;

	return 0;
}

uint8 SDGetResponse (uint8 bytes, uint8 *dat) {
	uint8 err;
	uint32 timeout;

	// Read first byte - the R1 response
	timeout = SD_RESPONSE_TIMEOUT + CNT;
	do {
		if (err = SPIShiftIn(8, SD_SPI_MODE_IN, dat, SD_SPI_BYTE_IN_SZ))
			return err;

		// Check for timeout
		if (0 < (timeout - CNT) && (timeout - CNT) < SD_WIGGLE_ROOM)
			return SD_READ_TIMEOUT;
	} while (0xff == *dat); // wait for transmission end

	if ((SD_RESPONSE_IDLE == *dat) || (SD_RESPONSE_ACTIVE == *dat)) {
		++dat;		// Increment pointer to next byte;
		--bytes;	// Decrement bytes counter

		// Read remaining bytes
		while (bytes--)
			if (err = SPIShiftIn(8, SD_SPI_MODE_IN, dat++, SD_SPI_BYTE_IN_SZ))
				return err;
	} else {
#ifdef SD_DEBUG
		g_sd_invalidResponse = *dat;
#endif
		return SD_INVALID_RESPONSE;
	}

	SPIShiftOut(8, 0xff, SD_SPI_MODE_OUT);

	return 0;
}

uint8 SDReadBlock (uint16 bytes, uint8 *dat) {
	uint8 i, err, checksum;
	uint32 timeout;

	if (!bytes)
		SDError(SD_INVALID_NUM_BYTES);

	// Read first byte - the R1 response
	timeout = SD_RESPONSE_TIMEOUT + CNT;
	do {
		if (err = SPIShiftIn(8, SD_SPI_MODE_IN, dat, SD_SPI_BYTE_IN_SZ))
			return err;

		// Check for timeout
		if (0 < (timeout - CNT) && (timeout - CNT) < SD_WIGGLE_ROOM)
			return SD_READ_TIMEOUT;
	} while (0xff == *dat); // wait for transmission end

	// Ensure this response is "active"
	if (SD_RESPONSE_ACTIVE == *dat) {
		//++dat;			// Commented out to write over this byte later (uncomment for debugging purposes only)

		// Ignore blank data again
		timeout = SD_RESPONSE_TIMEOUT + CNT;
		do {
			if (err = SPIShiftIn(8, SD_SPI_MODE_IN, dat, SD_SPI_BYTE_IN_SZ))
				return err;

			// Check for timeout
			if ((timeout - CNT) < SD_WIGGLE_ROOM)
				return SD_READ_TIMEOUT;
		} while (0xff == *dat); // wait for transmission end

		// Check for the data start identifier and continue reading data
		if (SD_DATA_START_ID == *dat) {
			//++dat;			// Commented out to write over this byte later (uncomment for debugging purposes only)
			// Read in requested data bytes
			while (bytes--) {
#ifdef SD_DEBUG
				if (err = SPIShiftIn(8, SD_SPI_MODE_IN, dat++, SD_SPI_BYTE_IN_SZ))
				return err;
#else
				SPIShiftIn(8, SD_SPI_MODE_IN, dat++, SD_SPI_BYTE_IN_SZ);
#endif
			}

			// Read two more bytes for checksum - throw away data
			for (i = 0; i < 2; ++i) {
				timeout = SD_RESPONSE_TIMEOUT + CNT;
				do {
					if (err = SPIShiftIn(8, SD_SPI_MODE_IN, &checksum, SD_SPI_BYTE_IN_SZ))
						return err;

					// Check for timeout
					if ((timeout - CNT) < SD_WIGGLE_ROOM)
						return SD_READ_TIMEOUT;
				} while (0xff == checksum); // wait for transmission end
			}

			// Send final 0xff
			if (err = SPIShiftOut(8, 0xff, SD_SPI_MODE_OUT))
				return err;
		} else {
#ifdef SD_DEBUG
			g_sd_invalidResponse = *dat;
#endif
			return SD_INVALID_RESPONSE;
		}
	} else {
#ifdef SD_DEBUG
		g_sd_invalidResponse = *dat;
#endif
		return SD_INVALID_RESPONSE;
	}

	return 0;
}

uint8 SDReadDataBlock (uint32 address, uint8 *dat) {
	uint8 err;

	GPIOPinClear(g_cs);
	if (err = SDSendCommand(SD_CMD_RD_BLOCK, address, SD_CRC_OTHER))
		SDError(err);

	if (err = SDReadBlock(SD_DATA_BLOCK_SIZE, dat))
		SDError(err);
	GPIOPinSet(g_cs);

	return 0;
}

#ifdef SD_VERBOSE
uint8 SDPrintHexBlock (uint8 *dat, uint16 bytes) {
	uint8 i, j;
	uint8 offset = ((uint32) dat) % SD_LINE_SIZE;
	uint8 *s;

	printf("Printing %u bytes from address 0x%04X:\nRow\t", bytes, dat);
	for (i = 0; i < SD_LINE_SIZE; ++i)
	printf("0x%X  ", i);
	__simple_printf("\n");

	// Adjust bytes for the line size
	if ((offset + bytes) % SD_LINE_SIZE) {
		bytes /= SD_LINE_SIZE;
		++bytes;
	} else
	bytes /= SD_LINE_SIZE;

	// Check if the starting address is not aligned correctly and adjust accordingly
	if (offset)
	dat -= offset;

	for (i = 0; i < bytes; ++i) {
		s = dat + SD_LINE_SIZE * i;
		printf("0x%04X:\t", s, i);
		for (j = 0; j < SD_LINE_SIZE; ++j)
		printf("0x%02X ", s[j]);
		__simple_printf(" - ");
		for (j = 0; j < SD_LINE_SIZE; ++j) {
			if ((' ' <= s[j]) && (s[j] <= '~'))
			__simple_printf("%c", s[j]);
			else
			__simple_printf(".");
		}

		__simple_printf("\n");
	}

	return 0;
}
#endif

#ifdef SD_DEBUG
void SDError (const uint8 err, ...) {
	va_list list;
	char str[] = "SD Error %u: %s\n";

	switch (err) {
		case SD_INVALID_CMD:
		__simple_printf(str, (err - SD_ERRORS_BASE), "Invalid command");
		break;
		case SD_READ_TIMEOUT:
		__simple_printf(str, (err - SD_ERRORS_BASE), "Timed out during read");
		break;
		case SD_INVALID_NUM_BYTES:
		__simple_printf(str, (err - SD_ERRORS_BASE), "Invalid number of bytes");
		break;
		case SD_INVALID_RESPONSE:
#ifdef SD_VERBOSE
		printf("SD Error %u: %s0x%02X\n", (err - SD_ERRORS_BASE),
				"Invalid response\n\tReceived: ", g_sd_invalidResponse);
#else
		__simple_printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
				"Invalid response\n\tReceived: ", g_sd_invalidResponse);
#endif
		break;
		case SD_INVALID_INIT:
#ifdef SD_VERBOSE
		printf("SD Error %u: %s\n\tResponse: 0x%02X\n", (err - SD_ERRORS_BASE),
				"Invalid response during initialization", va_arg(list, uint32));
#else
		va_start(list, 1);
		__simple_printf("SD Error %u: %s\n\tResponse: %u\n", (err - SD_ERRORS_BASE),
				"Invalid response during initialization", va_arg(list, uint32));
		va_end(list);
#endif
		break;
		default:
		// Is the error an SPI error?
		if (err > SD_ERRORS_BASE && err < (SD_ERRORS_BASE + SD_ERRORS_LIMIT))
		__simple_printf("Unknown SD error %u\n", (err - SPI_ERRORS_BASE));
		else
		__simple_printf("Unknown error %u\n", (err));
		break;
	}
	while (1)
	;
}
#endif
