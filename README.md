PropWare v2.0 Documentation - Nightly   {#mainpage}
=====================================

[TOC]

Overview
--------

### What PropWare IS
PropWare is a combination of three key pieces:
* Easy, configurable and fast C++ objects for the Propeller (includes the C Simple 
  Library from Parallax, libpropeller, and my own PropWare namespace)
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
When JetBrain's releases their new C++ IDE 
([CLion](http://blog.jetbrains.com/objc/2014/02/jetbrains-c-ide-status-update-and-video-report/)),
it will support CMake out-of-the-box and PropWare will be ready for it (I am currently 
using the EAP version).

### Key Features
* More C++ for the Propeller!
* CMake for the Propeller!
* Support for Eclipse, CLion and other full-featured IDEs!
* Active development - I'm open to requests for new objects/peripherals!
* MIT license on all source code files

Installation
------------
Coming soon! For now, just try running `INSTALL.py` from within the `util` directory.

Developing and Contributing to PropWare
---------------------------------------
PropWare is developed by David Zemon.

Contributions can be made by either 
[contacting David Zemon](http://david.zemon.name/professional/contact.shtml) or forking 
the repository and creating a pull request.

Useful links
------------

* [PropWare source code](https://github.com/SwimDude0614/PropWare)
* [PropWare binary distribution](http://david.zemon.name/downloads/PropWare_Binaries/)
* [Parallax forums](http://forums.parallax.com/forumdisplay.php/65-Propeller-1-Multicore-Microcontroller)
* [Simple IDE](http://learn.parallax.com/node/640): Light-weight IDE for Parallax Propeller 
* [PropGCC](https://code.google.com/p/propgcc/): GCC compiler for Parallax Propeller
* [OBEX](http://obex.parallax.com/): Parallax Object Exchange
* [libpropeller](https://github.com/libpropeller/libpropeller): C++ Propeller GCC objects
* [libpropelleruino](https://code.google.com/p/lib-propelleruino/): Arduino-compatible 
  source for the Propeller

TODO
----
* SD & FAT16/32 driver
  * Traverse directories (implies ability to open files/directories not in the current 
    directory)
* Add "SPIShiftIn_Multi", which will repeatedly shift in values and allow for the selection 
  between \*_Fast or normal using a parameter
