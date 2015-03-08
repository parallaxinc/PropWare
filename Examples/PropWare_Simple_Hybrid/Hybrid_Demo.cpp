/**
 * @file    Hybrid_Demo.cpp
 *
 * @author  David Zemon
 *
 * If your existing project already uses some of the Simple libraries (such as
 * `print` or `printi`), but you wish to use the `PropWare::Printer` class for
 * formatted printing, you may want to define a class structure like the
 * following:
 */

#include <PropWare/printcapable.h>
#include <simple/simpletext.h>
#include <PropWare/printer/printer.h>

class SimplePrinter : public PropWare::PrintCapable {
    public:
        SimplePrinter () {}

        virtual void put_char (const char c) {
            ::putChar(c);
        }

        virtual void puts (char const string[]) {
            ::putStr(string);
        }
};

int main () {
    SimplePrinter           mySimpleCompatiblePrinter;
    const PropWare::Printer myPrinter(&mySimpleCompatiblePrinter);

    putStr("Hello from the Simple function!\n");
    myPrinter.puts("Hello from PropWare's Printer!" CRLF);
    myPrinter.print("Hello from yet another Printer function!" CRLF);
    myPrinter.printf("All methods have their own merrits. Choose one that works well for you." CRLF);
    myPrinter.println("Printer::println() can be handy if you just want to print a single string");
    myPrinter << "For lovers of C++ streams, you can even use the << operator!" CRLF;

    return 0;
}


