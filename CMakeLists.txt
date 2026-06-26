cmake_minimum_required(VERSION 3.12)

project(fflowfirefly CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(FFlowCMake CONFIG REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Build type: Debug|Release" FORCE)
endif()

option(BUILD_SHARED_LIBS "build shared libraries" ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# rpaths (see https://cmake.org/Wiki/CMake_RPATH_handling#Always_full_RPATH)
if (POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)
endif()
if(APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")


find_package(GMP REQUIRED)
find_package(FLINT REQUIRED)
find_package(FFLOW REQUIRED)
find_package(FIREFLY REQUIRED)
include_directories(SYSTEM
    ${GMP_INCLUDE_DIRS}
    ${FLINT_INCLUDE_DIRS}
    ${FFLOW_INCLUDE_DIRS}
    ${FIREFLY_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GMP_TARGETS}
    ${FLINT_TARGETS}
    ${FFLOW_TARGETS}
    ${FIREFLY_TARGETS})


include_directories(
    BEFORE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

set(FFLOWFIREFLY_SOURCES
    src/fflowfirefly.cc
    )

set(FFLOWFIREFLY_HEADERS
    include/fflowfirefly.h
    )

add_library(fflowfirefly ${FFLOWFIREFLY_SOURCES})
target_link_libraries(fflowfirefly ${LIBS})

install(FILES ${FFLOWFIREFLY_HEADERS} DESTINATION include)
install(TARGETS fflowfirefly ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)


set(PYTHON_INTERFACE OFF CACHE BOOL "Build Python interface")
set(PYTHON_INTERFACE_INSTALL "" CACHE PATH
    "Installation path for Python interface")
if (PYTHON_INTERFACE)
    find_package (Python3 COMPONENTS Interpreter Development)
    if (Python3_FOUND)
        file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_python_gen)
        add_custom_command(
            DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/include/fflowfirefly.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/python_interface/build.py"
            COMMAND
            ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/python_interface/build.py"
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/_python_gen/_cffi_fflowfirefly.c"
            WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/_python_gen"
        )
        add_custom_target(
            fflowfirefly_python_interface_src ALL
            DEPENDS
            "${PROJECT_BINARY_DIR}/_python_gen/_cffi_fflowfirefly.c"
        )
        Python3_add_library(_cffi_fflowfirefly
            MODULE
            "${PROJECT_BINARY_DIR}/_python_gen/_cffi_fflowfirefly.c"
        )
        add_dependencies(_cffi_fflowfirefly fflowfirefly_python_interface_src)
        target_link_libraries(_cffi_fflowfirefly PRIVATE fflowfirefly ${LIBS})
        if (PYTHON_INTERFACE_INSTALL)
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/python_interface/fflowfirefly.py"
                DESTINATION ${PYTHON_INTERFACE_INSTALL})
            install(TARGETS _cffi_fflowfirefly
                DESTINATION ${PYTHON_INTERFACE_INSTALL})
        else()
            execute_process(COMMAND ${Python3_EXECUTABLE} -m site --user-site
                OUTPUT_VARIABLE PYTHON_INTERFACE_USER_SITE
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/python_interface/fflowfirefly.py"
                DESTINATION ${PYTHON_INTERFACE_USER_SITE})
            install(TARGETS _cffi_fflowfirefly
                DESTINATION ${PYTHON_INTERFACE_USER_SITE})
        endif()
    endif()
endif()


# uninstall
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
