# This file sets the basic flags for the ECOGC language in CMake.
# It also loads the available platform file for the system-compiler
# if it exists.
# It also loads a system - compiler - processor (or target hardware)
# specific file, which is mainly useful for crosscompiling and embedded systems.

set(CMAKE_ECOGC_OUTPUT_EXTENSION .ecog)

set(_INCLUDED_FILE 0)

set(CMAKE_BASE_NAME gcc)

#include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}-${CMAKE_SYSTEM_PROCESSOR} OPTIONAL)

# This should be included before the _INIT variables are
# used to initialize the cache.  Since the rule variables
# have if blocks on them, users can still define them here.
# But, it should still be after the platform file so changes can
# be made to those values.

if(CMAKE_USER_MAKE_RULES_OVERRIDE)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE "${_override}")
endif()

if(CMAKE_USER_MAKE_RULES_OVERRIDE_ECOGC)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE_ECOGC} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE_ECOGC "${_override}")
endif()


# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
if(NOT CMAKE_MODULE_EXISTS)
  set(CMAKE_SHARED_MODULE_ECOGC_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
  set(CMAKE_SHARED_MODULE_CREATE_ECOGC_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})
endif()

# avoid just having a space as the initial value for the cache
if(CMAKE_C_FLAGS_INIT STREQUAL " ")
  set(CMAKE_C_FLAGS_INIT)
endif()
set (CMAKE_ECOGC_FLAGS "${CMAKE_C_FLAGS_INIT}" CACHE STRING
     "Flags used by the compiler during all build types.")

if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
# default build type is none
  if(NOT CMAKE_NO_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_INIT} CACHE STRING
      "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
  endif()
  set (CMAKE_ECOGC_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT}" CACHE STRING
    "Flags used by the compiler during debug builds.")
  set (CMAKE_ECOGC_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL_INIT}" CACHE STRING
    "Flags used by the compiler during release builds for minimum size.")
  set (CMAKE_ECOGC_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE_INIT}" CACHE STRING
    "Flags used by the compiler during release builds.")
  set (CMAKE_ECOGC_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
    "Flags used by the compiler during release builds with debug info.")
endif()

if(CMAKE_C_STANDARD_LIBRARIES_INIT)
  set(CMAKE_ECOGC_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES_INIT}"
    CACHE STRING "Libraries linked by default with all C applications.")
  mark_as_advanced(CMAKE_ECOGC_STANDARD_LIBRARIES)
endif()

include(CMakeCommonLanguageInclude)

# now define the following rule variables

# CMAKE_C_CREATE_SHARED_LIBRARY
# CMAKE_C_CREATE_SHARED_MODULE
# CMAKE_C_COMPILE_OBJECT
# CMAKE_C_LINK_EXECUTABLE

# variables supplied by the generator at use time
# <TARGET>
# <TARGET_BASE> the target without the suffix
# <OBJECTS>
# <OBJECT>
# <LINK_LIBRARIES>
# <FLAGS>
# <LINK_FLAGS>

# C compiler information
# <CMAKE_C_COMPILER>
# <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS>
# <CMAKE_SHARED_MODULE_CREATE_C_FLAGS>
# <CMAKE_C_LINK_FLAGS>

# Static library tools
# <CMAKE_AR>
# <CMAKE_RANLIB>

set(CMAKE_INCLUDE_FLAG_ECOGC ${CMAKE_INCLUDE_FLAG_C})

set(CMAKE_ECOGC_ARCHIVE_CREATE ${CMAKE_C_ARCHIVE_CREATE})
set(CMAKE_ECOGC_ARCHIVE_APPEND ${CMAKE_C_ARCHIVE_APPEND})
set(CMAKE_ECOGC_ARCHIVE_FINISH ${CMAKE_C_ARCHIVE_FINISH})
if (CMAKE_VERSION VERSION_EQUAL "3.4.0" OR CMAKE_VERSION VERSION_GREATER "3.4.0")
    set(INCLUDES "<INCLUDES>")
endif ()
set(CMAKE_ECOGC_COMPILE_OBJECT
"<CMAKE_C_COMPILER> <DEFINES> ${CMAKE_ECOGC_FLAGS_INIT} <FLAGS> ${INCLUDES} -o <OBJECT> -c <SOURCE>"
"${CMAKE_COMMAND} -DOBJCOPY=\"${CMAKE_OBJCOPY}\" -DFILE=<OBJECT> -P ${PROPWARE_RUN_OBJCOPY}")

mark_as_advanced(
CMAKE_ECOGC_FLAGS
CMAKE_ECOGC_FLAGS_DEBUG
CMAKE_ECOGC_FLAGS_MINSIZEREL
CMAKE_ECOGC_FLAGS_RELEASE
CMAKE_ECOGC_FLAGS_RELWITHDEBINFO
)
set(CMAKE_ECOGC_INFORMATION_LOADED 1)

