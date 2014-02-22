/**
 * @file    sd.cpp
 *
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

// Includes
#include <sd.h>

namespace PropWare {

/**
 * Public constants
 */
const uint8_t SD::LINE_SIZE = 16;
const uint16_t SD::SECTOR_SIZE = SD_SECTOR_SIZE;
const uint32_t SD::DEFAULT_SPI_FREQ = 1800000;
const int8_t SD::FOLDER_ID = -1;
const uint8_t SD::SHELL_INPUT_LEN = SD_SHELL_INPUT_LEN;
const uint8_t SD::SHELL_CMD_LEN = SD_SHELL_CMD_LEN;
const uint8_t SD::SHELL_ARG_LEN = SD_SHELL_ARG_LEN;
const char SD::SHELL_EXIT[] = "exit";
const char SD::SHELL_LS[] = "ls";
const char SD::SHELL_CAT[] = "cat";
const char SD::SHELL_CD[] = "cd";
const char SD::SHELL_TOUCH[] = "touch";

/**
 * Private constants
 */
const uint32_t SD::SPI_INIT_FREQ = 200000;
const SPI_Mode SD::SPI_MODE = SPI_MODE_0;
const SPI_BitMode SD::SPI_BITMODE = SPI_MSB_FIRST;
const uint32_t SD::RESPONSE_TIMEOUT = CLKFREQ / 10;
const uint32_t SD::WIGGLE_ROOM = RESPONSE_TIMEOUT / 50;
const uint8_t SD::SECTOR_SIZE_SHIFT = 9;
const uint8_t SD::CMD_IDLE = 0x40 + 0;
const uint8_t SD::CMD_INTERFACE_COND = 0x40 + 8;
const uint8_t SD::CMD_RD_CSD = 0x40 + 9;
const uint8_t SD::CMD_RD_CID = 0x40 + 10;
const uint8_t SD::CMD_RD_BLOCK = 0x40 + 17;
const uint8_t SD::CMD_WR_BLOCK = 0x40 + 24;
const uint8_t SD::CMD_WR_OP = 0x40 + 41;
const uint8_t SD::CMD_APP = 0x40 + 55;
const uint8_t SD::CMD_READ_OCR = 0x40 + 58;
const uint32_t SD::HOST_VOLTAGE_3V3 = 0x01;
const uint32_t SD::R7_CHECK_PATTERN = 0xAA;
const uint32_t SD::SD::ARG_CMD8 = ((SD::HOST_VOLTAGE_3V3 << 8) |
        SD::R7_CHECK_PATTERN);
const uint32_t SD::ARG_LEN = 5;
const uint8_t SD::CRC_IDLE = 0x95;
const uint8_t SD::CRC_CMD8 = 0x87;
const uint8_t SD::CRC_ACMD = 0x77;
const uint8_t SD::CRC_OTHER = 0x01;
const uint8_t SD::RESPONSE_IDLE = 0x01;
const uint8_t SD::RESPONSE_ACTIVE = 0x00;
const uint8_t SD::DATA_START_ID = 0xFE;
const uint8_t SD::RESPONSE_LEN_R1 = 1;
const uint8_t SD::RESPONSE_LEN_R3 = 5;
const uint8_t SD::RESPONSE_LEN_R7 = 5;
const uint8_t SD::RSPNS_TKN_BITS = 0x0f;
const uint8_t SD::RSPNS_TKN_ACCPT = (0x02 << 1) | 1;
const uint8_t SD::RSPNS_TKN_CRC = (0x05 << 1) | 1;
const uint8_t SD::RSPNS_TKN_WR = (0x06 << 1) | 1;
const uint8_t SD::FAT_16 = 2;
const uint8_t SD::FAT_32 = 4;
const uint8_t SD::BOOT_SECTOR_ID = 0xEB;
const uint8_t SD::BOOT_SECTOR_ID_ADDR = 0;
const uint16_t SD::BOOT_SECTOR_BACKUP = 0x1C6;
const uint8_t SD::CLUSTER_SIZE_ADDR = 0x0D;
const uint8_t SD::RSVD_SCTR_CNT_ADDR = 0x0E;
const uint8_t SD::NUM_FATS_ADDR = 0x10;
const uint8_t SD::ROOT_ENTRY_CNT_ADDR = 0x11;
const uint8_t SD::TOT_SCTR_16_ADDR = 0x13;
const uint8_t SD::FAT_SIZE_16_ADDR = 0x16;
const uint8_t SD::TOT_SCTR_32_ADDR = 0x20;
const uint8_t SD::FAT_SIZE_32_ADDR = 0x24;
const uint8_t SD::ROOT_CLUSTER_ADDR = 0x2c;
const uint16_t SD::FAT12_CLSTR_CNT = 4085;
const uint16_t SD::FAT16_CLSTR_CNT = 65525;
const uint8_t SD::FILE_ENTRY_LENGTH = 32;
const uint8_t SD::DELETED_FILE_MARK = 0xE5;
const uint8_t SD::FILE_NAME_LEN = SD_FILE_NAME_LEN;
const uint8_t SD::FILE_EXTENSION_LEN = SD_FILE_EXTENSION_LEN;
const uint8_t SD::FILENAME_STR_LEN = SD_FILENAME_STR_LEN;
const uint8_t SD::FILE_ATTRIBUTE_OFFSET = 0x0B;
const uint8_t SD::FILE_START_CLSTR_LOW = 0x1A;
const uint8_t SD::FILE_START_CLSTR_HIGH = 0x14;
const uint8_t SD::FILE_LEN_OFFSET = 0x1C;
const int8_t SD::FREE_CLUSTER = 0;
const int8_t SD::RESERVED_CLUSTER = 1;
const int8_t SD::RSVD_CLSTR_VAL_BEG = -15;
const int8_t SD::BAD_CLUSTER = -8;
const int32_t SD::EOC_BEG = -7;
const int32_t SD::EOC_END = -1;
const uint8_t SD::READ_ONLY = BIT_0;
const char SD::READ_ONLY_CHAR = 'r';
const char SD::READ_ONLY_CHAR_ = 'w';
const uint8_t SD::HIDDEN_FILE = BIT_1;
const char SD::HIDDEN_FILE_CHAR = 'h';
const char SD::HIDDEN_FILE_CHAR_ = '.';
const uint8_t SD::SYSTEM_FILE = BIT_2;
const char SD::SYSTEM_FILE_CHAR = 's';
const char SD::SYSTEM_FILE_CHAR_ = '.';
const uint8_t SD::VOLUME_ID = BIT_3;
const char SD::VOLUME_ID_CHAR = 'v';
const char SD::VOLUME_ID_CHAR_ = '.';
const uint8_t SD::SUB_DIR = BIT_4;
const char SD::SUB_DIR_CHAR = 'd';
const char SD::SUB_DIR_CHAR_ = 'f';
const uint8_t SD::ARCHIVE = BIT_5;
const char SD::ARCHIVE_CHAR = 'a';
const char SD::ARCHIVE_CHAR_ = '.';

/**
 * @brief       Short-hand for checking if a function through an error and
 *              handling it
 *
 * @param[in]   x   Any function that might throw an SD error
 */
#define sd_check_errors(x)  if ((err = x)) this->error(err)

/*********************************
 *** Public Method Definitions ***
 *********************************/
SD::SD () {
    this->m_fileID = 0;

#ifdef SD_OPTION_FILE_WRITE
    this->m_fatMod = false;
#endif
}

uint8_t SD::start (const uint32_t mosi, const uint32_t miso,
        const uint32_t sclk, const uint32_t cs, const int32_t freq) {
    uint8_t i, j, k, err;
    uint8_t response[16];
    uint8_t stageCleared;  // Flag to signal when one stage has completed

    // Set CS for output and initialize high
    this->m_cs = cs;
    gpio_set_dir(cs, GPIO_DIR_OUT);
    gpio_pin_set(cs);

    // Start SPI module
    if ((err = spi_start(mosi, miso, sclk, SD::SPI_INIT_FREQ, SD::SPI_MODE,
            SD::SPI_BITMODE)))
        this->error(err);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Starting SD card...\n");
#endif

    // Attempt initialization no more than 10 times
    stageCleared = false;
    for (i = 0; i < 10 && !stageCleared; ++i) {
        // Initialization loop (reset SD card)
        for (j = 0; j < 10; ++j) {
            waitcnt(CLKFREQ / 10 + CNT);

            // Send at least 72 clock cycles to enable the SD card
            gpio_pin_set(cs);
            for (k = 0; k < 5; ++k)
                check_errors(spi_shift_out(16, -1));

            // Be very super 100% sure that all clocks have finished ticking
            // before setting chip select low
            check_errors(spi_wait());
            waitcnt(CLKFREQ / 10 + CNT);

            // Chip select goes low for the duration of this function
            gpio_pin_clear(cs);

            // Send SD into idle state, retrieve a response and ensure it is the
            // "idle" response
            sd_check_errors(this->send_command(SD::CMD_IDLE, 0, SD::CRC_IDLE));
            this->get_response(SD::RESPONSE_LEN_R1, response);
            if (SD::RESPONSE_IDLE == this->m_firstByteResponse)
                j = 10;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            else
                printf("Failed attempt at CMD0: 0x%02X\n",
                        this->m_firstByteResponse);
#endif
        }
        if (SD::RESPONSE_IDLE != this->m_firstByteResponse)
            this->error(SD::INVALID_INIT);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("SD card in idle state. Now sending CMD8...\n");
#endif

        // Inform SD card that the Propeller uses the 2.7-3.6V range;
        // An echo is expected as response
        sd_check_errors(
                this->send_command(SD::CMD_INTERFACE_COND, SD::ARG_CMD8,
                        SD::CRC_CMD8));
        sd_check_errors(this->get_response(SD::RESPONSE_LEN_R7, response));
        if (SD::RESPONSE_IDLE == this->m_firstByteResponse) {
            // The card is idle, that's good. Let's make sure we get the correct
            // response back
            if ((SD::HOST_VOLTAGE_3V3 == response[2])
                    && (SD::R7_CHECK_PATTERN == response[3]))
                // All responses were A-OKAY, exit the outer
                stageCleared = true;
        }
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        if (!stageCleared)
            printf("Failed attempt at CMD8: 0x%02X, 0x%02X, 0x%02X;\n",
                    this->m_firstByteResponse, response[2], response[3]);
#endif
    }

    // If CMD8 never succeeded, throw an error
    if (!stageCleared)
        this->error(SD::CMD8_FAILURE);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("CMD8 succeeded. Requesting operating conditions...\n");
#endif

    // Request operating conditions register and ensure response begins with R1
    sd_check_errors(this->send_command(SD::CMD_READ_OCR, 0, SD::CRC_OTHER));
    sd_check_errors(this->get_response(SD::RESPONSE_LEN_R3, response));
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    this->print_hex_block(response, SD::RESPONSE_LEN_R3);
#endif
    if (SD::RESPONSE_IDLE != this->m_firstByteResponse)
        this->error(SD::INVALID_INIT);
    // TODO: Parse the voltage bits to double-check that 3.3V is okay and ensure
    //       that bit 31 is low, indicating the card is done powering up

    // Spin up the card and bring to active state
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("OCR read successfully. Sending into active state...\n");
#endif
    stageCleared = false;
    for (i = 0; i < 8 && !stageCleared; ++i) {
        waitcnt(CLKFREQ / 10 + CNT);
        sd_check_errors(this->send_command(SD::CMD_APP, 0, SD::CRC_OTHER));
        sd_check_errors(this->get_response(1, response));
        printf("Sent CMD55... ");
        sd_check_errors(
                this->send_command(SD::CMD_WR_OP, /*BIT_30*/0, SD::CRC_OTHER));
        sd_check_errors(this->get_response(1, response));
        printf("Sent ACMD41\n");
        if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse)
            stageCleared = true;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        else
            printf("Failed attempt at active state: 0x%02X\n",
                    this->m_firstByteResponse);
#endif
    }
    if (!stageCleared)
        this->error(SD::INVALID_RESPONSE);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Activated!\n");
#endif

    // Initialization nearly complete, increase clock
//#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
//    printf("Increasing clock to full speed\n");
//#endif
//    if (-1 == freq || 0 == freq)
//        spi_set_clock(SD::DEFAULT_SPI_FREQ);
//    else
//        spi_set_clock(freq);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    // If debugging requested, print to the screen CSD and CID registers from SD
    // card
    printf("Requesting CSD...\n");
    sd_check_errors(this->send_command(SD::CMD_RD_CSD, 0, SD::CRC_OTHER));
    sd_check_errors(this->read_block(16, response));
    printf("CSD Contents:\n");
    this->print_hex_block(response, 16);
    putchar('\n');

    printf("Requesting CID...\n");
    sd_check_errors(this->send_command(SD::CMD_RD_CID, 0, SD::CRC_OTHER));
    sd_check_errors(this->read_block(16, response));
    printf("CID Contents:\n");
    this->print_hex_block(response, 16);
    putchar('\n');
#endif

    // We're finally done initializing everything. Set chip select high again to
    // release the SPI port
    gpio_pin_set(cs);

    // Initialization complete
    return 0;
}

uint8_t SD::mount (void) {
    uint8_t err, temp;

    // FAT system determination variables:
    uint32_t rsvdSectorCount, numFATs, rootEntryCount, totalSectors, FATSize,
            dataSectors;
    uint32_t bootSector = 0;
    uint32_t clusterCount;

    // Read in first sector
    sd_check_errors(this->read_data_block(bootSector, this->m_buf.buf));
    // Check if sector 0 is boot sector or MBR; if MBR, skip to boot sector at
    // first partition
    if (SD::BOOT_SECTOR_ID != this->m_buf.buf[SD::BOOT_SECTOR_ID_ADDR]) {
        bootSector = this->read_rev_dat32(
                &(this->m_buf.buf[SD::BOOT_SECTOR_BACKUP]));
        sd_check_errors(this->read_data_block(bootSector, this->m_buf.buf));
    }

    // Print the boot sector if requested
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG && \
        defined SD_OPTION_VERBOSE_BLOCKS)
    printf("***BOOT SECTOR***\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    // Do this whether it is FAT16 or FAT32
    temp = this->m_buf.buf[SD::CLUSTER_SIZE_ADDR];
#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Preliminary sectors per cluster: %u\n", temp);
#endif
    while (temp) {
        temp >>= 1;
        ++this->m_sectorsPerCluster_shift;
    }
    --this->m_sectorsPerCluster_shift;
    rsvdSectorCount = this->read_rev_dat16(
            &((this->m_buf.buf)[SD::RSVD_SCTR_CNT_ADDR]));
    numFATs = this->m_buf.buf[SD::NUM_FATS_ADDR];
#ifdef SD_OPTION_FILE_WRITE
    if (2 != numFATs)
        this->error(SD::TOO_MANY_FATS);
#endif
    rootEntryCount = this->read_rev_dat16(
            &(this->m_buf.buf[SD::ROOT_ENTRY_CNT_ADDR]));

    // Check if FAT size is valid in 16- or 32-bit location
    FATSize = this->read_rev_dat16(&(this->m_buf.buf[SD::FAT_SIZE_16_ADDR]));
    if (!FATSize)
        FATSize = this->read_rev_dat32(
                &(this->m_buf.buf[SD::FAT_SIZE_32_ADDR]));

    // Check if FAT16 total sectors is valid
    totalSectors = this->read_rev_dat16(
            &(this->m_buf.buf[SD::TOT_SCTR_16_ADDR]));
    if (!totalSectors)
        totalSectors = this->read_rev_dat32(
                &(this->m_buf.buf[SD::TOT_SCTR_32_ADDR]));

    // Compute necessary numbers to determine FAT type (12/16/32)
    this->m_rootDirSectors = (rootEntryCount * 32) >> SD::SECTOR_SIZE_SHIFT;
    dataSectors = totalSectors
            - (rsvdSectorCount + numFATs * FATSize + rootEntryCount);
    clusterCount = dataSectors >> this->m_sectorsPerCluster_shift;

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Sectors per cluster: %u\n", 1 << this->m_sectorsPerCluster_shift);
    printf("Reserved sector count: 0x%08X / %u\n", rsvdSectorCount,
            rsvdSectorCount);
    printf("Number of FATs: 0x%02X / %u\n", numFATs, numFATs);
    printf("Total sector count: 0x%08X / %u\n", totalSectors, totalSectors);
    printf("Total cluster count: 0x%08X / %u\n", clusterCount, clusterCount);
    printf("Total data sectors: 0x%08X / %u\n", dataSectors, dataSectors);
    printf("FAT Size: 0x%04X / %u\n", FATSize, FATSize);
    printf("Root directory sectors: 0x%08X / %u\n", this->m_rootDirSectors,
            this->m_rootDirSectors);
    printf("Root entry count: 0x%08X / %u\n", rootEntryCount, rootEntryCount);
#endif

    // Determine and store FAT type
    if (SD::FAT12_CLSTR_CNT > clusterCount)
        this->error(SD::INVALID_FILESYSTEM);
    else if (SD::FAT16_CLSTR_CNT > clusterCount) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("\n***FAT type is FAT16***\n");
#endif
        this->m_filesystem = SD::FAT_16;
        this->m_entriesPerFatSector_Shift = 8;
    } else {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("\n***FAT type is FAT32***\n");
#endif
        this->m_filesystem = SD::FAT_32;
        this->m_entriesPerFatSector_Shift = 7;
    }

    // Find start of FAT
    this->m_fatStart = bootSector + rsvdSectorCount;

    //    this->m_filesystem = SD::FAT_16;
    // Find root directory address
    switch (this->m_filesystem) {
        case SD::FAT_16:
            this->m_rootAddr = FATSize * numFATs + this->m_fatStart;
            this->m_firstDataAddr = this->m_rootAddr + this->m_rootDirSectors;
            break;
        case SD::FAT_32:
            this->m_firstDataAddr = this->m_rootAddr = bootSector
                    + rsvdSectorCount + FATSize * numFATs;
            this->m_rootAllocUnit = this->read_rev_dat32(
                    &(this->m_buf.buf[SD::ROOT_CLUSTER_ADDR]));
            break;
    }

#ifdef SD_OPTION_FILE_WRITE
    // If files will be written to, the second FAT must also be updated - the
    // first sector address of which is stored here
    this->m_fatSize = FATSize;
#endif

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Start of FAT: 0x%08X\n", this->m_fatStart);
    printf("Root directory alloc. unit: 0x%08X\n", this->m_rootAllocUnit);
    printf("Root directory sector: 0x%08X\n", this->m_rootAddr);
    printf("Calculated root directory sector: 0x%08X\n",
            this->find_sector_from_alloc(this->m_rootAllocUnit));
    printf("First data sector: 0x%08X\n", this->m_firstDataAddr);
#endif

    // Store the first sector of the FAT
    sd_check_errors(this->read_data_block(this->m_fatStart, this->m_fat));
    this->m_curFatSector = 0;

    // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG && \
        defined SD_OPTION_VERBOSE_BLOCKS)
    printf("\n***First File Allocation Table***\n");
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    // Read in the root directory, set root as current
    sd_check_errors(this->read_data_block(this->m_rootAddr, this->m_buf.buf));
    this->m_buf.curClusterStartAddr = this->m_rootAddr;
    if (SD::FAT_16 == this->m_filesystem) {
        this->m_dir_firstAllocUnit = -1;
        this->m_buf.curAllocUnit = -1;
    } else {
        this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit =
                this->m_rootAllocUnit;
        sd_check_errors(
                this->get_fat_value(this->m_buf.curAllocUnit,
                        &this->m_buf.nextAllocUnit));
    }
    this->m_buf.curClusterStartAddr = this->m_rootAddr;
    this->m_buf.curSectorOffset = 0;

    // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("***Root directory***\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint8_t SD::unmount (void) {
    uint8_t err;

    // If the directory buffer was modified, write it
    if (this->m_buf.mod)
        sd_check_errors(
                this->write_data_block(
                        this->m_buf.curClusterStartAddr
                                + this->m_buf.curSectorOffset,
                        this->m_buf.buf));

    // If the FAT sector was modified, write it
    if (this->m_fatMod) {
        sd_check_errors(
                this->write_data_block(this->m_curFatSector + this->m_fatStart,
                        this->m_fat));
        sd_check_errors(
                this->write_data_block(
                        this->m_curFatSector + this->m_fatStart
                                + this->m_fatSize, this->m_fat));
    }

    return 0;
}
#endif

uint8_t SD::chdir (const char *d) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;

    this->m_buf.id = SD::FOLDER_ID;

    // Attempt to find the file and return an error code if not found
    sd_check_errors(this->find(d, &fileEntryOffset));

    // File entry was found successfully, load it into the buffer and update
    // status variables
#ifdef SD_OPTION_FILE_WRITE
    // If the previous sector was modified, write it back to the SD card
    // before reading
    if (this->m_buf.mod)
        this->write_data_block(
                this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset,
                this->m_buf.buf);
    this->m_buf.mod = false;
#endif

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("%s found at offset 0x%04X from address 0x%08X\n", d,
            fileEntryOffset,
            this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset);
#endif

    if (SD::FAT_16 == this->m_filesystem)
        this->m_buf.curAllocUnit = this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_LOW]));
    else {
        this->m_buf.curAllocUnit = this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_LOW]));
        this->m_buf.curAllocUnit |= this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_HIGH]))
                << 16;
        // Clear the highest 4 bits - they are always reserved
        this->m_buf.curAllocUnit &= 0x0FFFFFFF;
    }
    this->get_fat_value(this->m_buf.curAllocUnit, &(this->m_buf.nextAllocUnit));
    if (0 == this->m_buf.curAllocUnit) {
        this->m_buf.curAllocUnit = -1;
        this->m_dir_firstAllocUnit = this->m_rootAllocUnit;
    } else
        this->m_dir_firstAllocUnit = this->m_buf.curAllocUnit;
    this->m_buf.curSectorOffset = 0;
    this->read_data_block(this->m_buf.curClusterStartAddr, this->m_buf.buf);

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Opening directory from...\n");
    printf("\tAllocation unit 0x%08X\n", this->m_buf.curAllocUnit);
    printf("\tCluster starting address 0x%08X\n",
            this->m_buf.curClusterStartAddr);
    printf("\tSector offset 0x%04X\n", this->m_buf.curSectorOffset);
#ifdef SD_OPTION_VERBOSE_BLOCKS
    printf("And the first directory sector looks like....\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif
#endif
    return 0;
}

uint8_t SD::fopen (const char *name, SD::File *f, const SD::FileMode mode) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Attempting to open %s\n", name);
#endif

    if (NULL == f->buf)
        this->error(SD::FILE_WITHOUT_BUFFER);

    f->id = this->m_fileID++;
    f->rPtr = 0;
    f->wPtr = 0;
#if (defined SD_OPTION_DEBUG && !(defined SD_OPTION_FILE_WRITE))
    if (FILE_MODE_R != mode)
    sd_error(SD::INVALID_FILE_MODE);
#endif
    f->mode = mode;
    f->mod = false;

    // Attempt to find the file
    if ((err = this->find(name, &fileEntryOffset))) {
#ifdef SD_OPTION_FILE_WRITE
        // If the file didn't exist and you're trying to read from it, that's a
        // problem
        if (SD::FILE_MODE_R == mode)
            return err;

        // Find returned an error, ensure it was either file-not-found or EOC
        // and then create the file
        if (SD::EOC_END == err) {
            // File wasn't found and the cluster is full; add another to the
            // directory
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Directory cluster was full, adding another...\n");
#endif
            sd_check_errors(this->extend_fat(&this->m_buf));
            sd_check_errors(this->load_next_sector(&this->m_buf));
        }
        if (SD::EOC_END == err || SD::FILENAME_NOT_FOUND == err) {
            // File wasn't found, but there is still room in this cluster (or a
            // new cluster was just added)
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Creating a new directory entry...\n");
#endif
            sd_check_errors(this->create_file(name, &fileEntryOffset));
        } else
#endif
            // SDFind returned unknown error - throw it
            this->error(err);
    }

    // `name` was found successfully, determine if it is a file or directory
    if (SD::SUB_DIR
            & this->m_buf.buf[fileEntryOffset + SD::FILE_ATTRIBUTE_OFFSET])
        this->error(SD::ENTRY_NOT_FILE);

    // Passed the file-not-directory test, load it into the buffer and update
    // status variables
    f->buf->id = f->id;
    f->curSector = 0;
    if (SD::FAT_16 == this->m_filesystem)
        f->buf->curAllocUnit = this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_LOW]));
    else {
        f->buf->curAllocUnit = this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_LOW]));
        f->buf->curAllocUnit |= this->read_rev_dat16(
                &(this->m_buf.buf[fileEntryOffset + SD::FILE_START_CLSTR_HIGH]))
                << 16;

        // Clear the highest 4 bits - they are always reserved
        f->buf->curAllocUnit &= 0x0FFFFFFF;
    }
    f->firstAllocUnit = f->buf->curAllocUnit;
    f->curCluster = 0;
    f->buf->curClusterStartAddr = this->find_sector_from_alloc(
            f->buf->curAllocUnit);
    f->dirSectorAddr = this->m_buf.curClusterStartAddr
            + this->m_buf.curSectorOffset;
    f->fileEntryOffset = fileEntryOffset;
    sd_check_errors(
            this->get_fat_value(f->buf->curAllocUnit,
                    &(f->buf->nextAllocUnit)));
    f->buf->curSectorOffset = 0;
    f->length = this->read_rev_dat32(
            &(this->m_buf.buf[fileEntryOffset + SD::FILE_LEN_OFFSET]));
#ifdef SD_OPTION_FILE_WRITE
    // Determine the number of sectors currently allocated to this file; useful
    // in the case that the file needs to be extended
    f->maxSectors = f->length >> SD::SECTOR_SIZE_SHIFT;
    if (!(f->maxSectors))
        f->maxSectors = 1 << this->m_sectorsPerCluster_shift;
    while (f->maxSectors % (1 << this->m_sectorsPerCluster_shift))
        ++(f->maxSectors);
    f->buf->mod = false;
#endif
    sd_check_errors(
            this->read_data_block(f->buf->curClusterStartAddr, f->buf->buf));

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
    this->print_hex_block(f->buf->buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint8_t SD::fclose (SD::File *f) {
    uint8_t err;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Closing file...\n");
#endif
    // If the currently loaded sector has been modified, save the changes
    if ((f->buf->id == f->id) && f->buf->mod) {
        sd_check_errors(
                this->write_data_block(
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->buf));;
        f->buf->mod = false;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Modified sector in file has been saved...\n");
        printf("\tDestination address: 0x%08X / %u\n",
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->curClusterStartAddr + f->buf->curSectorOffset);
        printf("\tFile first sector address: 0x%08X / %u\n",
                this->find_sector_from_alloc(f->firstAllocUnit),
                this->find_sector_from_alloc(f->firstAllocUnit));
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
        if ((this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset)
                != f->dirSectorAddr) {
            // If it isn't, load it...
            if (this->m_buf.mod)
                // And if it's been modified since the last read, save it...
                sd_check_errors(
                        this->write_data_block(
                                this->m_buf.curClusterStartAddr
                                        + this->m_buf.curSectorOffset,
                                this->m_buf.buf));
            sd_check_errors(
                    this->read_data_block(f->dirSectorAddr, this->m_buf.buf));
        }
        // Finally, edit the length of the file
        this->write_rev_dat32(
                &(this->m_buf.buf[f->fileEntryOffset + SD::FILE_LEN_OFFSET]),
                f->length);
        this->m_buf.mod = 01;
    }

    return 0;
}

uint8_t SD::fputc (const char c, SD::File *f) {
    uint8_t err;
    // Determines byte-offset within a sector
    uint16_t sectorPtr = f->wPtr % SD::SECTOR_SIZE;
    // Determine the needed file sector
    uint32_t sectorOffset = (f->wPtr >> SD::SECTOR_SIZE_SHIFT);

    // Determine if the correct sector is loaded
    if (f->buf->id != f->id)
        sd_check_errors(this->reload_buf(f));

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
            sd_check_errors(this->extend_fat(f->buf));
            f->maxSectors += 1 << this->m_sectorsPerCluster_shift;
        }

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Loading new file sector at file-offset: 0x%08X / %u\n",
                sectorOffset, sectorOffset);
#endif
        // SDLoadSectorFromOffset() will ensure that, if the current buffer has
        // been modified, it is written back to the SD card before loading a new
        // one
        sd_check_errors(this->load_sector_from_offset(f, sectorOffset));
    }

    if (++(f->wPtr) > f->length) {
        ++(f->length);
        f->mod = true;
    }
    f->buf->buf[sectorPtr] = c;
    f->buf->mod = true;

    return 0;
}

uint8_t SD::fputs (char *s, SD::File *f) {
    uint8_t err;

    while (*s)
        sd_check_errors(fputc(*(s++), f));

    return 0;
}
#endif

char SD::fgetc (SD::File *f) {
    char c;
    uint16_t ptr = f->rPtr % SD::SECTOR_SIZE;

    // Determine if the currently loaded sector is what we need
    uint32_t sectorOffset = (f->rPtr >> SD::SECTOR_SIZE_SHIFT);

    // Determine if the correct sector is loaded
    if (f->buf->id != f->id)
        this->reload_buf(f);
    else if (sectorOffset != f->curSector) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("File sector offset: 0x%08X / %u\n", sectorOffset, sectorOffset);
#endif
        this->load_sector_from_offset(f, sectorOffset);
    }
    ++(f->rPtr);
    c = f->buf->buf[ptr];
    return c;
}

char * SD::fgets (char s[], uint32_t size, SD::File *f) {
    /* Code taken from fgets.c in the propgcc source, originally written by Eric
     * R. Smith and (slightly) modified to fit this SD driver
     */
    uint32_t c;
    uint32_t count = 0;

    --size;
    while (count < size) {
        c = this->fgetc(f);
        if ((uint32_t) EOF == c)
            break;
        s[count++] = c;
        if ('\n' == c)
            break;
    }
    s[count] = 0;
    return (0 < count) ? s : NULL;
}

inline uint8_t SD::feof (SD::File *f) {
    return f->length == f->rPtr;
}

uint8_t SD::fseekr (SD::File *f, const int32_t offset,
        const SD::FilePos origin) {
    switch (origin) {
        case SEEK_SET:
            f->rPtr = offset;
            break;
        case SEEK_CUR:
            f->rPtr = f->rPtr + offset;
            break;
        case SEEK_END:
            f->rPtr = f->length + offset - 1;
            break;
        default:
            return SD::INVALID_PTR_ORIGIN;
            break;
    }

    return 0;
}

uint8_t SD::fseekw (SD::File *f, const int32_t offset,
        const SD::FilePos origin) {
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
            return SD::INVALID_PTR_ORIGIN;
            break;
    }

    return 0;
}

uint32_t SD::ftellr (const SD::File *f) {
    return f->rPtr;
}

uint32_t SD::ftellw (const SD::File *f) {
    return f->wPtr;
}

#ifdef SD_OPTION_SHELL
int8_t SD::shell (SD::File *f) {
    char usrInput[SD_SHELL_INPUT_LEN] = "";
    char cmd[SD_SHELL_CMD_LEN] = "";
    char arg[SD_SHELL_ARG_LEN] = "";
    char uppercaseName[SD_SHELL_ARG_LEN] = "";
    uint8_t i, j, err;

    printf("Welcome to David's quick shell! "
            "There is no help, nor much to do.\n");
    printf("Have fun...\n");

    // Loop until the user types the SD::SHELL_EXIT string
    while (strcmp(usrInput, this->SHELL_EXIT)) {
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
        if (!strcmp(cmd, this->SHELL_LS))
            err = this->shell_ls();
        else if (!strcmp(cmd, this->SHELL_CAT))
            err = this->shell_cat(uppercaseName, f);
        else if (!strcmp(cmd, this->SHELL_CD))
            err = this->chdir(uppercaseName);
#ifdef SD_OPTION_FILE_WRITE
        else if (!strcmp(cmd, this->SHELL_CAT))
            err = this->shell_touch(uppercaseName);
#endif
#ifdef SD_OPTION_VERBOSE_BLOCKS
        else if (!strcmp(cmd, "d"))
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif
        else if (!strcmp(cmd, SD::SHELL_EXIT))
            break;
        else if (strcmp(usrInput, ""))
            printf("Invalid command: %s\n", cmd);

        // Check for errors
        if ((uint8_t) SD::EOC_END == err)
            printf("\tError, entry not found: \"%s\"\n", arg);
        else if ((uint8_t) SD::ENTRY_NOT_FILE == err)
            printf("\tError, entry not a file: \"%s\"\n", arg);
        else if ((uint8_t) SD::FILE_ALREADY_EXISTS == err)
            printf("\tError, file already exists: \"%s\"\n", arg);
        else if (err)
            printf("Error occurred: 0x%02X / %u\n", err, err);

        // Erase the command and argument strings
        for (i = 0; i < SD::SHELL_CMD_LEN; ++i)
            cmd[i] = 0;
        for (i = 0; i < SD::SHELL_ARG_LEN; ++i)
            uppercaseName[i] = arg[i] = 0;
        err = 0;
    }

    return 0;
}

int8_t SD::shell_ls (void) {
    uint8_t err;
    uint16_t fileEntryOffset = 0;
    char string[SD_FILENAME_STR_LEN];  // Allocate space for a filename string

    // If we aren't looking at the beginning of a cluster, we must backtrack to
    // the beginning and then begin listing files
    if (this->m_buf.curSectorOffset
            || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                    != this->m_buf.curClusterStartAddr)) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("'ls' requires a backtrack to beginning of directory's "
                "cluster\n");
#endif
        this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                this->m_dir_firstAllocUnit);
        this->m_buf.curSectorOffset = 0;
        this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
        sd_check_errors(
                this->get_fat_value(this->m_buf.curAllocUnit,
                        &(this->m_buf.nextAllocUnit)));
        sd_check_errors(
                this->read_data_block(this->m_buf.curClusterStartAddr,
                        this->m_buf.buf));
    }

    // Loop through all files in the current directory until we find the correct
    // one; Function will exit normally without an error code if the file is not
    // found
    while (this->m_buf.buf[fileEntryOffset]) {
        // Check if file is valid, retrieve the name if it is
        if ((SD::DELETED_FILE_MARK != this->m_buf.buf[fileEntryOffset])
                && !(SD::SYSTEM_FILE
                        & this->m_buf.buf[fileEntryOffset
                                + SD::FILE_ATTRIBUTE_OFFSET]))
            this->print_file_entry(&(this->m_buf.buf[fileEntryOffset]), string);

        // Increment to the next file
        fileEntryOffset += SD::FILE_ENTRY_LENGTH;

        // If it was the last entry in this sector, proceed to the next one
        if (SD::SECTOR_SIZE == fileEntryOffset) {
            // Last entry in the sector, attempt to load a new sector
            // Possible error value includes end-of-chain marker
            if ((err = this->load_next_sector(&this->m_buf))) {
                if ((uint8_t) SD::EOC_END == err)
                    break;
                else
                    this->error(err);
            }

            fileEntryOffset = 0;
        }
    }

    return 0;
}

int8_t SD::shell_cat (const char *name, SD::File *f) {
    uint8_t err;

    // Attempt to find the file
    if ((err = this->fopen(name, f, SD::FILE_MODE_R))) {
        if ((uint8_t) SD::EOC_END == err)
            return err;
        else
            this->error(err);
    } else {
        // Loop over each character and print them to the screen one-by-one
        while (!this->feof(f))
            // Using SDfgetc() instead of SDfgets to ensure compatibility with
            // binary files
            // TODO: Should probably create something better to output binary
            //       files don't ya think!?
            putchar(this->fgetc(f));
        putchar('\n');
    }

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
int8_t SD::shell_touch (const char name[]) {
    uint8_t err;
    uint16_t fileEntryOffset;

    // Attempt to find the file if it already exists
    if ((err = this->find(name, &fileEntryOffset))) {
        // Error occured - hopefully it was a "file not found" error
        if (SD::FILENAME_NOT_FOUND == err)
            // File wasn't found, let's create it
            err = this->create_file(name, &fileEntryOffset);
        return err;
    }

    // If SDFind() returns 0, the file already existed and an error should be
    // thrown
    return SD::FILE_ALREADY_EXISTS;
}
#endif
#endif

#if (defined SD_OPTION_VERBOSE || defined SD_OPTION_VERBOSE_BLOCKS)
int8_t SD::print_hex_block (uint8_t *dat, uint16_t bytes) {
    uint8_t i, j;
    uint8_t *s;

    printf("Printing %u bytes...\n", bytes);
    printf("Offset\t");
    for (i = 0; i < SD::LINE_SIZE; ++i)
        printf("0x%X  ", i);
    putchar('\n');

    if (bytes % SD::LINE_SIZE)
        bytes = bytes / SD::LINE_SIZE + 1;
    else
        bytes /= SD::LINE_SIZE;

    for (i = 0; i < bytes; ++i) {
        s = (uint8_t *) (dat + SD::LINE_SIZE * i);
        printf("0x%04X:\t", SD::LINE_SIZE * i);
        for (j = 0; j < SD::LINE_SIZE; ++j)
            printf("0x%02X ", s[j]);
        printf(" - ");
        for (j = 0; j < SD::LINE_SIZE; ++j) {
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

/**********************************
 *** Private Method Definitions ***
 **********************************/
int8_t SD::send_command (const uint8_t cmd, const uint32_t arg,
        const uint8_t crc) {
    uint8_t err;

    // Send out the command
    check_errors(spi_shift_out(8, cmd));

    // Send argument
    check_errors(spi_shift_out(16, (arg >> 16)));
    check_errors(spi_shift_out(16, arg & WORD_0));

    // Send sixth byte - CRC
    check_errors(spi_shift_out(8, crc));

    return 0;
}

int8_t SD::get_response (uint8_t bytes, uint8_t *dat) {
    uint8_t err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                spi_shift_in(8, &this->m_firstByteResponse,
                        sizeof(this->m_firstByteResponse)));

        // Check for timeout
        if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
            return SD::READ_TIMEOUT;
    } while (0xff == this->m_firstByteResponse);  // wait for transmission end

    // First byte in a response should always be either IDLE or ACTIVE. If this
    // one wans't, throw an error. If it was, decrement the bytes counter and
    // read in all remaining bytes
    if ((SD::RESPONSE_IDLE == this->m_firstByteResponse)
            || (SD::RESPONSE_ACTIVE == this->m_firstByteResponse)) {
        --bytes;    // Decrement bytes counter

        // Read remaining bytes
        while (bytes--)
            check_errors(spi_shift_in(8, dat++, sizeof(*dat)));
    } else
        return SD::INVALID_RESPONSE;

    // Responses should always be followed up by outputting 8 clocks with MOSI
    // high
    check_errors(spi_shift_out(8, 0xff));

    return 0;
}

int8_t SD::read_block (uint16_t bytes, uint8_t *dat) {
    uint8_t i, err, checksum;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                spi_shift_in(8, &this->m_firstByteResponse,
                        sizeof(this->m_firstByteResponse)));

        // Check for timeout
        if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
            return SD::READ_TIMEOUT;
    } while (0xff == this->m_firstByteResponse);  // wait for transmission end

    // Ensure this response is "active"
    if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse) {
        // Ignore blank data again
        timeout = SD::RESPONSE_TIMEOUT + CNT;
        do {
            check_errors(spi_shift_in(8, dat, sizeof(*dat)));

            // Check for timeout
            if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
                return SD::READ_TIMEOUT;
        } while (SD::DATA_START_ID != *dat);  // wait for transmission end

        // Check for the data start identifier and continue reading data
        if (SD::DATA_START_ID == *dat) {
            // Read in requested data bytes
#if (defined SPI_FAST_SECTOR)
            if (SD::SECTOR_SIZE == bytes) {
                spi_shift_in_sector(dat, 1);
                bytes = 0;
            }
#endif
            while (bytes--) {
#if (defined SD_OPTION_DEBUG)
                check_errors(spi_shift_in(8, dat++, sizeof(*dat)));
#elif (defined SPI_FAST)
                spi_shift_in_fast(8, dat++, sizeof(*dat));
#else
                spi_shift_in(8, dat++, sizeof(*dat));
#endif
            }

            // Read two more bytes for checksum - throw away data
            for (i = 0; i < 2; ++i) {
                timeout = SD::RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(spi_shift_in(8, &checksum, sizeof(checksum)));

                    // Check for timeout
                    if ((timeout - CNT) < SD::WIGGLE_ROOM)
                        return SD::READ_TIMEOUT;
                } while (0xff == checksum);  // wait for transmission end
            }

            // Send final 0xff
            check_errors(spi_shift_out(8, 0xff));
        } else {
            return SD::INVALID_DAT_STRT_ID;
        }
    } else
        return SD::INVALID_RESPONSE;

    return 0;
}

int8_t SD::write_block (uint16_t bytes, uint8_t *dat) {
    uint8_t err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                spi_shift_in(8, &this->m_firstByteResponse,
                        sizeof(this->m_firstByteResponse)));

        // Check for timeout
        if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
            return SD::READ_TIMEOUT;
    } while (0xff == this->m_firstByteResponse);  // wait for transmission end

// Ensure this response is "active"
    if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse) {
        // Received "active" response

        // Send data Start ID
        check_errors(spi_shift_out(8, SD::DATA_START_ID));

        // Send all bytes
        while (bytes--) {
#if (defined SD_OPTION_DEBUG)
            check_errors(spi_shift_out(8, *(dat++)));
#elif (defined SPI_FAST)
            spi_shift_out_fast(8, *(dat++));
#else
            spi_shift_out(8, *(dat++));
#endif
        }

        // Receive and digest response token
        timeout = SD::RESPONSE_TIMEOUT + CNT;
        do {
            check_errors(
                    spi_shift_in(8, &this->m_firstByteResponse,
                            sizeof(this->m_firstByteResponse)));

            // Check for timeout
            if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
                return SD::READ_TIMEOUT;
        } while (0xff == this->m_firstByteResponse);  // wait for transmission end
        if (SD::RSPNS_TKN_ACCPT
                != (this->m_firstByteResponse & (uint8_t) SD::RSPNS_TKN_BITS))
            return SD::INVALID_RESPONSE;
    }

    return 0;
}

int8_t SD::read_data_block (uint32_t address, uint8_t *dat) {
    uint8_t err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        spi_shift_in(8, &temp, 1);

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Reading block at sector address: 0x%08X / %u\n", address, address);
#endif

    gpio_pin_clear(this->m_cs);
    check_errors(this->send_command(SD::CMD_RD_BLOCK, address, SD::CRC_OTHER));

    if ((err = this->read_block(SD::SECTOR_SIZE, dat))) {
#ifdef SD_OPTION_DEBUG
        this->m_sectorRdAddress = address;
#endif
        return err;
    }
    gpio_pin_set(this->m_cs);

    return 0;
}

int8_t SD::write_data_block (uint32_t address, uint8_t *dat) {
    uint8_t err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        spi_shift_in(8, &temp, 1);

#if (defined SD_OPTION_DEBUG && defined SD_OPTION_VERBOSE)
    printf("Writing block at address: 0x%08X / %u\n", address, address);
#endif

    gpio_pin_clear(this->m_cs);
    check_errors(this->send_command(SD::CMD_WR_BLOCK, address, SD::CRC_OTHER));

    check_errors(this->write_block(SD::SECTOR_SIZE, dat));
    gpio_pin_set(this->m_cs);

    return 0;
}

uint16_t SD::read_rev_dat16 (const uint8_t buf[]) {
    return (buf[1] << 8) + buf[0];
}

uint32_t SD::read_rev_dat32 (const uint8_t buf[]) {
    return (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
}

#ifdef SD_OPTION_FILE_WRITE
void SD::write_rev_dat16 (uint8_t buf[], const uint16_t dat) {
    buf[1] = (uint8_t) (dat >> 8);
    buf[0] = (uint8_t) dat;
}

void SD::write_rev_dat32 (uint8_t buf[], const uint32_t dat) {
    buf[3] = (uint8_t) (dat >> 24);
    buf[2] = (uint8_t) (dat >> 16);
    buf[1] = (uint8_t) (dat >> 8);
    buf[0] = (uint8_t) dat;
}
#endif

uint32_t SD::find_sector_from_path (const char *path) {
    // TODO: Return an actual path

    /*if ('/' == path[0]) {
     } // Start from the root address
     else {
     } // Start from the current directory*/

    return this->m_rootAddr;
}

uint32_t SD::find_sector_from_alloc (uint32_t allocUnit) {
    if (SD::FAT_32 == this->m_filesystem)
        allocUnit -= this->m_rootAllocUnit;
    else
        allocUnit -= 2;
    allocUnit <<= this->m_sectorsPerCluster_shift;
    allocUnit += this->m_firstDataAddr;
    return allocUnit;
}

int8_t SD::get_fat_value (const uint32_t fatEntry, uint32_t *value) {
    uint8_t err;
    uint32_t firstAvailableAllocUnit;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Reading from the FAT...\n");
    printf("\tLooking for entry: 0x%08X / %u\n", fatEntry, fatEntry);
#endif

    // Do we need to load a new fat sector?
    if ((fatEntry >> this->m_entriesPerFatSector_Shift)
            != this->m_curFatSector) {
#ifdef SD_OPTION_FILE_WRITE
        // If the currently loaded FAT sector has been modified, save it
        if (this->m_fatMod) {
            this->write_data_block(this->m_curFatSector + this->m_fatStart,
                    this->m_fat);
            this->write_data_block(
                    this->m_curFatSector + this->m_fatStart + this->m_fatSize,
                    this->m_fat);
            this->m_fatMod = false;
        }
#endif
        // Need new sector, load it
        this->m_curFatSector = fatEntry >> this->m_entriesPerFatSector_Shift;
        check_errors(
                this->read_data_block(this->m_curFatSector + this->m_fatStart,
                        this->m_fat));
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
        this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
    }
    firstAvailableAllocUnit = this->m_curFatSector
            << this->m_entriesPerFatSector_Shift;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("\tLooks like I need FAT sector: 0x%08X / %u\n",
            this->m_curFatSector, this->m_curFatSector);
    printf("\tWith an offset of: 0x%04X / %u\n",
            (fatEntry - firstAvailableAllocUnit) << 2,
            (fatEntry - firstAvailableAllocUnit) << 2);
#endif

    // The necessary FAT sector has been loaded and the next allocation unit is
    // known, proceed with loading the next data sector and incrementing the
    // cluster variables

    // Retrieve the next allocation unit number
    if (SD::FAT_16 == this->m_filesystem)
        *value = this->read_rev_dat16(
                &this->m_fat[(fatEntry - firstAvailableAllocUnit) << 1]);
    else
        /* Implied check for (SD::FAT_32 == this->m_filesystem) */
        *value = this->read_rev_dat32(
                &this->m_fat[(fatEntry - firstAvailableAllocUnit) << 2]);
    // Clear the highest 4 bits - they are always reserved
    *value &= 0x0FFFFFFF;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("\tReceived value: 0x%08X / %u\n", *value, *value);
#endif

    return 0;
}

int8_t SD::load_next_sector (SD::Buffer *buf) {
#ifdef SD_OPTION_FILE_WRITE
    if (buf->mod)
        this->write_data_block(buf->curClusterStartAddr + buf->curSectorOffset,
                buf->buf);
#endif

    // Check for the end-of-chain marker (end of file)
    if (((uint32_t) SD::EOC_BEG) <= buf->nextAllocUnit)
        return SD::EOC_END;

    // Are we looking at the root directory of a FAT16 system?
    if (SD::FAT_16 == this->m_filesystem
            && this->m_rootAddr == (buf->curClusterStartAddr)) {
        // Root dir of FAT16; Is it the last sector in the root directory?
        if (this->m_rootDirSectors == (buf->curSectorOffset))
            return SD::EOC_END;
        // Root dir of FAT16; Not last sector
        else
            // Any error from reading the data block will be returned to calling
            // function
            return this->read_data_block(++(buf->curSectorOffset), buf->buf);
    }
    // We are looking at a generic data cluster.
    else {
        // Gen. data cluster; Have we reached the end of the cluster?
        if (((1 << this->m_sectorsPerCluster_shift) - 1)
                > (buf->curSectorOffset)) {
            // Gen. data cluster; Not the end; Load next sector in the cluster

            // Any error from reading the data block will be returned to
            // calling function
            return this->read_data_block(
                    ++(buf->curSectorOffset) + buf->curClusterStartAddr,
                    buf->buf);
        }
        // End of generic data cluster; Look through the FAT to find the next cluster
        else
            return this->inc_cluster(buf);
    }

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("New sector loaded:\n");
    this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

int8_t SD::load_sector_from_offset (SD::File *f, const uint32_t offset) {
    uint8_t err;
    uint32_t clusterOffset = offset >> this->m_sectorsPerCluster_shift;

#ifdef SD_OPTION_FILE_WRITE
    // If the buffer has been modified, write it before loading the next sector
    if (f->buf->mod) {
        this->write_data_block(
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->buf);
        f->buf->mod = false;
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
            check_errors(
                    this->get_fat_value(f->buf->curAllocUnit,
                            &(f->buf->nextAllocUnit)));
        }
        f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                f->buf->curAllocUnit);
    } else if (f->curCluster > clusterOffset) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need to backtrack through the FAT to find the cluster\n");
#endif
        // Desired cluster is an earlier cluster than the currently loaded one -
        // this requires starting from the beginning and working forward
        f->buf->curAllocUnit = f->firstAllocUnit;
        check_errors(
                this->get_fat_value(f->buf->curAllocUnit,
                        &(f->buf->nextAllocUnit)));
        f->curCluster = 0;
        while (clusterOffset--) {
            ++(f->curCluster);
            f->buf->curAllocUnit = f->buf->nextAllocUnit;
            check_errors(
                    this->get_fat_value(f->buf->curAllocUnit,
                            &(f->buf->nextAllocUnit)));
        }
        f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                f->buf->curAllocUnit);
    }

    // Followed by finding the correct sector
    f->buf->curSectorOffset = offset % (1 << this->m_sectorsPerCluster_shift);
    f->curSector = offset;
    this->read_data_block(f->buf->curClusterStartAddr + f->buf->curSectorOffset,
            f->buf->buf);

    return 0;
}

int8_t SD::inc_cluster (SD::Buffer *buf) {
    uint8_t err;

#ifdef SD_OPTION_FILE_WRITE
    // If the sector has been modified, write it back to the SD card before
    // reading again
    if (buf->mod) {
        check_errors(
                this->write_data_block(
                        buf->curClusterStartAddr + buf->curSectorOffset,
                        buf->buf));
    }
    buf->mod = false;
#endif

    // Update this->m_cur*
    if (((uint32_t) SD::EOC_BEG) <= buf->curAllocUnit
            && ((uint32_t) SD::EOC_END) <= buf->curAllocUnit)
        return SD::READING_PAST_EOC;
    buf->curAllocUnit = buf->nextAllocUnit;
    // Only look ahead to the next allocation unit if the current alloc unit is
    // not EOC
    if (!(((uint32_t) SD::EOC_BEG) <= buf->curAllocUnit
            && ((uint32_t) SD::EOC_END) <= buf->curAllocUnit))
        // Current allocation unit is not EOC, read the next one
        check_errors(
                this->get_fat_value(buf->curAllocUnit, &(buf->nextAllocUnit)));
    buf->curClusterStartAddr = this->find_sector_from_alloc(buf->curAllocUnit);
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
    check_errors(this->read_data_block(buf->curClusterStartAddr, buf->buf));
    this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
    return 0;
#else
    return sd_read_data_block(buf->curClusterStartAddr, buf->buf);
#endif
}

void SD::get_filename (const uint8_t *buf, char *filename) {
    uint8_t i, j = 0;

    // Read in the first 8 characters - stop when a space is reached or 8
    // characters have been read, whichever comes first
    for (i = 0; i < SD::FILE_NAME_LEN; ++i) {
        if (0x05 == buf[i])
            filename[j++] = 0xe5;
        else if (' ' != buf[i])
            filename[j++] = buf[i];
    }

    // Determine if there is more past the first 8 - Again, stop when a space is
    // reached
    if (' ' != buf[SD::FILE_NAME_LEN]) {
        filename[j++] = '.';
        for (i = SD::FILE_NAME_LEN;
                i < SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN; ++i) {
            if (' ' != buf[i])
                filename[j++] = buf[i];
        }
    }

    // Insert null-terminator
    filename[j] = 0;
}

int8_t SD::find (const char *filename, uint16_t *fileEntryOffset) {
    uint8_t err;
    char readEntryName[SD::FILENAME_STR_LEN];

#ifdef SD_OPTION_FILE_WRITE
    // Save the current buffer
    if (this->m_buf.mod) {
        check_errors(
                this->write_data_block(
                        this->m_buf.curClusterStartAddr
                                + this->m_buf.curSectorOffset,
                        this->m_buf.buf));
        this->m_buf.mod = false;
    }
#endif

    *fileEntryOffset = 0;

    // If we aren't looking at the beginning of the directory cluster, we must
    // backtrack to the beginning and then begin listing files
    if (this->m_buf.curSectorOffset
            || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                    != this->m_buf.curClusterStartAddr)) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("'find' requires a backtrack to beginning of directory's "
                "cluster\n");
#endif
        this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                this->m_dir_firstAllocUnit);
        this->m_buf.curSectorOffset = 0;
        this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
        check_errors(
                this->get_fat_value(this->m_buf.curAllocUnit,
                        &this->m_buf.nextAllocUnit));
        check_errors(
                this->read_data_block(this->m_buf.curClusterStartAddr,
                        this->m_buf.buf));
    }
    this->m_buf.id = SD::FOLDER_ID;

    // Loop through all entries in the current directory until we find the
    // correct one
    // Function will exit normally with SD::EOC_END error code if the file is not
    // found
    while (this->m_buf.buf[*fileEntryOffset]) {
        // Check if file is valid, retrieve the name if it is
        if (!(SD::DELETED_FILE_MARK == this->m_buf.buf[*fileEntryOffset])) {
            this->get_filename(&(this->m_buf.buf[*fileEntryOffset]),
                    readEntryName);
            if (!strcmp(filename, readEntryName))
                // File names match, return 0 to indicate a successful search
                return 0;
        }

        // Increment to the next file
        *fileEntryOffset += SD::FILE_ENTRY_LENGTH;

        // If it was the last entry in this sector, proceed to the next one
        if (SD::SECTOR_SIZE == *fileEntryOffset) {
            // Last entry in the sector, attempt to load a new sector
            // Possible error value includes end-of-chain marker
            check_errors(this->load_next_sector(&this->m_buf));

            *fileEntryOffset = 0;
        }
    }

    return FILENAME_NOT_FOUND;
}

int8_t SD::reload_buf (SD::File *f) {
    uint8_t err;

    // Function is only called if it has already been determined that the buffer
    // needs to be loaded - no checks need to be run

#ifdef SD_OPTION_FILE_WRITE
    // If the currently loaded buffer has been modified, save it
    if (f->buf->mod) {
        check_errors(
                this->write_data_block(
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->buf));
        f->buf->mod = false;
    }
#endif

    // Set current values to show that the first sector of the file is loaded.
    // SDLoadSectorFromOffset() loads the sector unconditionally before
    // returning so we do not need to load the sector here
    f->buf->curAllocUnit = f->firstAllocUnit;
    f->buf->curClusterStartAddr = this->find_sector_from_alloc(
            f->firstAllocUnit);
    f->buf->curSectorOffset = 0;
    check_errors(
            this->get_fat_value(f->firstAllocUnit, &(f->buf->nextAllocUnit)));

    // Proceed with loading the sector
    check_errors(this->load_sector_from_offset(f, f->curSector));
    f->buf->id = f->id;

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
uint32_t SD::find_empty_space (const uint8_t restore) {
    uint16_t allocOffset = 0;
    uint32_t fatSectorAddr = this->m_curFatSector + this->m_fatStart;
    uint32_t retVal;
    // NOTE: this->m_curFatSector is not modified until end of function - it is
    // used throughout this function as the original starting point

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("\n*** SDFindEmptySpace() initialized with FAT sector 0x%08X / %u "
            "loaded ***\n", this->m_curFatSector, this->m_curFatSector);
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

    // Find the first empty allocation unit and write the EOC marker
    if (SD::FAT_16 == this->m_filesystem) {
        // Loop until we find an empty cluster
        while (this->read_rev_dat16(&(this->m_fat[allocOffset]))) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
            printf("Searching the following sector...\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
            // Stop when we either reach the end of the current block or find an
            // empty cluster
            while (this->read_rev_dat16(&(this->m_fat[allocOffset]))
                    && (SD::SECTOR_SIZE > allocOffset))
                allocOffset += SD::FAT_16;
            // If we reached the end of a sector...
            if (SD::SECTOR_SIZE <= allocOffset) {
                // If the currently loaded FAT sector has been modified, save it
                if (this->m_fatMod) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("FAT sector has been modified; saving now... ");
#endif
                    this->write_data_block(this->m_curFatSector, this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatSize,
                            this->m_fat);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("done!\n");
#endif
                    this->m_fatMod = false;
                }
                // Read the next fat sector
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08X / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                this->read_data_block(++fatSectorAddr, this->m_fat);
            }
        }
        this->write_rev_dat16(this->m_fat + allocOffset,
                (uint16_t) SD::EOC_END);
        this->m_fatMod = true;
    } else /* Implied and not needed: "if (SD::FAT_32 == this->m_filesystem)" */{
        // In FAT32, the first 7 usable clusters seem to be un-officially
        // reserved for the root directory
        if (0 == this->m_curFatSector)
            allocOffset = 9 * this->m_filesystem;

        // Loop until we find an empty cluster
        while (this->read_rev_dat32(&(this->m_fat[allocOffset])) & 0x0fffffff) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
            printf("Searching the following sector...\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
            // Stop when we either reach the end of the current block or find an
            // empty cluster
            while ((this->read_rev_dat32(&(this->m_fat[allocOffset]))
                    & 0x0fffffff) && (SD::SECTOR_SIZE > allocOffset))
                allocOffset += SD::FAT_32;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
            printf("Broke while loop... why? Offset = 0x%04X / %u\n",
                    allocOffset, allocOffset);
#endif
            // If we reached the end of a sector...
            if (SD::SECTOR_SIZE <= allocOffset) {
                if (this->m_fatMod) {
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("FAT sector has been modified; saving now... ");
#endif
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart,
                            this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart
                                    + this->m_fatSize, this->m_fat);
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                    printf("done!\n");
#endif
                    this->m_fatMod = false;
                }
                // Read the next fat sector
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08X / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                this->read_data_block(++fatSectorAddr, this->m_fat);
                allocOffset = 0;
            }
        }
        this->write_rev_dat32(&(this->m_fat[allocOffset]),
                ((uint32_t) SD::EOC_END) & 0x0fffffff);
        this->m_fatMod = true;
    }

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Available space found: 0x%08X / %u\n",
            (this->m_curFatSector << this->m_entriesPerFatSector_Shift)
                    + allocOffset / this->m_filesystem,
            (this->m_curFatSector << this->m_entriesPerFatSector_Shift)
                    + allocOffset / this->m_filesystem);
#endif

    // If we loaded a new fat sector (and then modified it directly above),
    // write the sector before re-loading the original
    if ((fatSectorAddr != (this->m_curFatSector + this->m_fatStart))
            && this->m_fatMod) {
        this->write_data_block(fatSectorAddr, this->m_fat);
        this->write_data_block(fatSectorAddr + this->m_fatSize, this->m_fat);
        this->m_fatMod = false;
        this->read_data_block(this->m_curFatSector + this->m_fatStart,
                this->m_fat);
    } else
        this->m_curFatSector = fatSectorAddr - this->m_fatStart;

    // Return new address to end-of-chain
    retVal = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
    retVal += allocOffset / this->m_filesystem;
    return retVal;
}

int8_t SD::extend_fat (SD::Buffer *buf) {
    uint8_t err;
    uint32_t newAllocUnit;
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    printf("Extending file or directory now...\n");
#endif

    // Do we need to load a different sector of the FAT or is the correct one
    // currently loaded? (Correct means the sector currently containing the EOC
    // marker)
    if ((buf->curAllocUnit >> this->m_entriesPerFatSector_Shift)
            != this->m_curFatSector) {

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
        printf("Need new FAT sector. Loading: 0x%08X / %u\n",
                buf->curAllocUnit >> this->m_entriesPerFatSector_Shift,
                buf->curAllocUnit >> this->m_entriesPerFatSector_Shift);
        printf("... because the current allocation unit is: 0x%08X / %u\n",
                buf->curAllocUnit, buf->curAllocUnit);
#endif
        // Need new sector, save the old one...
        if (this->m_fatMod) {
            this->write_data_block(this->m_curFatSector + this->m_fatStart,
                    this->m_fat);
            this->write_data_block(
                    this->m_curFatSector + this->m_fatStart + this->m_fatSize,
                    this->m_fat);
            this->m_fatMod = false;
        }
        // And load the new one...
        this->m_curFatSector = buf->curAllocUnit
                >> this->m_entriesPerFatSector_Shift;
        check_errors(
                this->read_data_block(this->m_curFatSector + this->m_fatStart,
                        this->m_fat));
    }

#ifdef SD_OPTION_DEBUG
    // This function should only be called when a file or directory has reached
    // the end of its cluster chain
    if (((uint32_t) SD::EOC_BEG)
            <= this->read_rev_dat32(
                    &(this->m_fat[(buf->curAllocUnit
                            % (1 << this->m_entriesPerFatSector_Shift))
                            * this->m_filesystem])))
        return SD::INVALID_FAT_APPEND;
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    // Display the currently loaded FAT.... for no reason... not sure why I
    // wanted to do this...
    printf("This is the sector that *should* contain the EOC marker...\n");
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

    // Find where the next cluster of the file should be stored...
    newAllocUnit = this->find_empty_space(1);

    // Now that we know the allocation unit, write it to the FAT buffer
    if (SD::FAT_16 == this->m_filesystem) {
        this->write_rev_dat16(
                &(this->m_fat[(buf->curAllocUnit
                        % (1 << this->m_entriesPerFatSector_Shift))
                        * this->m_filesystem]), (uint16_t) newAllocUnit);
    } else {
        this->write_rev_dat32(
                &(this->m_fat[(buf->curAllocUnit
                        % (1 << this->m_entriesPerFatSector_Shift))
                        * this->m_filesystem]), newAllocUnit);
    }
    buf->nextAllocUnit = newAllocUnit;
    this->m_fatMod = true;  // And mark the buffer as modified

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("After modification, the FAT now looks like...\n");
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

    return 0;
}

int8_t SD::create_file (const char *name, const uint16_t *fileEntryOffset) {
    uint8_t i, j;
    // *name is only checked for uppercase
    char uppercaseName[SD::FILENAME_STR_LEN];
    uint32_t allocUnit;

#ifdef SD_OPTION_DEBUG
#ifdef SD_OPTION_VERBOSE
    printf("Creating new file: %s\n", name);
#endif
    // Parameter checking...
    if (SD::FILENAME_STR_LEN < strlen(name))
        return SD::INVALID_FILENAME;

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
        this->m_buf.buf[*fileEntryOffset + i] = name[i];
    // Check if there is an extension
    if (name[i]) {
        // There might be an extension - pad first name with spaces
        for (j = i; j < SD::FILE_NAME_LEN; ++j)
            this->m_buf.buf[*fileEntryOffset + j] = ' ';
        // Check if there is a period, as one would expect for a file name with
        // an extension
        if ('.' == name[i]) {
            // Extension exists, write it
            ++i;  // Skip the period
            // Insert extension, character-by-character
            for (j = SD::FILE_NAME_LEN; name[i]; ++j)
                this->m_buf.buf[*fileEntryOffset + j] = name[i++];
            // Pad extension with spaces
            for (; j < SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN; ++j)
                this->m_buf.buf[*fileEntryOffset + j] = ' ';
        }
        // If it wasn't a period or null terminator, throw an error
        else
            return SD::INVALID_FILENAME;
    }
    // No extension, pad with spaces
    else
        for (; i < (SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN); ++i)
            this->m_buf.buf[*fileEntryOffset + i] = ' ';

    /* 2) Write attribute field... */
    // TODO: Allow for file attribute flags to be set, such as SD::READ_ONLY,
    //       SD::SUB_DIR, etc
    this->m_buf.buf[*fileEntryOffset + SD::FILE_ATTRIBUTE_OFFSET] = SD::ARCHIVE;  // Archive flag should be set because the file is new
    this->m_buf.mod = true;

#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_DEBUG)
    SD::print_file_entry(&(this->m_buf.buf[*fileEntryOffset]), uppercaseName);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    SD::print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif

    /* 3) Find a spot in the FAT (do not check for a full FAT, assume space is
     * available)
     */
    allocUnit = this->find_empty_space(0);
    this->write_rev_dat16(
            &(this->m_buf.buf[*fileEntryOffset + SD::FILE_START_CLSTR_LOW]),
            (uint16_t) allocUnit);
    if (SD::FAT_32 == this->m_filesystem)
        this->write_rev_dat16(
                &(this->m_buf.buf[*fileEntryOffset + SD::FILE_START_CLSTR_HIGH]),
                (uint16_t) (allocUnit >> 16));

    /* 4) Write the size of the file (currently 0) */
    this->write_rev_dat32(
            &(this->m_buf.buf[*fileEntryOffset + SD::FILE_LEN_OFFSET]), 0);

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE && \
        defined SD_OPTION_DEBUG)
    printf("New file entry at offset 0x%08X / %u looks like...\n",
            *fileEntryOffset, *fileEntryOffset);
    SD::print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif

    this->m_buf.mod = 1;

    return 0;
}
#endif

#if (defined SD_OPTION_SHELL || defined SD_OPTION_VERBOSE)
void SD::print_file_entry (const uint8_t *fileEntry, char filename[]) {
    print_file_attributes(fileEntry[SD::FILE_ATTRIBUTE_OFFSET]);
    this->get_filename(fileEntry, filename);
    printf("\t\t%s", filename);
    if (SD::SUB_DIR & fileEntry[SD::FILE_ATTRIBUTE_OFFSET])
        putchar('/');
    putchar('\n');
}

void SD::print_file_attributes (const uint8_t flags) {
    // Print file attributes
    if (SD::READ_ONLY & flags)
        putchar(SD::READ_ONLY_CHAR);
    else
        putchar(SD::READ_ONLY_CHAR_);

    if (SD::HIDDEN_FILE & flags)
        putchar(SD::HIDDEN_FILE_CHAR);
    else
        putchar(SD::HIDDEN_FILE_CHAR_);

    if (SD::SYSTEM_FILE & flags)
        putchar(SD::SYSTEM_FILE_CHAR);
    else
        putchar(SD::SYSTEM_FILE_CHAR_);

    if (SD::VOLUME_ID & flags)
        putchar(SD::VOLUME_ID_CHAR);
    else
        putchar(SD::VOLUME_ID_CHAR_);

    if (SD::SUB_DIR & flags)
        putchar(SD::SUB_DIR_CHAR);
    else
        putchar(SD::SUB_DIR_CHAR_);

    if (SD::ARCHIVE & flags)
        putchar(SD::ARCHIVE_CHAR);
    else
        putchar(SD::ARCHIVE_CHAR_);
}
#endif

#ifdef SD_OPTION_DEBUG
void SD::error (const uint8_t err) {
    char str[] = "SD Error %u: %s\n";

    switch (err) {
        case SD::INVALID_CMD:
            printf(str, (err - SD_ERRORS_BASE), "Invalid command");
            break;
        case SD::READ_TIMEOUT:
            printf(str, (err - SD_ERRORS_BASE), "Timed out during read");
            printf("\tRead sector address was: 0x%08X / %u",
                    this->m_sectorRdAddress, this->m_sectorRdAddress);
            break;
        case SD::INVALID_NUM_BYTES:
            printf(str, (err - SD_ERRORS_BASE), "Invalid number of bytes");
            break;
        case SD::INVALID_RESPONSE:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02X\nThe following bits are set:\n",
                    (err - SD_ERRORS_BASE),
                    "Invalid first-byte response\n\tReceived: ",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
                    "Invalid first-byte response\n\tReceived: ",
                    this->m_firstByteResponse);
#endif
            first_byte_expansion(this->m_firstByteResponse);
            break;
        case SD::INVALID_INIT:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s\n\tResponse: 0x%02X\n",
                    (err - SD_ERRORS_BASE),
                    "Invalid response during initialization",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s\n\tResponse: %u\n", (err - SD_ERRORS_BASE),
                    "Invalid response during initialization",
                    this->m_firstByteResponse);
#endif
            break;
        case SD::INVALID_FILESYSTEM:
            printf(str, (err - SD_ERRORS_BASE), "Invalid filesystem");
            break;
        case SD::INVALID_DAT_STRT_ID:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02X\n", (err - SD_ERRORS_BASE),
                    "Invalid data-start ID\n\tReceived: ",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", (err - SD_ERRORS_BASE),
                    "Invalid data-start ID\n\tReceived: ",
                    this->m_firstByteResponse);
#endif
            break;
        case SD::FILENAME_NOT_FOUND:
            printf(str, (err - SD_ERRORS_BASE), "Filename not found");
            break;
        case SD::EMPTY_FAT_ENTRY:
            printf(str, (err - SD_ERRORS_BASE), "FAT points to empty entry");
            break;
        case SD::CORRUPT_CLUSTER:
            printf(str, (err - SD_ERRORS_BASE), "SD cluster is corrupt");
            break;
        case SD::INVALID_PTR_ORIGIN:
            printf(str, (err - SD_ERRORS_BASE), "Invalid pointer origin");
            break;
        case SD::ENTRY_NOT_FILE:
            printf(str, (err - SD_ERRORS_BASE),
                    "Requested file entry is not a file");
            break;
        case SD::INVALID_FILENAME:
            printf(str, (err - SD_ERRORS_BASE),
                    "Invalid filename - please use 8.3 format");
            break;
        case SD::INVALID_FAT_APPEND:
            printf(str, (err - SD_ERRORS_BASE),
                    "FAT entry append was attempted unnecessarily");
            break;
        case SD::FILE_ALREADY_EXISTS:
            printf(str, (err - SD_ERRORS_BASE),
                    "Attempting to create an already existing file");
            break;
        case SD::INVALID_FILE_MODE:
            printf(str, (err - SD_ERRORS_BASE), "Invalid file mode");
            break;
        case SD::TOO_MANY_FATS:
            printf(str, (err - SD_ERRORS_BASE),
                    "This driver is only capable of writing files on FAT "
                            "partitions with two (2) copies of the FAT");
            break;
        case SD::FILE_WITHOUT_BUFFER:
            printf(str, (err - SD_ERRORS_BASE),
                    "SDfopen() was passed a file struct with "
                            "an uninitialized buffer");
            break;
        case SD::CMD8_FAILURE:
            printf(str, (err - SD_ERRORS_BASE), "CMD8 never received a proper "
                    "response; This is most likely to occur when the SD card "
                    "does not support the 3.3V I/O used by the Propeller");
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

void SD::first_byte_expansion (const uint8_t response) {
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

}
