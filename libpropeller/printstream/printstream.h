#ifndef LIBPROPELLER_PRINTSTREAM_H_
#define LIBPROPELLER_PRINTSTREAM_H_

#include "libpropeller/numbers/numbers.h"
#include "libpropeller/streaminterface/streaminterface.h"

#include <stdio.h>

template<class OutputType>
class PrintStream {
public:
    OutputStream<OutputType> * os;

    PrintStream(OutputStream<OutputType> * os) {
        this->os = os;
    }

    PrintStream(OutputType * ot) {
        this->os = ot;
    }

    /** Transmit a string (printf function-alike).
     *
     * This function is based on the requirements found on this page:
     * http://www.cplusplus.com/reference/cstdio/printf/
     * 
     * @warning This function does not implement the full printf functionality.
     * 
     * Formatters must follow the following prototype:
     * %[flags][width]specifier
     * The following flags are supported
     * - 0 (only with x,X,b,B). Sets padding to the 0 character instead of space.
     * This function supports the following specifiers:

    
     * - d or i : signed decimal integer. The width specifier will pad with 
     *          spaces (or 0s if the 0 flag is set) if necessary. If padding
     *          spaces then the sign, if any, will be adjacent to the digits. If
     *          padding with 0's then the sign will be the first character (ie,
     *          "%5d",-123 will produce " -123" while "%05d",-123 will produce
     *          "-0123".
     * - x or X : hexadecimal integer. All caps (with either). The width
     *          specifier will pad with spaces (or 0s if the 0 flag is set) if
     *          necessary
     * - c      : output a single character.
     * - s      : output a string of characters, 0 terminated.
     * - %      : output a % symbol.
     * 
     * Each specifier must have a matching typed optional argument.
     * 
     * Behavior is undefined if % is used without a specifier.
     * 
     * @param formatString the string to send, optionally with specifiers.
     * @param ...    additional arguments. Depending on the formatString string, the
     *                  function may expect a sequence of additional arguments.
     * @returns      on success, the total number of characters written. On
     *                  error, a negative number is returned.
     */
    int Format(const char * formatString, ...) {
        va_list list;
        va_start(list, formatString);
        int result = Format(formatString, list);
        va_end(list);
        return result;
    }

    /** Same as Format, but with an explicit va_list.
     * 
     * Intended as a work around for the limitation that variac arguments cannot 
     * be passed directly. In most cases you will not need to use this function.
     * 
     * @param formatString
     * @param list
     * @returns
     */
    int Format(const char * formatString, va_list list) {
        if (formatString == NULL) {
            return 0;
        }

        int bytesWritten = 0;

        for (int stringIndex = 0; formatString[stringIndex] != 0; stringIndex++) {

            if (formatString[stringIndex] == '%') {
                //Found formatter!
                stringIndex++;
                //Check for flags:
                bool padZero = false;
                int padAmount = 0;
                if (formatString[stringIndex] == '0') {
                    padZero = true;
                    stringIndex++;
                }
                if (formatString[stringIndex] >= '1' and formatString[stringIndex] <= '9') {
                    char paddingBuffer[5];
                    int paddingIndex = 0;


                    //Non freezing version.				
                    /*if (formatString[stringIndex] >= '0' and formatString[stringIndex] <= '9') {
                        paddingBuffer[paddingIndex++] = formatString[stringIndex++];
                        if (formatString[stringIndex] >= '0' and formatString[stringIndex] <= '9') {
                            paddingBuffer[paddingIndex++] = formatString[stringIndex++];
                            if (formatString[stringIndex] >= '0' and formatString[stringIndex] <= '9') {
                                paddingBuffer[paddingIndex++] = formatString[stringIndex++];
                                if (formatString[stringIndex] >= '0' and formatString[stringIndex] <= '9') {
                                    paddingBuffer[paddingIndex++] = formatString[stringIndex++];
                                }
                            }
                        }
                    }*/

                    //TO DO(SRLM): figure out what is happening with the freezing version.
                    //I think it freezes because of the CMM and fcache combination.
                    //Freezing version:				
                    while (formatString[stringIndex] >= '0' and formatString[stringIndex] <= '9') {
                        paddingBuffer[paddingIndex++] = formatString[stringIndex];
                        //printf("+%c+", formatString[stringIndex]);				
                        stringIndex++;
                    }
                    paddingBuffer[paddingIndex] = 0;
                    padAmount = Numbers::Dec(paddingBuffer);
                    //				printf("paddingBuffer[0] = %c\r\n", paddingBuffer[0]);
                    //				printf("paddingBuffer[1] = %c\r\n", paddingBuffer[1]);
                    //				printf("paddingIndex = %i\r\n", paddingIndex);
                    //				printf("padAmount: %i\r\n", padAmount);


                }



                if (formatString[stringIndex] == 0) {
                    // Throw away the '%' character if it's at the end of the string.
                    break;
                }
                if (formatString[stringIndex] == 'd' || formatString[stringIndex] == 'i') {
                    int number = va_arg(list, int);
                    int digits = Numbers::DecDigits(number);
                    if (padAmount > 0) {
                        if (padZero == true) {
                            os->Put(Numbers::ZeroPad(Numbers::Dec(number), padAmount, true) );
                        } else {
                            for (int i = padAmount - digits; i > 0; --i) {
                                os->Put(' ');
                            }
                            os->Put(Numbers::Dec(number));
                        }
                    }else{
                        os->Put(Numbers::Dec(number));
                    }

                    
                    bytesWritten += digits;
                } else if (formatString[stringIndex] == 'x' || formatString[stringIndex] == 'X') {
                    int number = va_arg(list, int);
                    int digits = Numbers::HexDigits(number);
                    if (padAmount > 0) {
                        for (int i = padAmount - digits; i > 0; --i) {
                            if (padZero) {
                                os->Put('0');
                            } else {
                                os->Put(' ');
                            }
                        }
                    }

                    os->Put(Numbers::Hex(number, digits));
                    bytesWritten += digits;
                } else if (formatString[stringIndex] == 'c') {
                    char character = (char) (va_arg(list, int));
                    os->Put(character);
                    bytesWritten++;
                } else if (formatString[stringIndex] == 's') {
                    char * string = (char *) (va_arg(list, int));
                    while (*string != 0) {
                        os->Put(*string++);
                        bytesWritten++;
                    }
                } else if (formatString[stringIndex] == '%') {
                    os->Put('%');
                    bytesWritten++;
                }

            } else {
                os->Put(formatString[stringIndex]);
                bytesWritten++;
            }
        }
        return bytesWritten;
    }

};

#endif // LIBPROPELLER_PRINTSTREAM_H_