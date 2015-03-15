PropWare Documentation
======================

### What is PropWare?
PropWare is a combination of three key pieces:
* Easy, configurable and fast C++ objects for the Propeller (includes the C Simple Library from
  Parallax, libpropeller by SRLM, and my own PropWare namespace)
* An easy-to-use and robust build system using CMake (lots of examples for anyone not yet familiar)
* Thorough documentation regarding both source code and setup

My goal is to provide both novice and expert users alike a single environment
that approaches the simplicity of SimpleIDE or Arduino without sacrificing a
single ounce of power or flexibility. This is accomplished entirely with CMake
and Python. An installation script is provided in Python that will set up your
environment for you (including downloading dependencies) and CMake gives you a
wonderfully easy way to build your binaries. If you are an Eclipse user, CMake
provides a simple command that will generate an Eclipse project for you (no need
for a special Propeller plugin!). JetBrains' new C++ IDE, ([CLion](http://www.jetbrains.com/clion/)),
supports CMake out-of-the-box and is the recommended tool for use with PropWare
applications.

### Key Features
* More C++ for the Propeller!
* CMake for the Propeller!
* Support for Eclipse, CLion and other full-featured IDEs!
* Active development - I'm open to requests for new objects/peripherals!
* MIT license on all source code files

Getting Started
---------------

[Chapter 1: Installation](http://david.zemon.name/PropWare/md_docs_Install.html)<br />
[Chapter 2: CMake & Make Tutorial](http://david.zemon.name/PropWare/md_docs_CMakeTutorial.html)<br />
[Chapter 3: Run an example project](http://david.zemon.name/PropWare/md_docs_RunningExampleProjects.html)<br />
[Chapter 4: Create a PropWare Project](http://david.zemon.name/PropWare/md_docs_AppDeployment.html)<br />

Appendix
* [Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_CMakeForPropware.html)
* [Appendix B: C++ Crash Course](http://david.zemon.name/PropWare/md_docs_CXXCrashCourse.html)
* [Appendix C: Limitations](http://david.zemon.name/PropWare/md_docs_Limitations.html)
* [Appendix D: Troubleshooting](http://david.zemon.name/PropWare/md_docs_Troubleshooting.html)

Developing and Contributing to PropWare
---------------------------------------
PropWare is developed by David Zemon.

Code contributions can be made by either 
[contacting David Zemon](http://david.zemon.name/professional/contact.shtml) or forking the repository and creating a  
pull request.

Useful links
------------
* PropWare Links
  * [Home](http://david.zemon.name/PropWare)
  * [Issue Tracker](https://github.com/DavidZemon/PropWare/issues)
  * [Source code](https://github.com/DavidZemon/PropWare)
  * [PropWare binary](http://david.zemon.name/downloads/PropWare_Binaries/PropWare_current.zip)
  * [Forum thread](http://forums.parallax.com/showthread.php/157005-FYI-PropWare-Complete-build-system-and-library-for-PropGCC)
* [Parallax forums](http://forums.parallax.com/forumdisplay.php/65-Propeller-1-Multicore-Microcontroller)
* [JetBrains CLion](http://www.jetbrains.com/clion/): Full-featured, cross-platform C/C++ IDE
* [PropGCC](https://code.google.com/p/propgcc/): GCC compiler for Parallax Propeller
  * Binary distributions for [Windows](http://david.zemon.name/downloads/propellergcc-alpha_v1_9_0-i686-windows.zip),
    [Linux](http://david.zemon.name/downloads/propellergcc-alpha_v1_9_0_2408-i686-linux.tar.gz), and 
    [Mac](http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz)
* [CMake](http://www.cmake.org/): Cross-platform C/C++ build tool used by PropWare
* [OBEX](http://obex.parallax.com/): Parallax Object Exchange
* [Omnia Creator](http://omniacreator.com/): IDE, build system, graphics for the Propeller
* [libpropeller](https://github.com/libpropeller/libpropeller): C++ Propeller GCC objects (included with PropWare)
* [libpropelleruino](https://code.google.com/p/lib-propelleruino/): Arduino-compatible source for the Propeller
