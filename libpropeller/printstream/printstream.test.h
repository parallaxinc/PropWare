#include <stdio.h>

#include <propeller.h>

#include "unity.h"

#include "libpropeller/serial/serial.h"
#include "libpropeller/streaminterface/streaminterface.h"
#include "libpropeller/printstream/printstream.h"

#include "c++-alloc.h"


const int rxpin = 18;
const int txpin = 19;

const int baud = 460800;

const int ctspin = 20; //Input to Propeller
const int rtspin = 21; //Output from Propeller (currently not used by driver, but driven.

const int MAXTIME = 10; //maximum time (in milliseconds) to wait for GetCCheck for tests. Prevents hangups.


Serial input;
Serial output;
PrintStream<Serial> * sut;

class UnityTests {
public:
    static void setUp(void){
        
        input.Start(19,-1,baud);
        output.Start(31,18,baud);
        
        waitcnt(16000 + CNT);
        
        input.GetFlush();
        
        sut = new PrintStream<Serial>(&output);
    }
    
    static void tearDown(void){
        input.Stop();
        output.Stop();
        delete sut;
        sut = NULL;
    }
    
    static void test_PutBufferFormatted(void) {
        TEST_ASSERT_EQUAL_INT(3, sut->Format("abc"));



        TEST_ASSERT_EQUAL_INT('a', input.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('b', input.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('c', input.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT(-1, input.Get(MAXTIME));
    }
    
    // -----------------------------------------------------------------------------

    static void test_PutPrintfReturnsWrittenBytes(void) {
        int size = 30;
        char inputBuffer[size];

        TEST_ASSERT_EQUAL_INT(17, sut->Format("My:%i, Your:%i", 123, -531));
        TEST_ASSERT_EQUAL_INT(17, input.Get(inputBuffer, 17, MAXTIME));
        input.GetFlush();
    }

    static void test_PutPrintfBasic(void) {

        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }

        sut->Format("My number: %i.", 123);
        input.Get(inputBuffer, 15, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My number: 123.", inputBuffer);


    }

    static void test_PutPrintfMultipleIntegers(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }

        sut->Format("My:%i, Your:%i", 123, -531);
        input.Get(inputBuffer, 17, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:123, Your:-531", inputBuffer);
    }

    static void test_PutPrintfNoSpecifiers(void) {
        char string[] = "Hello, World.";
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }

        sut->Format(string);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING(string, inputBuffer);
    }

    static void test_PutPrintfHexSpecifiers(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%x, Your:%X", 0xAB, 0xCDE);
        input.Get(inputBuffer, 15, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:AB, Your:CDE", inputBuffer);
    }

    static void test_PutPrintfDecpad(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%10d", 1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:      1234", inputBuffer);
    }

    static void test_PutPrintfDecpadSmaller(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%2d", 1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:1234", inputBuffer);
    }
    
    static void test_PutPrintfDecpadZero(void){
        int size = 30;
        char inputBuffer[size];
        for(int i = 0; i < size; i++){
            inputBuffer[i] = 0;
        }
        sut->Format("My:%010d", 1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:0000001234", inputBuffer);
    }
    
    static void test_PutPrintfDecpadZeroNegative(void){
        int size = 30;
        char inputBuffer[size];
        for(int i = 0; i < size; i++){
            inputBuffer[i] = 0;
        }
        sut->Format("My:%010d", -1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:-000001234", inputBuffer);
    }

    static void test_PutPrinfHexpad(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%10x", 0x1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:      1234", inputBuffer);
    }

    static void test_PutPrinfHexpadTooSmall(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%2x", 0x1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:1234", inputBuffer);
    }

    static void test_PutPrinfHexpadZero(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%010x", 0x1234);
        input.Get(inputBuffer, 13, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:0000001234", inputBuffer);
    }

    static void test_PutPrintfChar(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%c", 'a');
        input.Get(inputBuffer, 4, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:a", inputBuffer);
    }

    static void test_PutPrintfString(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("My:%s", "World");
        input.Get(inputBuffer, 8, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("My:World", inputBuffer);
    }

    static void test_PutPrinfAllTogether(void) {
        int size = 30;
        char inputBuffer[size];
        for (int i = 0; i < size; i++) {
            inputBuffer[i] = 0;
        }
        sut->Format("%x%i%s%c%03x%4i", 0x23, 32, "hello", 'w', 0xF, 13);
        input.Get(inputBuffer, 17, MAXTIME);
        TEST_ASSERT_EQUAL_STRING("2332hellow00F  13", inputBuffer);
    }

    static void test_PutPrintfPercentSignAtEndOfStringDisappears(void) {
        const int size = strlen("Hello");
        char inputBuffer[size];
        sut->Format("Hello'%");
        output.Put("'");
        input.Get(inputBuffer, size, MAXTIME);
        TEST_ASSERT_EQUAL_MEMORY("Hello''", inputBuffer, size);

    }

    static void test_PutPrintfTwoPercentSigns(void) {
        const int size = strlen("Hello% ");
        char inputBuffer[size];
        sut->Format("Hello%% ");
        input.Get(inputBuffer, size, MAXTIME);
        TEST_ASSERT_EQUAL_MEMORY("Hello% ", inputBuffer, size);

    }
};