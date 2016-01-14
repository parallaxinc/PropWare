
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-g++, arm-elf-ar etc.). This works
# currently with the GNU crosscompilers.
#
# Sets the following variables:
#   CMAKE_CXX_COMPILER
#   CMAKE_COMPILER_IS_GNUCXX
#   CMAKE_AR
#   CMAKE_RANLIB
#
# If not already set before, it also sets
#   _CMAKE_TOOLCHAIN_PREFIX

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

# Load system-specific compiler preferences for this language.
include(Platform/${CMAKE_SYSTEM_NAME}-CXX OPTIONAL)
if(NOT CMAKE_ECOGCXX_COMPILER_NAMES)
  set(CMAKE_ECOGCXX_COMPILER_NAMES CC)
endif()

if(NOT CMAKE_ECOGCXX_COMPILER)
  message(FATAL_ERROR "CMAKE_ECOGCXX_COMPILER must be defined")
endif()
mark_as_advanced(CMAKE_ECOGCXX_COMPILER)

# Each entry in this list is a set of extra flags to try
# adding to the compile line to see if it helps produce
# a valid identification file.
set(CMAKE_ECOGCXX_COMPILER_ID_TEST_FLAGS
  # Try compiling to an object file only.
  "-c -xc++"
)

# Build a small source file to identify the compiler.
if(NOT CMAKE_ECOGCXX_COMPILER_ID_RUN)
  set(CMAKE_ECOGCXX_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  set(CMAKE_ECOGCXX_COMPILER_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_ECOGCXX_COMPILER_ID_PLATFORM_CONTENT)

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(ECOGCXX ECOGCXXFLAGS CMakeCXXCompilerId.cpp)

  # Set old compiler and platform id variables.
  if("${CMAKE_ECOGCXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCXX 1)
  endif()
endif()

if (NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_ECOGCXX_COMPILER}" PATH)
endif ()

# if we have a g++ cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-g++, arm-elf-g++ or i586-mingw32msvc-g++ , optionally
# with a 3-component version number at the end (e.g. arm-eabi-gcc-4.5.2).
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names like
# "arm-unknown-nto-qnx6.3.0-gcc.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"
if (CMAKE_CROSSCOMPILING  AND NOT  _CMAKE_TOOLCHAIN_PREFIX)
  get_filename_component(COMPILER_BASENAME "${CMAKE_ECOGCXX_COMPILER}" NAME)
  if (COMPILER_BASENAME MATCHES "^(.+-)(clan)?[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
  elseif("${CMAKE_ECOGCXX_COMPILER_ID}" MATCHES "Clang")
    set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_ECOGCXX_COMPILER_TARGET}-)
  elseif(COMPILER_BASENAME MATCHES "QCC(\\.exe)?$")
    if(CMAKE_ECOGCXX_COMPILER_TARGET MATCHES "gcc_nto([^_le]+)(le)?.*$")
      set(_CMAKE_TOOLCHAIN_PREFIX nto${CMAKE_MATCH_1}-)
    endif()
  endif ()
endif ()

# configure all variables set in this file
configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeECOGCXXCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeECOGCXXCompiler.cmake
  @ONLY
  )
set(CMAKE_ECOGCXX_COMPILER_ENV_VAR "CXX")
