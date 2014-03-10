// Copyright 2013 SRLM and Red9

#include <propeller.h>
#include <stdio.h>

#include "unity.h"
#include "serial.h"
#include "c++-alloc.h"

/**
Required Hardware:
 pins @a rxpin and @a txpin connected by a ~10kOhm resistor
 pins @a rtspin and @a cts pin connected by a ~10kOhm resistor
 */

const int rxpin = 18;
const int txpin = 19;

const int baud = 460800;

const int ctspin = 20; //Input to Propeller
const int rtspin = 21; //Output from Propeller (currently not used by driver, but driven.

const int MAXTIME = 10; //maximum time (in milliseconds) to wait for GetCCheck for tests. Prevents hangups.

Serial sut;

class UnityTests {
public:

    static void setUp(void) {
        sut.Start(rxpin, txpin, baud);
    }

    static void tearDown(void) {
        sut.Stop();
    }

    // -----------------------------------------------------------------------------

    static void test_PinsConnected(void) {
        sut.Stop();


        DIRA |= 1 << rtspin;
        DIRA &= ~(1 << ctspin);

        OUTA |= 1 << rtspin;
        TEST_ASSERT_TRUE((INA & (1 << ctspin)) != 0);

        OUTA &= ~(1 << rtspin);
        TEST_ASSERT_TRUE((INA & (1 << ctspin)) == 0);



        DIRA |= 1 << txpin;
        DIRA &= ~(1 << rxpin);

        OUTA |= 1 << txpin;
        TEST_ASSERT_TRUE((INA & (1 << rxpin)) != 0);

        OUTA &= ~(1 << txpin);
        TEST_ASSERT_TRUE((INA & (1 << rxpin)) == 0);
    }

    static void test_Start(void) {
        sut.Stop();
        int32_t result = sut.Start(rxpin, txpin, baud);
        TEST_ASSERT_TRUE(result);
    }

    static void test_SetsPinDirectionsCorrectly(void) {
        sut.Stop();

        // Swap the expected direction of pins.
        DIRA |= 1 << rxpin;
        DIRA &= ~(1 << txpin);

        sut.Start(rxpin, txpin, baud);

        const char value = 'A';
        sut.Put(value);
        TEST_ASSERT_EQUAL_INT(value, sut.Get(MAXTIME));
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

    static void test_StopStopsCog(void) {
        int beforeCount = help_CountNumberOfFreeCogs();
        sut.Stop();
        TEST_ASSERT_EQUAL_INT(beforeCount + 1, help_CountNumberOfFreeCogs());
    }

    static void test_DestructorCallsStop(void) {
        sut.Stop();

        int beforeCount = help_CountNumberOfFreeCogs();
        {
            Serial temp;
            temp.Start(rxpin, txpin, baud);
            TEST_ASSERT_EQUAL_INT(beforeCount - 1, help_CountNumberOfFreeCogs());
        }

        TEST_ASSERT_EQUAL_INT(beforeCount, help_CountNumberOfFreeCogs());
    }

    // -----------------------------------------------------------------------------
    // Single or Few Character Checks
    // -----------------------------------------------------------------------------

    static void test_PutCGetC(void) {
        sut.Put('a');
        TEST_ASSERT_EQUAL_INT('a', sut.Get(MAXTIME));
    }

    static void test_PutCGetCLowerByteBound(void) {
        sut.Put('\0');
        TEST_ASSERT_EQUAL_INT(0, sut.Get(MAXTIME));
    }

    static void test_PutCGetCUpperByteBound(void) {
        sut.Put(255);
        TEST_ASSERT_EQUAL_INT(255, sut.Get(MAXTIME));
    }

    static void test_GetCCheckNoPutC(void) {
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
    }

    static void test_GetCCheck(void) {
        sut.Put('G');
        TEST_ASSERT_EQUAL_INT('G', sut.Get(MAXTIME));
    }

    static void test_GetCCheckNoWait(void) {
        sut.Put('T');
        TEST_ASSERT_EQUAL_INT('T', sut.Get(1));
    }

    static void test_GetCCheckTimeTimeout(void) {
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(1));
    }

    static void test_GetCValidateTimeoutDuration(void) {

        const int kTimeoutDuration = 10 * 1000;
        const int kMaximumTimeoutDelta = 100;

        for (int i = 0; i < 10; i++) {
            unsigned int start_CNT = CNT;
            sut.Get(kTimeoutDuration / 1000);
            unsigned int actual_timeout_duration = (CNT - start_CNT) / (CLKFREQ / 1000000);
            TEST_ASSERT_INT_WITHIN(kMaximumTimeoutDelta, kTimeoutDuration, actual_timeout_duration);
        }


    }

    static void test_RxTxNoExtraTxChars(void) {
        sut.Put('z');
        sut.Get(MAXTIME);
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
    }

    // -----------------------------------------------------------------------------
    // String and Long Buffer Tests
    // -----------------------------------------------------------------------------

    static void test_PutBufferNoPointer(void) {
        TEST_ASSERT_EQUAL_INT(0, sut.PutFormatted((char *) NULL));
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));
    }

    static void test_PutS(void) {
        TEST_ASSERT_EQUAL_INT(5, sut.PutFormatted("Hello"));
        TEST_ASSERT_EQUAL_INT('H', sut.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('e', sut.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('l', sut.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('l', sut.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT('o', sut.Get(MAXTIME));
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME));



    }

    // -----------------------------------------------------------------------------
    // Extra Functions Test
    // -----------------------------------------------------------------------------

    static void test_GetFlush(void) {
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty before test
        sut.Put('1');
        waitcnt(CLKFREQ / 100 + CNT); //In LMM mode, it's too fast to flush...
        sut.GetFlush();
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty after flush
    }

    static void test_GetFlushEmptyBuffer(void) {
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty before test
        sut.GetFlush();
        TEST_ASSERT_EQUAL_INT(-1, sut.Get(MAXTIME)); //Empty after flush
    }



    // -----------------------------------------------------------------------------
    // Baud Checks
    // -----------------------------------------------------------------------------

    static void test_SetBaud(void) {
        TEST_ASSERT_TRUE(sut.SetBaud(9600));
    }

    static void test_SetBaudTooHigh(void) {
        TEST_ASSERT_EQUAL_INT(0, sut.SetBaud(1000000));
    }

    static void test_SetBaudToZero(void) {
        TEST_ASSERT_EQUAL_INT(0, sut.SetBaud(0));
    }

    static void test_SetBaudTransmitAfterBaudChange(void) {
        sut.SetBaud(9600);
        sut.Put('a');
        TEST_ASSERT_EQUAL_INT('a', sut.Get(MAXTIME));
    }

    static void test_Setbaudclock(void) {
        TEST_ASSERT_TRUE(sut.SetBaudClock(9600, 80000000));
    }

    
    // -----------------------------------------------------------------------------

    static void test_GetBuffer(void) {
        char string[] = "Hello World!";
        int size = 12;
        char inputBuffer[size + 1];
        inputBuffer[size] = 0; //null terminate
        sut.PutFormatted(string);
        TEST_ASSERT_EQUAL_INT(size, sut.Get(inputBuffer, size, MAXTIME));
        TEST_ASSERT_EQUAL_STRING(string, inputBuffer);
    }

    static void test_GetBufferString(void) {
        char string[] = "Hello World!\n";
        int size = 13;
        char buffer[50];
        sut.PutFormatted(string);
        TEST_ASSERT_EQUAL_INT(size, sut.Get(buffer, '\n'));
        TEST_ASSERT_EQUAL_STRING(string, buffer);
    }

    // -----------------------------------------------------------------------------

    static void transmitAlphabet(void * param) {
        // ~60 ms long
        for (int i = 'A'; i <= 'z'; i++) {
            sut.Put(i);
            waitcnt(CLKFREQ / 1000 + CNT);
        }


        cogstop(cogid());
    }

    static void test_CTSPinBasic(void) {
        sut.Stop();
        sut.Start(rxpin, txpin, baud, ctspin);

        int stacksize = sizeof (_thread_state_t) + sizeof (int)*8;
        int *stack = (int*) malloc(stacksize);
        cogstart(transmitAlphabet, NULL, stack, stacksize);

        waitcnt(CLKFREQ * 5 / 1000 + CNT); // Give it some time to transmit a few chars

        // Turn off transmission
        DIRA |= 1 << rtspin;
        OUTA |= 1 << rtspin;

        // Flush out buffer
        int current = 0;
        int last = current;
        while (current != -1) {
            last = current;
            current = sut.Get(MAXTIME);
        }

        //At this point, no more characters.

        // Turn on transmission
        OUTA &= ~(1 << rtspin);
        waitcnt(CLKFREQ / 10 + CNT); // Give it time to transmit the last of it's characters

        for (int i = last + 1; i <= 'z'; i++) {
            TEST_ASSERT_EQUAL_INT(i, sut.Get(MAXTIME));
        }

        TEST_ASSERT_EQUAL_INT(-1, sut.Get(0));
    }

    // -----------------------------------------------------------------------------

    static void test_PutBuffer(void) {
        char data [] = "Hello, long string!";
        const int length = strlen(data) + 1;

        char inputbuffer[length];

        TEST_ASSERT_EQUAL_INT(length, sut.Put(data, length));

        sut.Get(inputbuffer, length, MAXTIME);

        TEST_ASSERT_EQUAL_STRING(data, inputbuffer);

    }

    static void test_PutBufferNullTerminatedString(void) {
        char data [] = "Hello, long string!";
        const int length = strlen(data);

        char inputbuffer[length];

        TEST_ASSERT_EQUAL_INT(length, sut.Put(data));

        sut.Get(inputbuffer, length, MAXTIME);

        TEST_ASSERT_EQUAL_MEMORY(data, inputbuffer, length);
    }


    // -----------------------------------------------------------------------------

    static void test_GetCountEmpty(void) {
        TEST_ASSERT_EQUAL_INT(0, sut.GetCount());
    }

    static void test_GetCountOne(void) {
        sut.Put(32);
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_EQUAL_INT(1, sut.GetCount());
    }

    static void test_GetCountFew(void) {
        for (int i = 0; i < Serial::kBufferLength / 2; i++) {
            sut.Put(i);
        }
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_EQUAL_INT(Serial::kBufferLength / 2, sut.GetCount());
    }

    static void test_GetCountWrapAround(void) {
        for (int i = 0; i < Serial::kBufferLength - 1; i++) {
            sut.Put(i);
            sut.Get(i);
        }

        sut.Put(32);
        sut.Put(33);
        sut.Put(34);

        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_EQUAL_INT(3, sut.GetCount());
    }




};



















































