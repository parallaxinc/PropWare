Creating a PropWare Application   {#projectCreation}
===============================

PropWare makes building and running applications very easy. Below are three different methods for building and
deploying a PropGCC application using PropWare.

From Eclipse (recommended)
--------------------------
-# If you have Eclipse, ensure the C/C++ Developer Tools (CDT) plugin is installed; For new Eclipse users, find
   `Eclipse IDE for C/C++ Developers` [here](http://www.eclipse.org/downloads/) and download the appropriate package
-# Create 
-# That's it! You now have Eclipse up and running with PropGCC and PropWare! All of its auto-completion, code
   highlighting, and quick and easy compilation! Projects can be compiled with Eclipse's built-in `Build Project`
   found in the `Project` menu. Though you can load your program to the Propeller via some Eclipse magic, I find
   it's a lot more trouble than it's worth. Simply open a terminal, navigate to &lt; project root&gt;/Debug and
   type either `make -f ../Makefile run` to test your application in RAM or `make -f ../Makefile install` to load
   your application to EEPROM

At the Terminal
---------------
-# Create a project directory and your source files
   * All C/C++ files must have an accompanying header file with a similar name; (i.e., if you have a file
     main.cpp, you must have main.h as well); You are welcome to edit common.mk to not search for header
     files; See the comment near the top of the file for my reasoning behind including it and decide for
     yourself
-# Create a Makefile for your project, following the template set under the Examples directory
-# Create a `Debug` folder in the project directory and run `make -f ../Makefile` within it
   * This Debug folder allows for seamless Eclipse integration; If you are certain you do not want to use
     Eclipse, you can run make wherever you choose. The remaining steps assume you are running from within
     the Debug directory
-# The `run` target for make will load your executable into the Propeller's RAM and begin running the application:
   `make -f ../Makefile run`
-# When you are ready to install the program to the Propeller's EEPROM, the `install` target will load your
   executable into both EEPROM and RAM and then begin execution: `make -f ../Makefile install`

Importing Example Projects into Eclipse
---------------------------------------
Coming soon!

This is a test and a reminder to use cool things later!!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class PropWare {
    public:
        PropWare () {
            printf("Hello, PropWare!\n");
        }
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
