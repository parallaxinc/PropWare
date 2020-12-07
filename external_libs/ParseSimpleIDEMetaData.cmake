# Determine if a string starts with the give substring
function(starts_with string substring output_variable)
    string(FIND "${string}" "${substring}" RESULT)
    if (0 EQUAL RESULT)
        set(${output_variable} TRUE PARENT_SCOPE)
    else ()
        set(${output_variable} FALSE PARENT_SCOPE)
    endif ()
endfunction()

function(split_lines input output)
    string(REGEX REPLACE "\r" "" temp "${input}")
    string(REGEX REPLACE ";" "\\\\;" temp "${temp}")
    string(REGEX REPLACE "\n" ";" temp "${temp}")
    set("${output}" "${temp}" PARENT_SCOPE)
endfunction()

# Replace "foo -> foo" with "foo"
function(remove_link_symbol input output)
    string(REPLACE " -> " ";" TEMP "${input}")
    list(GET TEMP 0 RESULT)
    set("${output}" "${RESULT}" PARENT_SCOPE)
endfunction()

function(add_simple_sources root_dir file_contents target_name)
    split_lines("${file_contents}" FILE_LINES)

    # Remove the first line, because SimpleIDE project files always ignore the first line when creating libraries
    list(REMOVE_AT FILE_LINES 0)

    set(NEW_SOURCES "")
    set(COMPILER_FLAGS "")
    foreach (line ${FILE_LINES})
        starts_with("${line}" ">" IS_NOT_SOURCE_FILE)  # If the line doesn't start with a '>', then it's source code
        if (IS_NOT_SOURCE_FILE)
            # Various compiler flags are declared via the project files, so let's parse those now...

            # Optimization...
            if (line MATCHES ">optimize=")
                string(REPLACE ">optimize=" "" OPTIMIZATION_LEVEL "${line}")
                list(APPEND COMPILER_FLAGS "${OPTIMIZATION_LEVEL}")
            elseif (line MATCHES ">defs::")
                string(REPLACE ">defs::" "" temp "${line}")
                string(REPLACE " " ";" temp "${temp}")
                list(APPEND COMPILER_FLAGS ${temp})
            elseif (line MATCHES ">-l" OR line MATCHES ">-create_library")
                # Skip linker lines, and obviously "-create_library" is not a GCC argument
            elseif (line MATCHES ">-")
                string(REPLACE ">" "" NEW_FLAG "${line}")
                list(APPEND COMPILER_FLAGS "${NEW_FLAG}")
            endif ()
        else ()
            remove_link_symbol("${line}" relativeFilepath)

            # FIXME: https://github.com/parallaxinc/Simple-Libraries/pull/204
            if (relativeFilepath STREQUAL "imu_gryoAvailable.c")
                set(relativeFilepath imu_gyroAvailable.c)
            endif ()

            set(source_filepath "${root_dir}/${relativeFilepath}")
            if (NOT FILE_HAS_MAIN)
                list(APPEND NEW_SOURCES "${source_filepath}")
            endif ()
        endif ()
    endforeach ()

    # Set all of the compiler flags for these sources
    foreach (source_file ${NEW_SOURCES})
        string(REPLACE ";" " " COMPILER_FLAGS_STRING "${COMPILER_FLAGS}")
        set_source_files_properties("${source_file}" PROPERTIES COMPILE_FLAGS "${COMPILER_FLAGS_STRING}")
    endforeach ()

    # Add the sources
    list(LENGTH NEW_SOURCES NUMBER_OF_SOURCES)
    if (NUMBER_OF_SOURCES GREATER 0)
        if (TARGET ${target_name})
            target_sources(${target_name} PRIVATE ${NEW_SOURCES})
        else ()
            add_library(${target_name} STATIC ${NEW_SOURCES})
            set_linker(${target_name})
        endif ()
    else ()
        message("No non-main sources found in directory ${root_dir}")
    endif ()
endfunction()

function(parse_simpleide_project_files target_name filepath1)
    foreach (project_filepath ${filepath1} ${ARGN})
        file(READ "${project_filepath}" FILE_CONTENTS)

        if (FILE_CONTENTS MATCHES ".*>-create_library.*")
            get_filename_component(ROOT_DIR "${project_filepath}" DIRECTORY)
            add_simple_sources("${ROOT_DIR}" "${FILE_CONTENTS}" "${target_name}")
        endif ()
    endforeach ()
endfunction()
