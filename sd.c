/**
 * @file    sd.c
 */
/**
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

// Includes
#include <sd.h>

#define sd_check_errors(x)  if ((err = x)) sd_error(err)

#ifdef SD_OPTION_DEBUG
/* @brief   Print an error through UART string followed by entering an infinite
 *          loop
 *
 * @param   err     Error number used to determine error string
 */
static void sd_error (const uint8_t err);

/**
 * @brief   Print to screen each status bit individually with human-readable
 *          descriptions
 *
 * @param   response    first-byte response from the SD card
 */
static void sd_first_byte_expansion (const uint8_t response);
#else
// Exit calling function by returning 'err'
#define sd_error(err)                return err
#endif

/*** Global variable declarations ***/
// Initialization variables
static uint32_t g_sd_cs;  // Chip select pin mask
static uint8_t g_sd_filesystem;  // Filesystem type - one of SD_FAT_16 or SD_FAT_32
static uint8_t g_sd_sectorsPerCluster_shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
static uint32_t g_sd_rootDirSectors;  // Number of sectors for the root directory
static uint32_t g_sd_fatStart;  // Starting block address of the FAT
static uint32_t g_sd_rootAddr;  // Starting block address of the root directory
static uint32_t g_sd_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
static uint32_t g_sd_firstDataAddr;  // Starting block address of the first data cluster

// FAT filesystem variables
static uint8_t g_sd_fat[SD_SECTOR_SIZE];        // Buffer for FAT entries only
#ifdef SD_OPTION_FILE_WRITE
static uint8_t g_sd_fatMod = 0;  // Has the currently loaded FAT sector been modified
static uint32_t g_sd_fatSize;
#endif
static uint16_t g_sd_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
static uint32_t g_sd_curFatSector;  // Store the current FAT sector loaded into g_sd_fat

static uint32_t g_sd_dir_firstAllocUnit;  // Store the current directory's starting allocation unit

SD_Buffer g_sd_buf;

// Assigned to a file and then to each buffer that it touches - overwritten by
// other functions and used as a check by the file to determine if the buffer
// needs to be reloaded with its sector
static uint8_t g_sd_fileID = 0;

// First byte response receives special treatment to allow for proper debugging
static uint8_t g_sd_firstByteResponse;

#ifdef SD_OPTION_DEBUG
// variable is needed to help determine what is causing seemingly random
// timeouts
uint32_t g_sd_sectorRdAddress;
#endif

/***********************************
 *** Public Function Definitions ***
 ***********************************/
uint8_t sd_start (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t cs, const uint32_t freq) {
    uint8_t i, j, k, err;
    uint8_t response[16];
    uint8_t cmd8Success = 0; // Flag to signal when CMD8 has succeeded

    // Set CS for output and initialize high
    g_sd_cs = cs;
    gpio_set_dir(cs, GPIO_DIR_OUT);
    gpio_pin_set(cs);

    // Start SPI module
    if ((err = spi_start(mosi, miso, sclk, SD_SPI_INIT_FREQ, SD_SPI_MODE,
    SD_SPI_BITMODE)))
        sd_error(err);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Starting SD card...\n");
#endif

    // Attempt initialization no more than 10 times
    // TODO: replace the 10x repetitive loop with timeout counter set to 1000ms
    for (i = 0; i < 10 && !cmd8Success; ++i) {
        // Initialization loop (reset SD card)
        for (j = 0; j < 10; ++j) {
            waitcnt(CLKFREQ / 10 + CNT);

            // Send at least 72 clock cycles to enable the SD card
            gpio_pin_set(cs);
            for (k = 0; k < 5; ++k)
                check_errors(spi_shift_out(16, -1));
            check_errors(spi_wait());

            // Chip select goes low for the duration of this function
            gpio_pin_clear(cs);

            // Send SD into idle state, retrieve a response and ensure it is the
            // "idle" response
            sd_check_errors(sd_send_command(SD_CMD_IDLE, 0, SD_CRC_IDLE));
            sd_get_response(SD_RESPONSE_LEN_R1, response);
            if (SD_RESPONSE_IDLE == g_sd_firstByteResponse)
                j = 10;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            else
                printf("Failed attempt at CMD0: 0x%02X\n",
                        g_sd_firstByteResponse);
#endif
        }
        if (SD_RESPONSE_IDLE != g_sd_firstByteResponse)
            sd_error(SD_INVALID_INIT);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("SD card in idle state. Now sending CMD8...\n");
#endif

        // Inform SD card that the Propeller uses the 2.7-3.6V range;
        // An echo is expected as response
        sd_check_errors(
                sd_send_command(SD_CMD_INTERFASE_COND, SD_ARG_CMD8,
                        SD_CRC_CMD8));
        sd_check_errors(sd_get_response(SD_RESPONSE_LEN_R7, response));
        if (SD_RESPONSE_IDLE == g_sd_firstByteResponse) {
            // The card is idle, that's good. Let's make sure we get the correct
            // response back
            if ((SD_HOST_VOLTAGE_3V3 == response[2]) &&
                    (SD_R7_CHECK_PATTERN == response[3]))
                // All responses were A-OKAY, exit the outer
                cmd8Success = 1;
        }
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        else
            printf("Failed attempt at CMD8: 0x%02X, 0x%02X, 0x%02X;\n",
                    g_sd_firstByteResponse, response[2], response[3]);
#endif
    }

    // If CMD8 never succeeded, throw an error
    if (!cmd8Success)
        sd_error(SD_CMD8_FAILURE);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("CMD8 succeeded. Requesting operating conditions...\n");
#endif

    // Request operating conditions register and ensure response begins with R1
    sd_check_errors(sd_send_command(SD_CMD_READ_OCR, 0, SD_CRC_OTHER));
    sd_check_errors(sd_get_response(SD_RESPONSE_LEN_R3, response));
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    sd_print_hex_block(response, SD_RESPONSE_LEN_R3);
#endif
    if (SD_RESPONSE_IDLE != g_sd_firstByteResponse)
        sd_error(SD_INVALID_INIT);

    // Spin up the card and bring to active state
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("OCR read successfully. Sending into active state...\n");
#endif
    for (i = 0; i < 8; ++i) {
        waitcnt(CLKFREQ / 10 + CNT);
        sd_check_errors(sd_send_command(SD_CMD_APP, 0, SD_CRC_OTHER));
        sd_check_errors(sd_get_response(1, response));
        sd_check_errors(sd_send_command(SD_CMD_WR_OP, BIT_30, SD_CRC_OTHER));
        sd_get_response(1, response);
        if (SD_RESPONSE_ACTIVE == g_sd_firstByteResponse)
            break;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        else
            printf("Failed attempt at active state: 0x%02X\n",
                    g_sd_firstByteResponse);
#endif
    }
    if (SD_RESPONSE_ACTIVE != g_sd_firstByteResponse)
        sd_error(SD_INVALID_RESPONSE);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Activated!\n");
#endif

    // Initialization nearly complete, increase clock
    if (((uint32_t) -1) != freq)
        spi_set_clock(freq);
    else
        spi_set_clock(SD_DEFAULT_SPI_FREQ);

    // If debugging requested, print to the screen CSD and CID registers from SD
    // card
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Requesting CSD...\n");
    sd_check_errors(sd_send_command(SD_CMD_RD_CSD, 0, SD_CRC_OTHER));
    sd_check_errors(sd_read_block(16, response));
    printf("CSD Contents:\n");
    sd_print_hex_block(response, 16);
    putchar('\n');

    printf("Requesting CID...\n");
    sd_check_errors(sd_send_command(SD_CMD_RD_CID, 0, SD_CRC_OTHER));
    sd_check_errors(sd_read_block(16, response));
    printf("CID Contents:\n");
    sd_print_hex_block(response, 16);
    putchar('\n');
#endif
    gpio_pin_set(cs);

    // Initialization complete
    return 0;
}

uint8_t sd_mount (void) {
    uint8_t err, temp;

    // FAT system determination variables:
    uint32_t rsvdSectorCount, numFATs, rootEntryCount, totalSectors, FATSize,
            dataSectors;
    uint32_t bootSector = 0;
    uint32_t clusterCount;

    // Read in first sector
    sd_check_errors(sd_read_data_block(bootSector, g_sd_buf.buf));
    // Check if sector 0 is boot sector or MBR; if MBR, skip to boot sector at
    // first partition
    if (SD_BOOT_SECTOR_ID != g_sd_buf.buf[SD_BOOT_SECTOR_ID_ADDR]) {
        bootSector = sd_read_rev_dat32(&(g_sd_buf.buf[SD_BOOT_SECTOR_BACKUP]));
        sd_check_errors(sd_read_data_block(bootSector, g_sd_buf.buf));
    }

    // Print the boot sector if requested
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG && \
        defined SD_OPTION_VERBOSE_BLOCKS)
    printf("***BOOT SECTOR***\n");
    sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
    putchar('\n');
#endif

    // Do this whether it is FAT16 or FAT32
    temp = g_sd_buf.buf[SD_CLUSTER_SIZE_ADDR];
#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Preliminary sectors per cluster: %u\n", temp);
#endif
    while (temp) {
        temp >>= 1;
        ++g_sd_sectorsPerCluster_shift;
    }
    --g_sd_sectorsPerCluster_shift;
    rsvdSectorCount = sd_read_rev_dat16(
            &((g_sd_buf.buf)[SD_RSVD_SCTR_CNT_ADDR]));
    numFATs = g_sd_buf.buf[SD_NUM_FATS_ADDR];
#ifdef SD_OPTION_FILE_WRITE
    if (2 != numFATs)
        sd_error(SD_TOO_MANY_FATS);
#endif
    rootEntryCount = sd_read_rev_dat16(&(g_sd_buf.buf[SD_ROOT_ENTRY_CNT_ADDR]));

    // Check if FAT size is valid in 16- or 32-bit location
    FATSize = sd_read_rev_dat16(&(g_sd_buf.buf[SD_FAT_SIZE_16_ADDR]));
    if (!FATSize)
        FATSize = sd_read_rev_dat32(&(g_sd_buf.buf[SD_FAT_SIZE_32_ADDR]));

    // Check if FAT16 total sectors is valid
    totalSectors = sd_read_rev_dat16(&(g_sd_buf.buf[SD_TOT_SCTR_16_ADDR]));
    if (!totalSectors)
        totalSectors = sd_read_rev_dat32(&(g_sd_buf.buf[SD_TOT_SCTR_32_ADDR]));

    // Compute necessary numbers to determine FAT type (12/16/32)
    g_sd_rootDirSectors = (rootEntryCount * 32) >> SD_SECTOR_SIZE_SHIFT;
    dataSectors = totalSectors
            - (rsvdSectorCount + numFATs * FATSize + rootEntryCount);
    clusterCount = dataSectors >> g_sd_sectorsPerCluster_shift;

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Sectors per cluster: %u\n", 1 << g_sd_sectorsPerCluster_shift);
    printf("Reserved sector count: 0x%08X / %u\n", rsvdSectorCount,
            rsvdSectorCount);
    printf("Number of FATs: 0x%02X / %u\n", numFATs, numFATs);
    printf("Total sector count: 0x%08X / %u\n", totalSectors, totalSectors);
    printf("Total cluster count: 0x%08X / %u\n", clusterCount, clusterCount);
    printf("Total data sectors: 0x%08X / %u\n", dataSectors, dataSectors);
    printf("FAT Size: 0x%04X / %u\n", FATSize, FATSize);
    printf("Root directory sectors: 0x%08X / %u\n", g_sd_rootDirSectors,
            g_sd_rootDirSectors);
    printf("Root entry count: 0x%08X / %u\n", rootEntryCount, rootEntryCount);
#endif

    // Determine and store FAT type
    if (SD_FAT12_CLSTR_CNT > clusterCount)
        sd_error(SD_INVALID_FILESYSTEM);
    else if (SD_FAT16_CLSTR_CNT > clusterCount) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("\n***FAT type is FAT16***\n");
#endif
        g_sd_filesystem = SD_FAT_16;
        g_sd_entriesPerFatSector_Shift = 8;
    } else {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("\n***FAT type is FAT32***\n");
#endif
        g_sd_filesystem = SD_FAT_32;
        g_sd_entriesPerFatSector_Shift = 7;
    }

    // Find start of FAT
    g_sd_fatStart = bootSector + rsvdSectorCount;

    //    g_sd_filesystem = SD_FAT_16;
    // Find root directory address
    switch (g_sd_filesystem) {
        case SD_FAT_16:
            g_sd_rootAddr = FATSize * numFATs + g_sd_fatStart;
            g_sd_firstDataAddr = g_sd_rootAddr + g_sd_rootDirSectors;
            break;
        case SD_FAT_32:
            g_sd_firstDataAddr = g_sd_rootAddr = bootSector + rsvdSectorCount
                    + FATSize * numFATs;
            g_sd_rootAllocUnit = sd_read_rev_dat32(
                    &(g_sd_buf.buf[SD_ROOT_CLUSTER_ADDR]));
            break;
    }

#ifdef SD_OPTION_FILE_WRITE
    // If files will be written to, the second FAT must also be updated - the
    // first sector address of which is stored here
    g_sd_fatSize = FATSize;
#endif

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Start of FAT: 0x%08X\n", g_sd_fatStart);
    printf("Root directory alloc. unit: 0x%08X\n", g_sd_rootAllocUnit);
    printf("Root directory sector: 0x%08X\n", g_sd_rootAddr);
    printf("Calculated root directory sector: 0x%08X\n",
            sd_find_sector_from_alloc(g_sd_rootAllocUnit));
    printf("First data sector: 0x%08X\n", g_sd_firstDataAddr);
#endif

    // Store the first sector of the FAT
    sd_check_errors(sd_read_data_block(g_sd_fatStart, g_sd_fat));
    g_sd_curFatSector = 0;

    // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG && \
        defined SD_OPTION_VERBOSE_BLOCKS)
    printf("\n***First File Allocation Table***\n");
    sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
    putchar('\n');
#endif

    // Read in the root directory, set root as current
    sd_check_errors(sd_read_data_block(g_sd_rootAddr, g_sd_buf.buf));
    g_sd_buf.curClusterStartAddr = g_sd_rootAddr;
    if (SD_FAT_16 == g_sd_filesystem) {
        g_sd_dir_firstAllocUnit = -1;
        g_sd_buf.curAllocUnit = -1;
    } else {
        g_sd_buf.curAllocUnit = g_sd_dir_firstAllocUnit = g_sd_rootAllocUnit;
        if ((err = sd_get_fat_value(g_sd_buf.curAllocUnit,
                &g_sd_buf.nextAllocUnit)))
            return err;
    }
    g_sd_buf.curClusterStartAddr = g_sd_rootAddr;
    g_sd_buf.curSectorOffset = 0;

    // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("***Root directory***\n");
    sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint8_t sd_unmount (void) {
    uint8_t err;

    // If the directory buffer was modified, write it
    if (g_sd_buf.mod)
        if ((err = sd_write_data_block(
                g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset,
                g_sd_buf.buf)))
            return err;

    // If the FAT sector was modified, write it
    if (g_sd_fatMod) {
        if ((err = sd_write_data_block(g_sd_curFatSector + g_sd_fatStart,
                g_sd_fat)))
            return err;
        if ((err = sd_write_data_block(
                g_sd_curFatSector + g_sd_fatStart + g_sd_fatSize, g_sd_fat)))
            return err;
    }

    return 0;
}
#endif

uint8_t sd_chdir (const char *d) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;

    g_sd_buf.id = SD_FOLDER_ID;

    // Attempt to find the file
    if ((err = sd_find(d, &fileEntryOffset))) {
        return err;
    } else {
        // File entry was found successfully, load it into the buffer and update
        // status variables
#ifdef SD_OPTION_FILE_WRITE
        // If the previous sector was modified, write it back to the SD card
        // before reading
        if (g_sd_buf.mod)
            sd_write_data_block(
                    g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset,
                    g_sd_buf.buf);
        g_sd_buf.mod = 0;
#endif

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("%s found at offset 0x%04X from address 0x%08X\n", d,
                fileEntryOffset,
                g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset);
#endif

        if (SD_FAT_16 == g_sd_filesystem)
            g_sd_buf.curAllocUnit = sd_read_rev_dat16(
                    &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_LOW]));
        else {
            g_sd_buf.curAllocUnit = sd_read_rev_dat16(
                    &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_LOW]));
            g_sd_buf.curAllocUnit |= sd_read_rev_dat16(
                    &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_HIGH]))
                    << 16;
            // Clear the highest 4 bits - they are always reserved
            g_sd_buf.curAllocUnit &= 0x0FFFFFFF;
        }
        sd_get_fat_value(g_sd_buf.curAllocUnit, &(g_sd_buf.nextAllocUnit));
        if (0 == g_sd_buf.curAllocUnit) {
            g_sd_buf.curAllocUnit = -1;
            g_sd_dir_firstAllocUnit = g_sd_rootAllocUnit;
        } else
            g_sd_dir_firstAllocUnit = g_sd_buf.curAllocUnit;
        g_sd_buf.curSectorOffset = 0;
        sd_read_data_block(g_sd_buf.curClusterStartAddr, g_sd_buf.buf);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Opening directory from...\n");
        printf("\tAllocation unit 0x%08X\n", g_sd_buf.curAllocUnit);
        printf("\tCluster starting address 0x%08X\n",
                g_sd_buf.curClusterStartAddr);
        printf("\tSector offset 0x%04X\n", g_sd_buf.curSectorOffset);
#ifdef SD_OPTION_VERBOSE_BLOCKS
        printf("And the first directory sector looks like....\n");
        sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
        putchar('\n');
#endif
#endif
        return 0;
    }
}

uint8_t sd_fopen (const char *name, SD_File *f, const SD_FileMode mode) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Attempting to open %s\n", name);
#endif

    if (NULL == f->buf)
        sd_error(SD_FILE_WITHOUT_BUFFER);

    f->id = g_sd_fileID++;
    f->rPtr = 0;
    f->wPtr = 0;
#if (defined SD_OPTION_DEBUG && !(defined SD_OPTION_FILE_WRITE))
    if (SD_FILE_MODE_R != mode)
        sd_error(SD_INVALID_FILE_MODE);
#endif
    f->mode = mode;
    f->mod = 0;

    // Attempt to find the file
    if ((err = sd_find(name, &fileEntryOffset))) {
#ifdef SD_OPTION_FILE_WRITE
        // If the file didn't exist and you're trying to read from it, that's a
        // problem
        if (SD_FILE_MODE_R == mode)
            return err;

        // Find returned an error, ensure it was either file-not-found or EOC
        // and then create the file
        if (SD_EOC_END == err) {
            // File wasn't found and the cluster is full; add another to the
            // directory
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Directory cluster was full, adding another...\n");
#endif
            sd_check_errors(sd_extend_fat(&g_sd_buf));
            sd_check_errors(sd_load_next_sector(&g_sd_buf));
        }
        if (SD_EOC_END == err || SD_FILENAME_NOT_FOUND == err) {
            // File wasn't found, but there is still room in this cluster (or a
            // new cluster was just added)
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Creating a new directory entry...\n");
#endif
            sd_check_errors(sd_create_file(name, &fileEntryOffset));
        } else
#endif
            // SDFind returned unknown error - throw it
            sd_error(err);
    }

    // `name` was found successfully, determine if it is a file or directory
    if (SD_SUB_DIR & g_sd_buf.buf[fileEntryOffset + SD_FILE_ATTRIBUTE_OFFSET])
        sd_error(SD_ENTRY_NOT_FILE);

    // Passed the file-not-directory test, load it into the buffer and update
    // status variables
    f->buf->id = f->id;
    f->curSector = 0;
    if (SD_FAT_16 == g_sd_filesystem)
        f->buf->curAllocUnit = sd_read_rev_dat16(
                &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_LOW]));
    else {
        f->buf->curAllocUnit = sd_read_rev_dat16(
                &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_LOW]));
        f->buf->curAllocUnit |= sd_read_rev_dat16(
                &(g_sd_buf.buf[fileEntryOffset + SD_FILE_START_CLSTR_HIGH]))
                << 16;

        // Clear the highest 4 bits - they are always reserved
        f->buf->curAllocUnit &= 0x0FFFFFFF;
    }
    f->firstAllocUnit = f->buf->curAllocUnit;
    f->curCluster = 0;
    f->buf->curClusterStartAddr = sd_find_sector_from_alloc(
            f->buf->curAllocUnit);
    f->dirSectorAddr = g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset;
    f->fileEntryOffset = fileEntryOffset;
    sd_check_errors(
            sd_get_fat_value(f->buf->curAllocUnit, &(f->buf->nextAllocUnit)));
    f->buf->curSectorOffset = 0;
    f->length = sd_read_rev_dat32(
            &(g_sd_buf.buf[fileEntryOffset + SD_FILE_LEN_OFFSET]));
#ifdef SD_OPTION_FILE_WRITE
    // Determine the number of sectors currently allocated to this file; useful
    // in the case that the file needs to be extended
    f->maxSectors = f->length >> SD_SECTOR_SIZE_SHIFT;
    if (!(f->maxSectors))
        f->maxSectors = 1 << g_sd_sectorsPerCluster_shift;
    while (f->maxSectors % (1 << g_sd_sectorsPerCluster_shift))
        ++(f->maxSectors);
    f->buf->mod = 0;
#endif
    sd_check_errors(
            sd_read_data_block(f->buf->curClusterStartAddr, f->buf->buf));

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Opening file from...\n");
    printf("\tAllocation unit 0x%08X\n", f->buf->curAllocUnit);
    printf("\tNext allocation unit 0x%08X\n", f->buf->nextAllocUnit);
    printf("\tCluster starting address 0x%08X\n", f->buf->curClusterStartAddr);
    printf("\tSector offset 0x%04X\n", f->buf->curSectorOffset);
    printf("\tFile length 0x%08X\n", f->length);
    printf("\tMax sectors 0x%08X\n", f->maxSectors);
#ifdef SD_OPTION_VERBOSE_BLOCKS
    printf("And the first file sector looks like....\n");
    sd_print_hex_block(f->buf->buf, SD_SECTOR_SIZE);
    putchar('\n');
#endif
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint8_t sd_fclose (SD_File *f) {
    uint8_t err;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Closing file...\n");
#endif
    // If the currently loaded sector has been modified, save the changes
    if ((f->buf->id == f->id) && f->buf->mod) {
        sd_check_errors(
                sd_write_data_block(
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->buf));;
        f->buf->mod = 0;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Modified sector in file has been saved...\n");
        printf("\tDestination address: 0x%08X / %u\n",
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->curClusterStartAddr + f->buf->curSectorOffset);
        printf("\tFile first sector address: 0x%08X / %u\n",
                sd_find_sector_from_alloc(f->firstAllocUnit),
                sd_find_sector_from_alloc(f->firstAllocUnit));
#endif
    }

    // If we modified the length of the file...
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Closing file and \"f->mod\" value is %u\n", f->mod);
    printf("File length is: 0x%08X / %u\n", f->length, f->length);
#endif
    if (f->mod) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("File length has been modified - write it to the directory\n");
#endif
        // Then check if the directory sector is still loaded...
        if ((g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset)
                != f->dirSectorAddr) {
            // If it isn't, load it...
            if (g_sd_buf.mod)
                // And if it's been modified since the last read, save it...
                sd_check_errors(
                        sd_write_data_block(
                                g_sd_buf.curClusterStartAddr
                                        + g_sd_buf.curSectorOffset,
                                g_sd_buf.buf));
            sd_check_errors(sd_read_data_block(f->dirSectorAddr, g_sd_buf.buf));
        }
        // Finally, edit the length of the file
        sd_write_rev_dat32(
                &(g_sd_buf.buf[f->fileEntryOffset + SD_FILE_LEN_OFFSET]),
                f->length);
        g_sd_buf.mod = 01;
    }

    return 0;
}

uint8_t sd_fputc (const char c, SD_File *f) {
    uint8_t err;
    // Determines byte-offset within a sector
    uint16_t sectorPtr = f->wPtr % SD_SECTOR_SIZE;
    // Determine the needed file sector
    uint32_t sectorOffset = (f->wPtr >> SD_SECTOR_SIZE_SHIFT);

    // Determine if the correct sector is loaded
    if (f->buf->id != f->id)
        sd_check_errors(sd_reload_buf(f));

    // Even the the buffer was just reloaded, this snippet needs to be called in
    // order to extend the FAT if needed
    if (sectorOffset != f->curSector) {
        // Incorrect sector loaded
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need new sector:\n");
        printf("\tMax available sectors: 0x%08X / %u\n", f->maxSectors,
                f->maxSectors);
        printf("\tDesired file sector: 0x%08X / %u\n", sectorOffset,
                sectorOffset);
#endif

        // If the sector needed exceeds the available sectors, extend the file
        if (f->maxSectors == sectorOffset) {
            sd_check_errors(sd_extend_fat(f->buf));
            f->maxSectors += 1 << g_sd_sectorsPerCluster_shift;
        }

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Loading new file sector at file-offset: 0x%08X / %u\n",
                sectorOffset, sectorOffset);
#endif
        // SDLoadSectorFromOffset() will ensure that, if the current buffer has
        // been modified, it is written back to the SD card before loading a new
        // one
        sd_check_errors(sd_load_sector_from_offset(f, sectorOffset));
    }

    if (++(f->wPtr) > f->length) {
        ++(f->length);
        f->mod = 1;
    }
    f->buf->buf[sectorPtr] = c;
    f->buf->mod = 1;

    return 0;
}

uint8_t sd_fputs (char *s, SD_File *f) {
    uint8_t err;

    while (*s)
        sd_check_errors(sd_fputc(*(s++), f));

    return 0;
}
#endif

char sd_fgetc (SD_File *f) {
    char c;
    uint16_t ptr = f->rPtr % SD_SECTOR_SIZE;

    // Determine if the currently loaded sector is what we need
    uint32_t sectorOffset = (f->rPtr >> SD_SECTOR_SIZE_SHIFT);

    // Determine if the correct sector is loaded
    if (f->buf->id != f->id)
        sd_reload_buf(f);
    else if (sectorOffset != f->curSector) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("File sector offset: 0x%08X / %u\n", sectorOffset, sectorOffset);
#endif
        sd_load_sector_from_offset(f, sectorOffset);
    }
    ++(f->rPtr);
    c = f->buf->buf[ptr];
    return c;
}

char * sd_fgets (char s[], uint32_t size, SD_File *f) {
    /* Code taken from fgets.c in the propgcc source, originally written by Eric
     * R. Smith and (slightly) modified to fit this SD driver
     */
    uint32_t c;
    uint32_t count = 0;

    --size;
    while (count < size) {
        c = sd_fgetc(f);
        if ((uint32_t) SD_EOF == c)
            break;
        s[count++] = c;
        if ('\n' == c)
            break;
    }
    s[count] = 0;
    return (0 < count) ? s : NULL;
}

inline uint8_t sd_feof (SD_File *f) {
    return f->length == f->rPtr;
}

uint8_t sd_fseekr (SD_File *f, const int32_t offset, const SD_FilePos origin) {
    switch (origin) {
        case SEEK_SET:
            f->rPtr = offset;
            break;
        case SEEK_CUR:
            f->rPtr += offset;
            break;
        case SEEK_END:
            f->rPtr = f->length + offset - 1;
            break;
        default:
            return SD_INVALID_PTR_ORIGIN;
            break;
    }

    return 0;
}

uint8_t sd_fseekw (SD_File *f, const int32_t offset, const SD_FilePos origin) {
    switch (origin) {
        case SEEK_SET:
            f->wPtr = offset;
            break;
        case SEEK_CUR:
            f->wPtr += offset;
            break;
        case SEEK_END:
            f->wPtr = f->length + offset - 1;
            break;
        default:
            return SD_INVALID_PTR_ORIGIN;
            break;
    }

    return 0;
}

SD_FilePos sd_ftellr (const SD_File *f) {
    return f->rPtr;
}

SD_FilePos sd_ftellw (const SD_File *f) {
    return f->wPtr;
}

#ifdef SD_OPTION_SHELL
uint8_t sd_shell (SD_File *f) {
    char usrInput[SD_SHELL_INPUT_LEN] = "";
    char cmd[SD_SHELL_CMD_LEN] = "";
    char arg[SD_SHELL_ARG_LEN] = "";
    char uppercaseName[SD_SHELL_ARG_LEN] = "";
    uint8_t i, j, err;

    printf("Welcome to David's quick shell! "
            "There is no help, nor much to do.\n");
    printf("Have fun...\n");

    // Loop until the user types the SD_SHELL_EXIT string
    while (strcmp(usrInput, SD_SHELL_EXIT)) {
        printf(">>> ");
        gets(usrInput);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Received \"%s\" as the complete line\n", usrInput);
#endif

        // Retrieve command
        for (i = 0; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
            cmd[i] = usrInput[i];

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Received \"%s\" as command\n", cmd);
#endif

        // Retrieve argument if it exists (skip over spaces)
        if (0 != usrInput[i]) {
            j = 0;
            while (' ' == usrInput[i])
                ++i;
            for (; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
                arg[j++] = usrInput[i];
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("And \"%s\" as the argument\n", arg);
#endif
        }

        // Convert the arg to uppercase
        for (i = 0; arg[i]; ++i)
            if ('a' <= arg[i] && 'z' >= arg[i])
                uppercaseName[i] = arg[i] + 'A' - 'a';
            else
                uppercaseName[i] = arg[i];

        // Interpret the command
        if (!strcmp(cmd, SD_SHELL_LS))
            err = sd_shell_ls();
        else if (!strcmp(cmd, SD_SHELL_CAT))
            err = sd_shell_cat(uppercaseName, f);
        else if (!strcmp(cmd, SD_SHELL_CD))
            err = sd_chdir(uppercaseName);
#ifdef SD_OPTION_FILE_WRITE
        else if (!strcmp(cmd, SD_SHELL_TOUCH))
            err = sd_shell_touch(uppercaseName);
#endif
#ifdef SD_OPTION_VERBOSE_BLOCKS
        else if (!strcmp(cmd, "d"))
            sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
#endif
        else if (!strcmp(cmd, SD_SHELL_EXIT))
            break;
        else if (strcmp(usrInput, ""))
            printf("Invalid command: %s\n", cmd);

        // Check for errors
        if ((uint8_t) SD_EOC_END == err)
            printf("\tError, entry not found: \"%s\"\n", arg);
        else if ((uint8_t) SD_ENTRY_NOT_FILE == err)
            printf("\tError, entry not a file: \"%s\"\n", arg);
        else if ((uint8_t) SD_FILE_ALREADY_EXISTS == err)
            printf("\tError, file already exists: \"%s\"\n", arg);
        else if (err)
            printf("Error occurred: 0x%02X / %u\n", err, err);

        // Erase the command and argument strings
        for (i = 0; i < SD_SHELL_CMD_LEN; ++i)
            cmd[i] = 0;
        for (i = 0; i < SD_SHELL_ARG_LEN; ++i)
            uppercaseName[i] = arg[i] = 0;
        err = 0;
    }

    return 0;
}

uint8_t sd_shell_ls (void) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;
    char string[SD_FILENAME_STR_LEN];  // Allocate space for a filename string

    // If we aren't looking at the beginning of a cluster, we must backtrack to
    // the beginning and then begin listing files
    if (g_sd_buf.curSectorOffset
            || (sd_find_sector_from_alloc(g_sd_dir_firstAllocUnit)
                    != g_sd_buf.curClusterStartAddr)) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("'ls' requires a backtrack to beginning of directory's "
                "cluster\n");
#endif
        g_sd_buf.curClusterStartAddr = sd_find_sector_from_alloc(
                g_sd_dir_firstAllocUnit);
        g_sd_buf.curSectorOffset = 0;
        g_sd_buf.curAllocUnit = g_sd_dir_firstAllocUnit;
        if ((err = sd_get_fat_value(g_sd_buf.curAllocUnit,
                &(g_sd_buf.nextAllocUnit))))
            return err;
        if ((err = sd_read_data_block(g_sd_buf.curClusterStartAddr,
                g_sd_buf.buf)))
            return err;
    }

    // Loop through all files in the current directory until we find the correct
    // one; Function will exit normally without an error code if the file is not
    // found
    while (g_sd_buf.buf[fileEntryOffset]) {
        // Check if file is valid, retrieve the name if it is
        if ((SD_DELETED_FILE_MARK != g_sd_buf.buf[fileEntryOffset])
                && !(SD_SYSTEM_FILE
                        & g_sd_buf.buf[fileEntryOffset
                                + SD_FILE_ATTRIBUTE_OFFSET]))
            sd_print_file_entry(&(g_sd_buf.buf[fileEntryOffset]), string);

        // Increment to the next file
        fileEntryOffset += SD_FILE_ENTRY_LENGTH;

        // If it was the last entry in this sector, proceed to the next one
        if (SD_SECTOR_SIZE == fileEntryOffset) {
            // Last entry in the sector, attempt to load a new sector
            // Possible error value includes end-of-chain marker
            if ((err = sd_load_next_sector(&g_sd_buf))) {
                if ((uint8_t) SD_EOC_END == err)
                    break;
                else
                    sd_error(err);
            }

            fileEntryOffset = 0;
        }
    }

    return 0;
}

uint8_t sd_shell_cat (const char *name, SD_File *f) {
    uint8_t err;

    // Attempt to find the file
    if ((err = sd_fopen(name, f, SD_FILE_MODE_R))) {
        if ((uint8_t) SD_EOC_END == err)
            return err;
        else
            sd_error(err);
    } else {
        // Loop over each character and print them to the screen one-by-one
        while (!sd_feof(f))
            // Using SDfgetc() instead of SDfgets to ensure compatibility with
            // binary files
            // TODO: Should probably create something better to output binary
            //       files don't ya think!?
            putchar(sd_fgetc(f));
        putchar('\n');
    }

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint8_t sd_shell_touch (const char name[]) {
    uint8_t err;
    uint16_t fileEntryOffset;

    // Attempt to find the file if it already exists
    if ((err = sd_find(name, &fileEntryOffset))) {
        // Error occured - hopefully it was a "file not found" error
        if (SD_FILENAME_NOT_FOUND == err)
            // File wasn't found, let's create it
            err = sd_create_file(name, &fileEntryOffset);
        return err;
    }

    // If SDFind() returns 0, the file already existed and an error should be
    // thrown
    return SD_FILE_ALREADY_EXISTS;
}
#endif
#endif

#if (defined SD_OPTION_VERBOSE || defined SD_OPTION_VERBOSE_BLOCKS)
uint8_t sd_print_hex_block (uint8_t *dat, uint16_t bytes) {
    uint8_t i, j;
    uint8_t *s;

    printf("Printing %u bytes...\n", bytes);
    printf("Offset\t");
    for (i = 0; i < SD_LINE_SIZE; ++i)
        printf("0x%X  ", i);
    putchar('\n');

    if (bytes % SD_LINE_SIZE)
        bytes = bytes / SD_LINE_SIZE + 1;
    else
        bytes /= SD_LINE_SIZE;

    for (i = 0; i < bytes; ++i) {
        s = (uint8_t *) (dat + SD_LINE_SIZE * i);
        printf("0x%04X:\t", SD_LINE_SIZE * i);
        for (j = 0; j < SD_LINE_SIZE; ++j)
            printf("0x%02X ", s[j]);
        printf(" - ");
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

/************************************
 *** Private Function Definitions ***
 ************************************/
uint8_t sd_send_command (const uint8_t cmd, const uint32_t arg,
        const uint8_t crc) {
    uint8_t err;

    // Send out the command
    if ((err = spi_shift_out(8, cmd)))
        return err;

    // Send argument
    if ((err = spi_shift_out(16, (arg >> 16))))
        return err;
    if ((err = spi_shift_out(16, arg & WORD_0)))
        return err;

    // Send sixth byte - CRC
    if ((err = spi_shift_out(8, crc)))
        return err;

    return 0;
}

uint8_t sd_get_response (uint8_t bytes, uint8_t *dat) {
    uint8_t err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD_RESPONSE_TIMEOUT + CNT;
    do {
        if ((err = spi_shift_in(8, &g_sd_firstByteResponse,
                sizeof(g_sd_firstByteResponse))))
            return err;

        // Check for timeout
        if (abs(timeout - CNT) < SD_WIGGLE_ROOM)
            return SD_READ_TIMEOUT;
    } while (0xff == g_sd_firstByteResponse);  // wait for transmission end

    // First byte in a response should always be either IDLE or ACTIVE. If this
    // one wans't, throw an error. If it was, decrement the bytes counter and
    // read in all remaining bytes
    if ((SD_RESPONSE_IDLE == g_sd_firstByteResponse)
            || (SD_RESPONSE_ACTIVE == g_sd_firstByteResponse)) {
        --bytes;    // Decrement bytes counter

        // Read remaining bytes
        while (bytes--)
            if ((err = spi_shift_in(8, dat++, sizeof(*dat))))
                return err;
    } else
        return SD_INVALID_RESPONSE;

    // Responses should always be followed up by outputting 8 clocks with MOSI
    // high
    if ((err = spi_shift_out(8, 0xff)))
        return err;

    return 0;
}

uint8_t sd_read_block (uint16_t bytes, uint8_t *dat) {
    uint8_t i, err, checksum;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD_RESPONSE_TIMEOUT + CNT;
    do {
        if ((err = spi_shift_in(8, &g_sd_firstByteResponse,
                sizeof(g_sd_firstByteResponse))))
            return err;

        // Check for timeout
        if (0 < (timeout - CNT) && (timeout - CNT) < SD_WIGGLE_ROOM)
            return SD_READ_TIMEOUT;
    } while (0xff == g_sd_firstByteResponse);  // wait for transmission end

    // Ensure this response is "active"
    if (SD_RESPONSE_ACTIVE == g_sd_firstByteResponse) {
        // Ignore blank data again
        timeout = SD_RESPONSE_TIMEOUT + CNT;
        do {
            if ((err = spi_shift_in(8, dat, sizeof(*dat))))
                return err;

            // Check for timeout
            if ((timeout - CNT) < SD_WIGGLE_ROOM)
                return SD_READ_TIMEOUT;
        } while (SD_DATA_START_ID != *dat);  // wait for transmission end

        // Check for the data start identifier and continue reading data
        if (SD_DATA_START_ID == *dat) {
            // Read in requested data bytes
#if (defined SPI_FAST_SECTOR)
            if (SD_SECTOR_SIZE == bytes) {
                spi_shift_in_sector(dat, 1);
                bytes = 0;
            }
#endif
            while (bytes--) {
#if (defined SD_OPTION_DEBUG)
                if ((err = spi_shift_in(8, dat++, sizeof(*dat))))
                    return err;
#elif (defined SPI_FAST)
                spi_shift_in_fast(8, dat++, sizeof(*dat));
#else
                spi_shift_in(8, dat++, SD_SPI_BYTE_IN_SZ);
#endif
            }

            // Read two more bytes for checksum - throw away data
            for (i = 0; i < 2; ++i) {
                timeout = SD_RESPONSE_TIMEOUT + CNT;
                do {
                    if ((err = spi_shift_in(8, &checksum, sizeof(checksum))))
                        return err;

                    // Check for timeout
                    if ((timeout - CNT) < SD_WIGGLE_ROOM)
                        return SD_READ_TIMEOUT;
                } while (0xff == checksum);  // wait for transmission end
            }

            // Send final 0xff
            if ((err = spi_shift_out(8, 0xff)))
                return err;
        } else {
            return SD_INVALID_DAT_STRT_ID;
        }
    } else
        return SD_INVALID_RESPONSE;

    return 0;
}

uint8_t sd_write_block (uint16_t bytes, uint8_t *dat) {
    uint8_t err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD_RESPONSE_TIMEOUT + CNT;
    do {
        if ((err = spi_shift_in(8, &g_sd_firstByteResponse,
                sizeof(g_sd_firstByteResponse))))
            return err;

        // Check for timeout
        if (0 < (timeout - CNT) && (timeout - CNT) < SD_WIGGLE_ROOM)
            return SD_READ_TIMEOUT;
    } while (0xff == g_sd_firstByteResponse);  // wait for transmission end

// Ensure this response is "active"
    if (SD_RESPONSE_ACTIVE == g_sd_firstByteResponse) {
        // Received "active" response

        // Send data Start ID
        if ((err = spi_shift_out(8, SD_DATA_START_ID)))
            return err;

        // Send all bytes
        while (bytes--) {
#if (defined SD_OPTION_DEBUG)
            if ((err = spi_shift_out(8, *(dat++))))
                return err;
#elif (defined SPI_FAST)
            spi_shift_out_fast(8, *(dat++));
#else
            spi_shift_out(8, *(dat++));
#endif
        }

        // Receive and digest response token
        timeout = SD_RESPONSE_TIMEOUT + CNT;
        do {
            if ((err = spi_shift_in(8, &g_sd_firstByteResponse,
                    sizeof(g_sd_firstByteResponse))))
                return err;

            // Check for timeout
            if (0 < (timeout - CNT) && (timeout - CNT) < SD_WIGGLE_ROOM)
                return SD_READ_TIMEOUT;
        } while (0xff == g_sd_firstByteResponse);  // wait for transmission end
        if (SD_RSPNS_TKN_ACCPT
                != (g_sd_firstByteResponse & (uint8_t) SD_RSPNS_TKN_BITS))
            return SD_INVALID_RESPONSE;
    }

    return 0;
}

uint8_t sd_read_data_block (uint32_t address, uint8_t *dat) {
    uint8_t err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        spi_shift_in(8, &temp, 1);

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Reading block at sector address: 0x%08X / %u\n", address, address);
#endif

    gpio_pin_clear(g_sd_cs);
    if ((err = sd_send_command(SD_CMD_RD_BLOCK, address,
    SD_CRC_OTHER)))
        return err;

    if ((err = sd_read_block(SD_SECTOR_SIZE, dat))) {
#ifdef SD_OPTION_DEBUG
        g_sd_sectorRdAddress = address;
#endif
        return err;
    }
    gpio_pin_set(g_sd_cs);

    return 0;
}

uint8_t sd_write_data_block (uint32_t address, uint8_t *dat) {
    uint8_t err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        spi_shift_in(8, &temp, 1);

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Writing block at address: 0x%08X / %u\n", address, address);
#endif

    gpio_pin_clear(g_sd_cs);
    if ((err = sd_send_command(SD_CMD_WR_BLOCK, address,
    SD_CRC_OTHER)))
        return err;

    if ((err = sd_write_block(SD_SECTOR_SIZE, dat)))
        return err;
    gpio_pin_set(g_sd_cs);

    return 0;
}

uint16_t sd_read_rev_dat16 (const uint8_t buf[]) {
    return (buf[1] << 8) + buf[0];
}

uint32_t sd_read_rev_dat32 (const uint8_t buf[]) {
    return (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
}

#ifdef SD_OPTION_FILE_WRITE
void sd_write_rev_dat16 (uint8_t buf[], const uint16_t dat) {
    buf[1] = (uint8_t) (dat >> 8);
    buf[0] = (uint8_t) dat;
}

void sd_write_rev_dat32 (uint8_t buf[], const uint32_t dat) {
    buf[3] = (uint8_t) (dat >> 24);
    buf[2] = (uint8_t) (dat >> 16);
    buf[1] = (uint8_t) (dat >> 8);
    buf[0] = (uint8_t) dat;
}
#endif

uint32_t sd_find_sector_from_path (const char *path) {
    // TODO: Return an actual path

    /*if ('/' == path[0]) {
     } // Start from the root address
     else {
     } // Start from the current directory*/

    return g_sd_rootAddr;
}

uint32_t sd_find_sector_from_alloc (uint32_t allocUnit) {
    if (SD_FAT_32 == g_sd_filesystem)
        allocUnit -= g_sd_rootAllocUnit;
    else
        allocUnit -= 2;
    allocUnit <<= g_sd_sectorsPerCluster_shift;
    allocUnit += g_sd_firstDataAddr;
    return allocUnit;
}

uint8_t sd_get_fat_value (const uint32_t fatEntry, uint32_t *value) {
    uint8_t err;
    uint32_t firstAvailableAllocUnit;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Reading from the FAT...\n");
    printf("\tLooking for entry: 0x%08X / %u\n", fatEntry, fatEntry);
#endif

    // Do we need to load a new fat sector?
    if ((fatEntry >> g_sd_entriesPerFatSector_Shift) != g_sd_curFatSector) {
#ifdef SD_OPTION_FILE_WRITE
        // If the currently loaded FAT sector has been modified, save it
        if (g_sd_fatMod) {
            sd_write_data_block(g_sd_curFatSector + g_sd_fatStart, g_sd_fat);
            sd_write_data_block(
                    g_sd_curFatSector + g_sd_fatStart + g_sd_fatSize, g_sd_fat);
            g_sd_fatMod = 0;
        }
#endif
        // Need new sector, load it
        g_sd_curFatSector = fatEntry >> g_sd_entriesPerFatSector_Shift;
        if ((err = sd_read_data_block(g_sd_curFatSector + g_sd_fatStart,
                g_sd_fat)))
            return err;
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
        sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif
    }
    firstAvailableAllocUnit = g_sd_curFatSector
            << g_sd_entriesPerFatSector_Shift;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("\tLooks like I need FAT sector: 0x%08X / %u\n", g_sd_curFatSector,
            g_sd_curFatSector);
    printf("\tWith an offset of: 0x%04X / %u\n",
            (fatEntry - firstAvailableAllocUnit) << 2,
            (fatEntry - firstAvailableAllocUnit) << 2);
#endif

    // The necessary FAT sector has been loaded and the next allocation unit is
    // known, proceed with loading the next data sector and incrementing the
    // cluster variables

    // Retrieve the next allocation unit number
    if (SD_FAT_16 == g_sd_filesystem)
        *value = sd_read_rev_dat16(
                &g_sd_fat[(fatEntry - firstAvailableAllocUnit) << 1]);
    else
        /* Implied check for (SD_FAT_32 == g_sd_filesystem) */
        *value = sd_read_rev_dat32(
                &g_sd_fat[(fatEntry - firstAvailableAllocUnit) << 2]);
    // Clear the highest 4 bits - they are always reserved
    *value &= 0x0FFFFFFF;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("\tReceived value: 0x%08X / %u\n", *value, *value);
#endif

    return 0;
}

uint8_t sd_load_next_sector (SD_Buffer *buf) {
#ifdef SD_OPTION_FILE_WRITE
    if (buf->mod)
        sd_write_data_block(buf->curClusterStartAddr + buf->curSectorOffset,
                buf->buf);
#endif

    // Check for the end-of-chain marker (end of file)
    if (((uint32_t) SD_EOC_BEG) <= buf->nextAllocUnit)
        return SD_EOC_END;

    // Are we looking at the root directory of a FAT16 system?
    if (SD_FAT_16 == g_sd_filesystem
            && g_sd_rootAddr == (buf->curClusterStartAddr)) {
        // Root dir of FAT16; Is it the last sector in the root directory?
        if (g_sd_rootDirSectors == (buf->curSectorOffset))
            return SD_EOC_END;
        // Root dir of FAT16; Not last sector
        else
            // Any error from reading the data block will be returned to calling
            // function
            return sd_read_data_block(++(buf->curSectorOffset), buf->buf);
    }
    // We are looking at a generic data cluster.
    else {
        // Gen. data cluster; Have we reached the end of the cluster?
        if (((1 << g_sd_sectorsPerCluster_shift) - 1)
                > (buf->curSectorOffset)) {
            // Gen. data cluster; Not the end; Load next sector in the cluster

            // Any error from reading the data block will be returned to
            // calling function
            return sd_read_data_block(
                    ++(buf->curSectorOffset) + buf->curClusterStartAddr,
                    buf->buf);
        }
        // End of generic data cluster; Look through the FAT to find the next cluster
        else
            return sd_inc_cluster(buf);
    }

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("New sector loaded:\n");
    sd_print_hex_block(buf->buf, SD_SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

uint8_t sd_load_sector_from_offset (SD_File *f, const uint32_t offset) {
    uint8_t err;
    uint32_t clusterOffset = offset >> g_sd_sectorsPerCluster_shift;

#ifdef SD_OPTION_FILE_WRITE
    // If the buffer has been modified, write it before loading the next sector
    if (f->buf->mod) {
        sd_write_data_block(
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->buf);
        f->buf->mod = 0;
    }
#endif

    // Find the correct cluster
    if (f->curCluster < clusterOffset) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need to fast-forward through the FAT to find the cluster\n");
#endif
        // Desired cluster comes after the currently loaded one - this is easy
        // and requires continuing to look forward through the FAT from the
        // current position
        clusterOffset -= f->curCluster;
        while (clusterOffset--) {
            ++(f->curCluster);
            f->buf->curAllocUnit = f->buf->nextAllocUnit;
            if ((err = sd_get_fat_value(f->buf->curAllocUnit,
                    &(f->buf->nextAllocUnit))))
                return err;
        }
        f->buf->curClusterStartAddr = sd_find_sector_from_alloc(
                f->buf->curAllocUnit);
    } else if (f->curCluster > clusterOffset) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need to backtrack through the FAT to find the cluster\n");
#endif
        // Desired cluster is an earlier cluster than the currently loaded one -
        // this requires starting from the beginning and working forward
        f->buf->curAllocUnit = f->firstAllocUnit;
        if ((err = sd_get_fat_value(f->buf->curAllocUnit,
                &(f->buf->nextAllocUnit))))
            return err;
        f->curCluster = 0;
        while (clusterOffset--) {
            ++(f->curCluster);
            f->buf->curAllocUnit = f->buf->nextAllocUnit;
            if ((err = sd_get_fat_value(f->buf->curAllocUnit,
                    &(f->buf->nextAllocUnit))))
                return err;
        }
        f->buf->curClusterStartAddr = sd_find_sector_from_alloc(
                f->buf->curAllocUnit);
    }

    // Followed by finding the correct sector
    f->buf->curSectorOffset = offset % (1 << g_sd_sectorsPerCluster_shift);
    f->curSector = offset;
    sd_read_data_block(f->buf->curClusterStartAddr + f->buf->curSectorOffset,
            f->buf->buf);

    return 0;
}

uint8_t sd_inc_cluster (SD_Buffer *buf) {
    uint8_t err;

#ifdef SD_OPTION_FILE_WRITE
    // If the sector has been modified, write it back to the SD card before
    // reading again
    if (buf->mod) {
        if ((err = sd_write_data_block(
                buf->curClusterStartAddr + buf->curSectorOffset, buf->buf)))
            return err;
    }
    buf->mod = 0;
#endif

    // Update g_sd_cur*
    if (SD_EOC_BEG <= buf->curAllocUnit && SD_EOC_END <= buf->curAllocUnit)
        return SD_READING_PAST_EOC;
    buf->curAllocUnit = buf->nextAllocUnit;
    // Only look ahead to the next allocation unit if the current alloc unit is
    // not EOC
    if (!(SD_EOC_BEG <= buf->curAllocUnit && SD_EOC_END <= buf->curAllocUnit))
        // Current allocation unit is not EOC, read the next one
        if ((err = sd_get_fat_value(buf->curAllocUnit, &(buf->nextAllocUnit))))
            return err;
    buf->curClusterStartAddr = sd_find_sector_from_alloc(buf->curAllocUnit);
    buf->curSectorOffset = 0;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Incrementing the cluster. New parameters are:\n");
    printf("\tCurrent allocation unit: 0x%08X / %u\n", buf->curAllocUnit,
            buf->curAllocUnit);
    printf("\tNext allocation unit: 0x%08X / %u\n", buf->nextAllocUnit,
            buf->nextAllocUnit);
    printf("\tCurrent cluster starting address: 0x%08X / %u\n",
            buf->curClusterStartAddr, buf->curClusterStartAddr);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    if ((err = sd_read_data_block(buf->curClusterStartAddr, buf->buf)))
        return err;
    sd_print_hex_block(buf->buf, SD_SECTOR_SIZE);
    return 0;
#else
    return sd_read_data_block(buf->curClusterStartAddr, buf->buf);
#endif
}

void sd_get_filename (const uint8_t *buf, char *filename) {
    uint8_t i, j = 0;

    // Read in the first 8 characters - stop when a space is reached or 8
    // characters have been read, whichever comes first
    for (i = 0; i < SD_FILE_NAME_LEN; ++i) {
        if (0x05 == buf[i])
            filename[j++] = 0xe5;
        else if (' ' != buf[i])
            filename[j++] = buf[i];
    }

    // Determine if there is more past the first 8 - Again, stop when a space is
    // reached
    if (' ' != buf[SD_FILE_NAME_LEN]) {
        filename[j++] = '.';
        for (i = SD_FILE_NAME_LEN; i < SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN;
                ++i) {
            if (' ' != buf[i])
                filename[j++] = buf[i];
        }
    }

    // Insert null-terminator
    filename[j] = 0;
}

uint8_t sd_find (const char *filename, uint16_t *fileEntryOffset) {
    uint8_t err;
    char readEntryName[SD_FILENAME_STR_LEN];

#ifdef SD_OPTION_FILE_WRITE
    // Save the current buffer
    if (g_sd_buf.mod) {
        if ((err = sd_write_data_block(
                g_sd_buf.curClusterStartAddr + g_sd_buf.curSectorOffset,
                g_sd_buf.buf)))
            return err;
        g_sd_buf.mod = 0;
    }
#endif

    *fileEntryOffset = 0;

    // If we aren't looking at the beginning of the directory cluster, we must
    // backtrack to the beginning and then begin listing files
    if (g_sd_buf.curSectorOffset
            || (sd_find_sector_from_alloc(g_sd_dir_firstAllocUnit)
                    != g_sd_buf.curClusterStartAddr)) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("'find' requires a backtrack to beginning of directory's "
                "cluster\n");
#endif
        g_sd_buf.curClusterStartAddr = sd_find_sector_from_alloc(
                g_sd_dir_firstAllocUnit);
        g_sd_buf.curSectorOffset = 0;
        g_sd_buf.curAllocUnit = g_sd_dir_firstAllocUnit;
        if ((err = sd_get_fat_value(g_sd_buf.curAllocUnit,
                &g_sd_buf.nextAllocUnit)))
            return err;
        if ((err = sd_read_data_block(g_sd_buf.curClusterStartAddr,
                g_sd_buf.buf)))
            return err;
    }
    g_sd_buf.id = SD_FOLDER_ID;

    // Loop through all entries in the current directory until we find the
    // correct one
    // Function will exit normally with SD_EOC_END error code if the file is not
    // found
    while (g_sd_buf.buf[*fileEntryOffset]) {
        // Check if file is valid, retrieve the name if it is
        if (!(SD_DELETED_FILE_MARK == g_sd_buf.buf[*fileEntryOffset])) {
            sd_get_filename(&(g_sd_buf.buf[*fileEntryOffset]), readEntryName);
            if (!strcmp(filename, readEntryName))
                // File names match, return 0 to indicate a successful search
                return 0;
        }

        // Increment to the next file
        *fileEntryOffset += SD_FILE_ENTRY_LENGTH;

        // If it was the last entry in this sector, proceed to the next one
        if (SD_SECTOR_SIZE == *fileEntryOffset) {
            // Last entry in the sector, attempt to load a new sector
            // Possible error value includes end-of-chain marker
            if ((err = sd_load_next_sector(&g_sd_buf)))
                return err;

            *fileEntryOffset = 0;
        }
    }

    return SD_FILENAME_NOT_FOUND;
}

uint8_t sd_reload_buf (SD_File *f) {
    uint8_t err;

    // Function is only called if it has already been determined that the buffer
    // needs to be loaded - no checks need to be run

#ifdef SD_OPTION_FILE_WRITE
    // If the currently loaded buffer has been modified, save it
    if (f->buf->mod) {
        if ((err = sd_write_data_block(
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->buf)))
            return err;
        f->buf->mod = 0;
    }
#endif

    // Set current values to show that the first sector of the file is loaded.
    // SDLoadSectorFromOffset() loads the sector unconditionally before
    // returning so we do not need to load the sector here
    f->buf->curAllocUnit = f->firstAllocUnit;
    f->buf->curClusterStartAddr = sd_find_sector_from_alloc(f->firstAllocUnit);
    f->buf->curSectorOffset = 0;
    if ((err = sd_get_fat_value(f->firstAllocUnit, &(f->buf->nextAllocUnit))))
        return err;

    // Proceed with loading the sector
    if ((err = sd_load_sector_from_offset(f, f->curSector)))
        return err;
    f->buf->id = f->id;

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint32_t sd_find_empty_space (const uint8_t restore) {
    uint16_t allocOffset = 0;
    uint32_t fatSectorAddr = g_sd_curFatSector + g_sd_fatStart;
    uint32_t retVal;
    // NOTE: g_sd_curFatSector is not modified until end of function - it is
    // used throughout this function as the original starting point

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("\n*** SDFindEmptySpace() initialized with FAT sector 0x%08X / %u "
            "loaded ***\n", g_sd_curFatSector, g_sd_curFatSector);
    sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif

    // Find the first empty allocation unit and write the EOC marker
    if (SD_FAT_16 == g_sd_filesystem) {
        // Loop until we find an empty cluster
        while (sd_read_rev_dat16(&(g_sd_fat[allocOffset]))) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
            printf("Searching the following sector...\n");
            sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif
            // Stop when we either reach the end of the current block or find an
            // empty cluster
            while (sd_read_rev_dat16(&(g_sd_fat[allocOffset]))
                    && (SD_SECTOR_SIZE > allocOffset))
                allocOffset += SD_FAT_16;
            // If we reached the end of a sector...
            if (SD_SECTOR_SIZE <= allocOffset) {
                // If the currently loaded FAT sector has been modified, save it
                if (g_sd_fatMod) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("FAT sector has been modified; saving now... ");
#endif
                    sd_write_data_block(g_sd_curFatSector, g_sd_fat);
                    sd_write_data_block(g_sd_curFatSector + g_sd_fatSize,
                            g_sd_fat);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("done!\n");
#endif
                    g_sd_fatMod = 0;
                }
                // Read the next fat sector
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08X / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                sd_read_data_block(++fatSectorAddr, g_sd_fat);
            }
        }
        sd_write_rev_dat16(g_sd_fat + allocOffset, (uint16_t) SD_EOC_END);
        g_sd_fatMod = 1;
    } else /* Implied and not needed: "if (SD_FAT_32 == g_sd_filesystem)" */{
        // In FAT32, the first 7 usable clusters seem to be un-officially
        // reserved for the root directory
        if (0 == g_sd_curFatSector)
            allocOffset = 9 * g_sd_filesystem;

        // Loop until we find an empty cluster
        while (sd_read_rev_dat32(&(g_sd_fat[allocOffset])) & 0x0fffffff) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
            printf("Searching the following sector...\n");
            sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif
            // Stop when we either reach the end of the current block or find an
            // empty cluster
            while ((sd_read_rev_dat32(&(g_sd_fat[allocOffset])) & 0x0fffffff)
                    && (SD_SECTOR_SIZE > allocOffset))
                allocOffset += SD_FAT_32;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Broke while loop... why? Offset = 0x%04X / %u\n",
                    allocOffset, allocOffset);
#endif
            // If we reached the end of a sector...
            if (SD_SECTOR_SIZE <= allocOffset) {
                if (g_sd_fatMod) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("FAT sector has been modified; saving now... ");
#endif
                    sd_write_data_block(g_sd_curFatSector + g_sd_fatStart,
                            g_sd_fat);
                    sd_write_data_block(
                            g_sd_curFatSector + g_sd_fatStart + g_sd_fatSize,
                            g_sd_fat);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("done!\n");
#endif
                    g_sd_fatMod = 0;
                }
                // Read the next fat sector
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08X / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                sd_read_data_block(++fatSectorAddr, g_sd_fat);
                allocOffset = 0;
            }
        }
        sd_write_rev_dat32(&(g_sd_fat[allocOffset]),
                ((uint32_t) SD_EOC_END) & 0x0fffffff);
        g_sd_fatMod = 1;
    }

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Available space found: 0x%08X / %u\n",
            (g_sd_curFatSector << g_sd_entriesPerFatSector_Shift)
                    + allocOffset / g_sd_filesystem,
            (g_sd_curFatSector << g_sd_entriesPerFatSector_Shift)
                    + allocOffset / g_sd_filesystem);
#endif

    // If we loaded a new fat sector (and then modified it directly above),
    // write the sector before re-loading the original
    if ((fatSectorAddr != (g_sd_curFatSector + g_sd_fatStart)) && g_sd_fatMod) {
        sd_write_data_block(fatSectorAddr, g_sd_fat);
        sd_write_data_block(fatSectorAddr + g_sd_fatSize, g_sd_fat);
        g_sd_fatMod = 0;
        sd_read_data_block(g_sd_curFatSector + g_sd_fatStart, g_sd_fat);
    } else
        g_sd_curFatSector = fatSectorAddr - g_sd_fatStart;

    // Return new address to end-of-chain
    retVal = g_sd_curFatSector << g_sd_entriesPerFatSector_Shift;
    retVal += allocOffset / g_sd_filesystem;
    return retVal;
}

uint8_t sd_extend_fat (SD_Buffer *buf) {
    uint8_t err;
    uint32_t newAllocUnit;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Extending file or directory now...\n");
#endif

    // Do we need to load a different sector of the FAT or is the correct one
    // currently loaded? (Correct means the sector currently containing the EOC
    // marker)
    if ((buf->curAllocUnit >> g_sd_entriesPerFatSector_Shift)
            != g_sd_curFatSector) {

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need new FAT sector. Loading: 0x%08X / %u\n",
                buf->curAllocUnit >> g_sd_entriesPerFatSector_Shift,
                buf->curAllocUnit >> g_sd_entriesPerFatSector_Shift);
        printf("... because the current allocation unit is: 0x%08X / %u\n",
                buf->curAllocUnit, buf->curAllocUnit);
#endif
        // Need new sector, save the old one...
        if (g_sd_fatMod) {
            sd_write_data_block(g_sd_curFatSector + g_sd_fatStart, g_sd_fat);
            sd_write_data_block(
                    g_sd_curFatSector + g_sd_fatStart + g_sd_fatSize, g_sd_fat);
            g_sd_fatMod = 0;
        }
        // And load the new one...
        g_sd_curFatSector = buf->curAllocUnit >> g_sd_entriesPerFatSector_Shift;
        if ((err = sd_read_data_block(g_sd_curFatSector + g_sd_fatStart,
                g_sd_fat)))
            return err;
    }

#ifdef SD_OPTION_DEBUG
    // This function should only be called when a file or directory has reached
    // the end of its cluster chain
    if (SD_EOC_BEG
            <= sd_read_rev_dat32(
                    &(g_sd_fat[(buf->curAllocUnit
                            % (1 << g_sd_entriesPerFatSector_Shift))
                            * g_sd_filesystem])))
        return SD_INVALID_FAT_APPEND;
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    // Display the currently loaded FAT.... for no reason... not sure why I
    // wanted to do this...
    printf("This is the sector that *should* contain the EOC marker...\n");
    sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif

    // Find where the next cluster of the file should be stored...
    newAllocUnit = sd_find_empty_space(1);

    // Now that we know the allocation unit, write it to the FAT buffer
    if (SD_FAT_16 == g_sd_filesystem) {
        sd_write_rev_dat16(
                &(g_sd_fat[(buf->curAllocUnit
                        % (1 << g_sd_entriesPerFatSector_Shift))
                        * g_sd_filesystem]), (uint16_t) newAllocUnit);
    } else {
        sd_write_rev_dat32(
                &(g_sd_fat[(buf->curAllocUnit
                        % (1 << g_sd_entriesPerFatSector_Shift))
                        * g_sd_filesystem]), newAllocUnit);
    }
    buf->nextAllocUnit = newAllocUnit;
    g_sd_fatMod = 1;  // And mark the buffer as modified

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("After modification, the FAT now looks like...\n");
    sd_print_hex_block(g_sd_fat, SD_SECTOR_SIZE);
#endif

    return 0;
}

uint8_t sd_create_file (const char *name, const uint16_t *fileEntryOffset) {
    uint8_t i, j;
    // *name is only checked for uppercase
    char uppercaseName[SD_FILENAME_STR_LEN];
    uint32_t allocUnit;

#ifdef SD_OPTION_DEBUG
#ifdef SD_OPTION_VERBOSE
    printf("Creating new file: %s\n", name);
#endif
    // Parameter checking...
    if (SD_FILENAME_STR_LEN < strlen(name))
        return SD_INVALID_FILENAME;

    // Convert the name to uppercase
    for (i = 0; name[i]; ++i)
        if ('a' <= name[i] && 'z' >= name[i])
            uppercaseName[i] = name[i] + 'A' - 'a';
        else
            uppercaseName[i] = name[i];
#endif

    // Write the file fields in order...

    /* 1) Short file name */
    // Write first section
    for (i = 0; '.' != name[i] && 0 != name[i]; ++i)
        g_sd_buf.buf[*fileEntryOffset + i] = name[i];
    // Check if there is an extension
    if (name[i]) {
        // There might be an extension - pad first name with spaces
        for (j = i; j < SD_FILE_NAME_LEN; ++j)
            g_sd_buf.buf[*fileEntryOffset + j] = ' ';
        // Check if there is a period, as one would expect for a file name with
        // an extension
        if ('.' == name[i]) {
            // Extension exists, write it
            ++i;  // Skip the period
            // Insert extension, character-by-character
            for (j = SD_FILE_NAME_LEN; name[i]; ++j)
                g_sd_buf.buf[*fileEntryOffset + j] = name[i++];
            // Pad extension with spaces
            for (; j < SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN; ++j)
                g_sd_buf.buf[*fileEntryOffset + j] = ' ';
        }
        // If it wasn't a period or null terminator, throw an error
        else
            return SD_INVALID_FILENAME;
    }
    // No extension, pad with spaces
    else
        for (; i < (SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN); ++i)
            g_sd_buf.buf[*fileEntryOffset + i] = ' ';

    /* 2) Write attribute field... */
    // TODO: Allow for file attribute flags to be set, such as SD_READ_ONLY,
    //       SD_SUB_DIR, etc
    g_sd_buf.buf[*fileEntryOffset + SD_FILE_ATTRIBUTE_OFFSET] =
    SD_ARCHIVE;  // Archive flag should be set because the file is new
    g_sd_buf.mod = 1;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    sd_print_file_entry(&(g_sd_buf.buf[*fileEntryOffset]), uppercaseName);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
#endif

    /* 3) Find a spot in the FAT (do not check for a full FAT, assume space is
     * available)
     */
    allocUnit = sd_find_empty_space(0);
    sd_write_rev_dat16(
            &(g_sd_buf.buf[*fileEntryOffset + SD_FILE_START_CLSTR_LOW]),
            (uint16_t) allocUnit);
    if (SD_FAT_32 == g_sd_filesystem)
        sd_write_rev_dat16(
                &(g_sd_buf.buf[*fileEntryOffset + SD_FILE_START_CLSTR_HIGH]),
                (uint16_t) (allocUnit >> 16));

    /* 4) Write the size of the file (currently 0) */
    sd_write_rev_dat32(&(g_sd_buf.buf[*fileEntryOffset + SD_FILE_LEN_OFFSET]),
            0);

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("New file entry at offset 0x%08X / %u looks like...\n",
            *fileEntryOffset, *fileEntryOffset);
    sd_print_hex_block(g_sd_buf.buf, SD_SECTOR_SIZE);
#endif

    g_sd_buf.mod = 1;

    return 0;
}
#endif

#if (defined SD_OPTION_SHELL || defined SD_OPTION_VERBOSE)
inline void sd_print_file_entry (const uint8_t *fileEntry, char filename[]) {
    sd_print_file_attributes(fileEntry[SD_FILE_ATTRIBUTE_OFFSET]);
    sd_get_filename(fileEntry, filename);
    printf("\t\t%s", filename);
    if (SD_SUB_DIR & fileEntry[SD_FILE_ATTRIBUTE_OFFSET])
        putchar('/');
    putchar('\n');
}

void sd_print_file_attributes (const uint8_t flags) {
    // Print file attributes
    if (SD_READ_ONLY & flags)
        putchar(SD_READ_ONLY_CHAR);
    else
        putchar(SD_READ_ONLY_CHAR_);

    if (SD_HIDDEN_FILE & flags)
        putchar(SD_HIDDEN_FILE_CHAR);
    else
        putchar(SD_HIDDEN_FILE_CHAR_);

    if (SD_SYSTEM_FILE & flags)
        putchar(SD_SYSTEM_FILE_CHAR);
    else
        putchar(SD_SYSTEM_FILE_CHAR_);

    if (SD_VOLUME_ID & flags)
        putchar(SD_VOLUME_ID_CHAR);
    else
        putchar(SD_VOLUME_ID_CHAR_);

    if (SD_SUB_DIR & flags)
        putchar(SD_SUB_DIR_CHAR);
    else
        putchar(SD_SUB_DIR_CHAR_);

    if (SD_ARCHIVE & flags)
        putchar(SD_ARCHIVE_CHAR);
    else
        putchar(SD_ARCHIVE_CHAR_);
}
#endif

#ifdef SD_OPTION_DEBUG
void sd_error (const uint8_t err) {
    char str[] = "SD Error %u: %s\n";

    switch (err) {
        case SD_INVALID_CMD:
            printf(str, (err - SD_ERRORS_BASE), "Invalid command");
            break;
        case SD_READ_TIMEOUT:
            printf(str, (err - SD_ERRORS_BASE), "Timed out during read");
            printf("\tRead sector address was: 0x%08X / %u",
                    g_sd_sectorRdAddress, g_sd_sectorRdAddress);
            break;
        case SD_INVALID_NUM_BYTES:
            printf(str, (err - SD_ERRORS_BASE), "Invalid number of bytes");
            break;
        case SD_INVALID_RESPONSE:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02X\nThe following bits are set:\n",
                    (err - SD_ERRORS_BASE),
                    "Invalid first-byte response\n\tReceived: ",
                    g_sd_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
                    "Invalid first-byte response\n\tReceived: ",
                    g_sd_firstByteResponse);
#endif
            sd_first_byte_expansion(g_sd_firstByteResponse);
            break;
        case SD_INVALID_INIT:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s\n\tResponse: 0x%02X\n",
                    (err - SD_ERRORS_BASE),
                    "Invalid response during initialization",
                    g_sd_firstByteResponse);
#else
            printf("SD Error %u: %s\n\tResponse: %u\n", (err - SD_ERRORS_BASE),
                    "Invalid response during initialization",
                    g_sd_firstByteResponse);
#endif
            break;
        case SD_INVALID_FILESYSTEM:
            printf(str, (err - SD_ERRORS_BASE), "Invalid filesystem");
            break;
        case SD_INVALID_DAT_STRT_ID:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02X\n", (err - SD_ERRORS_BASE),
                    "Invalid data-start ID\n\tReceived: ",
                    g_sd_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
                    "Invalid data-start ID\n\tReceived: ",
                    g_sd_firstByteResponse);
#endif
            break;
        case SD_FILENAME_NOT_FOUND:
            printf(str, (err - SD_ERRORS_BASE), "Filename not found");
            break;
        case SD_EMPTY_FAT_ENTRY:
            printf(str, (err - SD_ERRORS_BASE), "FAT points to empty entry");
            break;
        case SD_CORRUPT_CLUSTER:
            printf(str, (err - SD_ERRORS_BASE), "SD cluster is corrupt");
            break;
        case SD_INVALID_PTR_ORIGIN:
            printf(str, (err - SD_ERRORS_BASE), "Invalid pointer origin");
            break;
        case SD_ENTRY_NOT_FILE:
            printf(str, (err - SD_ERRORS_BASE),
                    "Requested file entry is not a file");
            break;
        case SD_INVALID_FILENAME:
            printf(str, (err - SD_ERRORS_BASE),
                    "Invalid filename - please use 8.3 format");
            break;
        case SD_INVALID_FAT_APPEND:
            printf(str, (err - SD_ERRORS_BASE),
                    "FAT entry append was attempted unnecessarily");
            break;
        case SD_FILE_ALREADY_EXISTS:
            printf(str, (err - SD_ERRORS_BASE),
                    "Attempting to create an already existing file");
            break;
        case SD_INVALID_FILE_MODE:
            printf(str, (err - SD_ERRORS_BASE), "Invalid file mode");
            break;
        case SD_TOO_MANY_FATS:
            printf(str, (err - SD_ERRORS_BASE),
                    "This driver is only capable of writing files on FAT "
                            "partitions with two (2) copies of the FAT");
            break;
        case SD_FILE_WITHOUT_BUFFER:
            printf(str, (err - SD_ERRORS_BASE),
                    "SDfopen() was passed a file struct with "
                            "an uninitialized buffer");
            break;
        case SD_CMD8_FAILURE:
            printf(str, (err - SD_ERRORS_BASE), "CMD8 never received a proper "
                    "response");
            break;
        default:
            // Is the error an SPI error?
            if (err > SD_ERRORS_BASE
                    && err < (SD_ERRORS_BASE + SD_ERRORS_LIMIT))
                printf("Unknown SD error %u\n", (err - SD_ERRORS_BASE));
            // If not, print unknown error
            else
                printf("Unknown error %u\n", err);
            break;
    }
    while (1)
        ;
}

void sd_first_byte_expansion (const uint8_t response) {
    if (BIT_0 & response)
        printf("\t0: Idle\n");
    if (BIT_1 & response)
        printf("\t1: Erase reset\n");
    if (BIT_2 & response)
        printf("\t2: Illegal command\n");
    if (BIT_3 & response)
        printf("\t3: Communication CRC error\n");
    if (BIT_4 & response)
        printf("\t4: Erase sequence error\n");
    if (BIT_5 & response)
        printf("\t5: Address error\n");
    if (BIT_6 & response)
        printf("\t6: Parameter error\n");
    if (BIT_7 & response)
        printf("\t7: Something is really screwed up. This should always be 0."
                "\n");
}
#endif
