About PropWare
==============

Overview
--------

### What PropWare IS
PropWare is a combination of three key pieces:
* Easy, configurable and fast C++ objects for the Propeller (includes the C Simple 
  Library from Parallax, libpropeller by SRLM, and my own PropWare namespace)
* An easy-to-use and robust build system using CMake (lots of examples for anyone 
  not yet familiar)
* Thorough documentation regarding both source code and setup

### What PropWare is NOT
* PropWare is not a replacement for Parallax's Propeller C Learning System.
* PropWare does not and will not teach you how to write C or C++ applications.
* PropWare does not and will not teach you everything you need to know about the 
  Propeller and how to use it.
  
My goal is to provide both beginner and expert users alike a single environment 
that approaches the simplicity of SimpleIDE or Arduino without sacrificing a single 
ounce of power or flexibility. This is accomplished entirely with CMake and Python. 
An installation script is provided in Python that will set up your environment for 
you (including downloading dependencies) and CMake gives you a wonderfully easy way to 
build your binaries. If you are an Eclipse user, CMake provides a simple command that 
will generate an Eclipse project for you (no need for a special Propeller plugin!). 
JetBrains' new C++ IDE, 
([CLion](http://blog.jetbrains.com/objc/2014/02/jetbrains-c-ide-status-update-and-video-report/)),
supports CMake out-of-the-box and is the recommended tool for use PropWare applications.

### Key Features
* More C++ for the Propeller!
* CMake for the Propeller!
* Support for Eclipse, CLion and other full-featured IDEs!
* Active development - I'm open to requests for new objects/peripherals!
* MIT license on all source code files

Developing and Contributing to PropWare
---------------------------------------
PropWare is developed by David Zemon.

Code contributions can be made by either 
[contacting David Zemon](http://david.zemon.name/professional/contact.shtml) or forking 
the repository and creating a pull request.

Planned Enhancements/Known Issues
---------------------------------
* Add Spin language compatibility
* Add support for ActivityBot's [ADC](http://www.ti.com/lit/ds/symlink/adc124s021.pdf) and DAC
* SD Demo does not fit in LMM mode
* PropWare::SD class contains shell methods - these should be brought out to their class
* SD & FAT16/32 driver
  * Traverse directories (implies ability to open files/directories not in the current 
    directory)
* Add "SPIShiftIn_Multi", which will repeatedly shift in values and allow for the selection 
  between \*_Fast or normal using a parameter
