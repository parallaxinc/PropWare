Appendix C: Limitations
=======================

[<< Appendix B: C++ Crash Course](http://david.zemon.name/PropWare/md_docs_AppBCXXCrashCourse.xhtml)<br />
[>> Appendix D: Troubleshooting](http://david.zemon.name/PropWare/md_docs_AppDTroubleshooting.xhtml)

Spin
----

There are many different ways to use Spin on the Propeller. With the help of Propeller forums, I've compiled a 
[list](http://forums.parallax.com/showthread.php/157563-Combining-Spin-and-C) of all (most) of the different ways to 
use Spin and/or C/C++ within one project. Because there are so many different options, it will take quite some time 
before these options are available within %PropWare. The first step is to determine how a user selects which method to 
use. Only when that is finalized can work begin on the implementation. 

FdSerial
--------

Until %PropWare supports Spin, FdSerial requires a work-around in %PropWare projects. The work-around is simple -
download [pst.dat](http://david.zemon.name/downloads/pst.dat) into your project and add the filename to your 
CMakeLists.txt file in the `project` command. For example, if your CMakeLists.txt file contains:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
add_executable(${PROJECT_NAME}
    ${PROJECT_NAME}
    my_driver)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

then you should modify it to read

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
add_executable(${PROJECT_NAME}
    ${PROJECT_NAME}
    my_driver
    pst.dat)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[<< Appendix B: C++ Crash Course](http://david.zemon.name/PropWare/md_docs_AppBCXXCrashCourse.xhtml)<br />
[>> Appendix D: Troubleshooting](http://david.zemon.name/PropWare/md_docs_AppDTroubleshooting.xhtml)
