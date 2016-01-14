cmake_minimum_required(VERSION 3.0)

# This file is simply a wrapper to handle the Spin2dat files because spin2cpp does not allow specifying an output
# filename

# Variables that should be passed into this script
# OBJCOPY   GCC's objcopy executable
# SOURCE    File for objcopy to run over
# OUTPUT    Output file


get_filename_component(WORKING_DIR "${OUTPUT}" DIRECTORY)
get_filename_component(WORKING_DIR "${WORKING_DIR}" ABSOLUTE)

if (ENV{VERBOSE})
    message("${SPIN2CPP_COMMAND} --dat ${SOURCE}")
endif ()
execute_process(
    COMMAND "${SPIN2CPP_COMMAND}" --dat "${SOURCE}"
    WORKING_DIRECTORY "${WORKING_DIR}"
    RESULT_VARIABLE CMD_PASS
)

if (CMD_PASS)
    message(FATAL_ERROR "${CMD_PASS}")
else ()
    # Source file follows the pattern *.spin and output will follow the pattern *.spin.dat, but what we _need_ is *.dat
    # So, some very simple programming will get us the *.dat name that needs to be passed to the next script...
    get_filename_component(SOURCE_FILE "${SOURCE}" NAME)
    string(REPLACE "." ";" INPUT_FILE_NAME_COMPONENTS ${SOURCE_FILE})
    list(LENGTH INPUT_FILE_NAME_COMPONENTS NUMBER_OF_FILE_NAME_PARTS)
    math(EXPR FILE_EXTENSION_INDEX "${NUMBER_OF_FILE_NAME_PARTS} - 1")
    list(REMOVE_AT INPUT_FILE_NAME_COMPONENTS ${FILE_EXTENSION_INDEX})
    list(APPEND INPUT_FILE_NAME_COMPONENTS "dat")
    string(REPLACE ";" "." DAT_FILE_NAME "${INPUT_FILE_NAME_COMPONENTS}")
    set(DAT_FILE_PATH "${WORKING_DIR}/${DAT_FILE_NAME}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} "-DOBJCOPY=${OBJCOPY}" "-DSOURCE=${DAT_FILE_PATH}" "-DOUTPUT=${OUTPUT}" -P "${PropWare_DAT_SYMBOL_CONVERTER}"
        RESULT_VARIABLE CMD_PASS
    )
    if (CMD_PASS)
        message(FATAL_ERROR "See above error")
    endif ()
endif ()
