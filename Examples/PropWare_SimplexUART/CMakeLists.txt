#############################################################################
### Template code. Do not modify                                            #
                                                                            #
cmake_minimum_required (VERSION 3.0.0)                                      #
# Aside from cmake_minimum_required, this must be the first two lines       #
# of the file                                                               #
file(TO_CMAKE_PATH $ENV{PROPWARE_PATH} PROPWARE_PATH)                       #
set(CMAKE_TOOLCHAIN_FILE ${PROPWARE_PATH}/PropellerToolchain.cmake)         #
#############################################################################

set(BOARD QUICKSTART)
set(MODEL lmm)
set(COMMON_FLAGS "-Os")
set(C_FLAGS )
set(CXX_FLAGS )

project(SimplexUART_Demo)

add_executable(${PROJECT_NAME} ${PROJECT_NAME})

#############################################################################
### Template code. Do not modify                                            #
                                                                            #
include(${PROPWARE_PATH}/CMakePropellerFooter.cmake)                        #
#############################################################################
