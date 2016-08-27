function(read_source SOURCE_FILE OUTPUT)
    file(READ "${SOURCE_FILE}" ORIGINAL_FILE_CONTENT)
    string(REPLACE "<" "&lt;" PARTIAL_FIX "${ORIGINAL_FILE_CONTENT}")
    string(REPLACE ">" "&gt;" FIXED "${PARTIAL_FIX}")
    set(${OUTPUT} "${FIXED}" PARENT_SCOPE)
endfunction()

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(HTML_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/docs/static-web/api-develop")
    set(TEAMCITY_BUILD_ID "$ENV{TEAMCITY_BUILD_ID}")
    configure_file("${PROJECT_SOURCE_DIR}/Doxyfile.in"
        "${PROJECT_SOURCE_DIR}/Doxyfile"
        @ONLY)

    list(APPEND GENERATED_FILES_TO_CLEAN
        "${PROJECT_SOURCE_DIR}/Doxyfile")

    add_custom_target(docs
        "${DOXYGEN_EXECUTABLE}" Doxyfile
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif ()

configure_file("${PROJECT_SOURCE_DIR}/docs/static-web/src/download.html.in"
    "${PROJECT_SOURCE_DIR}/docs/static-web/src/download.html"
    @ONLY)

read_source("${PROJECT_SOURCE_DIR}/Examples/GettingStarted/CMakeLists.txt" GETTING_STARTED_CMAKE_CONFIG)
read_source("${PROJECT_SOURCE_DIR}/Examples/GettingStarted/GettingStarted.cpp" GETTING_STARTED_CPP)
configure_file("${PROJECT_SOURCE_DIR}/docs/static-web/src/getting-started.html.in"
    "${PROJECT_SOURCE_DIR}/docs/static-web/src/getting-started.html"
    @ONLY)

list(APPEND GENERATED_FILES_TO_CLEAN
    "${PROJECT_SOURCE_DIR}/docs/static-web/src/download.html"
    "${PROJECT_SOURCE_DIR}/docs/static-web/src/getting-started.html")

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    "${GENERATED_FILES_TO_CLEAN}")
