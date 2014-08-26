PropWare v2.0 Documentation - Nightly
=====================================

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
### Microsoft Windows
I'm sorry to say, there is a bug in the CMake files and PropWare is not currently usable on any Microsoft Windows 
operating system. If you'd like to get ready for PropWare though, go ahead and download/install any version of Python
2.7+ from [Python's website](https://www.python.org/downloads/). You'll need it for the installation script.

### Linux
Most, if not all, Linux distributions ship with a version of Python, so don't worry about that. Simply download PropWare
in any way that you like (Git is recommended, but a 
[binary distribution](http://david.zemon.name/downloads/PropWare_Binaries/) from here will work just fine) and execute
the INSTALL.py script. If you downloaded PropWare to `/home/david/PropWare`, you would open a terminal and type:

    cd /home/david/PropWare/util
    python INSTALL.py

You might see some questions pop up - just answer them as your heart desires. Since I'm still very new to this, _please_
[let me know](mailto:david@zemon.name) if there is anything confusing about my wording. When you're all done, exit the
terminal. If you opted to set root environment variables, you'll need to reboot your computer before PropWare is usable.

### Mac OSX
I have _no_ idea what will happen on a Mac. I don't own a Mac. If you are reading this and you have a Mac, I'd greatly
appreciate you [getting in contact with me](mailto:david@zemon.name) to help me iron out any existing bugs in the 
installation script. You don't have to have any prerequisite knowledge to help - I just need something to test on.

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
