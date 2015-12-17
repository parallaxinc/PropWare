About {#About}
=====

%PropWare is a combination of three key pieces:
* Easy, configurable and fast C++ objects for the Propeller (includes the C Simple Library from Parallax, libpropeller 
  by SRLM, and my own PropWare namespace)
* An easy-to-use and robust build system using CMake (lots of examples for anyone not yet familiar)
* Thorough documentation regarding both source code and setup

My goal is to provide both novice and expert users alike a single environment that approaches the simplicity of 
SimpleIDE or Arduino without sacrificing a single ounce of power or flexibility. This is accomplished entirely with 
CMake and Python. An installation script is provided in Python that will set up your environment for you (including 
downloading dependencies) and CMake gives you a wonderfully easy way to build your binaries. If you are an Eclipse 
user, CMake provides a simple command that will generate an Eclipse project for you (no need for a special Propeller 
plugin!). JetBrains' new C++ IDE, ([CLion](http://www.jetbrains.com/clion/)), supports CMake out-of-the-box and is 
the recommended tool for use with %PropWare applications.
