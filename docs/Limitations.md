Limitations {#Limitations}
===========

Spin
----

There are many different ways to use Spin on the Propeller. With the help of Propeller forums, I've compiled a 
[list](http://forums.parallax.com/showthread.php/157563-Combining-Spin-and-C) of all (most) of the different ways to 
use Spin and/or C/C++ within one project. Because there are so many different options, it will take quite some time 
before these options are available within %PropWare. The first step is to determine how a user selects which method to 
use. Only when that is finalized can work begin on the implementation. 

Currently, %PropWare's only support for Spin is extracting the DAT section of the file into a binary blob which can be
linked into the executable and invoked in a dedicated cog. This is helpful for extracting assembly drivers from Spin 
objects like FdSerial or FSRW.
