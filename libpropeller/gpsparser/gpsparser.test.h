#include <string.h>
#include "unity.h"
#include "libpropeller/c++allocate/c++allocate.h"

#include "libpropeller/serial/serial.h"
#include "libpropeller/gpsparser/gpsparser.h"


// TODO(SRLM) Get rid of the hardcoded test pins.
const int kPIN_USER_1 = 18;
const int kPIN_USER_2 = 19;

/* Hardware:
All tests requires that pins 18 and 19 be connected by a resistor.
 */

GPSParser * sut;

class UnityTests {
public:

    static void setUp(void) {
        sut = new GPSParser();
        sut->Start(kPIN_USER_1, kPIN_USER_2, 9600);
    }

    static void tearDown(void) {
        delete sut;
        sut = NULL;
    }

    static char FillBuffer(const char letter) {
        sut->getSerial()->Put(letter);
        return letter;
    }

    static const char * FillBuffer(const char * string) {
        for (int i = 0; string[i] != '\0'; i++) {
            sut->getSerial()->Put(string[i]);
        }

        return string;
    }

    static void test_GetIncompleteString(void) {
        //const char * line = 
        FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.");
        TEST_ASSERT_EQUAL_INT(NULL, sut->Get());
    }

    static void test_GetCompleteString(void) {
        const char * line =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
        TEST_ASSERT_EQUAL_MEMORY(line, sut->Get(), strlen(line) - 2);
    }

    static void test_GetMultipleStringsNoWait(void) {
        const char * line0 =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
        const char * line1 =
                FillBuffer("$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32\r\n");
        const char * line2 =
                FillBuffer("$GPRMC,1825035232574374,N*46\r\n");

        TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(), strlen(line0) - 2);
        TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(), strlen(line1) - 2);
        TEST_ASSERT_EQUAL_MEMORY(line2, sut->Get(), strlen(line2) - 2);
    }

    static void test_NullAfterString(void) {
        const char * line =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");

        TEST_ASSERT_EQUAL_MEMORY(line, sut->Get(), strlen(line) - 2);
        TEST_ASSERT_EQUAL_INT(NULL, sut->Get());
    }

    static void test_GetPartialStringAtBeginning(void) {
        //const char * line0 = 
        FillBuffer(".00,N,0.00,K,N*32\r\n");
        const char * line1 =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");

        TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(), strlen(line1) - 2);
        TEST_ASSERT_EQUAL_INT(NULL, sut->Get());

    }

    static void test_DiscardPgtopSentences(void) {
        const char * line0 =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
        //const char * line1 =
        FillBuffer("$PGTOP,11,2*6E\r\n");
        FillBuffer("$PGTOP,11,2O23052*6E\r\n");
        FillBuffer("$PGTOP,11,240509172450125270*6E\r\n");
        FillBuffer("$PGTOP,[){*}){[*}*+{[)+*11,2*6E\r\n");
        const char * line2 =
                FillBuffer("$GPRMC,1825035232574374,N*46\r\n");

        TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(), strlen(line0) - 2);
        TEST_ASSERT_EQUAL_MEMORY(line2, sut->Get(), strlen(line2) - 2);
        TEST_ASSERT_TRUE(sut->Get() == NULL);
    }

    static void test_MaxBytesCutoff(void) {
        const char * line0 = FillBuffer("$GPRMC Dummy Sentence");
        char buffer[100];
        TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(buffer, strlen(line0)+1), strlen(line0));
    }

    static void test_MaxBytesCutoffWithMoreBytesInBuffer(void) {
        const char * line0 = FillBuffer("$GPRMC Dummy Sentence");
        FillBuffer("Some noise...");
        const char * line1 = FillBuffer("$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32\r\n");
        char buffer[100];
        TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(buffer, strlen(line0)+1), strlen(line0));
        TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(), strlen(line1) - 2);
    }

    static void test_SwitchBetweenBuffers(void) {
        const int kNmeaLength = 85;
        char bufferA[kNmeaLength];
        char bufferB[kNmeaLength];

        const char * line0 =
                FillBuffer("$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46\r\n");
        const char * line1 =
                FillBuffer("$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32\r\n");
        const char * line2 =
                FillBuffer("$GPRMC,1825035232574374,N*46\r\n");


        TEST_ASSERT_EQUAL_MEMORY(line0, sut->Get(bufferA, kNmeaLength), strlen(line0) - 2);
        TEST_ASSERT_EQUAL_MEMORY(line1, sut->Get(bufferB, kNmeaLength), strlen(line1) - 2);
        TEST_ASSERT_EQUAL_MEMORY(line2, sut->Get(bufferA, kNmeaLength), strlen(line2) - 2);

    }
};
/*
Sample GPS Data, no Fix:

$GPRMC,180252.087,V,,,,,0.00,0.00,290113,,,N*46
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180253.087,,,,,0,0,,,M,,M,,*4A
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPRMC,180253.087,V,,,,,0.00,0.00,290113,,,N*47
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180254.087,,,,,0,0,,,M,,M,,*4D
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPRMC,180254.087,V,,,,,0.00,0.00,290113,,,N*40
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
$PGTOP,11,2*6E
$GPGGA,180255.087,,,,,0,0,,,M,,M,,*4C
$GPGSA,A,1,,,,,,,,,,,,,,,*1E
$GPGSV,1,1,00*79
$GPRMC,180255.087,V,,,,,0.00,0.00,290113,,,N*41
$GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32

 */
