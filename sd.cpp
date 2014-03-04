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

// Initialize string constants
const char SD::SHELL_EXIT[] = "exit";
const char SD::SHELL_LS[] = "ls";
const char SD::SHELL_CAT[] = "cat";
const char SD::SHELL_CD[] = "cd";
const char SD::SHELL_TOUCH[] = "touch";

const uint32_t SD::RESPONSE_TIMEOUT = 100 * MILLISECOND;
const uint32_t SD::WIGGLE_ROOM = 1000;

/*********************************
 *** Public Method Definitions ***
 *********************************/
SD::SD () {
    this->m_fileID = 0;
#ifdef SD_OPTION_FILE_WRITE
    this->m_fatMod = false;
#endif
}

SD::Buffer* SD::getGlobalBuffer () {
    return &(this->m_buf);
}

PropWare::ErrorCode SD::start (const PropWare::GPIO::Pin mosi,
        const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin sclk,
        const PropWare::GPIO::Pin cs, const int32_t freq) {
    PropWare::ErrorCode err;
    uint8_t response[16];

    // Set CS for output and initialize high
    this->m_spi = SPI::getSPI();
    this->m_cs = cs;
    GPIO::set_dir(cs, GPIO::OUT);
    GPIO::pin_set(cs);

    // Start SPI module
    if ((err = this->m_spi->start(mosi, miso, sclk, SD::SPI_INIT_FREQ,
            SD::SPI_MODE, SD::SPI_BITMODE)))
        return err;

    // Try and get the card up and responding to commands first
    check_errors(this->reset_and_verify_v2_0(response));

    check_errors(this->send_active(response));

    check_errors(this->increase_throttle(freq));

#ifdef SD_OPTION_VERBOSE
    check_errors(this->print_init_debug_blocks(response));
#endif

    // We're finally done initializing everything. Set chip select high again to
    // release the SPI port
    GPIO::pin_set(cs);

    // Initialization complete
    return 0;
}

PropWare::ErrorCode SD::reset_and_verify_v2_0 (uint8_t response[]) {
    PropWare::ErrorCode err;
    uint8_t i, j;
    bool stageCleared;

#ifdef SD_OPTION_VERBOSE
    printf("Starting SD card...\n");
#endif

    // Attempt initialization no more than 10 times
    stageCleared = false;
    for (i = 0; i < 10 && !stageCleared; ++i) {
        // Initialization loop (reset SD card)
        for (j = 0; j < 10 && !stageCleared; ++j) {
            check_errors(this->power_up());

            check_errors(this->reset(response, &stageCleared));
        }

        // If we couldn't go idle after 10 tries, give up
        if (!stageCleared)
            return SD::INVALID_INIT;

        stageCleared = false;
        check_errors(this->verify_v2_0(response, &stageCleared));
    }

    // If CMD8 never succeeded, throw an error
    if (!stageCleared)
        return SD::CMD8_FAILURE;

    // The card is idle, that's good. Let's make sure we get the correct
    // response back
    if ((SD::HOST_VOLTAGE_3V3 != response[2])
            || (SD::R7_CHECK_PATTERN != response[3]))
        return SD::CMD8_FAILURE;

    return 0;
}

PropWare::ErrorCode SD::power_up () {
    uint8_t i;
    PropWare::ErrorCode err;

    waitcnt(CLKFREQ / 10 + CNT);

    // Send at least 72 clock cycles to enable the SD card
    GPIO::pin_set(this->m_cs);
    for (i = 0; i < 128; ++i)
        check_errors(this->m_spi->shift_out(16, -1));

    // Be very super 100% sure that all clocks have finished ticking
    // before setting chip select low
    check_errors(this->m_spi->wait());
    waitcnt(10*MILLISECOND + CNT);

    // Chip select goes low for the duration of this function
    GPIO::pin_clear(this->m_cs);

    return 0;
}

PropWare::ErrorCode SD::reset (uint8_t response[], bool *isIdle) {
    PropWare::ErrorCode err;

    // Send SD into idle state, retrieve a response and ensure it is the
    // "idle" response
    check_errors(this->send_command(SD::CMD_IDLE, 0, SD::CRC_IDLE));
    this->get_response(SD::RESPONSE_LEN_R1, response);

    // Check if idle
    if (SD::RESPONSE_IDLE == this->m_firstByteResponse)
        *isIdle = true;
#ifdef SD_OPTION_VERBOSE
    else
    printf("Failed attempt at CMD0: 0x%02x\n", this->m_firstByteResponse);
#endif

    return 0;
}

PropWare::ErrorCode SD::verify_v2_0 (uint8_t response[], bool *stageCleared) {
    PropWare::ErrorCode err;

#ifdef SD_OPTION_VERBOSE
    printf("SD card in idle state. Now sending CMD8...\n");
#endif

    // Inform SD card that the Propeller uses the 2.7-3.6V range;
    check_errors(
            this->send_command(SD::CMD_INTERFACE_COND, SD::ARG_CMD8,
                    SD::CRC_CMD8));
    check_errors(this->get_response(SD::RESPONSE_LEN_R7, response));
    if (SD::RESPONSE_IDLE == this->m_firstByteResponse)
        *stageCleared = true;

    // Print an error message after every failure
#ifdef SD_OPTION_VERBOSE
    if (!stageCleared)
    printf("Failed attempt at CMD8: 0x%02x, 0x%02x, 0x%02x;\n",
            this->m_firstByteResponse, response[2], response[3]);
#endif

    return 0;
}

PropWare::ErrorCode SD::send_active (uint8_t response[]) {
    PropWare::ErrorCode err;
    uint8_t i;

    bool stageCleared = false;
    for (i = 0; i < 8 && !stageCleared; ++i) {
        // Send the application-specific pre-command
        check_errors(this->send_command(SD::CMD_APP, 0, SD::CRC_ACMD_PREP));
        check_errors(this->get_response(SD::RESPONSE_LEN_R1, response));

        // Request that the SD card go active!
        check_errors(this->send_command(SD::CMD_WR_OP, BIT_30, 0));
        check_errors(this->get_response(SD::RESPONSE_LEN_R1, response));

        // If the card ACKed with the active state, we're all good!
        if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse)
            stageCleared = true;
#ifdef SD_OPTION_VERBOSE
        else
        printf("Failed attempt at active state: 0x%02x\n",
                this->m_firstByteResponse);
#endif
    }

    // If we couldn't get the card to go active after 8 tries, give up
    if (!stageCleared)
        return SD::INVALID_RESPONSE;

#ifdef SD_OPTION_VERBOSE
    // We did it!
    printf("Activated!\n");
#endif

    return 0;
}

PropWare::ErrorCode SD::increase_throttle (const int32_t freq) {
    PropWare::ErrorCode err;

    // Initialization nearly complete, increase clock
#ifdef SD_OPTION_VERBOSE
    printf("Increasing clock to full speed\n");
#endif
    if (-1 == freq || 0 == freq) {
        check_errors(this->m_spi->set_clock(SD::DEFAULT_SPI_FREQ));
    } else {
        check_errors(this->m_spi->set_clock(freq));
    }

    return 0;
}

#ifdef SD_OPTION_VERBOSE
PropWare::ErrorCode SD::print_init_debug_blocks (uint8_t response[]) {
    PropWare::ErrorCode err;

    // Request operating conditions register and ensure response begins with R1
    check_errors(this->send_command(SD::CMD_READ_OCR, 0, SD::CRC_OTHER));
    check_errors(this->get_response(SD::RESPONSE_LEN_R3, response));
    printf("Operating Conditions Register (OCR)...\n");
    this->print_hex_block(response, SD::RESPONSE_LEN_R3);

    // If debugging requested, print to the screen CSD and CID registers from SD
    // card
    printf("Requesting CSD...\n");
    check_errors(this->send_command(SD::CMD_RD_CSD, 0, SD::CRC_OTHER));
    check_errors(this->read_block(16, response));
    printf("CSD Contents:\n");
    this->print_hex_block(response, 16);
    putchar('\n');

    printf("Requesting CID...\n");
    check_errors(this->send_command(SD::CMD_RD_CID, 0, SD::CRC_OTHER));
    check_errors(this->read_block(16, response));
    printf("CID Contents:\n");
    this->print_hex_block(response, 16);
    putchar('\n');

    return 0;
}
#endif

PropWare::ErrorCode SD::mount () {
    PropWare::ErrorCode err;

    // FAT system determination variables:
    SD::InitFATInfo fatInfo;
    fatInfo.bootSector = 0;

    check_errors(this->read_boot_sector(&fatInfo));

    check_errors(this->common_boot_sector_parser(&fatInfo));

    this->partition_info_parser(&fatInfo);

    check_errors(this->determine_fat_type(&fatInfo));

    this->store_root_info(&fatInfo);

    check_errors(this->read_fat_and_root_sectors());

    return 0;
}

PropWare::ErrorCode SD::read_boot_sector (InitFATInfo *fatInfo) {
    PropWare::ErrorCode err;
    // Read in first sector
    check_errors(this->read_data_block(fatInfo->bootSector, this->m_buf.buf));
    // Check if sector 0 is boot sector or MBR; if MBR, skip to boot sector at
    // first partition
    if (SD::BOOT_SECTOR_ID != this->m_buf.buf[SD::BOOT_SECTOR_ID_ADDR]) {
        fatInfo->bootSector = this->read_rev_dat32(
                &(this->m_buf.buf[SD::BOOT_SECTOR_BACKUP]));
        check_errors(
                this->read_data_block(fatInfo->bootSector, this->m_buf.buf));
    }

    // Print the boot sector if requested
#if (defined SD_OPTION_VERBOSE && \
	        defined SD_OPTION_VERBOSE_BLOCKS)
    printf("***BOOT SECTOR***\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

PropWare::ErrorCode SD::common_boot_sector_parser (InitFATInfo *fatInfo) {
    uint8_t temp;

    // Determine number of sectors per cluster
    temp = this->m_buf.buf[SD::CLUSTER_SIZE_ADDR];
#ifdef SD_OPTION_VERBOSE
    printf("Preliminary sectors per cluster: %u\n", temp);
#endif
    while (temp) {
        temp >>= 1;
        ++this->m_sectorsPerCluster_shift;
    }
    --this->m_sectorsPerCluster_shift;

    // Get the reserved sector count
    fatInfo->rsvdSectorCount = this->read_rev_dat16(
            &((this->m_buf.buf)[SD::RSVD_SCTR_CNT_ADDR]));

    // Total number of FATs
    fatInfo->numFATs = this->m_buf.buf[SD::NUM_FATS_ADDR];
#ifdef SD_OPTION_FILE_WRITE
    if (2 != fatInfo->numFATs)
        return SD::TOO_MANY_FATS;
#endif

    // Number of entries in the root directory
    fatInfo->rootEntryCount = this->read_rev_dat16(
            &(this->m_buf.buf[SD::ROOT_ENTRY_CNT_ADDR]));

    return 0;
}

void SD::partition_info_parser (InitFATInfo *fatInfo) {
    // Check if FAT size is valid in 16- or 32-bit location
    fatInfo->FATSize = this->read_rev_dat16(
            &(this->m_buf.buf[SD::FAT_SIZE_16_ADDR]));
    if (!(fatInfo->FATSize))
        fatInfo->FATSize = this->read_rev_dat32(
                &(this->m_buf.buf[SD::FAT_SIZE_32_ADDR]));

    // Check if FAT16 total sectors is valid
    fatInfo->totalSectors = this->read_rev_dat16(
            &(this->m_buf.buf[SD::TOT_SCTR_16_ADDR]));
    if (!(fatInfo->totalSectors))
        fatInfo->totalSectors = this->read_rev_dat32(
                &(this->m_buf.buf[SD::TOT_SCTR_32_ADDR]));

    // Compute necessary numbers to determine FAT type (12/16/32)
    this->m_rootDirSectors = (fatInfo->rootEntryCount * 32)
            >> SD::SECTOR_SIZE_SHIFT;
    fatInfo->dataSectors = fatInfo->totalSectors
            - (fatInfo->rsvdSectorCount + fatInfo->numFATs * fatInfo->FATSize
                    + fatInfo->rootEntryCount);
    fatInfo->clusterCount = fatInfo->dataSectors
            >> this->m_sectorsPerCluster_shift;

#ifdef SD_OPTION_VERBOSE
    printf("Sectors per cluster: %u\n", 1 << this->m_sectorsPerCluster_shift);
    printf("Reserved sector count: 0x%08x / %u\n", fatInfo->rsvdSectorCount,
            fatInfo->rsvdSectorCount);
    printf("Number of FATs: 0x%02x / %u\n", fatInfo->numFATs, fatInfo->numFATs);
    printf("Total sector count: 0x%08x / %u\n", fatInfo->totalSectors,
            fatInfo->totalSectors);
    printf("Total cluster count: 0x%08x / %u\n", fatInfo->clusterCount,
            fatInfo->clusterCount);
    printf("Total data sectors: 0x%08x / %u\n", fatInfo->dataSectors,
            fatInfo->dataSectors);
    printf("FAT Size: 0x%04x / %u\n", fatInfo->FATSize, fatInfo->FATSize);
    printf("Root directory sectors: 0x%08x / %u\n", this->m_rootDirSectors,
            this->m_rootDirSectors);
    printf("Root entry count: 0x%08x / %u\n", fatInfo->rootEntryCount,
            fatInfo->rootEntryCount);
#endif
}

PropWare::ErrorCode SD::determine_fat_type (InitFATInfo *fatInfo) {
    // Determine and store FAT type
    if (SD::FAT12_CLSTR_CNT > fatInfo->clusterCount)
        return SD::INVALID_FILESYSTEM;
    else if (SD::FAT16_CLSTR_CNT > fatInfo->clusterCount) {
#ifdef SD_OPTION_VERBOSE
        printf("\n***FAT type is FAT16***\n");
#endif
        this->m_filesystem = SD::FAT_16;
        this->m_entriesPerFatSector_Shift = 8;
    } else {
#ifdef SD_OPTION_VERBOSE
        printf("\n***FAT type is FAT32***\n");
#endif
        this->m_filesystem = SD::FAT_32;
        this->m_entriesPerFatSector_Shift = 7;
    }

    return 0;
}

void SD::store_root_info (InitFATInfo *fatInfo) {
    // Find start of FAT
    this->m_fatStart = fatInfo->bootSector + fatInfo->rsvdSectorCount;

    //    this->m_filesystem = SD::FAT_16;
    // Find root directory address
    switch (this->m_filesystem) {
        case SD::FAT_16:
            this->m_rootAddr = fatInfo->FATSize * fatInfo->numFATs
                    + this->m_fatStart;
            this->m_firstDataAddr = this->m_rootAddr + this->m_rootDirSectors;
            break;
        case SD::FAT_32:
            this->m_firstDataAddr = this->m_rootAddr = fatInfo->bootSector
                    + fatInfo->rsvdSectorCount
                    + fatInfo->FATSize * fatInfo->numFATs;
            this->m_rootAllocUnit = this->read_rev_dat32(
                    &(this->m_buf.buf[SD::ROOT_CLUSTER_ADDR]));
            break;
    }

#ifdef SD_OPTION_FILE_WRITE
    // If files will be writable, the second FAT must also be updated - the
    // first sector's address is stored here
    this->m_fatSize = fatInfo->FATSize;
#endif

#ifdef SD_OPTION_VERBOSE
    printf("Start of FAT: 0x%08x\n", this->m_fatStart);
    printf("Root directory alloc. unit: 0x%08x\n", this->m_rootAllocUnit);
    printf("Root directory sector: 0x%08x\n", this->m_rootAddr);
    printf("Calculated root directory sector: 0x%08x\n",
            this->find_sector_from_alloc(this->m_rootAllocUnit));
    printf("First data sector: 0x%08x\n", this->m_firstDataAddr);
#endif
}

PropWare::ErrorCode SD::read_fat_and_root_sectors () {
    PropWare::ErrorCode err;

    // Store the first sector of the FAT
    check_errors(this->read_data_block(this->m_fatStart, this->m_fat));
    this->m_curFatSector = 0;

    // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && \
            defined SD_OPTION_VERBOSE_BLOCKS)
    printf("\n***First File Allocation Table***\n");
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    // Read in the root directory, set root as current
    check_errors(this->read_data_block(this->m_rootAddr, this->m_buf.buf));
    this->m_buf.curClusterStartAddr = this->m_rootAddr;
    if (SD::FAT_16 == this->m_filesystem) {
        this->m_dir_firstAllocUnit = -1;
        this->m_buf.curAllocUnit = -1;
    } else {
        this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit =
                this->m_rootAllocUnit;
        check_errors(
                this->get_fat_value(this->m_buf.curAllocUnit,
                        &this->m_buf.nextAllocUnit));
    }
    this->m_buf.curClusterStartAddr = this->m_rootAddr;
    this->m_buf.curSectorOffset = 0;

    // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    printf("***Root directory***\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
PropWare::ErrorCode SD::unmount () {
    PropWare::ErrorCode err;

    // If the directory buffer was modified, write it
    if (this->m_buf.mod)
    check_errors(
            this->write_data_block(
                    this->m_buf.curClusterStartAddr
                    + this->m_buf.curSectorOffset,
                    this->m_buf.buf));

    // If the FAT sector was modified, write it
    if (this->m_fatMod) {
        check_errors(
                this->write_data_block(this->m_curFatSector + this->m_fatStart,
                        this->m_fat));
        check_errors(
                this->write_data_block(
                        this->m_curFatSector + this->m_fatStart
                        + this->m_fatSize, this->m_fat));
    }

    return 0;
}
#endif

PropWare::ErrorCode SD::chdir (const char *d) {
    PropWare::ErrorCode err;
    uint16_t fileEntryOffset = 0;

    this->m_buf.id = SD::FOLDER_ID;

    // Attempt to find the file and return an error code if not found
    check_errors(this->find(d, &fileEntryOffset));

    // If the returned entry isn't a file, throw an error
    if (!(SD::SUB_DIR & this->m_buf.buf[fileEntryOffset
                                        + SD::FILE_ATTRIBUTE_OFFSET]))
        return SD::ENTRY_NOT_DIR;

#ifdef SD_OPTION_FILE_WRITE
    // If the previous sector was modified, write it back to the SD card
    // before reading
    if (this->m_buf.mod)
    this->write_data_block(
            this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset,
            this->m_buf.buf);
    this->m_buf.mod = false;
#endif

#ifdef SD_OPTION_VERBOSE
    printf("%s found at offset 0x%04x from address 0x%08x\n", d,
            fileEntryOffset,
            this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset);
#endif

    // File entry was found successfully, load it into the buffer and update
    // status variables
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

#ifdef SD_OPTION_VERBOSE
    printf("Opening directory from...\n");
    printf("\tAllocation unit 0x%08x\n", this->m_buf.curAllocUnit);
    printf("\tCluster starting address 0x%08x\n",
            this->m_buf.curClusterStartAddr);
    printf("\tSector offset 0x%04x\n", this->m_buf.curSectorOffset);
#ifdef SD_OPTION_VERBOSE_BLOCKS
    printf("And the first directory sector looks like....\n");
    this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif
#endif
    return 0;
}

PropWare::ErrorCode SD::fopen (const char *name, SD::File *f, const SD::FileMode mode) {
    PropWare::ErrorCode err;
    uint16_t fileEntryOffset = 0;

#ifdef SD_OPTION_VERBOSE
    printf("Attempting to open %s\n", name);
#endif

    if (NULL == f->buf)
        return SD::FILE_WITHOUT_BUFFER;

    f->id = this->m_fileID++;
    f->rPtr = 0;
    f->wPtr = 0;
#ifndef SD_OPTION_FILE_WRITE
    if (FILE_MODE_R != mode)
        return SD::INVALID_FILE_MODE;
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
#ifdef SD_OPTION_VERBOSE
            printf("Directory cluster was full, adding another...\n");
#endif
            check_errors(this->extend_fat(&this->m_buf));
            check_errors(this->load_next_sector(&this->m_buf));
        }
        if (SD::EOC_END == err || SD::FILENAME_NOT_FOUND == err) {
            // File wasn't found, but there is still room in this cluster (or a
            // new cluster was just added)
#ifdef SD_OPTION_VERBOSE
            printf("Creating a new directory entry...\n");
#endif
            check_errors(this->create_file(name, &fileEntryOffset));
        } else
#endif
        // SDFind returned unknown error - throw it
        return err;
    }

    // `name` was found successfully, determine if it is a file or directory
    if (SD::SUB_DIR
            & this->m_buf.buf[fileEntryOffset + SD::FILE_ATTRIBUTE_OFFSET])
        return SD::ENTRY_NOT_FILE;

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
    check_errors(
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
    check_errors(
            this->read_data_block(f->buf->curClusterStartAddr, f->buf->buf));

#ifdef SD_OPTION_VERBOSE
    printf("Opening file from...\n");
    printf("\tAllocation unit 0x%08x\n", f->buf->curAllocUnit);
    printf("\tNext allocation unit 0x%08x\n", f->buf->nextAllocUnit);
    printf("\tCluster starting address 0x%08x\n", f->buf->curClusterStartAddr);
    printf("\tSector offset 0x%04x\n", f->buf->curSectorOffset);
    printf("\tFile length 0x%08x\n", f->length);
    printf("\tMax sectors 0x%08x\n", f->maxSectors);
#ifdef SD_OPTION_VERBOSE_BLOCKS
    printf("And the first file sector looks like....\n");
    this->print_hex_block(f->buf->buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif
#endif

    return 0;
}

#ifdef SD_OPTION_FILE_WRITE
PropWare::ErrorCode SD::fclose (SD::File *f) {
    PropWare::ErrorCode err;

#ifdef SD_OPTION_VERBOSE
    printf("Closing file...\n");
#endif
    // If the currently loaded sector has been modified, save the changes
    if ((f->buf->id == f->id) && f->buf->mod) {
        check_errors(
                this->write_data_block(
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->buf));;
        f->buf->mod = false;
#ifdef SD_OPTION_VERBOSE
        printf("Modified sector in file has been saved...\n");
        printf("\tDestination address: 0x%08x / %u\n",
                f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                f->buf->curClusterStartAddr + f->buf->curSectorOffset);
        printf("\tFile first sector address: 0x%08x / %u\n",
                this->find_sector_from_alloc(f->firstAllocUnit),
                this->find_sector_from_alloc(f->firstAllocUnit));
#endif
    }

    // If we modified the length of the file...
#ifdef SD_OPTION_VERBOSE
    printf("Closing file and \"f->mod\" value is %u\n", f->mod);
    printf("File length is: 0x%08x / %u\n", f->length, f->length);
#endif
    if (f->mod) {
#ifdef SD_OPTION_VERBOSE
        printf("File length has been modified - write it to the directory\n");
#endif
        // Then check if the directory sector is still loaded...
        if ((this->m_buf.curClusterStartAddr + this->m_buf.curSectorOffset)
                != f->dirSectorAddr) {
            // If it isn't, load it...
            if (this->m_buf.mod)
            // And if it's been modified since the last read, save it...
            check_errors(
                    this->write_data_block(
                            this->m_buf.curClusterStartAddr
                            + this->m_buf.curSectorOffset,
                            this->m_buf.buf));
            check_errors(
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

PropWare::ErrorCode SD::fputc (const char c, SD::File *f) {
    PropWare::ErrorCode err;
    // Determines byte-offset within a sector
    uint16_t sectorPtr = f->wPtr % SD::SECTOR_SIZE;
    // Determine the needed file sector
    uint32_t sectorOffset = (f->wPtr >> SD::SECTOR_SIZE_SHIFT);

    // Determine if the correct sector is loaded
    if (f->buf->id != f->id)
    check_errors(this->reload_buf(f));

    // Even the the buffer was just reloaded, this snippet needs to be called in
    // order to extend the FAT if needed
    if (sectorOffset != f->curSector) {
        // Incorrect sector loaded
#ifdef SD_OPTION_VERBOSE
        printf("Need new sector:\n");
        printf("\tMax available sectors: 0x%08x / %u\n", f->maxSectors,
                f->maxSectors);
        printf("\tDesired file sector: 0x%08x / %u\n", sectorOffset,
                sectorOffset);
#endif

        // If the sector needed exceeds the available sectors, extend the file
        if (f->maxSectors == sectorOffset) {
            check_errors(this->extend_fat(f->buf));
            f->maxSectors += 1 << this->m_sectorsPerCluster_shift;
        }

#ifdef SD_OPTION_VERBOSE
        printf("Loading new file sector at file-offset: 0x%08x / %u\n",
                sectorOffset, sectorOffset);
#endif
        // SDLoadSectorFromOffset() will ensure that, if the current buffer has
        // been modified, it is written back to the SD card before loading a new
        // one
        check_errors(this->load_sector_from_offset(f, sectorOffset));
    }

    if (++(f->wPtr) > f->length) {
        ++(f->length);
        f->mod = true;
    }
    f->buf->buf[sectorPtr] = c;
    f->buf->mod = true;

    return 0;
}

PropWare::ErrorCode SD::fputs (char *s, SD::File *f) {
    PropWare::ErrorCode err;

    while (*s)
    check_errors(fputc(*(s++), f));

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
#ifdef SD_OPTION_VERBOSE
        printf("File sector offset: 0x%08x / %u\n", sectorOffset, sectorOffset);
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

inline bool SD::feof (SD::File *f) {
    return f->length == f->rPtr;
}

PropWare::ErrorCode SD::fseekr (SD::File *f, const int32_t offset,
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

PropWare::ErrorCode SD::fseekw (SD::File *f, const int32_t offset,
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

int32_t SD::ftellr (const SD::File *f) {
    return f->rPtr;
}

int32_t SD::ftellw (const SD::File *f) {
    return f->wPtr;
}

#ifdef SD_OPTION_SHELL
PropWare::ErrorCode SD::shell (SD::File *f) {
    char usrInput[SD_SHELL_INPUT_LEN] = "";
    char cmd[SD_SHELL_CMD_LEN] = "";
    char arg[SD_SHELL_ARG_LEN] = "";
    char uppercaseName[SD_SHELL_ARG_LEN] = "";
    uint8_t i, j;
    PropWare::ErrorCode err;

    printf("Welcome to David's quick shell! "
            "There is no help, nor much to do.\n");
    printf("Have fun...\n");

    // Loop until the user types the SD::SHELL_EXIT string
    while (strcmp(usrInput, this->SHELL_EXIT)) {
        printf(">>> ");
        gets(usrInput);

#ifdef SD_OPTION_VERBOSE
        printf("Received \"%s\" as the complete line\n", usrInput);
#endif

        // Retrieve command
        for (i = 0; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
            cmd[i] = usrInput[i];

#ifdef SD_OPTION_VERBOSE
        printf("Received \"%s\" as command\n", cmd);
#endif

        // Retrieve argument if it exists (skip over spaces)
        if (0 != usrInput[i]) {
            j = 0;
            while (' ' == usrInput[i])
                ++i;
            for (; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
                arg[j++] = usrInput[i];
#ifdef SD_OPTION_VERBOSE
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

        // Handle errors; Print user errors and continue; Return system errors
        if (0 != err) {
            if (SD::BEG_ERROR <= err && err <= SD::END_USER_ERRORS) {
                SD::print_error_str((SD::ErrorCode) err);
            } else
                return err;
        }

        // Erase the command and argument strings
        for (i = 0; i < SD::SHELL_CMD_LEN; ++i)
            cmd[i] = 0;
        for (i = 0; i < SD::SHELL_ARG_LEN; ++i)
            uppercaseName[i] = arg[i] = 0;
        err = 0;
    }

    return 0;
}

PropWare::ErrorCode SD::shell_ls () {
    PropWare::ErrorCode err;
    uint16_t fileEntryOffset = 0;
    char string[SD_FILENAME_STR_LEN];  // Allocate space for a filename string

    // If we aren't looking at the beginning of a cluster, we must backtrack to
    // the beginning and then begin listing files
    if (this->m_buf.curSectorOffset
            || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                    != this->m_buf.curClusterStartAddr)) {
#ifdef SD_OPTION_VERBOSE
        printf("'ls' requires a backtrack to beginning of directory's "
                "cluster\n");
#endif
        this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                this->m_dir_firstAllocUnit);
        this->m_buf.curSectorOffset = 0;
        this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
        check_errors(
                this->get_fat_value(this->m_buf.curAllocUnit,
                        &(this->m_buf.nextAllocUnit)));
        check_errors(
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
                    return err;
            }

            fileEntryOffset = 0;
        }
    }

    return 0;
}

PropWare::ErrorCode SD::shell_cat (const char *name, SD::File *f) {
    PropWare::ErrorCode err;

    // Attempt to find the file
    if ((err = this->fopen(name, f, SD::FILE_MODE_R))) {
        if ((uint8_t) SD::EOC_END == err)
            return err;
        else
            return err;
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
PropWare::ErrorCode SD::shell_touch (const char name[]) {
    PropWare::ErrorCode err;
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
void SD::print_hex_block (uint8_t *dat, uint16_t bytes) {
    uint8_t i, j;
    uint8_t *s;

    printf("Printing %u bytes...\n", bytes);
    printf("Offset\t");
    for (i = 0; i < SD::LINE_SIZE; ++i)
    printf("0x%x  ", i);
    putchar('\n');

    if (bytes % SD::LINE_SIZE)
    bytes = bytes / SD::LINE_SIZE + 1;
    else
    bytes /= SD::LINE_SIZE;

    for (i = 0; i < bytes; ++i) {
        s = (uint8_t *) (dat + SD::LINE_SIZE * i);
        printf("0x%04x:\t", SD::LINE_SIZE * i);
        for (j = 0; j < SD::LINE_SIZE; ++j)
        printf("0x%02x ", s[j]);
        printf(" - ");
        for (j = 0; j < SD::LINE_SIZE; ++j) {
            if ((' ' <= s[j]) && (s[j] <= '~'))
            putchar(s[j]);
            else
            putchar('.');
        }

        putchar('\n');
    }
}
#endif

/**********************************
 *** Private Method Definitions ***
 **********************************/
PropWare::ErrorCode SD::send_command (const uint8_t cmd, const uint32_t arg,
        const uint8_t crc) {
    PropWare::ErrorCode err;

    // Send out the command
    check_errors(this->m_spi->shift_out(8, cmd));

    // Send argument
    check_errors(this->m_spi->shift_out(16, (arg >> 16)));
    check_errors(this->m_spi->shift_out(16, arg & WORD_0));

    // Send sixth byte - CRC
    check_errors(this->m_spi->shift_out(8, crc));

    return 0;
}

PropWare::ErrorCode SD::get_response (uint8_t bytes, uint8_t *dat) {
    PropWare::ErrorCode err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                this->m_spi->shift_in(8, &this->m_firstByteResponse,
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
            check_errors(this->m_spi->shift_in(8, dat++, sizeof(*dat)));
    } else
        return SD::INVALID_RESPONSE;

    // Responses should always be followed up by outputting 8 clocks with MOSI
    // high
    check_errors(this->m_spi->shift_out(16, -1));
    check_errors(this->m_spi->shift_out(16, -1));
    check_errors(this->m_spi->shift_out(16, -1));
    check_errors(this->m_spi->shift_out(16, -1));

    return 0;
}

PropWare::ErrorCode SD::read_block (uint16_t bytes, uint8_t *dat) {
    uint8_t i, err, checksum;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                this->m_spi->shift_in(8, &this->m_firstByteResponse,
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
            check_errors(this->m_spi->shift_in(8, dat, sizeof(*dat)));

            // Check for timeout
            if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
                return SD::READ_TIMEOUT;
        } while (SD::DATA_START_ID != *dat);  // wait for transmission end

        // Check for the data start identifier and continue reading data
        if (SD::DATA_START_ID == *dat) {
            // Read in requested data bytes
#if (defined SPI_FAST_SECTOR)
            if (SD::SECTOR_SIZE == bytes) {
                this->m_spi->shift_in_sector(dat, 1);
                bytes = 0;
            }
#endif
            while (bytes--) {
#ifdef SPI_FAST
                check_errors(this->m_spi->shift_in_fast(8, dat++, sizeof(*dat)));
#else
                check_errors(this->m_spi->shift_in(8, dat++, sizeof(*dat)));
#endif
            }

            // Read two more bytes for checksum - throw away data
            for (i = 0; i < 2; ++i) {
                timeout = SD::RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(
                            this->m_spi->shift_in(8, &checksum,
                                    sizeof(checksum)));

                    // Check for timeout
                    if ((timeout - CNT) < SD::WIGGLE_ROOM)
                        return SD::READ_TIMEOUT;
                } while (0xff == checksum);  // wait for transmission end
            }

            // Send final 0xff
            check_errors(this->m_spi->shift_out(8, 0xff));
        } else {
            return SD::INVALID_DAT_STRT_ID;
        }
    } else
        return SD::INVALID_RESPONSE;

    return 0;
}

PropWare::ErrorCode SD::write_block (uint16_t bytes, uint8_t *dat) {
    PropWare::ErrorCode err;
    uint32_t timeout;

    // Read first byte - the R1 response
    timeout = SD::RESPONSE_TIMEOUT + CNT;
    do {
        check_errors(
                this->m_spi->shift_in(8, &this->m_firstByteResponse,
                        sizeof(this->m_firstByteResponse)));

        // Check for timeout
        if (abs(timeout - CNT) < SD::WIGGLE_ROOM)
            return SD::READ_TIMEOUT;
    } while (0xff == this->m_firstByteResponse);  // wait for transmission end

// Ensure this response is "active"
    if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse) {
        // Received "active" response

        // Send data Start ID
        check_errors(this->m_spi->shift_out(8, SD::DATA_START_ID));

        // Send all bytes
        while (bytes--) {
#ifdef SPI_FAST
            this->m_spi->shift_out_fast(8, *(dat++));
#else
            check_errors(this->m_spi->shift_out(8, *(dat++)));
#endif
        }

        // Receive and digest response token
        timeout = SD::RESPONSE_TIMEOUT + CNT;
        do {
            check_errors(
                    this->m_spi->shift_in(8, &this->m_firstByteResponse,
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

PropWare::ErrorCode SD::read_data_block (uint32_t address, uint8_t *dat) {
    /**
     * Special error handling is needed to ensure that, if an error is thrown,
     * chip select is set high again before returning the error
     */
    PropWare::ErrorCode err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        this->m_spi->shift_in(8, &temp, 1);

#ifdef SD_OPTION_VERBOSE
    printf("Reading block at sector address: 0x%08x / %u\n", address, address);
#endif

    GPIO::pin_clear(this->m_cs);

    err = this->send_command(SD::CMD_RD_BLOCK, address, SD::CRC_OTHER);
    if (!err)
        err = this->read_block(SD::SECTOR_SIZE, dat);

    GPIO::pin_set(this->m_cs);

    if (err)
        return err;

    return 0;
}

PropWare::ErrorCode SD::write_data_block (uint32_t address, uint8_t *dat) {
    PropWare::ErrorCode err;
    uint8_t temp = 0;

    // Wait until the SD card is no longer busy
    while (!temp)
        this->m_spi->shift_in(8, &temp, 1);

#ifdef SD_OPTION_VERBOSE
    printf("Writing block at address: 0x%08x / %u\n", address, address);
#endif

    GPIO::pin_clear(this->m_cs);
    check_errors(this->send_command(SD::CMD_WR_BLOCK, address, SD::CRC_OTHER));

    check_errors(this->write_block(SD::SECTOR_SIZE, dat));
    GPIO::pin_set(this->m_cs);

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

PropWare::ErrorCode SD::get_fat_value (const uint32_t fatEntry, uint32_t *value) {
    PropWare::ErrorCode err;
    uint32_t firstAvailableAllocUnit;

#ifdef SD_OPTION_VERBOSE
    printf("Reading from the FAT...\n");
    printf("\tLooking for entry: 0x%08x / %u\n", fatEntry, fatEntry);
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
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
        this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
    }
    firstAvailableAllocUnit = this->m_curFatSector
            << this->m_entriesPerFatSector_Shift;

#ifdef SD_OPTION_VERBOSE
    printf("\tLooks like I need FAT sector: 0x%08x / %u\n",
            this->m_curFatSector, this->m_curFatSector);
    printf("\tWith an offset of: 0x%04x / %u\n",
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
#ifdef SD_OPTION_VERBOSE
    printf("\tReceived value: 0x%08x / %u\n", *value, *value);
#endif

    return 0;
}

PropWare::ErrorCode SD::load_next_sector (SD::Buffer *buf) {
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

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    printf("New sector loaded:\n");
    this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
    putchar('\n');
#endif

    return 0;
}

PropWare::ErrorCode SD::load_sector_from_offset (SD::File *f, const uint32_t offset) {
    PropWare::ErrorCode err;
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
#ifdef SD_OPTION_VERBOSE
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
#ifdef SD_OPTION_VERBOSE
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

PropWare::ErrorCode SD::inc_cluster (SD::Buffer *buf) {
    PropWare::ErrorCode err;

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

#ifdef SD_OPTION_VERBOSE
    printf("Incrementing the cluster. New parameters are:\n");
    printf("\tCurrent allocation unit: 0x%08x / %u\n", buf->curAllocUnit,
            buf->curAllocUnit);
    printf("\tNext allocation unit: 0x%08x / %u\n", buf->nextAllocUnit,
            buf->nextAllocUnit);
    printf("\tCurrent cluster starting address: 0x%08x / %u\n",
            buf->curClusterStartAddr, buf->curClusterStartAddr);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    check_errors(this->read_data_block(buf->curClusterStartAddr, buf->buf));
    this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
    return 0;
#else
    return this->read_data_block(buf->curClusterStartAddr, buf->buf);
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

PropWare::ErrorCode SD::find (const char *filename, uint16_t *fileEntryOffset) {
    PropWare::ErrorCode err;
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
#ifdef SD_OPTION_VERBOSE
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

PropWare::ErrorCode SD::reload_buf (SD::File *f) {
    PropWare::ErrorCode err;

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

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    printf("\n*** SDFindEmptySpace() initialized with FAT sector 0x%08x / %u "
            "loaded ***\n", this->m_curFatSector, this->m_curFatSector);
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

    // Find the first empty allocation unit and write the EOC marker
    if (SD::FAT_16 == this->m_filesystem) {
        // Loop until we find an empty cluster
        while (this->read_rev_dat16(&(this->m_fat[allocOffset]))) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
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
#ifdef SD_OPTION_VERBOSE
                    printf("FAT sector has been modified; saving now... ");
#endif
                    this->write_data_block(this->m_curFatSector, this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatSize,
                            this->m_fat);
#ifdef SD_OPTION_VERBOSE
                    printf("done!\n");
#endif
                    this->m_fatMod = false;
                }
                // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08x / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
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
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("Searching the following sector...\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
            // Stop when we either reach the end of the current block or find an
            // empty cluster
            while ((this->read_rev_dat32(&(this->m_fat[allocOffset]))
                            & 0x0fffffff) && (SD::SECTOR_SIZE > allocOffset))
            allocOffset += SD::FAT_32;

#ifdef SD_OPTION_VERBOSE
            printf("Broke while loop... why? Offset = 0x%04x / %u\n",
                    allocOffset, allocOffset);
#endif
            // If we reached the end of a sector...
            if (SD::SECTOR_SIZE <= allocOffset) {
                if (this->m_fatMod) {
#ifdef SD_OPTION_VERBOSE
                    printf("FAT sector has been modified; saving now... ");
#endif
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart,
                            this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart
                            + this->m_fatSize, this->m_fat);
#ifdef SD_OPTION_VERBOSE
                    printf("done!\n");
#endif
                    this->m_fatMod = false;
                }
                // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                printf("SDFindEmptySpace() is reading in sector address: "
                        "0x%08x / %u\n", fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                this->read_data_block(++fatSectorAddr, this->m_fat);
                allocOffset = 0;
            }
        }
        this->write_rev_dat32(&(this->m_fat[allocOffset]),
                ((uint32_t) SD::EOC_END) & 0x0fffffff);
        this->m_fatMod = true;
    }

#ifdef SD_OPTION_VERBOSE
    printf("Available space found: 0x%08x / %u\n",
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

PropWare::ErrorCode SD::extend_fat (SD::Buffer *buf) {
    PropWare::ErrorCode err;
    uint32_t newAllocUnit;
#ifdef SD_OPTION_VERBOSE
    printf("Extending file or directory now...\n");
#endif

    // Do we need to load a different sector of the FAT or is the correct one
    // currently loaded? (Correct means the sector currently containing the EOC
    // marker)
    if ((buf->curAllocUnit >> this->m_entriesPerFatSector_Shift)
            != this->m_curFatSector) {

#ifdef SD_OPTION_VERBOSE
        printf("Need new FAT sector. Loading: 0x%08x / %u\n",
                buf->curAllocUnit >> this->m_entriesPerFatSector_Shift,
                buf->curAllocUnit >> this->m_entriesPerFatSector_Shift);
        printf("... because the current allocation unit is: 0x%08x / %u\n",
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

    // This function should only be called when a file or directory has reached
    // the end of its cluster chain
    uint16_t entriesPerFatSector = 1 << this->m_entriesPerFatSector_Shift;
    uint16_t allocUnitOffset = buf->curAllocUnit % entriesPerFatSector;
    uint16_t fatPointerAddress = allocUnitOffset * this->m_filesystem;
    uint32_t nxtSctr = this->read_rev_dat32(&(this->m_fat[fatPointerAddress]));
    if ((uint32_t) SD::EOC_BEG <= nxtSctr)
        return SD::INVALID_FAT_APPEND;

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
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

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    printf("After modification, the FAT now looks like...\n");
    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

    return 0;
}

PropWare::ErrorCode SD::create_file (const char *name, const uint16_t *fileEntryOffset) {
    uint8_t i, j;
    // *name is only checked for uppercase
    char uppercaseName[SD::FILENAME_STR_LEN];
    uint32_t allocUnit;

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
            ++i;// Skip the period
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
    this->m_buf.buf[*fileEntryOffset + SD::FILE_ATTRIBUTE_OFFSET] = SD::ARCHIVE;// Archive flag should be set because the file is new
    this->m_buf.mod = true;

#ifdef SD_OPTION_VERBOSE
    SD::print_file_entry(&(this->m_buf.buf[*fileEntryOffset]), uppercaseName);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
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

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
    printf("New file entry at offset 0x%08x / %u looks like...\n",
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


void SD::print_error_str (const SD::ErrorCode err) const {
    const char str[] = "SD Error %u: %s\n";
    const uint8_t relativeError = err - SD::BEG_ERROR;

    switch (err) {
        case SD::INVALID_CMD:
            printf(str, relativeError, "Invalid command");
            break;
        case SD::READ_TIMEOUT:
            printf(str, relativeError, "Timed out "
                    "during read");
            break;
        case SD::INVALID_NUM_BYTES:
            printf(str, relativeError,
                    "Invalid number of bytes");
            break;
        case SD::INVALID_RESPONSE:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02x\nThe following bits are "
                    "set:\n", relativeError,
                    "Invalid first-byte response\n\tReceived: ",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", relativeError,
                    "Invalid first-byte response\n\tReceived: ",
                    this->m_firstByteResponse);
#endif
            this->first_byte_expansion();
            break;
        case SD::INVALID_INIT:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s\n\tResponse: 0x%02x\n",
                    relativeError,
                    "Invalid response during initialization",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s\n\tResponse: %u\n",
                    relativeError,
                    "Invalid response during initialization",
                    this->m_firstByteResponse);
#endif
            break;
        case SD::INVALID_FILESYSTEM:
            printf(str, relativeError,
                    "Invalid file system; Likely not"
                            " a FAT16 or FAT32 system");
            break;
        case SD::INVALID_DAT_STRT_ID:
#ifdef SD_OPTION_VERBOSE
            printf("SD Error %u: %s0x%02x\n", relativeError,
                    "Invalid data-start ID\n\tReceived: ",
                    this->m_firstByteResponse);
#else
            printf("SD Error %u: %s%u\n", relativeError,
                    "Invalid data-start ID\n\tReceived: ",
                    this->m_firstByteResponse);
#endif
            break;
        case SD::FILENAME_NOT_FOUND:
            printf(str, relativeError,
                    "Filename not found");
            break;
        case SD::EMPTY_FAT_ENTRY:
            printf(str, relativeError,
                    "FAT points to empty entry");
            break;
        case SD::CORRUPT_CLUSTER:
            printf(str, relativeError,
                    "SD cluster is corrupt");
            break;
        case SD::INVALID_PTR_ORIGIN:
            printf(str, relativeError,
                    "Invalid pointer origin");
            break;
        case SD::ENTRY_NOT_FILE:
            printf(str, relativeError,
                    "Requested file entry is not a file");
            break;
        case SD::INVALID_FILENAME:
            printf(str, relativeError,
                    "Invalid filename - please use 8.3 format");
            break;
        case SD::INVALID_FAT_APPEND:
            printf(str, relativeError,
                    "FAT entry append was attempted unnecessarily");
            break;
        case SD::FILE_ALREADY_EXISTS:
            printf(str, relativeError,
                    "Attempting to create an already existing file");
            break;
        case SD::INVALID_FILE_MODE:
            printf(str, relativeError,
                    "Invalid file mode");
            break;
        case SD::TOO_MANY_FATS:
            printf(str, relativeError,
                    "This driver is only capable of writing files on FAT "
                            "partitions with two (2) copies of the FAT");
            break;
        case SD::FILE_WITHOUT_BUFFER:
            printf(str, relativeError,
                    "SDfopen() was passed a file struct with "
                            "an uninitialized buffer");
            break;
        case SD::CMD8_FAILURE:
            printf(str, relativeError,
                    "CMD8 never received a proper response; This is most "
                            "likely to occur when the SD card does not "
                            "support the 3.3V I/O used by the Propeller");
            break;
        case SD::READING_PAST_EOC:
            printf(str, relativeError, "Reading past the"
                    " end-of-chain marker");
            break;
        case SD::ENTRY_NOT_DIR:
            printf(str, relativeError, "Requested name is not a"
                    " directory");
            break;
    }
}

void SD::first_byte_expansion () const {
    if (BIT_0 & this->m_firstByteResponse)
        printf("\t0: Idle\n");
    if (BIT_1 & this->m_firstByteResponse)
        printf("\t1: Erase reset\n");
    if (BIT_2 & this->m_firstByteResponse)
        printf("\t2: Illegal command\n");
    if (BIT_3 & this->m_firstByteResponse)
        printf("\t3: Communication CRC error\n");
    if (BIT_4 & this->m_firstByteResponse)
        printf("\t4: Erase sequence error\n");
    if (BIT_5 & this->m_firstByteResponse)
        printf("\t5: Address error\n");
    if (BIT_6 & this->m_firstByteResponse)
        printf("\t6: Parameter error\n");
    if (BIT_7 & this->m_firstByteResponse)
        printf("\t7: Something is really screwed up. This should always be 0."
                "\n");
}

}
