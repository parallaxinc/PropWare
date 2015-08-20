PropWare Documentation
======================

### What is %PropWare?
%PropWare is a combination of three key pieces:
* Easy, configurable and fast C++ objects for the Propeller (includes the C Simple Library from Parallax, libpropeller 
  by SRLM, and my own %PropWare namespace)
* An easy-to-use and robust build system using CMake (lots of examples for anyone not yet familiar)
* Thorough documentation regarding both source code and setup

My goal is to provide both novice and expert users alike a single environment that approaches the simplicity of 
SimpleIDE or Arduino without sacrificing a single ounce of power or flexibility. This is accomplished entirely with 
CMake and Python. An installation script is provided in Python that will set up your environment for you (including 
downloading dependencies) and CMake gives you a wonderfully easy way to build your binaries. If you are an Eclipse 
user, CMake provides a simple command that will generate an Eclipse project for you (no need for a special Propeller 
plugin!). JetBrains' new C++ IDE, ([CLion](http://www.jetbrains.com/clion/)), supports CMake out-of-the-box and is 
the recommended tool for use with %PropWare applications.

### Key Features
* More C++ for the Propeller
* Easy CMake build system
* Support for Eclipse, [CLion](http://www.jetbrains.com/clion/) and other full-featured IDEs
* Active development - I'm open to requests for new objects/peripherals
* MIT license

Getting Started
---------------

[Chapter 1: Installation](http://david.zemon.name/PropWare/md_docs_Ch1Install.html)<br />
[Chapter 2: CMake & Make Tutorial](http://david.zemon.name/PropWare/md_docs_Ch2CMakeTutorial.html)<br />
[Chapter 3: Run an example project](http://david.zemon.name/PropWare/md_docs_Ch3RunningExampleProjects.html)<br />
[Chapter 4: Create a PropWare Project](http://david.zemon.name/PropWare/md_docs_Ch4AppDeployment.html)<br />

Appendix
* [Appendix A: CMake for PropWare](http://david.zemon.name/PropWare/md_docs_AppACMakeForPropware.html)
* [Appendix B: C++ Crash Course](http://david.zemon.name/PropWare/md_docs_AppBCXXCrashCourse.html)
* [Appendix C: Limitations](http://david.zemon.name/PropWare/md_docs_AppCLimitations.html)
* [Appendix D: Troubleshooting](http://david.zemon.name/PropWare/md_docs_AppDTroubleshooting.html)

Developing and Contributing to %PropWare
---------------------------------------
%PropWare is developed by David Zemon.

Code contributions can be made by either 
[contacting David Zemon](http://david.zemon.name/professional/contact.shtml) or forking the repository and creating a  
pull request.

Useful links
------------
* %PropWare Links
  * [Home](http://david.zemon.name/PropWare)
  * [Installation](http://david.zemon.name/PropWare/md_docs_Ch1Install.html)
  * [Forum thread](http://forums.parallax.com/showthread.php/157005-FYI-PropWare-Complete-build-system-and-library-for-PropGCC)
  * [Issue Tracker](https://github.com/DavidZemon/PropWare/issues)
  * [Source code](https://github.com/DavidZemon/PropWare)
  * [Build Server](http://david.zemon.name:8111/project.html?projectId=PropWare&tab=projectOverview&guest=1)
* [Parallax forums](http://forums.parallax.com/forumdisplay.php/65-Propeller-1-Multicore-Microcontroller)
* [JetBrains CLion](http://www.jetbrains.com/clion/): Full-featured, cross-platform C/C++ IDE
* PropGCC: GCC compiler for Parallax Propeller
  * [Documentation](https://code.google.com/p/propgcc/)
  * [Source code](https://github.com/parallaxinc/propgcc)
  * GCC4 (recommended)
    * [Windows][1]
    * [Linux][2] 
    * [Mac][3]
    * [Raspberry Pi][4]
  * GCC5
    * [Windows][5]
    * [Linux][6] 
    * [Raspberry Pi][7]
* [CMake](http://www.cmake.org/): Cross-platform C/C++ build tool used by %PropWare
* [OBEX](http://obex.parallax.com/): Parallax Object Exchange
* [Omnia Creator](http://omniacreator.com/): IDE, build system, graphics for the Propeller
* [libpropeller](https://github.com/libpropeller/libpropeller): C++ Propeller GCC objects (included with %PropWare)
* [libpropelleruino](https://code.google.com/p/lib-propelleruino/): Arduino-compatible source for the Propeller

[1]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc4win32/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc4-win32.zip?guest=1
[2]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc4linuxX64/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc4-linux-x64.tar.gz?guest=1
[3]: http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz
[4]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc4rpi/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc4-rpi.tar.gz?guest=1
[5]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc5win32/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc5-win32.zip?guest=1
[6]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc5linuxX64/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc5-linux-x64.tar.gz?guest=1 
[7]: http://david.zemon.name:8111/repository/download/PropGCC5_Gcc5rpi/.lastSuccessful/propellergcc-alpha_v1_9_0-gcc5-rpi.tar.gz?guest=1
