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

#include <PropWare/hmi/output/printcapable.h>
#include <PropWare/hmi/output/printer.h>
#include <simpletext.h>

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

/**
 * @example     Hybrid_Demo.cpp
 *
 * Use PropWare's PropWare::Printer interface for easy formatting of text, but use Simple's serial driver. This
 * combination allows for easy object-oriented programming while still using the Parallax-authored serial driver.
 *
 * @include PropWare_Simple_Hybrid/CMakeLists.txt
 */
int main () {
    SimplePrinter           mySimpleCompatiblePrinter;
    const PropWare::Printer myPrinter(mySimpleCompatiblePrinter);

    putStr("Hello from the Simple function!\n");
    myPrinter.puts("Hello from PropWare's Printer!\n");
    myPrinter.print("Hello from yet another Printer function!\n");
    myPrinter.printf("All methods have their own merits. Choose one that works well for you.\n");
    myPrinter.println("Printer::println() can be handy if you just want to print a single string");
    myPrinter << "For lovers of C++ streams, you can even use the << operator!\n";

    return 0;
}


