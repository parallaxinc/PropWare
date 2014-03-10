// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "unity.h"

class UnityTests{
public:
	static void setUp(void){

	}

	static void tearDown(void){
		for(int i = 1; i < 8; ++i){
			cogstop(i);
		}
	}

	// -----------------------------------------------------------------------------
	// Test Shift operations. Right shift is compiler dependent: it can be either
	// arithemetic right shift or logical right shift. It usually (and the tests 
	// confirm this) depends on the variable:
	// unsigned -> use logical right shift
	// signed   -> use arithmetic right shift
	// -----------------------------------------------------------------------------

	static void test_ShiftLeft(void){
		volatile int x = 1;
		x = x << 2;
		TEST_ASSERT_EQUAL_INT(4, x);
	}

	static void test_ShiftRightNegative(void){
		volatile int x = -8;
		x = x >> 1;
		TEST_ASSERT_EQUAL_INT(-4, x);
	}

	static void test_ShiftRightUnsignedNumber(void){
		volatile unsigned int x = 0xFFFFFFFF;
		x = x >> 4;
		TEST_ASSERT_EQUAL_HEX32(0x0FFFFFFF, x);
	}


	static void test_ShiftRightSignedPositiveHighBitSet(void){
		volatile signed int x = 0xFfffFfff;
		x = x >> 16;
		TEST_ASSERT_EQUAL_HEX32(0xFfffFfff, x);
	}

	static void test_ShiftRightSignedPositiveHighBitNotSet(void){
		volatile signed int x = 0xFfff;
		x = x >> 8;
		TEST_ASSERT_EQUAL_HEX32(0xFf, x);
	}

	static void test_LeftShiftUnsigned(void){
		volatile unsigned int x = 0xF;
		x = x << 16;
		TEST_ASSERT_EQUAL_HEX32(0xF0000, x);
	}

	// -----------------------------------------------------------------------------
	// Propeller.h lock tests
	// Suggested rules for locks: Propeller Manual v1.2 pg 123
	// -----------------------------------------------------------------------------

	// @WARNING for all locks: Must lockclr the lock after setting it!



	static void test_lockretReturnsLock(void){
		int num_first = locknew();
		lockret(num_first);
		int num_second = locknew();
		TEST_ASSERT_EQUAL_INT(num_first, num_second);
		lockret(num_second);
	}

	static void test_locknewFirstLockIs1(void){
		int num = locknew();
	
		TEST_ASSERT_EQUAL_INT(1, num);
	
		lockret(num);
	}

	static void test_locknewSevenAvailableLocks(void){
		for(int i = 1; i < 8; i++){
			TEST_ASSERT_EQUAL_INT(i, locknew());
		}
	
		TEST_ASSERT_EQUAL_INT(-1, locknew());
	
		for(int i = 1; i < 8; i++){
			lockret(i);
		}
	}
	
	static void test_locksetAndlockclr(void){
		int lock = locknew();
		TEST_ASSERT_TRUE(lock != -1);
		
		TEST_ASSERT_FALSE(lockset(lock));
		TEST_ASSERT_TRUE(lockset(lock));
	
		lockclr(lock);
		lockret(lock);
	}

	static void test_ReturnSetLock(void){
		int lock = locknew();
		TEST_ASSERT_TRUE(lock != -1);
	
		lockset(lock);
		lockclr(lock);
		lockret(lock);
		int lock2 = locknew();
		TEST_ASSERT_EQUAL_INT(lock, lock2);
	
		lockret(lock2);

	}

	static void test_locksetIsEqualToCTrueFalseConstant(void){
		int lock = locknew();
		TEST_ASSERT_TRUE(lock != -1);

		//lockclr(lock);
	
		TEST_ASSERT_TRUE(lockset(lock) == false);
		TEST_ASSERT_TRUE(lockset(lock) != false);
	
		lockclr(lock);
		lockret(lock);

	}

	static void test_lockretReturnOrderDoesntMatter(void){
		int lockA = locknew();
		int lockB = locknew();
		
		// Sanity Check
		TEST_ASSERT_EQUAL_INT(lockB-1, lockA);
		
		lockret(lockA);
		int lockC = locknew();
		
		TEST_ASSERT_EQUAL_INT(lockB-1, lockC);
		
		lockret(lockA);
		lockret(lockC);
	}

	// -----------------------------------------------------------------------------
	// Type sizes
	// -----------------------------------------------------------------------------
	static void test_SizeofInt(void){
		TEST_ASSERT_EQUAL_INT(4, sizeof(int));
	}

	static void test_SizeofUnsignedInt(void){
		TEST_ASSERT_EQUAL_INT(4, sizeof(unsigned int));
	}

	static void test_SizeofChar(void){
		TEST_ASSERT_EQUAL_INT(1, sizeof(char));
	}

	static void test_SizeofUnsignedChar(void){
		TEST_ASSERT_EQUAL_INT(1, sizeof(unsigned char));
	}

	static void test_SizeofBool(void){
		TEST_ASSERT_EQUAL_INT(1, sizeof(bool));
	}

	static void test_SizeofShort(void){
		TEST_ASSERT_EQUAL_INT(2, sizeof(short));
	}

	static void test_SizeofUnsignedShort(void){
		TEST_ASSERT_EQUAL_INT(2, sizeof(unsigned short));
	}

	static void test_SizeofIntPointer(void){
		TEST_ASSERT_EQUAL_INT(4, sizeof(int *));
	}

	static void test_SizeofShortPointer(void){
		TEST_ASSERT_EQUAL_INT(4, sizeof(short *));
	}

	static void test_SizeofCharPointer(void){
		TEST_ASSERT_EQUAL_INT(4, sizeof(char *));
	}

	// -----------------------------------------------------------------------------

	static void test_WhatIsTrue(void){
		TEST_ASSERT_EQUAL_INT(1, true);
	}

	static void test_WhatIsFalse(void){
		TEST_ASSERT_EQUAL_INT(0, false);
	}

	static void test_OnlyTrueEqualsTrue(void){
		TEST_ASSERT_TRUE(2 != true);
	}

	static void test_AnyNonZeroNumberIsTrue(void){
		TEST_ASSERT_TRUE(1);
		TEST_ASSERT_TRUE(2);
		TEST_ASSERT_TRUE(200);
		TEST_ASSERT_TRUE(-1);
		TEST_ASSERT_TRUE(-200);
	}

	static void test_ZeroIsFalse(void){
		TEST_ASSERT_FALSE(0);
	}

	static void test_BooleanAndIsSameAsBitwiseAnd(void){
		TEST_ASSERT_TRUE((true  && false) == (true  & false));
		TEST_ASSERT_TRUE((false && true ) == (false & true ));
		TEST_ASSERT_TRUE((false && false) == (false & false));
		TEST_ASSERT_TRUE((true  && true ) == (true  & true ));
	}


	// -----------------------------------------------------------------------------

	static void test_WritingAnIntToACharWillTruncate(void){
		char data []= {0,0,0,0,0,0,0,0};
		data[4] = 0xFFFFFFFF;
		TEST_ASSERT_EQUAL_HEX8(0, data[0]);
		TEST_ASSERT_EQUAL_HEX8(0, data[1]);
		TEST_ASSERT_EQUAL_HEX8(0, data[2]);
		TEST_ASSERT_EQUAL_HEX8(0, data[3]);
		TEST_ASSERT_EQUAL_HEX8(0xFF, data[4]);
		TEST_ASSERT_EQUAL_HEX8(0, data[5]);
		TEST_ASSERT_EQUAL_HEX8(0, data[6]);
		TEST_ASSERT_EQUAL_HEX8(0, data[7]);
	}

	static void test_InitializingACharWith16BitsWillTruncate(void){
		char data = 0xABCD;
		TEST_ASSERT_EQUAL_INT(0xCD, data);
	}
	
	// -----------------------------------------------------------------------------


	static void FunctionThatEnds(void){
		waitcnt(CLKFREQ/10 + CNT);
	}

	static void FunctionThatEndsWithCogstop(void){
		waitcnt(CLKFREQ/10 + CNT);
		cogstop(cogid());
	}

	static void test_WhatHappensWhenACogReachesTheEnd(void){
		int stacksize = sizeof(_thread_state_t)+sizeof(int)*3 + sizeof(int)*100;
	
		int * stackA = (int*) malloc(stacksize);		
		int cogA = cogstart(FunctionThatEnds, NULL, stackA, stacksize);
	
		int * stackB = (int*) malloc(stacksize);		
		int cogB = cogstart(FunctionThatEnds, NULL, stackB, stacksize);
	
		waitcnt(CLKFREQ/10 + CNT);
	
		int * stackC = (int*) malloc(stacksize);		
		int cogC = cogstart(FunctionThatEnds, NULL, stackC, stacksize);

		TEST_ASSERT_EQUAL_INT(1, cogA);
		TEST_ASSERT_EQUAL_INT(2, cogB);
		TEST_ASSERT_EQUAL_INT(3, cogC);
	
		//cleanUp
		for(int i = 1; i < 8; ++i){
			cogstop(i);
		}	
	
		free(stackA);
		free(stackB);
		free(stackC);
	}
		
	static void test_WhatHappensWhenACogReachesTheEndWithCogstop(void){
		int stacksize = sizeof(_thread_state_t)+sizeof(int)*3 + sizeof(int)*100;
	
		int * stackA = (int*) malloc(stacksize);		
		int cogA = cogstart(FunctionThatEndsWithCogstop, NULL, stackA, stacksize);
	
		int * stackB = (int*) malloc(stacksize);		
		int cogB = cogstart(FunctionThatEnds, NULL, stackB, stacksize);
	
		waitcnt(CLKFREQ/5 + CNT);
	
		int * stackC = (int*) malloc(stacksize);		
		int cogC = cogstart(FunctionThatEndsWithCogstop, NULL, stackC, stacksize);

		TEST_ASSERT_EQUAL_INT(1, cogA);
		TEST_ASSERT_EQUAL_INT(2, cogB);
		TEST_ASSERT_EQUAL_INT(1, cogC);
	
		//cleanUp
		for(int i = 1; i < 8; ++i){
			cogstop(i);
		}	
		free(stackA);
		free(stackB);
		free(stackC);
	}

	// -----------------------------------------------------------------------------

	static void test_64bitIntegerBasic(void){
		volatile int64_t a = 0x1;
	
		a = a << 32;
		a = a >> 32;
		TEST_ASSERT_EQUAL_INT(0x1, a);
	}

	static void test_64bitIntegerAdd(void){
		volatile int64_t a = 0x100000000;
		a = a + a; //== 0x200000000;

	#ifndef UNITY_SUPPORT_64	
		a = a >> 32; //== 0x2;
		TEST_ASSERT_EQUAL_INT(0x2, a);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x200000000, a);
	#endif
	}

	static void test_64bitIntegerSubtract(void){
		volatile int64_t a = 0x500000000;
		a = a - 0x100000000;
	
	#ifndef UNITY_SUPPORT_64
		a = a >> 32;
		TEST_ASSERT_EQUAL_INT(0x4, a);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x400000000, a);
	#endif
	}

	static void test_64bitIntegerMultiply(void){
		volatile int64_t a = 0x3;
		volatile int64_t b = 0x300000000;
		a = a * b;
	
	#ifndef UNITY_SUPPORT_64
		a = a >> 32;	
		TEST_ASSERT_EQUAL_INT(0x9, a);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x900000000, a);
	#endif
	}

	static void test_64bitIntegerDivide(void){
		volatile int64_t a = 0x3;
		volatile int64_t b = 0x900000000;
		a = b / a;

	#ifndef UNITY_SUPPORT_64
		a = a >> 32;
		TEST_ASSERT_EQUAL_INT(0x3, a);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x300000000, a);
	#endif
	}

	static void test_64bitIntegerAddSpeed(void){

		//Nothing
		int startCNT = CNT;
		int endCNT = CNT;
		int nothingDelta = endCNT - startCNT;

		//64 bit
		volatile int64_t a64 = 0x500000000;
		startCNT = CNT;
		a64 = a64 + a64;
		endCNT = CNT;
		UnityPrint("64bit add deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');

		//32 bit
		volatile int32_t a32 = 0x50000;
		startCNT = CNT;
		a32 = a32 + a32;
		endCNT = CNT;
		UnityPrint("32bit add deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');
	
	
		TEST_ASSERT_EQUAL_INT(0xA0000, a32);	
	#ifndef UNITY_SUPPORT_64
		a64 = a64 >> 32;
		TEST_ASSERT_EQUAL_INT(0xA, a64);
	#else
		TEST_ASSERT_EQUAL_HEX64(0xA00000000, a64);
	#endif


	}


	static void test_64bitIntegerDivideSpeed(void){
		//Nothing
		int startCNT = CNT;
		int endCNT = CNT;
		int nothingDelta = endCNT - startCNT;


		//64 bit
		volatile int64_t a64 = 0x600000000;
		startCNT = CNT;
		a64 = a64 / 3;
		endCNT = CNT;	
		UnityPrint("64bit divide deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');
	
		//32 bit
		volatile int32_t a32 = 0x60000;
		startCNT = CNT;
		a32 = a32 / 3;
		endCNT = CNT;
		UnityPrint("32bit divide deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');
	
		TEST_ASSERT_EQUAL_INT(0x20000, a32);
	#ifndef UNITY_SUPPORT_64
		a64 = a64 >> 32;
		TEST_ASSERT_EQUAL_INT(0x2, a64);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x200000000, a64);
	#endif


	}


	static void test_64bitIntegerMultiplySpeed(void){
	
		//Nothing
		int startCNT = CNT;
		int endCNT = CNT;
		int nothingDelta = endCNT - startCNT;
	
		//64 bit
		volatile int64_t a64 = 0x600000000;
		startCNT = CNT;
		a64 = a64 * 0x30;
		endCNT = CNT;
		UnityPrint("64bit multiply deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');
	
	
		//32 bit
		volatile int32_t a32 = 0x60000;
		startCNT = CNT;
		a32 = a32 * 0x30;
		endCNT = CNT;
		UnityPrint("32bit multiply deltaCNT == ");
		UnityPrintNumber(endCNT - startCNT - nothingDelta);
		UNITY_OUTPUT_CHAR('\n');
		
		TEST_ASSERT_EQUAL_HEX32(0x1200000, a32);
	#ifndef UNITY_SUPPORT_64
		a64 = a64 >> 32;
		TEST_ASSERT_EQUAL_HEX32(0x120, a64);
	#else
		TEST_ASSERT_EQUAL_HEX64(0x12000000000, a64);
	#endif


	}

	// -----------------------------------------------------------------------------

	static void test_FloatVariableToInt(void){
		// Get the bits of a float number into an int
		const float floatNum = 0.01f;
		int number = *(int *)&floatNum;
		TEST_ASSERT_EQUAL_HEX32(0x3C23D70A, number);
	}

	// -----------------------------------------------------------------------------

	static void test_SignedVsUnsignedComparison(void){
		int sA = 0xFfffFfff;
		int sB = 0x0fffFfff;
	
		TEST_ASSERT_TRUE(sB > sA);

		unsigned int uA = 0xFfffFfff;
		unsigned int uB = 0x0fffFfff;
	
		TEST_ASSERT_TRUE(uA > uB);
	}

	static void test_UnsignedReverseRolloverSubtraction(void){
		unsigned int A = 0xF;
		unsigned int B = 0x10;
		unsigned int result = A - B;
		TEST_ASSERT_EQUAL_HEX32(0xFfffFfff, result);
	

	}

	// -----------------------------------------------------------------------------


	static void test_nullptrIsTheSameAsNULL(void){
		TEST_ASSERT_TRUE(nullptr == NULL);
	}


};















