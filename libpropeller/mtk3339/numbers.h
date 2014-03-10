#ifndef LIBPROPELLER_NUMBERS_H_
#define LIBPROPELLER_NUMBERS_H_

#include <string.h>
#include <limits.h>

/** Convert between numbers and the various string representations.
 * 
 * @author SRLM
 */
class Numbers {
public:

    /** Converts a decimal number to it's string representation.
     * 
     * @warning This function modifies the given string.
     * @warning Converted integers must be in the range of –2,147,483,648 to
     *          2,147,483,647 (32 bit signed integer). This range is not checked.
     *
     * Note: Effectively the same as the C atoi.
     *
     * @param number         The 32 bit 2's complement number to convert.
     * @param result_string  The address to store the string representation.
     * @returns              The starting address of the null terminated string.
     */

    static char * Dec(int number, char result_string[] = NULL) {
        if (result_string == NULL) {
            result_string = internalBuffer;
        }
        //Source: http://www.cplusplus.com/articles/D9j2Nwbp/
        int i, sign;

        if ((sign = number) < 0) /* record sign */
            number = -number; /* make n positive */
        i = 0;
        do { /* generate digits in reverse order */
            result_string[i++] = number % 10 + '0'; /* get next digit */
        } while ((number /= 10) > 0); /* delete it */
        if (sign < 0)
            result_string[i++] = '-';
        result_string[i] = '\0';
        Reverse(result_string);

        return result_string;
    }

    /** Pad a number string with '0' to achieve the minimum number of digits.
     * 
     * @param resultString The input string to modify, and the destination of the modifications. Must have enough space for the maximum value.
     * @param minDigits The minimum number of digits. If the number is less than this it will be zero padded.
     * @param countSign If true the sign is counted as a digit.
     * @return The resultString.
     */
    static char * ZeroPad(char * resultString, int minDigits, bool countSign = false) {

        int numDigits = strlen(resultString);
        int digitStartIndex = 0;
        
        if (resultString[0] == '-' || resultString[0] == '+') {
            digitStartIndex = 1;
            if(countSign == false){
                numDigits--;
            }
        }

        if (numDigits < minDigits) {
            int difference = minDigits - numDigits;
            for (int i = numDigits - 1; i >= 0; i--) {
                resultString[i + difference + digitStartIndex] = resultString[i + digitStartIndex];
            }

            for (int i = 0; i < difference; i++) {
                resultString[i + digitStartIndex] = '0';
            }

            resultString[minDigits + digitStartIndex] = '\0';
        }
        return resultString;


    }

    /** Converts the string representation of a decimal number to it's value.
     * 
     * @param number     the string representation of a  base 10 number.
     *                   Valid characters are '-' (optional) followed by '0'
     *                   through '9'.
     * @param terminator optionally specify a termination character to end the
     *                   string.
     * @returns          the 32 bit value of the representation. If any 
     *                   characters are not valid numbers or the terminator, 
     *                   then returns INT_MIN.
     */
    static int Dec(const char * number, const char terminator = '\0') {
        int result = 0;
        int index = 0;
        bool isNegative = false;
        if (number[index] == '-') {
            isNegative = true;
            index++;
        }
        for (; number[index] != terminator; index++) {
            // 'Crash' if non-numeric character is found.
            if (number[index] < '0' || number[index] > '9') {
                return INT_MIN;
            }

            //       Shift left      Add 1's unit
            result = (result * 10) + (number[index] - '0');
        }

        if (isNegative) {
            return -result;
        } else {
            return result;
        }
    }

    /** Converts a hex number to it's string representation.
     *
     * @warning This function modifies the given string.
     * @warning Converted integers must be 32 bit integers. This range is not
     *          checked. 
     *
     * @param number  The 32 bit number to convert.
     * @param num_digits The number of Hex digits to print.
     * @param result_string  The address to store the string representation. Defaults to the internal buffer if NULL.
     * @returns  The starting address of the string pointer.
     */
    static char * Hex(int number, const int num_digits, char result_string[] = NULL) {
        if (result_string == NULL) {
            result_string = internalBuffer;
        }
        static const char characters[] = "0123456789ABCDEF";
        int i;
        for (i = 0; i < num_digits; i++) {
            result_string[i] = characters[number & 0xF];
            number >>= 4;
        }
        result_string[i] = 0;
        Reverse(result_string);
        return result_string;
    }

    /** Converts a binary number to it's string representation.
     *
     * @warning This function modifies the given string.
     * @warning Converted integers must be 32 bit integers. This range is not
     *          checked. 
     *
     * @param number  The 32 bit number to convert.
     * @param num_digits The number of binary digits to print.
     * @param result_string  The address to store the string representation. Defaults to the internal buffer if NULL.
     * @returns  The starting address of the string pointer.
     */
    static char * Bin(int number, int num_digits, char result_string[] = NULL) {
        if (result_string == NULL) {
            result_string = internalBuffer;
        }
        // Below is C++ version:
        static const char characters[] = "01";
        int i;
        for (i = 0; i < num_digits; i++) {
            result_string[i] = characters[number & 0b1];
            number >>= 1;
        }
        result_string[i] = 0;
        Reverse(result_string);
        return result_string;

    }

    /** Calculate the number of digits in the decimal representation of a number.
     * 
     * @returns the number of digits that a call to Dec(int) with @a n will produce.
     */
    static int DecDigits(int number) {
        int sign = 0;
        if (number < 0) {
            number = -number;
            sign = 1;
        }
        if (number < 100000) {
            // 5 or less
            if (number < 100) {
                // 1 or 2
                if (number < 10)
                    return 1 + sign;
                else
                    return 2 + sign;
            } else {
                // 3 or 4 or 5
                if (number < 1000)
                    return 3 + sign;
                else {
                    // 4 or 5
                    if (number < 10000)
                        return 4 + sign;
                    else
                        return 5 + sign;
                }
            }
        } else {
            // 6 or more
            if (number < 10000000) {
                // 6 or 7
                if (number < 1000000)
                    return 6 + sign;
                else
                    return 7 + sign;
            } else {
                // 8 to 10
                if (number < 100000000)
                    return 8 + sign;
                else {
                    // 9 or 10
                    if (number < 1000000000)
                        return 9 + sign;
                    else
                        return 10 + sign;
                }
            }
        }
    }

    /** Calculate the number of digits in the hexadecimal representation of a number.
     *     
     * @returns the number of digits that a call to Hex(int, ...) with @a n will produce if there are no leading 0s (ie, counts all the hex digits from left to right, starting with the first non-zero hex digit).
     */
    static int HexDigits(const int n) {
        if ((n & 0xFFFF) == n) {
            //4 or less
            if ((n & 0xFF) == n) {
                //2 or less
                if ((n & 0xF) == n)
                    return 1;
                else
                    return 2;
            } else
                //4 or 3
                if ((n & 0xFFF) == n)
                return 3;
            else
                return 4;

        } else {
            //5 or more
            if ((n & 0xFFFFFF) == n) {
                //6 or 5
                if ((n & 0xFFFFF) == n)
                    return 5;
                else
                    return 6;
            } else {
                if ((n & 0xFFFFFFF) == n)
                    return 7;
                else
                    return 8;
            }
        }
    }

    /** Reverse the order of a string's characters, in place.
     * 
     * @warning This function modifies the given string.
     * 
     * @param s zero terminated string to reverse
     * @returns the reversed string starting address
     */
    static char * Reverse(char s[]) {
        //Source: http://www.cplusplus.com/articles/D9j2Nwbp/
        int i, j;
        char c;

        for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
            c = s[i];
            s[i] = s[j];
            s[j] = c;
        }
        return s;
    }

private:

    /** The internal string buffer. Note: holds at most one string, so multiple
     * sequential calls will overwrite the buffer's previous contents.
     */
    static char internalBuffer[];
};

#ifdef SINGLE_TRANSLATION_UNIT
#include "numbers.cpp"
#endif

#endif // LIBPROPELLER_NUMBERS_H_
