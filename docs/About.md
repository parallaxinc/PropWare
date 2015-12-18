About {#About}
=====

Inspiration
-----------

I've drawn inspiration from a lot of different projects over the years, but there are three big ones that helped to form
%PropWare into what it is today more than anything else.

### Arduino

Arduino is the king of "easy" in the microcontroller world. Though they aren't the fastest, and they aren't the 
cheapest, they have a monopoly on documentation and users. I use www.arduino.cc as my benchmark for ease of use
and documentation standards.

### TI's TivaWare

My first introduction to a hardware abstraction layer was with TI's StellarisWare (now called TivaWare). It opened my
eyes to a new world where I would be able to develop a complex application on a microcontroller, accessing all of its
hardware peripherals, without needing to pull out the datasheet every minute. The believe I can reproduce the same
power and flexibility on the Parallax Propeller.

### SRLM's %libpropeller

SRLM's %libpropeller was the wake up call that convinced me not only to use C++, but to use header files as much as 
possible. It has also given me a constant drive to add more peripherals and objects, simply for competition's sake, 
even before there were many users.

Overview
--------

%PropWare is a combination of three key pieces:
* Easy, configurable and fast C/C++ objects for the Propeller, including objects from
  * My own PropWare namespace
  * Parallax's C Simple Library
  * SRLM's libpropeller
* An easy-to-use and robust build system using CMake
* Thorough documentation and tutorials

Installation binaries make setup easy, and CMake functions make building complex applications quick. If you are an 
Eclipse user, CMake provides a simple command that will generate an Eclipse project (no need for a special Propeller 
plugin!). Students can take free advantage of JetBrains' new C++ IDE, ([CLion](http://www.jetbrains.com/clion/)), 
which supports CMake out-of-the-box and is the recommended tool for use with %PropWare applications.

Goals and Aspirations
---------------------

My goal is to provide both novice and expert users alike a single environment that approaches the simplicity of 
SimpleIDE or Arduino without compromising power or flexibility. I believe it is possible to create a single environment
capable of introducing first-time programmers to the Propeller, as well as satisfying a commercial-grade applications.
There are lots of ways %PropWare doesn't fit into this vision currently, but it is an active and ongoing effort to 
make improvements every day. 
