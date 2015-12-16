cmake_minimum_required(VERSION 3.0)

# Variables that should be passed into this script
# OBJCOPY   GCC's objcopy executable
# SOURCE    File for objcopy to run over
# OUTPUT    Output file
# VERBOSE   If 1, command is echoed

get_filename_component(SOURCE_FILE_DIRECTORY "${SOURCE}" DIRECTORY)
get_filename_component(SOURCE_FILE_DIRECTORY "${SOURCE_FILE_DIRECTORY}" ABSOLUTE)
get_filename_component(WORKING_DIR "${OUTPUT}" DIRECTORY)
get_filename_component(WORKING_DIR "${WORKING_DIR}" ABSOLUTE)
get_filename_component(INPUT_FILE_NAME "${SOURCE}" NAME)
get_filename_component(OUTPUT_FILE_NAME "${OUTPUT}" NAME)

# Copy file to binary directory only if it isn't already there
if (NOT SOURCE_FILE_DIRECTORY STREQUAL WORKING_DIR)
    if (VERBOSE)
        message("${CMAKE_COMMAND} -E copy ${SOURCE} ${WORKING_DIR}/${INPUT_FILE_NAME}")
    endif ()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE}" "${WORKING_DIR}/${INPUT_FILE_NAME}"
        RESULT_VARIABLE CMD_PASS
    )
endif ()

if (CMD_PASS)
    message(FATAL_ERROR "${CMD_PASS}")
else ()
    # Object copy
    if (VERBOSE)
        message("${OBJCOPY} -Ibinary -Opropeller-elf-gcc -Bpropeller ${INPUT_FILE_NAME} ${INPUT_FILE_NAME}")
    endif ()
    execute_process(
        COMMAND ${OBJCOPY} -Ibinary -Opropeller-elf-gcc -Bpropeller "${INPUT_FILE_NAME}" "${INPUT_FILE_NAME}"
        WORKING_DIRECTORY "${WORKING_DIR}"
        RESULT_VARIABLE CMD_PASS
    )
    if (CMD_PASS)
        message(FATAL_ERROR ${CMD_PASS})
    endif ()
endif ()

file(RENAME "${WORKING_DIR}/${INPUT_FILE_NAME}" "${WORKING_DIR}/${OUTPUT_FILE_NAME}")
