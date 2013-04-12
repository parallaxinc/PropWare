/* File:    sd.c
 * 
 * Author:  David Zemon
 */

// Includes
#include <sd.h>

// SPI config
#define SD_SPI_INIT_FREQ			400000					// Run SD initialization at 200 kHz
#define SD_SPI_FINAL_FREQ			1900000					// Speed clock to 1.8 MHz after initialization; TODO: implement a custom send/receive command that does not use waitcnt instructions
#define SD_SPI_POLARITY				SPI_POLARITY_LOW		// SD cards like low polarity
#define SD_SPI_MODE_OUT				SPI_MSB_FIRST
#define SD_SPI_MODE_IN				SPI_MSB_PRE
#define SD_SPI_BYTE_IN_SZ			1

// Misc. SD Definitions
#define SD_WIGGLE_ROOM				10000
#define SD_RESPONSE_TIMEOUT			CLKFREQ/10			// Wait 0.1 seconds for a response before timing out
#define SD_SECTOR_SIZE				512

// SD Commands
#define SD_CMD_IDLE					0x40 + 0			// Send card into idle state
#define	SD_CMD_SDHC					0x40 + 8			// Set SD card version (1 or 2) and voltage level range
#define SD_CMD_RD_CSD				0x40 + 9			// Request "Card Specific Data" block contents
#define SD_CMD_RD_CID				0x40 + 10			// Request "Card Identification" block contents
#define SD_CMD_RD_BLOCK				0x40 + 17			// Request data block
#define SD_CMD_READ_OCR				0x40 + 58			// Request "Operating Conditions Register" contents
#define SD_CMD_APP					0x40 + 55			// Inform card that following instruction is application specific
#define SD_CMD_WR_OP				0x40 + 41			// Send operating conditions for SDC
// SD Arguments
#define SD_CMD_VOLT_ARG				0x000001AA
#define SD_ARG_LEN					5

// SD CRCs
#define SD_CRC_IDLE					0x95
#define SD_CRC_SDHC					0x87
#define SD_CRC_ACMD					0x77
#define SD_CRC_OTHER				0x01

// SD Responses
#define SD_RESPONSE_IDLE			0x01
#define SD_RESPONSE_ACTIVE			0x00
#define SD_DATA_START_ID			0xFE
#define SD_RESPONSE_LEN_R1			1
#define SD_RESPONSE_LEN_R3			5
#define	SD_RESPONSE_LEN_R7			5

// Boot sector addresses/values
#define SD_FAT_16					2					// A FAT entry in FAT16 is 2-bytes
#define SD_FAT_32					4					// A FAT entry in FAT32 is 4-bytes
#define SD_BOOT_SECTOR_ID			0xeb
#define SD_BOOT_SECTOR_ID_ADDR		0
#define SD_BOOT_SECTOR_BACKUP		0x1c6
#define SD_CLUSTER_SIZE_ADDR		0x0d
#define SD_RSVD_SCTR_CNT_ADDR		0x0e
#define SD_NUM_FATS_ADDR			0x10
#define SD_ROOT_ENTRY_CNT_ADDR		0x11
#define SD_TOT_SCTR_16_ADDR			0x13
#define SD_FAT_SIZE_ADDR			0x16
#define SD_TOT_SCTR_32_ADDR			0x20
#define SD_FAT_SIZE_32_ADDR			0x24
#define SD_ROOT_CLUSTER_ADDR		0x2c
#define SD_FAT12_CLSTR_CNT			4085
#define SD_FAT16_CLSTR_CNT			65525

// FAT file/directory values
enum cluster_types {
	SD_ROOT_DIR, SD_SUB_DIR, SD_DATA_FILE
};
#define SD_FILE_ENTRY_LENGTH		32					// An entry in a directory uses 32 bytes
#define SD_DELETED_FILE_MARK		0xe5				// Marks that a file has been deleted here, continue to the next entry
#define SD_FILE_ATTRIBUTE_OFFSET	11					// Byte of a file entry to store attribute flags
#define SD_FILE_START_CLSTR_OFFSET	0x1a				// Starting cluster number
#define SD_FILE_LEN_OFFSET			0x1c				// Length of a file in bytes
#define SD_FILE_NAME_LEN			8					// 8 characters in the standard file name
#define SD_FILE_EXTENSION_LEN		3					// 3 character file name extension
#define SD_FILENAME_STR_LEN			SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
#define SD_FREE_CLUSTER				0					// Cluster is unused
#define SD_RESERVED_CLUSTER			1
#define SD_RSVD_CLSTR_VAL_BEG		-15					// First reserved cluster value
#define SD_RSVD_CLSTR_VAL_END		-9					// Last reserved cluster value
#define SD_BAD_CLUSTER				-8					// Cluster is corrupt
#define SD_EOC_BEG					-7					// Last marker for end-of-chain (end of file entry within FAT)
#define SD_EOC_END					-1					// First marker for end-of-chain
// FAT file attributes (definitions with trailing underscore represent character for a cleared attribute flag)
#define SD_READ_ONLY				BIT_0
#define SD_READ_ONLY_CHAR			'r'
#define SD_READ_ONLY_CHAR_			'w'
#define SD_HIDDEN_FILE				BIT_1
#define SD_HIDDEN_FILE_CHAR			'h'
#define SD_HIDDEN_FILE_CHAR_		'.'
#define SD_SYSTEM_FILE				BIT_2
#define SD_SYSTEM_FILE_CHAR			's'
#define SD_SYSTEM_FILE_CHAR_		'.'
#define SD_VOLUME_ID				BIT_3
#define SD_VOLUME_ID_CHAR			'v'
#define SD_VOLUME_ID_CHAR_			'.'
#define SD_SUB_DIR					BIT_4
#define SD_SUB_DIR_CHAR				'd'
#define SD_SUB_DIR_CHAR_			'f'
#define SD_ARCHIVE					BIT_5
#define SD_ARCHIVE_CHAR				'a'
#define SD_ARCHIVE_CHAR_			'.'

// File constants
#ifndef SD_EOF
#define SD_EOF						-1					// System dependent - may need to be defined elsewhere
#endif

/*** Global variable declarations ***/
// Initialization variables
uint32 g_sd_cs;											// Chip select pin mask
uint8 g_sd_filesystem;					// Filesystem type - one of SD_FAT_16 or SD_FAT_32
uint8 g_sd_sectorsPerCluster_shift;	// Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
uint32 g_sd_rootDirSectors;					// Number of sectors for the root directory
uint32 g_sd_fatStart;								// Starting block address of the FAT
uint32 g_sd_rootAddr;					// Starting block address of the root directory
uint32 g_sd_firstDataAddr;			// Starting block address of the first data cluster

// FAT filesystem variables
uint8 g_sd_buf[SD_SECTOR_SIZE]; // Buffer for initialization & directory contents; if not set for speed optimization, file contents will also be dumped here
uint8 g_sd_fat[SD_SECTOR_SIZE];							// Buffer for FAT entries only
uint16 g_sd_entriesPerFatSector_Shift;// How many FAT entries are in a single sector of the FAT
uint8 g_sd_curFatSector;			// Store the current FAT sector loaded into g_sd_fat
uint32 g_sd_curDirStartAddr;	// Store the current directory's starting sector number
uint32 g_sd_curAllocUnit;							// Store the current allocation unit
uint32 g_sd_curDirAllocUnit;	// Store the current directorie's first allocation unit
uint32 g_sd_curClusterStartAddr;	// Store the current cluster's starting sector numbe
uint32 g_sd_nextAllocUnit;		// Look-ahead at the next FAT entry used by the next file
uint8 g_sd_curSectorOffset;	// Store the current sector offset from the beginning of the cluster

// Open file variables
uint32 g_sd_fseek;
uint32 g_sd_ftell;

// Special global variables used onnly if speed optimization is enabled
#ifdef SD_SPEED_OVER_SPACE
uint8 g_sd_file[SD_SECTOR_SIZE];							// Buffer for file contents
uint32 g_sd_curAllocUnit_file;						// Store the current allocation unit
uint32 g_sd_curClusterStartAddr_file;// Store the current cluster's starting sector number
uint32 g_sd_nextAllocUnit_file;	// Look-ahead at the next FAT entry used by the next file
uint8 g_sd_curSectorOffset_file;// Store the current sector offset from the beginning of the cluster
#endif

#ifdef SD_DEBUG
uint8 g_sd_sd_invalidResponse;
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

/* @Brief: Read SD_SECTOR_SIZE-byte data block from SD card
 *
 * @param	address		Block address to read from SD card
 * @param	*dat		Location in chip memory to store data block
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDReadDataBlock (uint32 address, uint8 *dat);

/* @Brief: Read the standard length name of a file entry. If an extension exists, a period will be
 *         inserted before the extension. A null-terminator is always appended to the end
 *
 * @param	*buf		First byte in local memory containing a FAT entry
 * @param	*filename	Address in memory where the filename string will be stored
 *
 * @Pre: *buf must point to the first byte in a FAT entry - no error checking is executed on buf
 * @Pre: Errors may occur if at least 13 (8 + 1 + 3 + 1) bytes of memory are not allocated for filename
 *
 * @return
 */
void SDGetFilename (const uint8 *buf, uint8 *filename);

/* @Brief: Return byte-reversed 16-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * @param	dat[]		Address of first byte of data
 *
 * @return		Returns a normal (big-endian) 16-bit word
 */
uint16 SDConvertDat16 (const uint8 dat[]);

/* @Brief: Return byte-reversed 32-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * @param	dat[]		Address of first byte of data
 *
 * @return	Returns a normal (big-endian) 32-bit word
 */
uint32 SDConvertDat32 (const uint8 dat[]);

/* @Brief: Find and return the starting sector's address for a directory path given in a c-string. Use
 *         Unix-style path names (like /foo/bar/)
 *
 * @param	*path		C-string representing Unix-style path
 *
 * @return		Returns sector address of desired path
 */
// TODO: Implement minimalist error checking (-1 or 0 would be valid error codes)
// TODO: Allow for paths outside the current directory
uint32 SDGetSectorFromPath (const char *path);

/* @Brief: Find and return the starting sector's address for a given allocation unit (note - not cluster)
 *
 * @param	allocUnit	Allocation unit in FAT filesystem
 *
 * @return		Returns sector address of desired allocation unit
 */
uint32 SDGetSectorFromAlloc (const uint32 allocUnit);

/* @Brief: Find the next sector in the FAT, directory, or file. When it is found, load it into the
 *         appropriate global buffer
 *
 * @param	*buf		Array of SD_SECTOR_SIZE bytes that can be filled with the requested sector
 *
 * @return		Returns 0 upon success, otherwise error code
 */
uint8 SDLoadNextSector (uint8 *buf);

/* @Brief: When the final sector of a cluster is finished, SDIncCluster can be called. The appropriate
 *         global variables will be set according (incremented or set by the FAT) and the first sector
 *         of the next cluster will be read into the desired buffer.
 *
 * @param	*curSectorOffset		Address of current sector offset variable
 * @param	*nextAllocUnit			Address of the next allocation unit variable
 * @param	*curAllocUnit			Address of the current allocation unit variable
 * @param	*curClusterStartAddr	Address of the current clutser's starting address variable
 * @param	*buf					Array of SD_SECTOR_SIZE bytes used to hold a sector from the SD card
 *
 * @return
 */
uint8 SDIncCluster (uint8 *curSectorOffset, uint32 *nextAllocUnit, uint32 *curAllocUnit,
		uint32 *curClusterStartAddr, uint8 *buf);

/* @Brief: Load the first sector of a file (note - not directory) and initialize global variables dealing
 *         with files (seek/tell pointers)
 *
 * @param	filePtr		Offset amount from the beginning of the currently loaded sector; Used to read
 * 						file parameters such as allocation unit and size
 * @param	*fileLen	Length of the file in bytes will be stored into this address
 *
 * @return		Returns 0 upon success, else error code
 */
uint8 SDOpenFile_ptr (const uint16 filePtr, uint32 *fileLen);

#ifdef SD_SHELL
inline void SDPrintFileEntry (const uint8 *file, uint8 filename[]);
void SDPrintFileAttributes (const uint8 flag);
#endif

#ifdef SD_DEBUG
#include <stdio.h>
#include <stdarg.h>
/* Brief: Print an error through UART string followed by entering an infinite loop
 *
 * @param	err		Error number used to determine error string
 */
void SDError (const uint8 err, ...);

/* @Brief: Print to screen each status bit individually with human-readable descriptions
 *
 * @param	response		first-byte response from the SD card
 */
void SDFirstByteExpansion (const uint8 response);
#else
// Exit calling function by returning 'err'
#define SDError(err, ...)				return err
#endif

/****************************
 *** Function Definitions ***
 ****************************/
uint8 SDStart (const uint32 mosi, const uint32 miso, const uint32 sclk, const uint32 cs) {
	uint16 i, err;
	uint8 response[16];

	// Set CS for output and initialize high
	g_sd_cs = cs;
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

	// Spin up the card and bring to active state
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
#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("Requesting CSD...\n");
	if (err = SDSendCommand(SD_CMD_RD_CSD, 0, SD_CRC_OTHER))
		SDError(err);
	if (err = SDReadBlock(16, response))
		SDError(err);
	__simple_printf("CSD Contents:\n");
	SDPrintHexBlock(response, 16);
	putchar('\n');

	__simple_printf("Requesting CID...\n");
	if (err = SDSendCommand(SD_CMD_RD_CID, 0, SD_CRC_OTHER))
		SDError(err);
	if (err = SDReadBlock(16, response))
		SDError(err);
	__simple_printf("CID Contents:\n");
	SDPrintHexBlock(response, 16);
	putchar('\n');
#endif
	GPIOPinSet(cs);

	// Initialization complete
	return 0;
}

uint8 SDMount (void) {
	uint8 err, temp;

	// FAT system determination variables:
	uint32 rsvdSectorCount, numFATs, rootEntryCount, totalSectors, FATSize, rootCluster,
			dataSectors;
	uint32 bootSector = 0;
	uint32 clusterCount;

	// Read in first sector
	if (err = SDReadDataBlock(bootSector, g_sd_buf))
		SDError(err);
	// Check if sector 0 is boot sector or MBR; if MBR, skip to boot sector
	if (SD_BOOT_SECTOR_ID != g_sd_buf[SD_BOOT_SECTOR_ID_ADDR]) {
		bootSector = SDConvertDat32(&(g_sd_buf[SD_BOOT_SECTOR_BACKUP]));
		if (err = SDReadDataBlock(bootSector, g_sd_buf))
			SDError(err);
	}

	// Print the boot sector if requested
#if (defined SD_VERBOSE && defined SD_DEBUG && defined SD_VERBOSE_BLOCKS)
	__simple_printf("***BOOT SECTOR***\n");
	SDPrintHexBlock(g_sd_buf, SD_SECTOR_SIZE);
	putchar('\n');
#endif

	// Do this whether it is FAT16 or FAT32
	temp = g_sd_buf[SD_CLUSTER_SIZE_ADDR];
	while (temp) {
		temp >>= 1;
		++g_sd_sectorsPerCluster_shift;
	}
	--g_sd_sectorsPerCluster_shift;
	rsvdSectorCount = SDConvertDat16(&g_sd_buf[SD_RSVD_SCTR_CNT_ADDR]);
	numFATs = g_sd_buf[SD_NUM_FATS_ADDR];
	rootEntryCount = SDConvertDat16(&g_sd_buf[SD_ROOT_ENTRY_CNT_ADDR]);

	// Check if FAT size is valid in 16- or 32-bit location
	FATSize = SDConvertDat16(&g_sd_buf[SD_FAT_SIZE_ADDR]);
	if (!FATSize)
		FATSize = g_sd_buf[SD_FAT_SIZE_32_ADDR];

	// Check if FAT16 total sectors is valid
	totalSectors = SDConvertDat16(&g_sd_buf[SD_TOT_SCTR_16_ADDR]);
	if (!totalSectors)
		totalSectors = SDConvertDat32(&g_sd_buf[SD_TOT_SCTR_32_ADDR]);

	// Compute necessary numbers to determine FAT type (12/16/32)
	g_sd_rootDirSectors = rootEntryCount * 32 / SD_SECTOR_SIZE;
	dataSectors = totalSectors - (rsvdSectorCount + numFATs * FATSize + rootEntryCount);
	clusterCount = dataSectors >> g_sd_sectorsPerCluster_shift;
	g_sd_entriesPerFatSector_Shift = 5;

#if (defined SD_DEBUG && defined SD_VERBOSE)
	printf("Total sector count: 0x%08X / %u\n", totalSectors, totalSectors);
	printf("Total cluster count: 0x%08X / %u\n", clusterCount, clusterCount);
	printf("Total data sectors: 0x%08X / %u\n", dataSectors, dataSectors);
	printf("Sectors per cluster: %u\n", 1 << g_sd_sectorsPerCluster_shift);
	printf("FAT Size: 0x%04X / %u\n", FATSize, FATSize);
	printf("Root directory sectors: 0x%08X / %u\n", g_sd_rootDirSectors,
			g_sd_rootDirSectors);
	printf("Root entry count: 0x%08X / %u\n", rootEntryCount, rootEntryCount);
	printf("Reserved sector count: 0x%08X / %u\n", rsvdSectorCount, rsvdSectorCount);
#endif

	// Determine and store FAT type
	if (SD_FAT12_CLSTR_CNT > clusterCount)
		SDError(SD_INVALID_FILESYSTEM);
	else if (SD_FAT16_CLSTR_CNT > clusterCount) {
#if (defined SD_VERBOSE && defined SD_DEBUG)
		__simple_printf("\n***FAT type is FAT16***\n");
#endif
		g_sd_filesystem = SD_FAT_16;
	} else {
#if (defined SD_VERBOSE && defined SD_DEBUG)
		__simple_printf("\n***FAT type is FAT32***\n");
#endif
		g_sd_filesystem = SD_FAT_32;
		--g_sd_entriesPerFatSector_Shift;
	}

	// Find start of FAT
	g_sd_fatStart = bootSector + rsvdSectorCount;

//	g_sd_filesystem = SD_FAT_16;
	// Find root directory address
	switch (g_sd_filesystem) {
		case SD_FAT_16:
			g_sd_rootAddr = FATSize * numFATs + g_sd_fatStart;
			g_sd_firstDataAddr = g_sd_rootAddr + g_sd_rootDirSectors;
			break;
		case SD_FAT_32:
			rootCluster = SDConvertDat32(&g_sd_buf[SD_ROOT_CLUSTER_ADDR]);
			g_sd_rootAddr = g_sd_fatStart + rootCluster;
			g_sd_firstDataAddr = g_sd_rootAddr;
			break;
	}

#if (defined SD_VERBOSE && defined SD_DEBUG)
	printf("Start of FAT: 0x%08X\n", g_sd_fatStart);
	printf("Root directory: 0x%08X\n", g_sd_rootAddr);
	printf("First data sector: 0x%08X\n", g_sd_firstDataAddr);
#endif

	// Store the first sector of the FAT
	if (err = SDReadDataBlock(g_sd_fatStart, g_sd_fat))
		SDError(err);
	g_sd_curFatSector = 0;

	// Print FAT if desired
#if (defined SD_VERBOSE && defined SD_DEBUG && defined SD_VERBOSE_BLOCKS)
	__simple_printf("\n***First File Allocation Table***\n");
	SDPrintHexBlock(g_sd_fat, SD_SECTOR_SIZE);
	putchar('\n');
#endif

	// Read in the root directory, set root as current
	if (err = SDReadDataBlock(g_sd_rootAddr, g_sd_buf))
		SDError(err);
	g_sd_curDirStartAddr = g_sd_rootAddr;
	g_sd_curDirAllocUnit = -1;
	g_sd_curClusterStartAddr = g_sd_rootAddr;
	g_sd_curAllocUnit = -1;
	g_sd_curFatSector = -1;
	g_sd_curSectorOffset = 0;

	// Print root directory
#if (defined SD_VERBOSE && defined SD_DEBUG && defined SD_VERBOSE_BLOCKS)
	__simple_printf("***Root directory***\n");
	SDPrintHexBlock(g_sd_buf, SD_SECTOR_SIZE);
	putchar('\n');
#endif

	return 0;
}

void SDGetFilename (const uint8 *buf, uint8 *filename) {
	uint8 i, j = 0;

	// Read in the first 8 characters - stop when a space is reached or 8 characters have been read, whichever comes first
	for (i = 0; i < SD_FILE_NAME_LEN; ++i) {
		if (0x05 == buf[i])
			filename[j++] = 0xe5;
		else if (' ' != buf[i])
			filename[j++] = buf[i];
	}

	// Determine if there is more past the first 8 - Again, stop when a space is reached
	if (' ' != buf[SD_FILE_NAME_LEN]) {
		filename[j++] = '.';
		for (i = SD_FILE_NAME_LEN; i < SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN; ++i) {
			if (' ' != buf[i])
				filename[j++] = buf[i];
		}
	}

	// Insert null-terminator
	filename[j] = 0;
}

uint8 SDLoadNextSector (uint8 *buf) {
	uint8 err;
	uint32 firstAvailableAllocUnit, endAvailableAllocUnit; // Values representing which allocation units are available in the currently loaded FAT

	uint8 *curSectorOffset;
	uint32 *nextAllocUnit;
	uint32 *curAllocUnit;
	uint32 *curClusterStartAddr;

#if (defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("Loading next sector...\n");
#endif

	// Depending on optimization level and passed in parameter, set local variable to either generic
	// buffer or file-specific buffer
#ifdef SD_SPEED_OVER_SPACE
	if (buf == g_sd_buf) {
#endif
		curSectorOffset = &g_sd_curSectorOffset;
		nextAllocUnit = &g_sd_nextAllocUnit;
		curAllocUnit = &g_sd_curAllocUnit;
		curClusterStartAddr = &g_sd_curClusterStartAddr;
#ifdef SD_SPEED_OVER_SPACE
	} else {
		curSectorOffset = &g_sd_curSectorOffset_file;
		nextAllocUnit = &g_sd_nextAllocUnit_file;
		curAllocUnit = &g_sd_curAllocUnit_file;
		curClusterStartAddr = &g_sd_curClusterStartAddr_file;
	}
#endif

	// Are we looking at the root directory of a FAT16 system?
	if (SD_FAT_16 == g_sd_filesystem && g_sd_rootAddr == (*curClusterStartAddr)) {
		// Root dir of FAT16; Is it the last sector in the root directory?
		if (g_sd_rootDirSectors == (*curSectorOffset))
			return SD_EOC_END;
		// Root dir of FAT16; Not last sector
		else
			return SDReadDataBlock(++(*curSectorOffset), buf); // Any error from reading the data block will be returned to calling function
	}
	// We are looking at a generic data cluster.
	else {
		// Gen. data cluster; Have we reached the end of the cluster?
		if (((1 << g_sd_sectorsPerCluster_shift) - 1) > (*curSectorOffset)) {
			// Gen. data cluster; Not the end; Load next sector in the cluster
			return SDReadDataBlock(++(*curSectorOffset) + *curClusterStartAddr, buf); // Any error from reading the data block will be returned to calling function
		}
		// End of generic data cluster; Look through the FAT to find the next cluster
		else {
			// Determine start and end points in the currently loaded FAT sector (used to determine if we need to load a different FAT sector
			firstAvailableAllocUnit = g_sd_curFatSector << g_sd_entriesPerFatSector_Shift;
			endAvailableAllocUnit = (g_sd_curFatSector + 1)
					<< g_sd_entriesPerFatSector_Shift;

			// If the next FAT entry exists in the currently loaded FAT sector, allow the following two if-statements to run their course (evaluate false) and then run the SDIncCluster function

			// Next FAT not available; Determine which of incrementing and decrementing is necessary
			if (firstAvailableAllocUnit > *nextAllocUnit)
				// Decrementing is necessary to find FAT
				while (firstAvailableAllocUnit > *nextAllocUnit) {
					// Decrement current FAT sector until we reach the correct one
					SDReadDataBlock(--g_sd_curFatSector, g_sd_fat);
					firstAvailableAllocUnit = g_sd_curFatSector
							<< g_sd_entriesPerFatSector_Shift;
					endAvailableAllocUnit = (g_sd_curFatSector + 1)
							<< g_sd_entriesPerFatSector_Shift;

				}
			else if (*nextAllocUnit > endAvailableAllocUnit)
				// Incrementing is necessary to find FAT
				while (*nextAllocUnit > endAvailableAllocUnit) {
					// Increment the current FAT sector until we reach the correct one
					SDReadDataBlock(++g_sd_curFatSector + g_sd_fatStart, g_sd_fat);
					firstAvailableAllocUnit = g_sd_curFatSector
							<< g_sd_entriesPerFatSector_Shift;
					endAvailableAllocUnit = (g_sd_curFatSector + 1)
							<< g_sd_entriesPerFatSector_Shift;

				}

			// The necessary FAT sector has been loaded and the next allocation unit is known,
			// proceed with loading the next data sector and incrementing the cluster variables
			return SDIncCluster(curSectorOffset, nextAllocUnit, curAllocUnit,
					curClusterStartAddr, buf);
		}
	}

#if (defined SD_VERBOSE_BLOCKS && defined SD_VERBOSE && defined SD_DEBUG)
	__simple_printf("New sector loaded:\n");
	SDPrintHexBlock(buf, SD_SECTOR_SIZE);
	putchar('\n');
#endif

	return 0;
}

uint8 SDIncCluster (uint8 *curSectorOffset, uint32 *nextAllocUnit, uint32 *curAllocUnit,
		uint32 *curClusterStartAddr, uint8 *buf) {
	// Update g_sd_cur*
	*curAllocUnit = *nextAllocUnit;
	*curClusterStartAddr = (*curAllocUnit - 2)
			<< g_sd_sectorsPerCluster_shift + g_sd_firstDataAddr;
	*curSectorOffset = 0;

	// Retrieve the next allocation unit number
	*nextAllocUnit =
			SDConvertDat16(
					&g_sd_fat[(*curAllocUnit - g_sd_curFatSector
							<< g_sd_entriesPerFatSector_Shift) >> 4]);

	return SDReadDataBlock(*curClusterStartAddr, buf);
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
		g_sd_sd_invalidResponse = *dat;
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
			g_sd_sd_invalidResponse = *dat;
#endif
			return SD_INVALID_DAT_STRT_ID;
		}
	} else {
#ifdef SD_DEBUG
		g_sd_sd_invalidResponse = *dat;
#endif
		return SD_INVALID_RESPONSE;
	}

	return 0;
}

uint8 SDReadDataBlock (uint32 address, uint8 *dat) {
	uint8 err;

	GPIOPinClear(g_sd_cs);
	if (err = SDSendCommand(SD_CMD_RD_BLOCK, address, SD_CRC_OTHER))
		SDError(err);

	if (err = SDReadBlock(SD_SECTOR_SIZE, dat))
		SDError(err);
	GPIOPinSet(g_sd_cs);

	return 0;
}

uint16 SDConvertDat16 (const uint8 dat[]) {
	return (dat[1] << 8) + dat[0];
}

uint32 SDConvertDat32 (const uint8 dat[]) {
	return (dat[3] << 24) + (dat[2] << 16) + (dat[1] << 8) + dat[0];
}

uint32 SDGetSectorFromPath (const char *path) {
// TODO: Return an actual path
	return g_sd_rootAddr;
}

uint32 SDGetSectorFromAlloc (const uint32 allocUnit) {
	return ((uint32) (allocUnit - 2) << g_sd_sectorsPerCluster_shift) + g_sd_firstDataAddr;
}

uint8 SDOpenFile_ptr (const uint16 filePtr, uint32 *fileLen) {
#ifdef SD_SPEED_OVER_SPACE
	g_sd_curAllocUnit_file = SDConvertDat16(
			&g_sd_buf[filePtr + SD_FILE_START_CLSTR_OFFSET]);
	*fileLen = SDConvertDat32(&g_sd_buf[filePtr + SD_FILE_LEN_OFFSET]);
	g_sd_curClusterStartAddr_file = SDGetSectorFromAlloc(g_sd_curAllocUnit_file);
	g_sd_curSectorOffset_file = 0;
	SDReadDataBlock(g_sd_curClusterStartAddr_file, g_sd_file);

	// TODO: These debug lines only work if optimization is turn on. FIX IT!
#if (defined SD_VERBOSE && defined SD_DEBUG)
	printf("Opening file from...\n");
	printf("\tAllocation unit 0x%08X\n", g_sd_curAllocUnit_file);
	printf("\tCluster starting address 0x%08X\n", g_sd_curClusterStartAddr_file);
	printf("\tSector offset 0x%04X\n", g_sd_curSectorOffset_file);
#ifdef SD_VERBOSE_BLOCKS
	printf("And the first file sector looks like....\n");
	SDPrintHexBlock(g_sd_file, SD_SECTOR_SIZE);
	putchar('\n');
#endif
#endif
#else
	g_sd_curAllocUnit = SDConvertDat16(&g_sd_buf[filePtr + SD_FILE_START_CLSTR_OFFSET]);
	*fileLen = SDConvertDat32(&g_sd_buf[filePtr + SD_FILE_LEN_OFFSET]);
	g_sd_curClusterStartAddr = SDGetSectorFromAlloc(g_sd_curAllocUnit);
	g_sd_curSectorOffset = 0;
	SDReadDataBlock(g_sd_curClusterStartAddr, g_sd_buf);

#if (defined SD_VERBOSE && defined SD_DEBUG)
	printf("Opening file from...\n");
	printf("\tAllocation unit 0x%08X\n", g_sd_curAllocUnit);
	printf("\tCluster starting address 0x%08X\n", g_sd_curClusterStartAddr);
	printf("\tSector offset 0x%04X\n", g_sd_curSectorOffset);
#ifdef SD_VERBOSE_BLOCKS
	printf("And the first file sector looks like....\n");
	SDPrintHexBlock(g_sd_buf, SD_SECTOR_SIZE);
	putchar('\n');
#endif
#endif
#endif
	return 0;
}

#ifdef SD_SHELL
uint8 SD_Shell_ls (void) {
	uint8 err;
	uint16 rootIdx = 0, filePtr = 0;
	char string[SD_FILENAME_STR_LEN];			// Allocate space for a filename string

	// If we aren't looking at the beginning of a cluster, we must backtrack to the beginning and then begin listing files
	if (g_sd_curSectorOffset || g_sd_curDirStartAddr != g_sd_curClusterStartAddr) {
#if (defined SD_VERBOSE && defined SD_DEBUG)
		__simple_printf(
				"'ls' requires a backtrack to beginning of directory's cluster\n");
#endif
		g_sd_curClusterStartAddr = g_sd_curDirStartAddr;
		g_sd_curSectorOffset = 0;
		if (-1 != g_sd_curDirStartAddr)
			g_sd_curAllocUnit = g_sd_curDirAllocUnit;
		if (err = SDReadDataBlock(g_sd_curClusterStartAddr, g_sd_buf))
			SDError(err);
	}

	// stuff..
	while (g_sd_buf[filePtr]) {
		// If the file exists, print its name and attributes
		if (SD_DELETED_FILE_MARK != g_sd_buf[filePtr])
			SDPrintFileEntry(&g_sd_buf[filePtr], string);

		// If we have reached the end of this sector, proceed to the next sector if it exists
		if (SD_SECTOR_SIZE == filePtr) {
			if (err = SDReadDataBlock(++rootIdx, g_sd_buf))
				SDError(err);
			++g_sd_curSectorOffset;
			filePtr = 0;
		}

		filePtr += SD_FILE_ENTRY_LENGTH;
	}
	return 0;
}

uint8 SD_Shell_cat (const char *f) {
	uint8 err;
	uint32 next;
	uint16 filePtr = 0;
	uint16 tempSeek = 0;// Create a temporary file seek pointer to preserve the global seek pointer
	uint32 fileLen;
	char readFilename[SD_FILENAME_STR_LEN];

	// If we aren't looking at the beginning of a cluster, we must backtrack to the beginning and then begin listing files
	if (g_sd_curSectorOffset || (g_sd_curDirStartAddr != g_sd_curClusterStartAddr)) {
#if (defined SD_VERBOSE && defined SD_DEBUG)
		__simple_printf(
				"'cat' requires a backtrack to beginning of directory's cluster\n");
#endif
		g_sd_curClusterStartAddr = g_sd_curDirStartAddr;
		g_sd_curSectorOffset = 0;
		if (-1 != g_sd_curDirStartAddr)
			g_sd_curAllocUnit = g_sd_curDirAllocUnit;
		if (err = SDReadDataBlock(g_sd_curClusterStartAddr, g_sd_buf))
			SDError(err);
	}

	// Loop through all files in the current directory until we find the correct one
	while (g_sd_buf[filePtr]) {
		// Check if file is valid, retrieve the name if it is
		if (!(SD_DELETED_FILE_MARK == g_sd_buf[filePtr])) {
			SDGetFilename(&g_sd_buf[filePtr], readFilename);
			if (!strcmp(f, readFilename))
				break;// Break loop and use current value of 'filePtr' to read in the file
		}

		// Increment to the next file
		filePtr += SD_FILE_ENTRY_LENGTH;

		// If it was the last entry in this sector, proceed to the next one
		if (SD_SECTOR_SIZE == filePtr)
			SDLoadNextSector(g_sd_buf);
	}

	// Did the find loop quit without finding the file?
	if (!(g_sd_buf[filePtr]))
		// Find loop quit without finding the file; Report error
		__simple_printf("\tError: File not found\n");
	// File was found, let's print it to the screen now...
	else {
#if (defined SD_VERBOSE && defined SD_DEBUG)
		printf("%s found at offset 0x%04X from address 0x%08X\n", readFilename, filePtr,
				g_sd_curClusterStartAddr + g_sd_curSectorOffset);
#endif

		// Open it and begin putting characters on the screen
		if (err = SDOpenFile_ptr(filePtr, &fileLen))
			SDError(err);

		// Loop over each character and print them to the screen one-by-one
		while (fileLen--) {
			if (SD_SECTOR_SIZE == tempSeek) {
#ifdef SD_SPEED_OVER_SPACE
				SDLoadNextSector(g_sd_file);
#else
				SDLoadNextSector(g_sd_buf);
#endif
				tempSeek = 0;
			}
#ifdef SD_SPEED_OVER_SPACE
			putchar(g_sd_file[tempSeek++]);
#else
			putchar(g_sd_buf[tempSeek++]);
#endif
		}
	}

	return 0;
}

inline void SDPrintFileEntry (const uint8 *file, uint8 filename[]) {
	SDPrintFileAttributes(file[SD_FILE_ATTRIBUTE_OFFSET]);
	SDGetFilename(file, filename);
	__simple_printf("\t\t%s\n", filename);
}

void SDPrintFileAttributes (const uint8 flag) {
// Print file attributes
	if (SD_READ_ONLY & flag)
		putchar(SD_READ_ONLY_CHAR);
	else
		putchar(SD_READ_ONLY_CHAR_);

	if (SD_HIDDEN_FILE & flag)
		putchar(SD_HIDDEN_FILE_CHAR);
	else
		putchar(SD_HIDDEN_FILE_CHAR_);

	if (SD_SYSTEM_FILE & flag)
		putchar(SD_SYSTEM_FILE_CHAR);
	else
		putchar(SD_SYSTEM_FILE_CHAR_);

	if (SD_VOLUME_ID & flag)
		putchar(SD_VOLUME_ID_CHAR);
	else
		putchar(SD_VOLUME_ID_CHAR_);

	if (SD_SUB_DIR & flag)
		putchar(SD_SUB_DIR_CHAR);
	else
		putchar(SD_SUB_DIR_CHAR_);

	if (SD_ARCHIVE & flag)
		putchar(SD_ARCHIVE_CHAR);
	else
		putchar(SD_ARCHIVE_CHAR_);
}
#endif

#ifdef SD_VERBOSE
uint8 SDPrintHexBlock (uint8 *dat, uint16 bytes) {
	uint8 i, j;
	uint8 *s;

	__simple_printf("Printing %u bytes...\n", bytes);
	__simple_printf("Offset\t");
	for (i = 0; i < SD_LINE_SIZE; ++i)
		printf("0x%X  ", i);
	putchar('\n');

	if (bytes % SD_LINE_SIZE)
		bytes = bytes / SD_LINE_SIZE + 1;
	else
		bytes /= SD_LINE_SIZE;

	for (i = 0; i < bytes; ++i) {
		s = (uint8 *) (dat + SD_LINE_SIZE * i);
		printf("0x%04X:\t", SD_LINE_SIZE * i);
		for (j = 0; j < SD_LINE_SIZE; ++j)
			printf("0x%02X ", s[j]);
		__simple_printf(" - ");
		for (j = 0; j < SD_LINE_SIZE; ++j) {
			if ((' ' <= s[j]) && (s[j] <= '~'))
				putchar(s[j]);
			else
				putchar('.');
		}

		putchar('\n');
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
			printf("SD Error %u: %s0x%02X\nThe following bits are set:\n",
					(err - SD_ERRORS_BASE), "Invalid first-byte response\n\tReceived: ",
					g_sd_sd_invalidResponse);
#else
			__simple_printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
					"Invalid first-byte response\n\tReceived: ", g_sd_sd_invalidResponse);
#endif
			SDFirstByteExpansion(g_sd_sd_invalidResponse);
			break;
		case SD_INVALID_DAT_STRT_ID:
#ifdef SD_VERBOSE
			printf("SD Error %u: %s0x%02X\n", (err - SD_ERRORS_BASE),
					"Invalid data-start ID\n\tReceived: ", g_sd_sd_invalidResponse);
#else
			__simple_printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
					"Invalid data-start ID\n\tReceived: ", g_sd_sd_invalidResponse);
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
				__simple_printf("Unknown SD error %u\n", (err - SD_ERRORS_BASE));
// If not, print unknown error
			else
				__simple_printf("Unknown error %u\n", err);
			break;
	}
	while (1)
		;
}

void SDFirstByteExpansion (const uint8 response) {
	if (BIT_0 & response)
		__simple_printf("\t0: Idle\n");
	if (BIT_1 & response)
		__simple_printf("\t1: Erase reset\n");
	if (BIT_2 & response)
		__simple_printf("\t2: Illegal command\n");
	if (BIT_3 & response)
		__simple_printf("\t3: Communication CRC error\n");
	if (BIT_4 & response)
		__simple_printf("\t4: Erase sequence error\n");
	if (BIT_5 & response)
		__simple_printf("\t5: Address error\n");
	if (BIT_6 & response)
		__simple_printf("\t6: Parameter error\n");
	if (BIT_7 & response)
		__simple_printf(
				"\t7: Something is really screwed up. This should always be 0.\n");
}
#endif
