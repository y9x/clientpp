# clientpp lib config:

set(BUILD_SHARED_LIBS OFF)
set(URIPARSER_BUILD_DOCS OFF)
set(URIPARSER_BUILD_TESTS OFF)
set(URIPARSER_BUILD_TOOLS OFF)
set(URIPARSER_BUILD_CHAR ON)
set(URIPARSER_BUILD_WCHAR_T ON)
set(URIPARSER_ENABLE_INSTALL ON)
set(URIPARSER_WARNINGS_AS_ERRORS  ON)

# to update: replace CMAKE_CURRENT_SOURCE_DIR with URIPARSER

# default:

# uriparser - RFC 3986 URI parsing library
#
# Copyright (C) 2018, Sebastian Pipping <sebastian@pipping.org>
# All rights reserved.
#
# Redistribution and use in source  and binary forms, with or without
# modification, are permitted provided  that the following conditions
# are met:
#
#     1. Redistributions  of  source  code   must  retain  the  above
#        copyright notice, this list  of conditions and the following
#        disclaimer.
#
#     2. Redistributions  in binary  form  must  reproduce the  above
#        copyright notice, this list  of conditions and the following
#        disclaimer  in  the  documentation  and/or  other  materials
#        provided with the distribution.
#
#     3. Neither the  name of the  copyright holder nor the  names of
#        its contributors may be used  to endorse or promote products
#        derived from  this software  without specific  prior written
#        permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND  ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING, BUT NOT
# LIMITED TO,  THE IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS
# FOR  A  PARTICULAR  PURPOSE  ARE  DISCLAIMED.  IN  NO  EVENT  SHALL
# THE  COPYRIGHT HOLDER  OR CONTRIBUTORS  BE LIABLE  FOR ANY  DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT  LIABILITY,  OR  TORT (INCLUDING  NEGLIGENCE  OR  OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
cmake_minimum_required(VERSION 3.3)

project(uriparser
    VERSION
        0.9.6
    LANGUAGES
        C
)

# See https://verbump.de/ for what these numbers do
set(URIPARSER_SO_CURRENT    1)
set(URIPARSER_SO_REVISION  29)
set(URIPARSER_SO_AGE        0)

include(CheckCCompilerFlag)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#
# Configuration
#
option(BUILD_SHARED_LIBS "Build shared libraries (rather than static ones)" OFF)
option(URIPARSER_BUILD_DOCS "Build API documentation (requires Doxygen, Graphviz, and (optional) Qt's qhelpgenerator)" OFF)
option(URIPARSER_BUILD_TESTS "Build test suite (requires GTest >=1.8.0)" OFF)
option(URIPARSER_BUILD_TOOLS "Build tools (e.g. CLI \"uriparse\")" OFF)
option(URIPARSER_BUILD_CHAR "Build code supporting data type 'char'" ON)
option(URIPARSER_BUILD_WCHAR_T "Build code supporting data type 'wchar_t'" ON)
option(URIPARSER_ENABLE_INSTALL "Enable installation of uriparser" ON)
option(URIPARSER_WARNINGS_AS_ERRORS "Treat all compiler warnings as errors" ON)
set(URIPARSER_MSVC_RUNTIME "" CACHE STRING "Use of specific runtime library (/MT /MTd /MD /MDd) with MSVC")

if(NOT URIPARSER_BUILD_CHAR AND NOT URIPARSER_BUILD_WCHAR_T)
    message(SEND_ERROR "One or more of URIPARSER_BUILD_CHAR and URIPARSER_BUILD_WCHAR_T needs to be enabled.")
endif()
if(URIPARSER_BUILD_TESTS AND NOT (URIPARSER_BUILD_CHAR AND URIPARSER_BUILD_WCHAR_T))
    message(SEND_ERROR "URIPARSER_BUILD_TESTS=ON requires both URIPARSER_BUILD_CHAR=ON and URIPARSER_BUILD_WCHAR_T=ON.")
endif()
if(URIPARSER_BUILD_TOOLS AND NOT URIPARSER_BUILD_CHAR)
    message(SEND_ERROR "URIPARSER_BUILD_TOOLS=ON requires URIPARSER_BUILD_CHAR=ON.")
endif()

if(URIPARSER_BUILD_TESTS)
    # We have to call enable_language() before modifying any CMAKE_CXX_* variables
    enable_language(CXX)
endif()

macro(uriparser_apply_msvc_runtime_to ref)
    string(REGEX REPLACE "/M[DT]d?" ${URIPARSER_MSVC_RUNTIME} ${ref} "${${ref}}")
endmacro()

if(MSVC AND URIPARSER_MSVC_RUNTIME)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS_DEBUG)
    uriparser_apply_msvc_runtime_to(CMAKE_C_FLAGS_RELEASE)
endif()

macro(uriparser_install)
    if(URIPARSER_ENABLE_INSTALL)
        install(${ARGN})
    endif()
endmacro()

#
# Compiler checks
#
set(URIPARSER_EXTRA_COMPILE_FLAGS)

check_c_compiler_flag("-fvisibility=hidden" URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
if(URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
    set(URIPARSER_EXTRA_COMPILE_FLAGS "${URIPARSER_EXTRA_COMPILE_FLAGS} -fvisibility=hidden")
endif()

#
# UriConfig.h
#
check_symbol_exists(wprintf wchar.h HAVE_WPRINTF)
check_function_exists(reallocarray HAVE_REALLOCARRAY)  # no luck with CheckSymbolExists
configure_file(${URIPARSER}/src/UriConfig.h.in UriConfig.h)

#
# C library
#
set(API_HEADER_FILES
    ${URIPARSER}/include/uriparser/UriBase.h
    ${URIPARSER}/include/uriparser/UriDefsAnsi.h
    ${URIPARSER}/include/uriparser/UriDefsConfig.h
    ${URIPARSER}/include/uriparser/UriDefsUnicode.h
    ${URIPARSER}/include/uriparser/Uri.h
    ${URIPARSER}/include/uriparser/UriIp4.h
)
set(LIBRARY_CODE_FILES
    ${URIPARSER}/src/UriCommon.c
    ${URIPARSER}/src/UriCommon.h
    ${URIPARSER}/src/UriCompare.c
    ${URIPARSER}/src/UriEscape.c
    ${URIPARSER}/src/UriFile.c
    ${URIPARSER}/src/UriIp4Base.c
    ${URIPARSER}/src/UriIp4Base.h
    ${URIPARSER}/src/UriIp4.c
    ${URIPARSER}/src/UriMemory.c
    ${URIPARSER}/src/UriMemory.h
    ${URIPARSER}/src/UriNormalizeBase.c
    ${URIPARSER}/src/UriNormalizeBase.h
    ${URIPARSER}/src/UriNormalize.c
    ${URIPARSER}/src/UriParseBase.c
    ${URIPARSER}/src/UriParseBase.h
    ${URIPARSER}/src/UriParse.c
    ${URIPARSER}/src/UriQuery.c
    ${URIPARSER}/src/UriRecompose.c
    ${URIPARSER}/src/UriResolve.c
    ${URIPARSER}/src/UriShorten.c
)

add_library(uriparser
    ${API_HEADER_FILES}
    ${LIBRARY_CODE_FILES}
)

if(NOT MSVC)
    math(EXPR URIPARSER_SO_CURRENT_MINUS_AGE "${URIPARSER_SO_CURRENT} - ${URIPARSER_SO_AGE}")
    set_property(TARGET uriparser PROPERTY VERSION ${URIPARSER_SO_CURRENT_MINUS_AGE}.${URIPARSER_SO_AGE}.${URIPARSER_SO_REVISION})
    set_property(TARGET uriparser PROPERTY SOVERSION ${URIPARSER_SO_CURRENT_MINUS_AGE})
    if(WIN32)
        set_target_properties(uriparser PROPERTIES
            OUTPUT_NAME uriparser
            RUNTIME_OUTPUT_NAME uriparser-${URIPARSER_SO_CURRENT_MINUS_AGE}
            ARCHIVE_OUTPUT_NAME uriparser)
    endif()
endif()

set_property(
    TARGET
        uriparser
    PROPERTY
        PUBLIC_HEADER "${API_HEADER_FILES}"
)

target_compile_definitions(uriparser PRIVATE URI_LIBRARY_BUILD)
if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(uriparser PUBLIC URI_STATIC_BUILD)
endif()
if(NOT URIPARSER_BUILD_CHAR)
    target_compile_definitions(uriparser PUBLIC URI_NO_ANSI)
endif()
if(NOT URIPARSER_BUILD_WCHAR_T)
    target_compile_definitions(uriparser PUBLIC URI_NO_UNICODE)
endif()
if(URIPARSER_COMPILER_SUPPORTS_VISIBILITY)
    target_compile_definitions(uriparser PRIVATE URI_VISIBILITY)
endif()

target_include_directories(uriparser
    PUBLIC
        $<BUILD_INTERFACE:${URIPARSER}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}  # for UriConfig.h
)

uriparser_install(
    TARGETS
        uriparser
    EXPORT
        uriparser
    ARCHIVE
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
    LIBRARY
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
    RUNTIME
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/uriparser
)

#
# C command line tool
#
if(URIPARSER_BUILD_TOOLS)
    add_executable(uriparse
        ${URIPARSER}/tool/uriparse.c
    )

    target_link_libraries(uriparse PUBLIC uriparser)

    if(HAIKU)
        # Function inet_ntop needs -lsocket or -lnetwork (see pull request #45)
        check_library_exists(socket inet_ntop "" HAVE_LIBSOCKET__INET_NTOP)
        check_library_exists(network inet_ntop "" HAVE_LIBNETWORK__INET_NTOP)
        if(HAVE_LIBSOCKET__INET_NTOP)
            target_link_libraries(uriparse PUBLIC socket)
        endif()
        if(HAVE_LIBNETWORK__INET_NTOP)
            target_link_libraries(uriparse PUBLIC network)
        endif()
    endif()

    if(WIN32)
        target_link_libraries(uriparse PUBLIC ws2_32)
    endif()

    uriparser_install(
        TARGETS
            uriparse
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
    )
endif()

#
# C++ test runner
#
if(URIPARSER_BUILD_TESTS)
    if(MSVC AND URIPARSER_MSVC_RUNTIME)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS_DEBUG)
        uriparser_apply_msvc_runtime_to(CMAKE_CXX_FLAGS_RELEASE)
    endif()

    enable_testing()

    find_package(GTest 1.8.0 REQUIRED)

    add_executable(testrunner
        ${URIPARSER}/test/FourSuite.cpp
        ${URIPARSER}/test/MemoryManagerSuite.cpp
        ${URIPARSER}/test/test.cpp
        ${URIPARSER}/test/VersionSuite.cpp

        # These library code files have non-public symbols that the test suite
        # needs to link to, so they appear here as well:
        ${API_HEADER_FILES}
        ${LIBRARY_CODE_FILES}
    )

    target_compile_definitions(testrunner PRIVATE URI_STATIC_BUILD)

    if(MSVC)
        target_compile_definitions(testrunner PRIVATE -D_CRT_NONSTDC_NO_WARNINGS)
        target_compile_definitions(testrunner PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    endif()

    target_include_directories(testrunner SYSTEM PRIVATE
        ${GTEST_INCLUDE_DIRS}
    )

    target_include_directories(testrunner PRIVATE
        ${URIPARSER}/include
        ${CMAKE_CURRENT_BINARY_DIR}  # for UriConfig.h
    )

    target_link_libraries(testrunner PUBLIC
        ${GTEST_BOTH_LIBRARIES}
    )

    # NOTE: uriparser does not use pthreads itself but gtest does
    find_package(Threads REQUIRED)
    target_link_libraries(testrunner PRIVATE Threads::Threads)

    if(MSVC)
        # Specify unwind semantics so that MSVC knowns how to handle exceptions
        target_compile_options(testrunner PRIVATE /EHsc)
    endif()

    if(MINGW)
        set(_URIPARSER_TEST_COMMAND wine testrunner)
    else()
        set(_URIPARSER_TEST_COMMAND      testrunner)
    endif()

    add_test(
        NAME
            test
        COMMAND
            ${_URIPARSER_TEST_COMMAND}
    )

    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
endif()

#
# Compiler flags
#
if(URIPARSER_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_definitions(/WX)
    else()
        set(URIPARSER_EXTRA_COMPILE_FLAGS "${URIPARSER_EXTRA_COMPILE_FLAGS} -Werror")
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${URIPARSER_EXTRA_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${URIPARSER_EXTRA_COMPILE_FLAGS}")

#
# Doxygen API documentation
#
if(URIPARSER_BUILD_DOCS)
    find_package(Doxygen REQUIRED dot doxygen)

    set(QHG_LOCATION "" CACHE FILEPATH "Path to qhelpgenerator program (default: auto-detect)")
    if(NOT QHG_LOCATION)
        find_package(Qt5Help QUIET)
        if(TARGET Qt5::qhelpgenerator)
            get_target_property(QHG_LOCATION Qt5::qhelpgenerator LOCATION)
            mark_as_advanced(Qt5Core_DIR)
            mark_as_advanced(Qt5Gui_DIR)
            mark_as_advanced(Qt5Help_DIR)
            mark_as_advanced(Qt5Sql_DIR)
            mark_as_advanced(Qt5Widgets_DIR)
        endif()
    endif()

    include(FindHTMLHelp)

    # Generate Doxyfile
    if(HTML_HELP_COMPILER)
        set(GENERATE_HTMLHELP YES)
    else()
        set(GENERATE_HTMLHELP NO)
    endif()
    if(QHG_LOCATION)
        set(GENERATE_QHP YES)
    else()
        set(GENERATE_QHP NO)
    endif()
    configure_file(${URIPARSER}/doc/Doxyfile.in doc/Doxyfile @ONLY)
    configure_file(${URIPARSER}/doc/release.sh.in doc/release.sh @ONLY)

    add_custom_target(doc
        ALL
        COMMAND
            ${DOXYGEN_EXECUTABLE}
            Doxyfile
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT
            "Generating API documentation with Doxygen"
        VERBATIM
    )

    uriparser_install(
        DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}/doc/html
        DESTINATION
            ${CMAKE_INSTALL_DOCDIR}
    )
    if(QHG_LOCATION)
        uriparser_install(
            FILES
                ${CMAKE_CURRENT_BINARY_DIR}/doc/uriparser-${PROJECT_VERSION}.qch
            DESTINATION
                ${CMAKE_INSTALL_DOCDIR}
        )
    endif()
endif()

#
# CMake files for find_package(uriparser [..] CONFIG [..])
#
configure_package_config_file(
        ${URIPARSER}/cmake/uriparser-config.cmake.in
        cmake/uriparser-config.cmake
    INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
)
write_basic_package_version_file(
    cmake/uriparser-config-version.cmake
    COMPATIBILITY SameMajorVersion  # i.e. semver
)
export(
    TARGETS
        uriparser
    FILE
        cmake/uriparser-targets.cmake  # not going to be installed
)
uriparser_install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/uriparser-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/uriparser-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
)
uriparser_install(
    EXPORT
        uriparser
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/uriparser-${PROJECT_VERSION}/
    NAMESPACE
        uriparser::
)

#
# pkg-config file
#
if(NOT MSVC)
    if(CMAKE_INSTALL_LIBDIR MATCHES "^/")
        set(_URIPARSER_PKGCONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
    else()
        set(_URIPARSER_PKGCONFIG_LIBDIR "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(CMAKE_INSTALL_INCLUDEDIR MATCHES "^/")
        set(_URIPARSER_PKGCONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
    else()
        set(_URIPARSER_PKGCONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
    endif()

    configure_file(${URIPARSER}/liburiparser.pc.in liburiparser.pc @ONLY)
    uriparser_install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/liburiparser.pc
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/pkgconfig/
    )
endif()
