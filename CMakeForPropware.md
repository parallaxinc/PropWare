CMake for PropWare
===========================

Be sure to check out CMake's official documentation at [cmake.org](http://cmake.org/cmake/help/documentation.html).
Remember that PropWare uses CMake 3.0.

Bare Minimum
------------
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
################################################################################
### Template code. Do not modify                                               #
                                                                               #
cmake_minimum_required (VERSION 3.0.0)                                         #
# Aside from cmake_minimum_required, this must be the first two lines          #
# of the file                                                                  #
file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                          #
include(${PROPWARE_PATH}/CMakePropellerHeader.cmake)                           #
################################################################################

project(HelloWorld)

add_executable(${PROJECT_NAME} main.cpp)

################################################################################
### Template code. Do not modify                                               #
                                                                               #
# This must be the last line of the file                                       #
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)                           #
################################################################################
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Note the two sections marked as "Template code." Make sure these two pieces begin and end each of your
  CMakeLists.txt files. They load the standard Propeller settings for your project.
* Next we name our project: `project(HelloWorld)`. This name can be anything you'd like so long as there is no 
  whitespace.
* Finally, we need to tell CMake what files should be compiled: `add_executable(${PROJECT_NAME} main.cpp)`.
  The first parameter _must_ be exactly as you see it - PropWare's build system relies on your executable being named
  the same as your project. Next, simply list off each of your source files (separated by whitespace).
* There is LOTS more information about creating CMakeLists.txt files for PropWare projects on the [CMakeLists.txt 
  Files for PropWare](./md_CMakeListsForPropware.html) page.

Typical Use Case
----------------
Let's take the example of a phony project for a Quadcopter. We're using Parallax's Quickstart development board, 
the compressed-memory-model, and our code must be compiled with size optimizations. Our projects consists of the 
following files:

* Quadcopter.cpp
* motor_drivers.c
* avionics.S
* rf_transceiver.ecogc

We're also going to compile all ECOGC files with the `--bogus` flag because... we can.

Our CMakeLists.txt file might look something like this:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
################################################################################
### Template code. Do not modify                                               #
                                                                               #
cmake_minimum_required (VERSION 3.0.0)                                         #
# Aside from cmake_minimum_required, this must be the first two lines          #
# of the file                                                                  #
file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                          #
include(${PROPWARE_PATH}/CMakePropellerHeader.cmake)                           #
################################################################################

set(BOARD QUICKSTART)
set(MODEL cmm)
set(COMMON_FLAGS "-Os")
set(ECOGC_FLAGS "--bogus")

project(Quadcopter C CXX ASM ECOGC)

add_executable(${PROJECT_NAME} 
    ${PROJECT_NAME}
    motor_drivers
    avionics.S
    rf_transceiver.ecogc)

################################################################################
### Template code. Do not modify                                               #
                                                                               #
# This must be the last line of the file                                       #
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)                           #
################################################################################
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Common CMake variables in PropWare 
* BOARD: The `BOARD` variable can be used to identify your physical hardware. PropGCC uses this to determine
  how many bytes of EEPROM, default baud rate for RS232, and other functions. Any board available in SimpleIDE
  is available here as well - they are reading from the same configuration pool (`<PropGCC root>/propeller-load`).
* MODEL: \[default: `lmm`\] This option should be one of `cog`, `cmm`, `lmm`, `xmm-single`, `xmm-split`, or `xmmc`. Read 
  more about PropGCC's memory models [here](https://code.google.com/p/propgcc/wiki/PropGccInDepth).
* COMMON_FLAGS: Any options applied to the `COMMON_FLAGS` variable will be applied to assembly, C and C++ files.
  This includes cogc, ecogc, cogcpp, and ecogcpp but not Spin. Your optimization level should be set here to apply 
  across your entire project.

### Project Languages
CMake will default to enabling only C and C++ in your project. If your project uses more than these two languages, 
you must add with either the [project](http://www.cmake.org/cmake/help/v3.0/command/project.html#command:project) or 
[enable_language](http://www.cmake.org/cmake/help/v3.0/command/enable_language.html#command:enable_language) function.

### Adding Source Files
* All source files must be defined in the [add_executable](http://www.cmake.org/cmake/help/v3.0/command/add_executable)
  function
* Each source file is separated by any amount of whitespace
* Normal C and C++ files do not need their extensions explicitly defined.
* The various COG languages _do_ need their extensions explicitly defined, as does assembly.

Variables
---------

### COMMON_FLAGS
Flags applied to all PropGCC languages

### COMMON_COG_FLAGS
Flags applied to COGC, ECOGC, COGCXX, and ECOGXX

### \<LANG\>_FLAGS
Compilation flags used for a specific language.
* Note that C_FLAGS will be applied to C, COGC, and ECOGC and likewise for CXX_FLAGS
* Note that COGC_FLAGS will be applied after C_FLAGS and will therefore override any conflicting flags in the C_FLAGS
  variable (likewise for ECOGC, COGCXX, and ECOGCXX)

### MODEL
\[default: `lmm`\]

This option should be one of `cog`, `cmm`, `lmm`, `xmm-single`, `xmm-split`, or `xmmc`. Read  more about PropGCC's 
memory models [here](https://code.google.com/p/propgcc/wiki/PropGccInDepth). 

### BOARD
Used to identify your physical hardware. PropGCC uses this to determine how many bytes of EEPROM, default baud rate for 
RS232, and other functions. Any board available in SimpleIDE is available here as well - they are reading from the same 
configuration pool (`<PropGCC root>/propeller-load`).

Options
-------

CMake options allow you to have fine-grain control over what compilation flags are used by default. Most users will 
want to leave all options set to their default values, but those doing special projects or using conflict libraries 
can change the options to suit their needs.

### 32_BIT_DOUBLES
\[default: ON\]

Sets all variables of type `double` to use IEEE 32-bit floating point instead of 64. Equivalent to adding 
"-m32bit-doubles" to `COMMON_FLAGS`. Required for compatibility with numerous functions in the Simple library.

### AUTO_C_STD
\[default: ON\]

Sets the C standard to C99. Equivalent to adding "-std=c99" to `C_FLAGS`

### AUTO_CXX_STD
\[default: ON\]

Sets the C++ standard to gnu++0x. Equivalent to adding "-std=gnu++0x" to `CXX_FLAGS`.

### LINK_LIBPROPELLER
\[default: ON\]

Link the target executable with the libpropeller library. 

### LINK_PROPWARE
\[default: ON\]

Link the target executable with the PropWare library.

### LINK_SIMPLE
\[default: ON\]

Link the target executable with the Simple library.

### LINK_TINY
\[default: OFF\]

Link the target executable with the tiny library. Note: tiny has been deprecated by Parallax in favor of `print` and 
`printi`.

Default Compile Flags
---------------------
With all options left at their defaults, the following flags will be used. There is currently no option to disable 
`-fno-threadsafe-statics -fno-rtti`. If such an option is required, let the [author](mailto:david@zemon.name) know 
and an option will be added. It is believed that these flags will always be desired by Propeller users.

* ASM: None
* C: `-Wall -m32bit-doubles -std=c99`
* COGC: `-Wall -m32bit-doubles -std=c99 -mcog -xc -r`
* ECOGC: `-Wall -m32bit-doubles -std=c99 -mcog -xc -r`
* CXX: `-Wall -m32bit-doubles -std=gnu++0x -fno-threadsafe-statics -fno-rtti`
* COGCXX: `-Wall -m32bit-doubles -std=gnu++0x -fno-threadsafe-statics -fno-rtti -mcog -xc++ -r`
* ECOGCXX: `-Wall -m32bit-doubles -std=gnu++0x -fno-threadsafe-statics -fno-rtti -mcog -xc++ -r`
* Linker: None
* Archiver: `cr`
