/**
* @file        shell.h
*
* @author      David Zemon
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

#pragma once

#include <PropWare/PropWare.h>
#include <PropWare/sd.h>

namespace PropWare {

class Shell {
    public:
        /**
        * @name Shell Definitions
        * @{
        */
        /** Maximum number of characters allowed at the command prompt */
#define SD_SHELL_INPUT_LEN  128
        static const uint8_t SHELL_INPUT_LEN = SD_SHELL_INPUT_LEN;
        /**
        * Maximum number of characters for an individual command (does not
        * include parameters
        */
#define SD_SHELL_CMD_LEN    8
        static const uint8_t SHELL_CMD_LEN = SD_SHELL_CMD_LEN;
        /** Maximum number of characters for each command argument */
#define SD_SHELL_ARG_LEN    64
        static const uint8_t SHELL_ARG_LEN = SD_SHELL_ARG_LEN;
        /** String defining the "exit" command to quit the SD_Shell() function*/
        static const char    SHELL_EXIT[];
        /**
        * String defining the "ls" command to call SD_Shell_ls(); List dir
        * contents
        */
        static const char    SHELL_LS[];
        /**
        * String defining the "cat" command to call SD_Shell_cat(); Prints a
        * file
        */
        static const char    SHELL_CAT[];
        /**
        * String defining the "cd" command to call SD_Shell_cd(); Change
        * directory
        */
        static const char    SHELL_CD[];
        /** String defining the "touch" command; Creates an empty file */
        static const char    SHELL_TOUCH[];

        /**@}*/

        Shell (SD *sd, const Printer *printer, const Scanner *scanner)
                : m_sd(sd), m_printer(printer), m_scanner(scanner) {
        }

        /**
        * @brief       Provide the user with a very basic Unix-like shell. The
        *              following commands are available to the user: ls, cat,
        *              cd.
        *
        * @param[in]   *f  If a file is opened via a command such as 'cat', its
        *                  information will be stored at this address
        *
        * @return      Returns 0 upon success, error code otherwise
        */
        PropWare::ErrorCode shell (SD::File *f) {
            char                usrInput[SD_SHELL_INPUT_LEN]    = "";
            char                cmd[SD_SHELL_CMD_LEN]           = "";
            char                arg[SD_SHELL_ARG_LEN]           = "";
            char                uppercaseName[SD_SHELL_ARG_LEN] = "";
            uint8_t             i, j;
            PropWare::ErrorCode err                             = 0;

            this->m_printer->println("Welcome to David's quick shell! There is no help, nor much to do.");
            this->m_printer->println("Have fun...");

            // Loop until the user types the SD::SHELL_EXIT string
            while (strcmp(usrInput, this->SHELL_EXIT)) {
                this->m_printer->printf(">>> ");
                this->m_scanner->gets(usrInput, SD_SHELL_INPUT_LEN);

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
                    err = this->m_sd->chdir(uppercaseName);
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
                    this->m_printer->printf("Invalid command: %s\n", cmd);

                // Handle errors; Print user errors and continue; Return system
                // errors
                if (err) {
                    if (SD::BEG_ERROR <= err && err <= SD::END_USER_ERRORS)
                        this->m_sd->print_error_str(this->m_printer,
                                                    (SD::ErrorCode) err);
                    else
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


        /**
        * @brief       List the contents of a directory on the screen (similar
        *              to 'ls .')
        *
        * @note        TODO: Implement *abspath when SDGetSectorFromPath() is
        *              functional
        *
        * @return      Returns 0 upon success, error code otherwise
        */
        PropWare::ErrorCode shell_ls () {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            // Allocate space for a filename string
            char string[SD::FILENAME_STR_LEN];

            // If we aren't looking at the beginning of a cluster, we must
            // backtrack to the beginning and then begin listing files
            if (this->m_buf.curSectorOffset
                    || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                            != this->m_buf.curClusterStartAddr)) {
#ifdef SD_OPTION_VERBOSE
                printf("'ls' requires a backtrack to beginning of directory's "
                        "cluster\n");
#endif
                this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                        this->m_dir_firstAllocUnit);
                this->m_buf.curSectorOffset     = 0;
                this->m_buf.curAllocUnit        = this->m_dir_firstAllocUnit;
                check_errors(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                            &(this->m_buf.nextAllocUnit)));
                check_errors(
                        this->read_data_block(this->m_buf.curClusterStartAddr,
                                              this->m_buf.buf));
            }

            // Loop through all files in the current directory until we find the
            // correct one; Function will exit normally without an error code if
            // the file is not found
            while (this->m_buf.buf[fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if ((SD::DELETED_FILE_MARK != this->m_buf.buf[fileEntryOffset])
                        && !(SD::SYSTEM_FILE
                                & this->m_buf.buf[fileEntryOffset
                                        + SD::FILE_ATTRIBUTE_OFFSET]))
                    this->print_file_entry(&(this->m_buf.buf[fileEntryOffset]),
                                           string);

                // Increment to the next file
                fileEntryOffset += SD::FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
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

        /**
        * @brief       Dump the contents of a file to the screen (similar to
        *              'cat f');
        *
        * @note        Does not currently follow paths
        *
        * @param[in]   *name   Name of the file to open
        * @param[in]   *f      File object that can be used to open *name
        *
        * @return      Returns 0 upon success, error code otherwise
        */
        PropWare::ErrorCode shell_cat (const char *name,
                                       PropWare::SD::File *f) {
            PropWare::ErrorCode err;

            // Attempt to find the file
            if ((err = this->fopen(name, f, SD::FILE_MODE_R))) {
                return err;
            } else {
                // Loop over each character and print them to the screen
                // one-by-one
                while (!this->feof(f))
                    // Using SDfgetc() instead of SDfgets to ensure
                    // compatibility with binary files
                    // TODO: Should probably create something better to output
                    //       binary files don't ya think!?
                    putchar(this->fgetc(f));
                putchar('\n');
            }

            return 0;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
        * @brief       Create a new file, do not open it
        *
        * @param[in]   name[]  C-string name for the file to be created
        *
        * @return      Returns 0 upon success, error code otherwise
        */
        PropWare::ErrorCode shell_touch (const char name[]) {
            PropWare::ErrorCode err;
            uint16_t fileEntryOffset;

            // Attempt to find the file if it already exists
            if ((err = this->find(name, &fileEntryOffset))) {
                // Error occurred - hopefully it was a "file not found" error
                if (SD::FILENAME_NOT_FOUND == err)
                    // File wasn't found, let's create it
                    err = this->create_file(name, &fileEntryOffset);
                return err;
            }

            // If SDFind() returns 0, the file already existed and an error
            // should be thrown
            return SD::FILE_ALREADY_EXISTS;
        }
#endif

    protected:
        SD            *m_sd;
        const Printer *m_printer;
        const Scanner *m_scanner;
};

}
