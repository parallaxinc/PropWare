// Copyright 2013 SRLM and Red9

#include <propeller.h>
#include "unity.h"
#include "sd.h"
#include <stdlib.h>

const int kDoPin = 10;
const int kClkPin = 11;
const int kDiPin = 12;
const int kCsPin = 13;

const int kDoPinNoSd = 18;
const int kClkPinNoSd = 20;
const int kDiPinNoSd = 19;
const int kCsPinNoSd = 21;

/**
 * 
 * Hardware requirements:
 * SD card on pins 10-13
 * No SD card on pins 18-21, with safe to set these pins to arbitrary states.
 * 
 * Improvements:
 *  The current implementation uses a single instance of the class. It would be
 * better to use a fresh instance each time.
 * 
 * @todo(SRLM): Don't use hardcoded pins!
 * 
 */


SD sut;

class UnityTests {
public:

    static void help_TestFilename(const char * filename) {
        char letter = CNT & 0x7F; //Semi random noise...

        sut.Open(filename, 'w');
        sut.Put(letter);
        sut.Open(filename, 'r');
        TEST_ASSERT_EQUAL_INT(letter, sut.Get());
        TEST_ASSERT_EQUAL_INT(-1, sut.Get());
    }

    static int help_DeleteAllFiles(void) {
        const int MAXIMUM_FILE_COUNT = 1000;
        int count;
        for (count = 0; count < MAXIMUM_FILE_COUNT; count++) {
            
            char filename [13];
            sut.OpenRootDirectory();
            if (sut.NextFile(filename) != true) {
                break;
            }
            sut.Open(filename, 'd');
        }
        return count;
    }

    static void cog_DoNothing(void * arg) {
        waitcnt(CLKFREQ * 50 + CNT);
    }

    static int help_CountNumberOfFreeCogs(void) {
        const int stacksize = sizeof (_thread_state_t) + sizeof (int) * 10;
        int * cog_stack = (int*) malloc(stacksize);
        int cog_id = cogstart(cog_DoNothing, NULL, cog_stack, stacksize);

        int free_cogs = 0;

        if (cog_id != -1) {
            free_cogs = help_CountNumberOfFreeCogs() + 1;
            cogstop(cog_id);
        }

        free(cog_stack);

        return free_cogs;
    }

    static void setUp(void) {
        sut.ClearError();
        sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
    }

    static void tearDown(void) {
        help_DeleteAllFiles();
        sut.Unmount();
    }

    // -----------------------------------------------------------------------------
    // Mount Operations
    // -----------------------------------------------------------------------------

    static void test_Mount(void) {
        //Assume: mount in setUp();
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_MountMultiple(void) {
        sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
        TEST_ASSERT_FALSE(sut.HasError());
        sut.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
        TEST_ASSERT_FALSE(sut.HasError());

    }

    static void test_MultipleUnmounts(void) {
        sut.Unmount();
        TEST_ASSERT_EQUAL_INT(0, sut.GetError());
        sut.Unmount();
        TEST_ASSERT_EQUAL_INT(0, sut.GetError());
    }

    static void test_MountNoSd(void) {
        sut.Mount(kDoPinNoSd, kClkPinNoSd, kDiPinNoSd, kCsPinNoSd);
        TEST_ASSERT_EQUAL_INT(SD::kErrorCardNotReset, sut.GetError());
    }

    static void test_UnmountFreesCog(void) {
        int cogsFreeBefore = help_CountNumberOfFreeCogs();
        sut.Unmount();
        TEST_ASSERT_EQUAL_INT(cogsFreeBefore + 1, help_CountNumberOfFreeCogs());
    }

    static void test_DestructorFreesCog(void) {
        sut.Unmount();
        int cogsFreeBefore = help_CountNumberOfFreeCogs();
        {
            SD temp;
            temp.Mount(kDoPin, kClkPin, kDiPin, kCsPin);
            TEST_ASSERT_EQUAL_INT(cogsFreeBefore - 1, help_CountNumberOfFreeCogs());
        }
        TEST_ASSERT_EQUAL_INT(cogsFreeBefore, help_CountNumberOfFreeCogs());
    }
    
    // -----------------------------------------------------------------------------
    // File operations (open, close, etc.)
    // -----------------------------------------------------------------------------

    static void test_OpenExistingFileForWrite(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put('a');
        sut.Open("RANDOM.RND", 'w');
        sut.Put('b');
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_EQUAL_INT('b', sut.Get());

    }

    static void test_OpenNonexistentFileForRead(void) {
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_TRUE(sut.HasError());
    }

    static void test_OpenNonexistentFileForWrite(void) {
        sut.Open("RANDOM.RND", 'w');
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_OpenForDeleteNonexistentFile(void) {
        sut.Open("RANDOM.RND", 'd');
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_OpenForAppendNonexistentFile(void) {
        sut.Open("RANDOM.RND", 'a');
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_OpenTooLongFilename(void) {
        sut.Open("REALLONGNAME.RND", 'w');
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_CloseFileTwice(void) {
        sut.Close();
        TEST_ASSERT_FALSE(sut.HasError());
        sut.Close();
        TEST_ASSERT_FALSE(sut.HasError());
    }

    static void test_OpenSpecialCharacterFilenames(void) {
        help_TestFilename("$%-_@~`!.(){");
        help_TestFilename("}^#& ");
    }

    static void test_OpenFilenameWithExtensionOnly(void) {
        help_TestFilename(".WAT");
    }

    static void test_OpenFilenameWithoutExtension(void) {
        help_TestFilename("Hello");
    }

    static void test_OpenShorterFilenameThan8dot3(void) {
        help_TestFilename("a.a");
    }

    static void test_OpenEmptyFilename(void) {
        help_TestFilename("");
    }



    //TO DO(SRLM): I don't think I can test to tell if the pins are Tristated. This
    //is because each cog has it's own DIRA register, and the results, although OR'd
    //together with the other DIRA registers, is not accessible.
    //One solution might be to
    // 1. Tristate pins
    // 2. Set DIRA for SD pin to output
    // 3. Set OUTA for SD pin to low
    // 4. Set a neighboring pin (with resistor to SD pin) to input
    // 5. Read neighboring pin, check that it's low.
    // This wouldn't work in the case that the SPI driver is holding the pin low,
    // but in the other cases it would work.
    // 6. Set SD pin high
    // 7. Read neighboring pin, check that it's high.
    //static void test_CloseReleasePinsToTristate(void)
    //{
    //	const unsigned int kDoPinMask  = 1 << kDoPin;
    //	const unsigned int kClkPinMask = 1 << kClkPin;
    //	const unsigned int kDiPinMask  = 1 << kDiPin;
    //	const unsigned int kCsPinMask  = 1 << kCsPin;
    //	
    //	const unsigned int kSdPinMask = kDoPinMask | kClkPinMask | kDiPinMask | kCsPinMask;
    //	
    //	TEST_ASSERT_EQUAL_INT(0, sut.Open("RANDOM.RND", 'w'));
    //	TEST_ASSERT_BITS_HIGH(kSdPinMask, DIRA);
    //	sut.Close();
    //	TEST_ASSERT_BITS_LOW(kSdPinMask, 0xFFFFFFFF);
    //}



    // -----------------------------------------------------------------------------
    // Writing to and from files
    // -----------------------------------------------------------------------------

    static void test_PutChar(void) {
        sut.Open("RANDOM.RND", 'w');
        TEST_ASSERT_EQUAL_INT(0, sut.Put('a'));
    }

    static void test_GetCharFromExistingFile(void) {
        sut.Open("RANDOM.RND", 'd');
        sut.Open("RANDOM.RND", 'w');
        sut.Put('x');
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_EQUAL_INT('x', sut.Get());
    }

    static void test_GetCharAfterEndOfFile(void) {
        sut.Open("RANDOM.RND", 'd');
        sut.Open("RANDOM.RND", 'w');
        sut.Put('x');
        sut.Open("RANDOM.RND", 'r');
        sut.Get();
        TEST_ASSERT_EQUAL_INT(-1, sut.Get());
    }

    static void test_PutCharAppend(void) {
        sut.Open("APPEND.TXT", 'a');
        TEST_ASSERT_EQUAL_INT(0, sut.Put('-'));
        sut.Open("APPEND.TXT", 'r');
        TEST_ASSERT_EQUAL_INT('-', sut.Get());
        TEST_ASSERT_EQUAL_INT(-1, sut.Get());
    }

    static void test_Put(void) {
        sut.Open("RANDOM.RND", 'w');
        TEST_ASSERT_EQUAL_INT(5, sut.Put("Hello"));
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_EQUAL_INT('H', sut.Get());
        TEST_ASSERT_EQUAL_INT('e', sut.Get());
        TEST_ASSERT_EQUAL_INT('l', sut.Get());
        TEST_ASSERT_EQUAL_INT('l', sut.Get());
        TEST_ASSERT_EQUAL_INT('o', sut.Get());
        TEST_ASSERT_EQUAL_INT(-1, sut.Get());
    }

    static void test_PutSEmptyString(void) {
        sut.Open("RANDOM.RND", 'w');
        TEST_ASSERT_EQUAL_INT(0, sut.Put(""));
    }

    static void test_Get(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("World\0ABC", 6);

        char buffer[6];
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_EQUAL_INT(6, sut.Get(buffer, 6));
        TEST_ASSERT_EQUAL_STRING("World", buffer);
    }

    static void test_GetBufferPastEndOfFile(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("World\0", 6);

        char buffer[10];
        sut.Open("RANDOM.RND", 'r');
        TEST_ASSERT_EQUAL_INT(6, sut.Get(buffer, 10));
        TEST_ASSERT_EQUAL_STRING("World", buffer);
    }

    static void test_WriteLargeFile(void) {
        sut.Open("RANDOM.RND", 'w');

        const int kAlphabetCount = 2048;

        for (int i = 0; i < kAlphabetCount; i++) {
            for (int letter = 'a'; letter <= 'z'; letter++) {
                TEST_ASSERT_EQUAL_INT(0, sut.Put(letter));
            }
        }

        sut.Open("RANDOM.RND", 'r');
        for (int i = 0; i < kAlphabetCount; i++) {
            for (int letter = 'a'; letter <= 'z'; letter++) {
                TEST_ASSERT_EQUAL_INT(letter, sut.Get());
            }
        }

        TEST_ASSERT_EQUAL_INT(-1, sut.Get());
    }


    // -----------------------------------------------------------------------------
    // Test file system functionality
    // -----------------------------------------------------------------------------

    static void test_SetDate(void) {
        // Fat16 date and time information here:
        // http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html#LastWriteTime

        //Hour = 3, minute = 30, seconds = 58
        //         0bHHHHHMMMMMMSSSSS
        int time = 0b0001101111011101;

        // Year = 2000, month = 1, day = 2
        //         0bYYYYYYYMMMMDDDDD
        int date = 0b0010100000100010;

        int datetime = (date << 16) + time;

        TEST_ASSERT_BITS(0xFFFFFFFF, datetime, sut.SetDate(2000, 1, 2, 3, 30, 58));
    }

    static void test_SeekSmallFile(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("Hello World!");
        sut.Open("RANDOM.RND", 'r');

        for (int i = 0; i < 5; i++)
            sut.Get();

        TEST_ASSERT_EQUAL_INT(0, sut.Seek(2));
        TEST_ASSERT_EQUAL_INT('l', sut.Get());
    }

    static void test_SeekOnWriteAfterOpening(void) {
        sut.Open("RANDOM.RND", 'w');
        TEST_ASSERT_EQUAL_INT(-1, sut.Seek(0));
    }

    static void test_SeekOnWriteAfterWriting(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("Hello World!");
        TEST_ASSERT_EQUAL_INT(-1, sut.Seek(0));
    }

    static void test_SeekOnWriteAndCanStillWriteAfter(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("Hello");
        sut.Seek(0);
        sut.Put("World");
        sut.Open("RANDOM.RND", 'r');
        for (int i = 0; i < 5; i++)
            sut.Get();
        TEST_ASSERT_EQUAL_INT('W', sut.Get());
    }

    static void test_SeekOnRead(void) {
        sut.Open("RANDOM.RND", 'w');
        sut.Put("Hello World");
        sut.Open("RANDOM.RND", 'r');
        sut.Seek(6);
        TEST_ASSERT_EQUAL_INT('W', sut.Get());
    }

    static void test_SeekOnLargeFile(void) {
        //Should be more than 32KB	
        //32KB clusters * 1024 B/Clust / 16 byte test sequence * 1.5 cluster span = 3072
        //Repeat a 16 byte test sequence across 1.5 32KB clusters.

        sut.Open("RANDOM.RND", 'w');

        for (int i = 0; i < 3072; i++)
            for (char testchar = 'a'; testchar <= 'p'; testchar++)
                sut.Put(testchar);

        sut.Open("RANDOM.RND", 'r');

        //Back across cluster boundry
        TEST_ASSERT_EQUAL_INT(0, sut.Seek(0));
        TEST_ASSERT_EQUAL_INT('a', sut.Get());

        //Within 32KB cluster
        TEST_ASSERT_EQUAL_INT(0, sut.Seek(16 * 1024 + 3));
        TEST_ASSERT_EQUAL_INT('d', sut.Get());

        //Across cluster boundry
        TEST_ASSERT_EQUAL_INT(0, sut.Seek(40 * 1024 + 8));
        TEST_ASSERT_EQUAL_INT('i', sut.Get());
    }

    static void test_GetClusterSize(void) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(32768, sut.GetClusterSize(), "SD card should be formatted in 32K clusters.");
    }

    static void test_getNextFileFindsAllFiles(void) {
        //TODO(SRLM): Does this assume a that there are files on the disk at this point?
        const int MAXIMUM_FILE_COUNT = 100;
        int count;
        for (count = 0; count < MAXIMUM_FILE_COUNT; count++) {

            char filename [13];
            sut.OpenRootDirectory();
            if (sut.NextFile(filename) != true) {
                break;
            }
            sut.Open(filename, 'd');
        }

        TEST_ASSERT_TRUE(count != MAXIMUM_FILE_COUNT);

    }

    static void test_getNextFileFindsCorrectFiles(void) {


        const int FILECOUNT = 3;

        //Note: filenames must be uppercase!
        const char * filenames[FILECOUNT];
        filenames[0] = "NEXTA.TXT";
        filenames[1] = "NEXTB.TXT";
        filenames[2] = "NEXTC.TXT";

        bool filenameFound[FILECOUNT];
        for (int filenameFoundI = 0; filenameFoundI < FILECOUNT; filenameFoundI++) {
            filenameFound[filenameFoundI] = false;
        }


        sut.Open(filenames[0], 'w');
        sut.Put('A');

        sut.Open(filenames[1], 'w');
        sut.Put('B');

        sut.Open(filenames[2], 'w');
        sut.Put('C');

        sut.OpenRootDirectory();

        char nextFilename[13];
        while (sut.NextFile(nextFilename) == true) {
            int i;
            for (i = 0; i < FILECOUNT; i++) {
                if (strcmp(filenames[i], nextFilename) == 0) {
                    TEST_ASSERT_FALSE_MESSAGE(filenameFound[i], "Should not already be found.");
                    filenameFound[i] = true;
                    break;
                }
            }
            TEST_ASSERT_TRUE_MESSAGE(i != FILECOUNT, "Unmatched filename!");
        }

        for (int i = 0; i < FILECOUNT; i++) {
            TEST_ASSERT_TRUE(filenameFound[i]);
        }
    }

    static void test_OpenRootDirMultipleTimesInARowReturnsAllFilesEveryTime(void) {
        const int FILECOUNT = 3;

        //Note: filenames must be uppercase!
        const char * filenames[FILECOUNT];
        filenames[0] = "NEXTA.TXT";
        filenames[1] = "NEXTB.TXT";
        filenames[2] = "NEXTC.TXT";

        bool filenameFound[FILECOUNT];

        sut.Open(filenames[0], 'w');
        sut.Put('A');

        sut.Open(filenames[1], 'w');
        sut.Put('B');

        sut.Open(filenames[2], 'w');
        sut.Put('C');


        for (int iterationsThroughRoot = 0; iterationsThroughRoot < 5; iterationsThroughRoot++) {
            //printf("\r\nIteration: %i", iterationsThroughRoot);

            for (int filenameFoundI = 0; filenameFoundI < FILECOUNT; filenameFoundI++) {
                filenameFound[filenameFoundI] = false;
            }

            sut.OpenRootDirectory();

            char nextFilename[13];
            while (sut.NextFile(nextFilename) == true) {
                int i;
                for (i = 0; i < FILECOUNT; i++) {
                    if (strcmp(filenames[i], nextFilename) == 0) {
                        TEST_ASSERT_FALSE_MESSAGE(filenameFound[i], "Should not already be found.");
                        filenameFound[i] = true;
                        break;
                    }
                }
                TEST_ASSERT_TRUE_MESSAGE(i != FILECOUNT, "Unmatched filename!");
            }

            for (int i = 0; i < FILECOUNT; i++) {
                TEST_ASSERT_TRUE(filenameFound[i]);
            }
        }
    }

    static void test_GetFilesizeSmall(void) {
        const char filename[] = "FILESIZE.TXT";
        const char content[] = "Hello";
        sut.Open(filename, 'w');
        sut.Put(content);
        sut.Close();
        sut.Open(filename, 'r');
        TEST_ASSERT_EQUAL_INT(strlen(content), sut.GetFilesize());
        sut.Close();
    }

    static void test_GetFilesizeNothing(void) {
        const char filename[] = "EMPTY.TXT";
        sut.Open(filename, 'w');
        sut.Close();
        sut.Open(filename, 'r');
        TEST_ASSERT_EQUAL_INT(0, sut.GetFilesize());
        sut.Close();
    }

    static void test_GetFilesizeLotsOfContent(void) {
        const char filename[] = "LARGE.TXT";
        sut.Open(filename, 'w');

        const int kByteCount = 1024 * 128;

        for (int i = 0; i < kByteCount; i++) {
            sut.Put('A');
        }

        sut.Close();

        sut.Open(filename, 'r');

        TEST_ASSERT_EQUAL_INT(kByteCount, sut.GetFilesize());

        sut.Close();
    }

    static void test_GetFilesizeAfterReadingSome(void) {
        const char filename[] = "AFTER.TXT";
        const char content[] = "Some text to take up space";
        sut.Open(filename, 'w');
        sut.Put(content);
        sut.Close();
        sut.Open(filename, 'r');
        for (int i = 0; i < 5; i++) {
            sut.Get();
        }
        TEST_ASSERT_EQUAL_INT(strlen(content), sut.GetFilesize());
    }

    static void test_GetFilesizeAfterReadingPastEndOfFile(void) {
        const char filename[] = "AFTER.TXT";
        const char content[] = "Some text to take up space";
        sut.Open(filename, 'w');
        sut.Put(content);
        sut.Close();
        sut.Open(filename, 'r');
        while (sut.Get() != -1) {
        }

        TEST_ASSERT_EQUAL_INT(strlen(content), sut.GetFilesize());
    }
 
    
    static void test_NextFileGetFilesize(void){
        const char filename[] = "SOME.TXT";
        const char content[] = "Some text to write";
        sut.Open(filename, 'w');
        sut.Put(content);
        sut.Close();
        
        sut.OpenRootDirectory();
        
        int filesize = 0;
        char readFilename[12];
        TEST_ASSERT_TRUE(sut.NextFile(readFilename, filesize));
        TEST_ASSERT_EQUAL_INT(strlen(content), filesize);
        
    }
    
    static void test_NextFileGetModificationTime(void){
        const char filename[] = "SOME.TXT";
        const char content[] = "Some text to write";
        
        int year = 2013;
        int month = 11;
        int day = 16;
        int hour = 12;
        int minute = 59;
        int second = 48;
        
        sut.SetDate(year, month, day, hour, minute, second);
        
        sut.Open(filename, 'w');
        sut.Put(content);
        sut.Close();
        
        sut.OpenRootDirectory();
        
        int filesize;
        int year2, month2, day2, hour2, minute2, second2;
        char readFilename[12];
        TEST_ASSERT_TRUE(sut.NextFile(readFilename, filesize, year2, month2, day2, hour2, minute2, second2));
        TEST_ASSERT_EQUAL_INT(year, year2);
        TEST_ASSERT_EQUAL_INT(month, month2);
        TEST_ASSERT_EQUAL_INT(day, day2);
        TEST_ASSERT_EQUAL_INT(hour, hour2);
        TEST_ASSERT_EQUAL_INT(minute, minute2);
        TEST_ASSERT_EQUAL_INT(second, second2);
    }
    

};
 