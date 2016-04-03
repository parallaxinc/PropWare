cmake_minimum_required(VERSION 3.0)

# Variables that should be passed into this script
# SIZE_EXE      PropGCC's propeller-elf-size executable
# BINARY    Binary file to check

execute_process(
    COMMAND "${SIZE_EXE}" "${BINARY}"
    RESULT_VARIABLE RETURN_CODE
    OUTPUT_VARIABLE STDOUT
    ERROR_VARIABLE STDERR
)

function(remove_empty_elements_and_trim out_list in_list)
    foreach (item IN LISTS ${in_list})
        if (NOT item STREQUAL "")
            string(REPLACE " " "" item "${item}")
            list(APPEND formatted_list "${item}")
        endif ()
    endforeach ()

    set(${out_list} "${formatted_list}" PARENT_SCOPE)
endfunction()

if (NOT RETURN_CODE)
    # Remove carriage returns
    string(REPLACE "\r" "" STDOUT "${STDOUT}")

    # Split into multiple lines and save as separate variables
    string(REPLACE "\n" ";" LINES "${STDOUT}")
    list(GET LINES 0 HEADER)
    list(GET LINES 1 SIZES)

    # Split lines into words
    string(REPLACE "\t" ";" HEADER_WORDS_TEMP "${HEADER}")
    remove_empty_elements_and_trim(HEADER_WORDS HEADER_WORDS_TEMP)
    string(REPLACE "\t" ";" SIZE_WORDS_TEMP "${SIZES}")
    remove_empty_elements_and_trim(SIZE_WORDS SIZE_WORDS_TEMP)

    # Determine index of "bss" and "dec" word
    list(FIND HEADER_WORDS "bss" BSS_INDEX)
    list(FIND HEADER_WORDS "dec" DEC_INDEX)

    # Determine size of bss and dec sections
    list(GET SIZE_WORDS ${BSS_INDEX} BSS_SIZE)
    list(GET SIZE_WORDS ${DEC_INDEX} TOTAL_SIZE)

    # Subtract bss from total to get code size
    math(EXPR CODE_SIZE "${TOTAL_SIZE} - ${BSS_SIZE}")

    # All done! Print it!
    message("Code size  = ${CODE_SIZE}")
    message("Total size = ${TOTAL_SIZE}")
endif ()
