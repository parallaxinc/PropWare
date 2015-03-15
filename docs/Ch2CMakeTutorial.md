Chapter 2: CMake & Make Tutorial
================================

[Getting Started Index](http://david.zemon.name/PropWare/md_docs_GettingStarted.html)<br />
[<< Chapter 1: Installation](http://david.zemon.name/PropWare/md_docs_Install.html)<br />
[>> Chapter 3: Run an example project](http://david.zemon.name/PropWare/md_docs_RunningExampleProjects.html)

What is CMake?
--------------
CMake is a Makefile generator. CMake will scan your source code and, based on some predefined rules, 
generate `Make` scripts. Using Cmake is a two-step process: first generate the Makefiles, then compile your code by 
executing the Makefiles. CMake documentation can be found on 
[cmake.org](http://cmake.org/cmake/help/documentation.html)

What is Make?
-------------
Make is a much older tool that will, at its core, incrementally compile your code. This simply means that if you 
have a project with two files (`cat.c` and `dog.c`), and a change is made to `cat.c`, then only `cat.c` will be 
re-compiled. Lots of different flavors of Make have been developed over the years including GNU, MinGW, Ninja, and 
more - but we will focus on GNU since that is what ships with the Windows version of PropGCC and comes with most Linux 
operating systems. GNU's Make documentation can be found on 
[gnu.org](http://www.gnu.org/software/make/manual/html_node/index.html).

Getting Started
---------------

Two command line instructions are all you need to get started:

    cmake -G "Unix Makefiles" <source directory>
    make
    
Breakdown:
* `cmake -G "Unix Makefiles" <source directory>`: Here we perform the first step - generating the Makefiles. CMake will  
  scan your source directory, read your source files to discover dependencies (like header files), and create one or  
  more files named "Makefile".
   
  The first argument, `-G "Unix Makefiles"`, tells CMake exactly what flavor of Make we're using. As reasoned in the
  `What is Make?` section above, we will always be using `Unix Makefiles` for PropWare projects.

  The second argument is the source directory of your project. This directory must contain a `CMakeLists.txt` file as
  described in step 1 of [Creating a PropWare Application](http://david.zemon.name/PropWare/md_docs_AppDeployment.html).
  I prefer to do "out of source" builds - which means I create a new folder within my project where all of my generated
  and compiled files will end up. Generally, I name this folder `bin`. With this simple case, the complete command to
  generate the Makefiles would be `cmake -G "Unix Makefiles" ..`. The `..` is a cross-platform way of saying "the
  directory above me". For reference: `.` refers to the current directory.
  
* `make`: Make is old, flexible, and great. CMake has done all the work though and no extra arguments are needed for
  this command. After successfully running CMake, simply type `make` - character for character. When it
  completes successfully, your project will have been compiled into its binary format, ready to be programmed to your
  Propeller. PropWare provides some extra "targets" to make your life even easier though. Try typing `make debug` to 
  write your program to RAM and open a terminal (like Parallax's PST) or `make run` to write your program to EEPROM.
  
CMakeLists.txt
--------------

As described in [Creating a PropWare Application](http://david.zemon.name/PropWare/md_docs_AppDeployment.html), the CMakeLists.txt file is required for all
CMake (and therefore PropWare) applications. The file must be named "CMakeLists.txt" - character for character.

The contents of the file are simple - many useful tips can be gleaned from both 
[Creating a PropWare Application](http://david.zemon.name/PropWare/md_docs_AppDeployment.html) and 
[CMake for PropWare](http://david.zemon.name/PropWare/md_docs_CMakeForPropware.html).

CMakeCache.txt
--------------

I won't hold back - CMakeCache.txt can be a royal pain. CMake keeps track of your settings in this file and, 
a lot of times, you might change a setting without CMake taking notice. When this happens, just delete the 
`CMakeCache.txt` file and rerun your CMake command (as given above). As a bonus to using the out-of-source build
technique you can also just delete your entire build directory and start over - then you know with 100% confidence that
no leftover settings will linger around to haunt you.

Curses CMake
------------

Curses is a library that provides a psuedo-graphical user interface at the command line. Curses CMake (or ccmake) allows
you to easily configure some options for your project before compiling it. Try running `ccmake -G "Unix Makefiles 
<source directory>` next time and take a look.

Make Targets
------------

A make "target" is a set of commands that you would like Make to execute for you. If you run Make with no arguments, a
default target of "all" is chosen for you. Generally, the `all` target will compile your code. On the rare off-chance
that you made some changes to your code and Make hasn't picked them up (and therefore is not recompiling them), giving
Make the "clean" target will delete all compiled objects and let you start over (`make clean`). You can remove old files
and recompile new ones in a single step with `make clean all` (order matters).

PropWare provides two extra targets for your convenience. 
* The `debug` target will compile your code, write it to your Propeller's HUB RAM and then start a serial terminal to
  aid in debugging your application. When you are finished, press any of the arrow keys on your keyboard (don't use 
  `Control-C` - it doesn't seem to like that) and you will be returned to your normal command prompt.
* The `run` target will compile your code and load it into your Propeller's EEPROM. 

Debugging CMake Configurations
------------------------------

After generating your Makefiles, try adding `VERBOSE=1` to your Make command. This will tell Make to print each command
to the terminal before running it - rather than printing only a short summary of what is being done. The output is very 
verbose and can be cumbersome to read - but it is invaluable when determining what went wrong with your compilation.
Full command: `make VERBOSE=1`

[Getting Started Index](http://david.zemon.name/PropWare/md_docs_GettingStarted.html)<br />
[<< Chapter 1: Installation](http://david.zemon.name/PropWare/md_docs_Install.html)<br />
[>> Chapter 3: Run an example project](http://david.zemon.name/PropWare/md_docs_RunningExampleProjects.html)
