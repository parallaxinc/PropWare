cmake_minimum_required(VERSION 3.0)

# Variables that should be passed into this script
# OBJCOPY   GCC's objcopy executable
# FILE      File for objcopy to run over
# VERBOSE   If 1, command is echoed

get_filename_component(WORKING_DIR ${FILE} DIRECTORY)
get_filename_component(FILE_NAME ${FILE} NAME)
get_filename_component(MODULES_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
set(LOCALIZE_LIST_FILE "${MODULES_DIR}/PropellerCogLocalizeSymbols.txt")

string(REPLACE "." ";" NAME_COMPONENTS ${FILE_NAME})
list(LENGTH NAME_COMPONENTS NUM_COMPONENTS)
math(EXPR SKIP_COMPONENT "${NUM_COMPONENTS} - 2")
set(INDEX 0)
foreach (component ${NAME_COMPONENTS})
    if (NOT INDEX STREQUAL ${SKIP_COMPONENT})
        set(SHORT_NAME ${SHORT_NAME}.${component})
    endif ()
    math(EXPR INDEX "${INDEX} + 1")
endforeach ()
string(LENGTH ${SHORT_NAME} SHORT_NAME_LEN)
string(SUBSTRING ${SHORT_NAME} 1 ${SHORT_NAME_LEN} SHORT_NAME)

file(RENAME "${FILE}" "${WORKING_DIR}/${SHORT_NAME}")

# Object copy
if ($ENV{VERBOSE})
    message("${OBJCOPY} --localize-text --localize-symbols=${LOCALIZE_LIST_FILE} --rename-section .text=${SHORT_NAME} ${SHORT_NAME}")
endif()
execute_process(
    COMMAND "${OBJCOPY}" --localize-text "--localize-symbols=${LOCALIZE_LIST_FILE}" --rename-section .text=${SHORT_NAME} ${SHORT_NAME}
    WORKING_DIRECTORY "${WORKING_DIR}"
    RESULT_VARIABLE CMD_PASS)
if (${RESULT})
    message(FATAL_ERROR ${RESULT})
endif()

file(RENAME "${WORKING_DIR}/${SHORT_NAME}" "${FILE}")
