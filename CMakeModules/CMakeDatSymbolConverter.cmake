cmake_minimum_required(VERSION 3.0)

# Variables that should be passed into this script
# OBJCOPY   GCC's objcopy executable
# SOURCE    File for objcopy to run over
# OUTPUT    Output file
# VERBOSE   If 1, command is echoed

get_filename_component(WORKING_DIR "${OUTPUT}" DIRECTORY)
get_filename_component(INPUT_FILE_NAME "${SOURCE}" NAME)
get_filename_component(OUTPUT_FILE_NAME "${OUTPUT}" NAME)

# Copy file to binary directory
if (${VERBOSE})
    message("${CMAKE_COMMAND} -E copy ${SOURCE} ${INPUT_FILE_NAME}")
endif()
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE}" "${INPUT_FILE_NAME}"
    WORKING_DIRECTORY "${WORKING_DIR}"
    RESULT_VARIABLE CMD_PASS
)
if (${RESULT})
    message(FATAL_ERROR ${RESULT})
else ()
    # Object copy
    if (${VERBOSE})
        message("${OBJCOPY} -Ibinary -Opropeller-elf-gcc -Bpropeller ${INPUT_FILE_NAME} ${INPUT_FILE_NAME}")
    endif()
    execute_process(
        COMMAND ${OBJCOPY} -Ibinary -Opropeller-elf-gcc -Bpropeller "${INPUT_FILE_NAME}" "${INPUT_FILE_NAME}"
        WORKING_DIRECTORY "${WORKING_DIR}"
        RESULT_VARIABLE CMD_PASS)
    if (${RESULT})
        message(FATAL_ERROR ${RESULT})
    endif()
endif()

file(RENAME "${WORKING_DIR}/${INPUT_FILE_NAME}" "${WORKING_DIR}/${OUTPUT_FILE_NAME}")
