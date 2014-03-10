#ifndef LIBPROPELLER_SD_H_
#define LIBPROPELLER_SD_H_

#include <propeller.h>
#include <stdarg.h>
#include "libpropeller/sd/sdsafespi.h"

#include "libpropeller/printstream/printstream.h"

#define RET_IF_ERROR_NULL if(HasError()){return NULL;}
#define RET_IF_ERROR if(HasError()){return;}
#define THROW_NULL(value) {SetErrorCode((value)); return NULL;}
//#define THROW_FALSE(value) {SetErrorCode((value)); return false;}
#define THROW(value) {SetErrorCode((value)); return;}

/** FAT16/32 SD card interface.
 * 
 * This class is based on the Spin version of FSRW 2.6 by Rokicki and Lonesock. Thanks!
 * 
 * This object provides FAT16/32 file read/write access on a block device. Only 
 * one file can be open at a time. Open modes are 'r' (read), 'a' (append), 
 * 'w' (write), and 'd' (delete). Only the root directory is supported. No long 
 * filenames are supported. This object also supports traversing the root 
 * directory.
 * 
 * This object requires pullup resistors on the four SD card I/O lines. The 
 * pullup resistors should be approximately 10kOhms each.
 * 
 * The SPI DO, DI, and CLK lines can be shared with other SPI devices, as long 
 * as the @a Release() command is called after doing SD card activities and 
 * before doing other SPI activities. Note: this has not been tested at all.
 * 
 * Cluster size: If it's not 32768, it should be (32K clusters). To format a 
 * drive with FAT32 with 32K clusters, use the following command under linux:
 *     sudo fdisk -l
 *     sudo mkdosfs /dev/sdX -s 64 -F 32 -I
 * 
 * To check the filesystem under linux:
 *      sudo dosfsck -v /dev/sdX
 * 
 * Note: the mkdosfs command will format the entire disk, and erase all 
 * information on it.
 * 
 * If an exceptional error occurs then the @a HasError() function will return 
 * true. To see what went wrong, query the @a GetError() function. After you fix
 * the error, clear it with @a ClearError(); Every function may set the error,
 * although you're generally OK with just checking after @a Mount() and 
 * @a Open()
 * 
 * 
 * @warning Untested with multiple instances!!!
 * 
 * @warning The various @a Get() and @a Put() methods don't check to make sure
 * that a file is open. It's up to your code to make sure that a file is 
 * successfully opened before you need to use it.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 * 
 * Possible improvements:
 * Write a function that gets a string:
 *       int   Get(char * Ubuf, char EndOfStringChar); 
 */
class SD : public OutputStream<SD> {
public:
    static const int kNoError = SDSafeSPI::kNoError;

    // Mount Errors
    static const int kErrorNotFatVolume = -20;
    static const int kErrorBadBytesPerSector = -21;
    static const int kErrorBadSectorsPerCluster = -22;
    static const int kErrorNotTwoFats = -23;
    static const int kErrorBadFatSignature = -24;
    static const int kErrorBufNotLongwordAligned = -512;

    //Open Errors
    static const int kErrorFileNotFound = -1; //TODO(SRLM): For some reason, if I change the value of this error code then things slow way down. Why???
    static const int kErrorNoEmptyDirectoryEntry = -2;
    static const int kErrorBadArgument = -3;
    static const int kErrorNoWritePermission = -6;
    static const int kErrorEofWhileFollowingChain = -7;
    static const int kErrorBadClusterValue = -9;
    static const int kErrorBadClusterNumber = -26;
    static const int kErrorFileNotOpenForWriting = -27;

    // SdSafeSPI Errors
    static const int kErrorCardNotReset = SDSafeSPI::kErrorCardNotReset;
    static const int kError3v3NotSupported = SDSafeSPI::kError3v3NotSupported;
    static const int kErrorOcrFailed = SDSafeSPI::kErrorOcrFailed;
    static const int kErrorBlockNotLongAligned = SDSafeSPI::kErrorBlockNotLongAligned;
    // These errors are negated since they are thrown as negative in ASM section.
    static const int kErrorAsmNoReadToken = -SDSafeSPI::kErrorAsmNoReadToken;
    static const int kErrorAsmBlockNotWritten = -SDSafeSPI::kErrorAsmBlockNotWritten;
    // NOTE: errors -128 to -255 are reserved for reporting R1 response errors (SRLM ???)
    static const int kErrorSpiEngineNotRunning = SDSafeSPI::kErrorSpiEngineNotRunning;
    static const int kErrorCardBusyTimeout = SDSafeSPI::kErrorCardBusyTimeout;

    /** Stops the SPI driver cog.     
     */
    ~SD() {
        Unmount();
    }

    /** Mounts a volume. Closes any open files (if this is a remount). Requires 
     * a cog for the SD SPI driver.
     * 
     * @param basepin pins must be in the following order, from basepin up:
     * -# Basepin +0: DO
     * -# Basepin +1: CLK
     * -# Basepin +2: DI
     * -# Basepin +3: CS
     */
    void Mount(const int basepin) {
        Mount(basepin, (basepin + 1), (basepin + 2), (basepin + 3));
    }

    /** Mount a volume with explicit pin numbers. Does not require adjacent pins.
     * 
     * @param pin_do  The SPI Data Out pin (ouput relative to Propeller).
     * @param pin_clk The SPI Clock pin.
     * @param pin_di  The SPI Data In pin (input relative to Propeller).
     * @param pin_cs  The Chip Select pin.
     */
    void Mount(const int pin_do, const int pin_clk, const int pin_di, const int pin_cs) {
        if (file_date_time_ == 0) {
            SetDate(2010, 1, 1, 0, 0, 0);
        }

        //SRLM Addition: check to make sure that Buf and Buf2 are longword aligned.
        //Theoretically, this should have no runtime cost, but it looks like in CMM
        //and -Os it takes 16 bytes. It can be commented out if you're sure that
        //Buf and Buf2 are longword aligned.
        if ((((int) buffer_1_) & 0b11) != 0)
            THROW(kErrorBufNotLongwordAligned);
        if ((((int) buffer_2_) & 0b11) != 0)
            THROW(kErrorBufNotLongwordAligned);

        Unmount();
        RET_IF_ERROR;

        sd_spi_.Start(pin_do, pin_clk, pin_di, pin_cs);
        RET_IF_ERROR;

        last_read_ = (-1);
        dirty_ = 0;
        sd_spi_.ReadBlock(0, (char *) (&buffer_1_));
        RET_IF_ERROR;

        int start;
        if (GetFilesystemType() != kFileSystemUnknown) {
            start = 0;
        } else {
            start = ReverseBytesInLong(((char *) (&buffer_1_) + 0x1C6));
            sd_spi_.ReadBlock(start, buffer_1_);
            RET_IF_ERROR;
        }
        filesystem_ = GetFilesystemType();
        if (filesystem_ == kFileSystemUnknown) {
            THROW(kErrorNotFatVolume);
        }
        if (ReverseBytesInWord(((char *) (&buffer_1_) + 11)) != kSectorSize) {
            THROW(kErrorBadBytesPerSector);
        }
        int sectors_per_cluster = buffer_1_[13];
        if (sectors_per_cluster & (sectors_per_cluster - 1)) {
            THROW(kErrorBadSectorsPerCluster);
        }
        cluster_shift_ = 0;
        while (sectors_per_cluster > 1) {
            (cluster_shift_++);
            sectors_per_cluster = (Shr__(sectors_per_cluster, 1));
        }
        sectors_per_cluster = (1 << cluster_shift_);
        cluster_size_ = (kSectorSize << cluster_shift_);
        int reserved = ReverseBytesInWord(((char *) (&buffer_1_) + 14));
        if (buffer_1_[16] != 2) {
            THROW(kErrorNotTwoFats);
        }
        int sectors = ReverseBytesInWord(((char *) (&buffer_1_) + 19));
        if (sectors == 0) {
            sectors = ReverseBytesInLong(((char *) (&buffer_1_) + 32));
        }
        fat1_ = (start + reserved);
        if (filesystem_ == kFileSystemFAT32) {
            int root_entries = (16 << cluster_shift_);
            sectors_per_fat_ = ReverseBytesInLong(((char *) (&buffer_1_) + 36));
            data_region_ = ((fat1_ + (2 * sectors_per_fat_)) - (2 * sectors_per_cluster));
            root_directory_ = ((data_region_ + (ReverseBytesInWord(((char *) (&buffer_1_) + 44)) << cluster_shift_)) << kSectorShift);
            root_directory_end_ = (root_directory_ + (root_entries << kDirectoryShift));
            end_of_chain_ = 268435440;
        } else {
            int root_entries = ReverseBytesInWord(((char *) (&buffer_1_) + 17));
            sectors_per_fat_ = ReverseBytesInWord(((char *) (&buffer_1_) + 22));
            root_directory_ = ((fat1_ + (2 * sectors_per_fat_)) << kSectorShift);
            root_directory_end_ = (root_directory_ + (root_entries << kDirectoryShift));
            data_region_ = ((1 + (Shr__((root_directory_end_ - 1), kSectorShift))) - (2 * sectors_per_cluster));
            end_of_chain_ = 65520;
        }
        if (ReverseBytesInWord(((char *) (&buffer_1_) + 510)) != 43605) {
            THROW(kErrorBadFatSignature);
        }
        total_clusters_ = (Shr__(((sectors - data_region_) + start), cluster_shift_));
    }

    /** Closes any open files, and unmounts the SD card. Frees a cog.
     */
    void Unmount(void) {
        Close();
        sd_spi_.Stop();
    }

    /** Close any currently open file, and open a new one with the given file 
     * name and mode.
     * 
     * The filename should be in 8.3 format (up to eight characters, a period,
     * and up to three characters.  The filename will be converted to uppercase. 
     * Valid characters include A through Z, digits 0 through 9, space, and '$', 
     * '%', '-', '_', '@', '~', '`', '!', '(', ')', '{', '}', '^', '#', '&' and 
     * a single '.'. Filenames can be shorter than 8.3. The behavior for invalid 
     * filenames is undefined.
     * 
     * Modes:
     * - 'd' Delete a file.  Deleting a file will not result in kErrorFileNotFound, even if nothing was deleted.
     * - 'a' Append to a file. If the file exists then calls to @a Put() will add the bytes to the end of the file. Otherwise, the file is created.
     * - 'w' Write to a file. If the file exists, it will be replaced.
     * - 'r' Read from a file. If the file does not exist, then an error is set.
     * 
     * @param filename Filename in 8.3 format.
     * @param file_mode one of the modes described above
     */
    void Open(const char * filename, const char file_mode) {
        Close();
        RET_IF_ERROR;


        char cleaned_filename[11];

        int I = 0;
        while (I < 8 && filename[0] != '\0' && filename[0] != '.') {
            cleaned_filename[(I++)] = ConvertToUppercase((filename++)[0]);
        }
        while (I < 8) {
            cleaned_filename[(I++)] = ' ';
        }
        while (filename[0] != '\0' && filename[0] != '.') {
            (filename++);
        }
        if ((filename)[0] == '.') {
            (filename++);
        }
        while ((I < 11) && ((filename)[0])) {
            cleaned_filename[(I++)] = ConvertToUppercase((filename++)[0]);
        }
        while (I < 11) {
            cleaned_filename[(I++)] = ' ';
        }
        int sentinel = 0;
        int free_entry = 0;



        /* WARNING: Bug may be present if this loop should be working from high to low! The original
         Spin code had the following:
         
              repeat dirptr from rootdir to rootdirend - DIRSIZE step DIRSIZE
         
         Which may go either up or down, depending on the arguments.
         */
        for (int directory_pointer = root_directory_;
                directory_pointer <= root_directory_end_ - kDirectorySize;
                directory_pointer += kDirectorySize) {
            char * disk_filename = ReadByte(directory_pointer);
            RET_IF_ERROR;

            // 0xe5 is the indicator that the entry is deleted. Ox00 is the indicator for end of directory.
            if ((free_entry == 0) && (((disk_filename)[0] == 0) || ((disk_filename)[0] == 0xe5))) {
                free_entry = directory_pointer;
            }
            if ((disk_filename)[0] == 0) {
                sentinel = directory_pointer;
                break;
            }

            //Match the filename
            int i = 0;
            for (; i <= 10; i++) {
                if (cleaned_filename[i] != (disk_filename)[i]) {
                    break;
                }
            }

            // If they match, then do the action
            if ((i == 11) && (0 == ((disk_filename)[0x0b] & 0x18))) { // this always returns
                current_cluster_ = ReverseBytesInWord((disk_filename + 0x1a));
                if (filesystem_ == kFileSystemFAT32) {
                    current_cluster_ = (current_cluster_ + (ReverseBytesInWord((disk_filename + 0x14)) << 16));
                }
                first_cluster_of_file_ = current_cluster_;
                total_filesize_ = ReverseBytesInLong((disk_filename + 0x1c));

                //Mode is Read
                if (file_mode == 'r') {
                    OpenForRead();
                    return;
                }
                if ((disk_filename)[11] & 0xd9) {
                    THROW(kErrorNoWritePermission);
                }

                //Mode is Delete
                if (file_mode == 'd') {
                    OpenForDelete(disk_filename);
                    return;
                } else if (file_mode == 'w') {
                    OpenForWrite(disk_filename, directory_pointer);
                    return;
                } else if (file_mode == 'a') {
                    OpenForAppend(directory_pointer);
                    return;
                } else {
                    THROW(kErrorBadArgument);
                }

            }
        }

        if (file_mode == 'd') { //If we got here it's because we didn't find anything to delete.
            return;
        }

        if ((file_mode != 'w') && (file_mode != 'a')) {
            THROW(kErrorFileNotFound);
        }
        directory_entry_position_ = free_entry;
        if (directory_entry_position_ == 0) {
            THROW(kErrorNoEmptyDirectoryEntry);
        }

        // write (or new append): create valid directory entry
        char * S = ReadByte(directory_entry_position_);
        RET_IF_ERROR;
        memset((void *) S, 0, kDirectorySize);
        memcpy((void *) S, (void *) &cleaned_filename, 11);
        WriteReversedWord((S + 0x1a), 0);
        WriteReversedWord((S + 0x14), 0);
        WriteReversedLong((S + 0x0e), file_date_time_); // write create time and date
        WriteReversedLong((S + 0x16), file_date_time_); // write last modified date and time
        if ((directory_entry_position_ == sentinel) && ((directory_entry_position_ + kDirectorySize) < root_directory_end_)) {
            WriteReversedWord(ReadByte((directory_entry_position_ + kDirectorySize)), 0);
        }
        FlushIfDirty();
        RET_IF_ERROR;

        cluster_write_offset_ = 0;
        current_cluster_ = 0;
        buffer_end_ = kSectorSize;
    }

    /** Flush and close the currently open file if any.  
     * 
     * Clears any errors.
     * 
     * Also reset the pointers 
     * to valid values. Also, releases the SD pins to tristate.
     */
    void Close(void) {
        ClearError();
        if (directory_entry_position_) {
            Flush();
            RET_IF_ERROR;
        }
        current_buffer_location_ = 0;
        buffer_end_ = 0;
        total_filesize_ = 0;
        seek_position_ = 0;
        remaining_cluster_bytes_ = 0;
        cluster_write_offset_ = 0;
        directory_entry_position_ = 0;
        current_cluster_ = 0;
        first_cluster_of_file_ = 0;
        sd_spi_.ReleaseCard();
    }

    /** Read and return a single character from the currently open file.
     * 
     * @return -1 if the end of the file is reached. Otherwise, returns the 
     * character in the lower byte.
     */
    int Get(void) {
        int T;
        if (current_buffer_location_ >= buffer_end_) {
            T = FillBuffer();
            RET_IF_ERROR_NULL;
            if (T <= 0) {
                return (-1);
            }
        }
        return buffer_1_[(current_buffer_location_++)];
    }

    /** Read bytes into a buffer from currently open file.
     * 
     * Note that this function does not null terminate a string.
     * 
     * @param read_buffer The buffer to store the data. The buffer may be as large as you want.
     * @param bytes_to_read_count The number of bytes to read.
     * @return  Returns the number of bytes successfully read, or a negative 
     * number if there is an error. If the end of file has been reached, then 
     * this may be less than bytes_to_read_count.
     */
    int Get(char * read_buffer, int bytes_to_read_count) {
        int T;
        int R = 0;
        while (bytes_to_read_count > 0) {
            if (current_buffer_location_ >= buffer_end_) {
                T = FillBuffer();
                if (T <= 0) {
                    if (R > 0) {
                        return R;
                    }
                    return T;
                }
            }
            T = (Min__((buffer_end_ - current_buffer_location_), bytes_to_read_count));
            if (((T | (int) read_buffer) | current_buffer_location_) & 0x3) {
                memcpy((void *) read_buffer, (void *) (void *) (((int) (&buffer_1_) + current_buffer_location_)), 1 * (T));
            } else {
                memmove((void *) read_buffer, (void *) (void *) (((int) (&buffer_1_) + current_buffer_location_)), 4 * ((Shr__(T, 2))));
            }
            current_buffer_location_ = (current_buffer_location_ + T);
            R = (R + T);
            read_buffer = (read_buffer + T);
            bytes_to_read_count = (bytes_to_read_count - T);
        }
        return R;
    }

    /** Write a single character to the file.
     * 
     * @param  C The character to write.
     * @return  0 if successful, a negative number if an error occurred.
     */
    int Put(const char C) {
        if (current_buffer_location_ == kSectorSize) {
            if (FlushBuffer(kSectorSize, 0) < 0) {
                return (-1);
            }
        }
        buffer_1_[(current_buffer_location_++)] = C;
        return 0;
    }

    /** Write a null-terminated string to the file. 
     * 
     * @param B The null-terminated string to write. No size limitations. Does 
     * not write the null terminator.
     * @return the number of bytes successfully written, or a negative number 
     * if there is an error.
     */
    int Put(const char * B) {
        return Put(B, strlen(B));
    }

    /** Write bytes from buffer into the currently open file.
     * 
     * @param buffer The buffer to pull the data from. The buffer may be as large as 
     * you want.
     * @param byte_count the number of bytes to write.
     * @return the number of bytes successfully written, or a negative number 
     * if there was an error.
     */
    int Put(const char * buffer, int byte_count) {
        int total_bytes_written = 0;
        while (byte_count > 0) {
            if (current_buffer_location_ >= buffer_end_) {
                FlushBuffer(current_buffer_location_, 0);
                RET_IF_ERROR_NULL;

            }
            int bytes_to_write = (Min__((buffer_end_ - current_buffer_location_), byte_count));
            memcpy((void *) (void *) (((int) (&buffer_1_) + current_buffer_location_)), (void *) buffer, bytes_to_write);

            total_bytes_written = (total_bytes_written + bytes_to_write);
            current_buffer_location_ = (current_buffer_location_ + bytes_to_write);
            buffer = (buffer + bytes_to_write);
            byte_count = (byte_count - bytes_to_write);
        }
        return total_bytes_written;
    }

    int PutFormatted(const char * formatString, ...) {
        PrintStream<SD> ps(this);

        va_list list;
        va_start(list, formatString);
        int result = ps.Format(formatString, list);
        va_end(list);
        return result;
    }

    /** Set up for a directory file listing.
     * 
     * Close the currently open file, and set up the read buffer for calls to 
     * @a nextfile().
     */
    void OpenRootDirectory(void) {
        Close();
        RET_IF_ERROR;

        int off = (root_directory_ - (data_region_ << kSectorShift));
        current_cluster_ = (Shr__(off, (cluster_shift_ + kSectorShift)));
        seek_position_ = (off - (current_cluster_ << (cluster_shift_ + kSectorShift)));
        remaining_cluster_bytes_ = (root_directory_end_ - root_directory_);
        total_filesize_ = (seek_position_ + remaining_cluster_bytes_);
    }

    /** Find the next file in the root directory and extract its (8.3) name into 
     * filename.  The buffer must be sized to hold at least 13 characters 
     * (8 + 1 + 3 + 1).
     * 
     * @param filename The extracted filename
     * @return true if there is a valid filename, false otherwise.
     */

    bool NextFile(char * filename) {
        int filesize, year, month, day, hour, minute, second;
        return NextFile(filename, filesize, year, month, day, hour, minute, second);
    }
    
    bool NextFile(char * filename, int & filesize) {
        int year, month, day, hour, minute, second;
        return NextFile(filename, filesize, year, month, day, hour, minute, second);
    }

    bool NextFile(char * filename,
            int & filesize,
            int & year, int & month, int & day, int & hour, int & minute, int & second) {

        while (true) {
            if (current_buffer_location_ >= buffer_end_) {
                if (FillBuffer() < 0) {
                    return false;
                }
                if (((Shr__(seek_position_, kSectorShift)) & ((1 << cluster_shift_) - 1)) == 0) {
                    (current_cluster_++);
                }
            }

            char * at = (char *) ((int) &buffer_1_ + current_buffer_location_);

            if ((at)[0] == 0) {
                return false;
            }
            current_buffer_location_ = (current_buffer_location_ + kDirectorySize);
            if (((at)[0] != 0xe5) && ((at)[0] != 0xeb)
                    && (((at)[0x0b] & 0x18) == 0)) {
                char * lns = filename;

                for (int i = 0; i <= 10; i++) {
                    filename[0] = (at)[i];
                    filename++;
                    if (at[i] != ' ') {
                        lns = filename;
                    }
                    if (i == 7 || i == 10) {
                        filename = lns;
                        if (i == 7) {
                            filename[0] = '.';
                            filename++;
                        }
                    }
                }
                filename[0] = 0;
                
                filesize = ReverseBytesInLong(at + 28);
                ExtractDateTime(ReverseBytesInLong(at + 22), year, month, day, hour, minute, second);
                
                return true;
            }
        }
    }

    /** Change the read pointer to a different position in the file.
     * 
     * Seek() works only in 'r' (read) mode.
     * 
     * @param position The position to seek to, relative to the beginning of the file. Units?
     * 
     * @return 0 on success, a negative number on failure (such as seeking 
     * during write). Failures may include seeking outside the file size.
     */
    int Seek(const int position) {
        if (((directory_entry_position_) || (position < 0)) || (position > total_filesize_)) {
            return (-1);
        }
        int delta = ((seek_position_ - buffer_end_) & (-cluster_size_));
        if (position < delta) {
            current_cluster_ = first_cluster_of_file_;
            remaining_cluster_bytes_ = (Min__(cluster_size_, total_filesize_));
            seek_position_ = 0;
            current_buffer_location_ = 0;
            buffer_end_ = 0;
            delta = 0;
        }
        while (position >= (delta + cluster_size_)) {
            current_cluster_ = NextCluster();
            RET_IF_ERROR_NULL;

            seek_position_ = (seek_position_ + cluster_size_);
            delta = (delta + cluster_size_);
            remaining_cluster_bytes_ = (Min__(cluster_size_, (total_filesize_ - seek_position_)));
            current_buffer_location_ = 0;
            buffer_end_ = 0;
        }
        if (buffer_end_ == 0
                || position < (seek_position_ - buffer_end_)
                || position >= (seek_position_ - buffer_end_) + kSectorSize
                ) { // must change buffer
            //Warning: this section does not seem to be covered by unit tests. What's required for coverage?
            int delta_2 = (seek_position_ + remaining_cluster_bytes_);
            seek_position_ = (position & -kSectorSize);
            remaining_cluster_bytes_ = (delta_2 - seek_position_);
            FillBuffer();
            RET_IF_ERROR_NULL;
        }
        current_buffer_location_ = position & (kSectorSize - 1);
        return 0;
    }

    /** Set the current date and time for file creation and last modified.
     * 
     * This date and time will remain constant until the next time SetDate() is 
     * called.
     * 
     * @warning parameter limits are not checked. Ie, a month of 13 will not 
     * generate an error.
     * 
     * @param year   The year   (range 1980 - 2106, all 4 digits!)
     * @param month  The month  (range 1-12)
     * @param day    The day    (range 1-31)
     * @param hour   The hour   (range 0-23)
     * @param minute The minute (range 0-59)
     * @param second The second (range 0-59)
     * @return the FAT16 date format (you can safely ignore the return in all 
     * cases, unless you want to test the correctness of the function).
     */
    int SetDate(const int year, const int month, const int day,
            const int hour, const int minute, const int second) {
        file_date_time_ = ((year - 1980) << 25) + (month << 21) + (day << 16);
        file_date_time_ += (hour << 11) + (minute << 5) + (second >> 1);
        return file_date_time_;
    }
    
    /** If there was an error in the SD routines then this function will return
     * an error code.
     * 
     * @return The error code.
     */
    bool HasError(void) const {
        return (error_ != kNoError) || sd_spi_.HasError();
    }

    /** Resets the error flag to kNoError.
     */
    void ClearError(void) {
        error_ = kNoError;
        sd_spi_.ClearError();
    }

    /** Get the error code.
     * 
     * @return The error code.
     */
    int GetError(void) const {
        if (error_ != kNoError) {
            return error_;
        } else {
            return sd_spi_.GetError();
        }
    }

    /** Get the FAT cluster size.
     * @return the size of the cluster, in bytes.
     */
    int GetClusterSize(void) const {
        return cluster_size_;
    }

    /** Get the current FAT cluster count.
     * 
     * What does this mean? I (SRLM) don't know. I also don't know how to test it, so it is not tested.
     * 
     * @return the cluster count.
     */
    int GetClusterCount(void) const {
        return total_clusters_;
    }

    /**
     * 
     * @return The size of the current (read) file, in bytes.
     */
    int GetFilesize(void) const {
        return total_filesize_;
    }

private:

    // Note: these filesystem numbers should not be changed!
    static const int kFileSystemUnknown = 0;
    static const int kFileSystemFAT16 = 1;
    static const int kFileSystemFAT32 = 2;


    static const int kSectorSize = 512;
    static const int kSectorShift = 9;
    static const int kDirectorySize = 32;
    static const int kDirectoryShift = 5;

    SDSafeSPI sd_spi_;

    int current_cluster_;
    int total_filesize_;
    int seek_position_;
    int remaining_cluster_bytes_;
    int current_buffer_location_;
    int buffer_end_; // The last valid character (read) or free position (write)
    int directory_entry_position_;
    int cluster_write_offset_;
    int last_fat_entry_;
    int first_cluster_of_file_;

    int filesystem_;
    int root_directory_;
    int root_directory_end_;
    int data_region_;
    int cluster_shift_;
    int cluster_size_;
    int fat1_;
    int total_clusters_;
    int sectors_per_fat_;
    int end_of_chain_;
    int file_date_time_;
    int last_read_;
    int dirty_;


    int error_;

    /*
      Buffering:  two sector buffers.  These two buffers must be longword
      aligned!  To ensure this, make sure they are the first byte variables
      defined in this object.
     * 
     * These buffers don't seem to need to be volatile (all unit tests pass
     * whether they are or not), but for some reason the code seems to run 4%
     * faster if they are declared volatile. So, here they are.
     */
    char buffer_1_[512];
    char buffer_2_[512];

    /** In case of Bad Things(TM) happening, exit as gracefully as possible.
     * 
     * @param abort_code passed through to return.
     */
    void SetErrorCode(const int abort_code) {
        error_ = abort_code;
    }

    /**
     *     On metadata writes, if we are updating the FAT region, also update the second FAT region.
     */
    void WriteBlock(const int block_index, char * buffer_address) {
        sd_spi_.WriteBlock(block_index, buffer_address);
        RET_IF_ERROR;
        if (block_index >= fat1_) {
            if (block_index < (fat1_ + sectors_per_fat_)) {
                sd_spi_.WriteBlock(block_index + sectors_per_fat_, buffer_address);
                RET_IF_ERROR;
            }
        }
    }

    /* If the metadata block is dirty, write it out.
     */
    void FlushIfDirty(void) {
        if (dirty_) {
            WriteBlock(last_read_, buffer_2_);
            RET_IF_ERROR;
            dirty_ = 0;
        }
    }

    /** Read a block into the metadata buffer, if that block is not already there.
     */
    void ReadBlock(const int block_index) {
        if (block_index != last_read_) {
            FlushIfDirty();
            RET_IF_ERROR;
            sd_spi_.ReadBlock(block_index, buffer_2_);
            RET_IF_ERROR;
            last_read_ = block_index;
        }
    }

    /** Get a byte-reversed word from a (possibly odd) address.
     */
    int ReverseBytesInWord(const char * data) const {
        return ((data)[0] + ((data)[1] << 8));
    }

    /** Get a byte-reversed long from a (possibly odd) address.
     */
    int ReverseBytesInLong(const char * data) const {
        return (ReverseBytesInWord(data) + (ReverseBytesInWord((data + 2)) << 16));
    }

    /** Read a cluster entry.
     */
    int ReverseBytesInCluster(const char * cluster) const {
        if (filesystem_ == kFileSystemFAT16) {
            return ReverseBytesInWord(cluster);
        } else {
            return ReverseBytesInLong(cluster);
        }
    }

    /** Write a byte-reversed word to a (possibly odd) address, and mark the metadata buffer as dirty.
     */
    void WriteReversedWord(char * result, const int data) {
        result++[0] = data;
        result[0] = data >> 8;
        dirty_ = 1;
    }

    /** Write a byte-reversed long to a (possibly odd) address, and mark the metadata buffer as dirty.
     */
    void WriteReversedLong(char * result, const int data) {
        WriteReversedWord(result, data);
        WriteReversedWord(result + 2, data >> 16);
    }

    /** Write a cluster entry.
     */
    void WriteReversedCluster(char * result, const int data) {
        //   Write a cluster entry.
        if (filesystem_ == kFileSystemFAT16) {
            WriteReversedWord(result, data);
        } else {
            WriteReversedLong(result, data);
        }
    }

    int GetFilesystemType(void) {
        const int kFAT1 = 'F' + ('A' << 8) + ('T' << 16) + ('1' << 24);
        const int kFAT3 = 'F' + ('A' << 8) + ('T' << 16) + ('3' << 24);

        if ((ReverseBytesInLong(&buffer_1_[0x36]) == kFAT1) && (buffer_1_[58] == '6')) {
            return kFileSystemFAT16;
        }
        if ((ReverseBytesInLong(&buffer_1_[0x52]) == kFAT3) && (buffer_1_[86] == '2')) {
            return kFileSystemFAT32;
        }
        return kFileSystemUnknown;
    }

    /** Read a byte address from the disk through the metadata buffer and return a pointer to that location.
     */
    char * ReadByte(const int Byteloc) {
        ReadBlock((Shr__(Byteloc, kSectorShift)));
        RET_IF_ERROR_NULL;

        return ((char *) (&buffer_2_) + (Byteloc & 0x1ff));
    }

    /** Read a fat location and return a pointer to the location of that entry.
     */
    char * ReadFAT(const int Clust) {
        last_fat_entry_ = ((fat1_ << kSectorShift) + (Clust << filesystem_));
        return ReadByte(last_fat_entry_);
    }

    /** Follow the fat chain and update the writelink.
     */
    int FollowFATChain(void) {
        char * temp = ReadFAT(current_cluster_);
        RET_IF_ERROR_NULL;
        cluster_write_offset_ = last_fat_entry_;
        return ReverseBytesInCluster(temp);
    }

    /** Read the next cluster and return it.  Set up writelink to point to the cluster we just read, for later updating.  If the cluster number is bad, return a negative number.
     */
    int NextCluster(void) {
        int result = FollowFATChain();
        RET_IF_ERROR_NULL;
        if ((result < 2) || (result >= total_clusters_)) {
            THROW_NULL(kErrorBadClusterValue);
        }
        return result;
    }

    /** Free an entire cluster chain.  Used by remove and by overwrite. Assumes the pointer has already been cleared/set to end of chain.
     */
    void FreeClusters(int cluster) {
        while (cluster < end_of_chain_) {
            if (cluster < 2) {
                THROW(kErrorBadClusterNumber);
            }
            char * byte_pointer = ReadFAT(cluster);
            RET_IF_ERROR;

            cluster = ReverseBytesInCluster(byte_pointer);
            WriteReversedCluster(byte_pointer, 0);
        }
        FlushIfDirty();
        RET_IF_ERROR;
    }

    /*
    This is just a pass-through function to allow the block layer
     to tristate the I/O pins to the card.
     */
    void Release(void) {
        sd_spi_.ReleaseCard();
    }

    /**  Calculate the block address of the current data location.
     */
    int CalculateCurrentBlockAddress(void) const {
        return (current_cluster_ << cluster_shift_) +data_region_
                + (Shr__(seek_position_, kSectorShift) & ((1 << cluster_shift_) - 1));

    }

    /** Compute the upper case version of a character.
     */
    char ConvertToUppercase(const char character) const {
        if (('a' <= character) && (character <= 'z')) {
            return (character - 32);
        }
        return character;
    }

    /** Flush the current buffer, if we are open for write.  This may allocate a new cluster if needed.  If metadata is true, the metadata is written through to disk including any FAT cluster allocations and also the file size in the directory entry.
     */
    int FlushBuffer(int r_cnt, const int flush_metadata) {
        if (directory_entry_position_ == 0) {
            THROW_NULL(kErrorFileNotOpenForWriting);
        }
        if (r_cnt > 0) { // must *not* allocate cluster if flushing an empty buffer
            if (remaining_cluster_bytes_ < kSectorSize) {
                // find a new cluster could be anywhere!  If possible, stay on the
                // same page used for the last cluster.
                int Newcluster = (-1);
                int Cluststart = (current_cluster_ & (~((Shr__(kSectorSize, filesystem_)) - 1)));
                int Count = 2;
                while (1) {
                    ReadFAT(Cluststart);
                    RET_IF_ERROR_NULL;

                    int I;
                    {
                        int _limit__0025 = (kSectorSize - (1 << filesystem_));
                        int _step__0026 = (1 << filesystem_);
                        I = 0;
                        if (I >= _limit__0025) _step__0026 = -_step__0026;
                        do {
                            if (buffer_2_[I] == 0) {
                                if (ReverseBytesInCluster(((char *) (&buffer_2_) + I)) == 0) {
                                    Newcluster = (Cluststart + (Shr__(I, filesystem_)));
                                    if (Newcluster >= total_clusters_) {
                                        Newcluster = (-1);
                                    }
                                    break;
                                }
                            }
                            I = (I + _step__0026);
                        } while (((_step__0026 > 0) && (I <= _limit__0025)) || ((_step__0026 < 0) && (I >= _limit__0025)));
                    }
                    if (Newcluster > 1) {
                        WriteReversedCluster(((char *) (&buffer_2_) + I), (end_of_chain_ + 15));
                        if (cluster_write_offset_ == 0) {
                            WriteReversedWord((ReadByte(directory_entry_position_) + 26), Newcluster);
                            cluster_write_offset_ = (directory_entry_position_ & (kSectorSize - filesystem_));
                            WriteReversedLong((((char *) (&buffer_2_) + cluster_write_offset_) + 28), (seek_position_ + current_buffer_location_));
                            if (filesystem_ == kFileSystemFAT32) {
                                WriteReversedWord((((char *) (&buffer_2_) + cluster_write_offset_) + 20), (Shr__(Newcluster, 16)));
                            }
                        } else {
                            WriteReversedCluster(ReadByte(cluster_write_offset_), Newcluster);
                        }
                        cluster_write_offset_ = (last_fat_entry_ + I);
                        current_cluster_ = Newcluster;
                        remaining_cluster_bytes_ = cluster_size_;
                        break;
                    } else {
                        Cluststart = (Cluststart + (Shr__(kSectorSize, filesystem_)));
                        if (Cluststart >= total_clusters_) {
                            Cluststart = 0;
                            (Count--);
                            if (r_cnt < 0) {
                                r_cnt = -5; //No space left on device
                                break;
                            }
                        }
                    }
                }
            }
            if (remaining_cluster_bytes_ >= kSectorSize) {
                sd_spi_.WriteBlock(CalculateCurrentBlockAddress(), (char *) (&buffer_1_));
                RET_IF_ERROR_NULL;

                if (r_cnt == kSectorSize) { // full buffer, clear it
                    seek_position_ = (seek_position_ + r_cnt);
                    remaining_cluster_bytes_ = (remaining_cluster_bytes_ - r_cnt);
                    current_buffer_location_ = 0;
                    buffer_end_ = r_cnt;
                }
            }
        }
        if ((r_cnt < 0) || (flush_metadata)) { // update metadata even if error
            ReadBlock((Shr__(directory_entry_position_, kSectorShift))); // flushes unwritten FAT too
            RET_IF_ERROR_NULL;

            WriteReversedLong((((char *) (&buffer_2_) + (directory_entry_position_ & (kSectorSize - filesystem_))) + 28), (seek_position_ + current_buffer_location_));
            FlushIfDirty();
            RET_IF_ERROR_NULL;
        }
        if (r_cnt < 0) {
            THROW_NULL(r_cnt);
        }
        return r_cnt;
    }

    /** Call flush with the current data buffer location, and the flush metadata flag set.
     */
    int Flush(void) {
        return FlushBuffer(current_buffer_location_, 1);
    }

    /** Get some data into an empty buffer.  If no more data is available, return -1.  Otherwise return the number of bytes read into the buffer.
     */
    int FillBuffer(void) {
        if (seek_position_ >= total_filesize_) {
            return -1;
        }
        if (remaining_cluster_bytes_ == 0) {
            current_cluster_ = NextCluster();
            if (current_cluster_ < 0) {
                return current_cluster_;
            }
            remaining_cluster_bytes_ = (Min__(cluster_size_, (total_filesize_ - seek_position_)));
        }
        sd_spi_.ReadBlock(CalculateCurrentBlockAddress(), buffer_1_);
        RET_IF_ERROR_NULL;

        int bytes_read = kSectorSize;
        if ((seek_position_ + bytes_read) >= total_filesize_) {
            bytes_read = (total_filesize_ - seek_position_);
        }
        seek_position_ = (seek_position_ + bytes_read);
        remaining_cluster_bytes_ = (remaining_cluster_bytes_ - bytes_read);
        current_buffer_location_ = 0;
        buffer_end_ = bytes_read;
        return bytes_read;
    }

    /** Open a file for reading.
     * 
     */
    void OpenForRead(void) {
        remaining_cluster_bytes_ = (Min__(cluster_size_, total_filesize_));
    }

    /** Open a file for deletion.
     * 
     * @param string
     */
    void OpenForDelete(char * string) {
        WriteReversedWord(string, 0xe5);
        if (current_cluster_) {
            FreeClusters(current_cluster_);
            RET_IF_ERROR;
        }
        FlushIfDirty();
        RET_IF_ERROR;
    }

    /** Open a file for writing.
     */
    void OpenForWrite(char * string, const int dir_pointer) {
        WriteReversedWord((string + 0x1a), 0);
        WriteReversedWord((string + 0x14), 0);
        WriteReversedLong((string + 0x1c), 0);
        cluster_write_offset_ = 0;
        directory_entry_position_ = dir_pointer;
        if (current_cluster_) {
            FreeClusters(current_cluster_);
            RET_IF_ERROR;

        }
        buffer_end_ = kSectorSize;
        current_cluster_ = 0;
        total_filesize_ = 0;
        remaining_cluster_bytes_ = 0;
    }

    /** Open a file for appending.
     * 
     * @todo(SRLM): Can I replace some of this code with seek?
     * 
     * @param dir_pointer
     */
    void OpenForAppend(const int dir_pointer) {
        // this code will eventually be moved to seek
        remaining_cluster_bytes_ = total_filesize_;
        int free_entry = cluster_size_;
        if (current_cluster_ >= end_of_chain_) {
            current_cluster_ = 0;
        }
        while (remaining_cluster_bytes_ > free_entry) {
            if (current_cluster_ < 2) {
                THROW(kErrorEofWhileFollowingChain);
            }
            current_cluster_ = NextCluster();
            RET_IF_ERROR;

            remaining_cluster_bytes_ = (remaining_cluster_bytes_ - free_entry);
        }
        seek_position_ = (total_filesize_ & !(kSectorSize - 1));
        buffer_end_ = kSectorSize;
        current_buffer_location_ = (remaining_cluster_bytes_ & 0x1ff);
        cluster_write_offset_ = 0;
        directory_entry_position_ = dir_pointer;
        if (current_buffer_location_) {
            sd_spi_.ReadBlock(CalculateCurrentBlockAddress(), (char *) (&buffer_1_));
            RET_IF_ERROR;

            remaining_cluster_bytes_ = (free_entry - (seek_position_ & (free_entry - 1)));
        } else {
            if ((current_cluster_ < 2) || (remaining_cluster_bytes_ == free_entry)) {
                remaining_cluster_bytes_ = 0;
            } else {
                remaining_cluster_bytes_ = (free_entry - (seek_position_ & (free_entry - 1)));
            }
        }
        if (current_cluster_ >= 2) {
            FollowFATChain();
            RET_IF_ERROR;

        }
    }
    
    /** Extract the date and time from a FAT date time entry.
     * 
     * The decoding was taken from here: http://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
     */
    void ExtractDateTime(int FATDate,
            int & year, int & month, int & day,
            int & hour, int & minute, int & second){
        year = ((FATDate >> 25) & 0b1111111) + 1980;
        month = (FATDate >> 21) & 0b1111;
        day = (FATDate >> 16) & 0b11111;
        hour = (FATDate >> 11) & 0b11111;
        minute = (FATDate >> 5) & 0b111111;
        second = (FATDate & 0b11111) << 1;
    }

    /** Get the minimum of two integers.
     * 
     * @param a
     * @param b
     * @return 
     */
    static int Min__(const int a, const int b) {
        return a < b ? a : b;
    }

    /** Shift a variable right (no sign extension)
     * 
     * @param a
     * @param b
     * @return 
     */
    static int Shr__(const unsigned int a, const unsigned int b) {
        return (a >> b);
    }

};

#endif // LIBPROPELLER_SD_H_
