cmake_minimum_required(VERSION 3.19)

set(MAX_CAPTURE_FRAMES "100")
project(stacktrace VERSION 1.1 DESCRIPTION "stacktrace" LANGUAGES CXX)

add_library(stacktrace INTERFACE include/stacktrace_exception.h include/code_position_exception.h)

if (WIN32)
    set(_OS_NAME "win")
elseif (UNIX)
    set(_OS_NAME "linux")
elseif (APPLE)
    set(_OS_NAME "mac")
endif()

# set defaults
set(IS_STACKTRACE_DEFAULT ON)
set(IS_DECODE_DEFAULT ON)

file(READ "compat-cfg.json" _COMPATIBILITY_TABLE_JSON)

if (WIN32)
    set(STACKTRACE_BACKEND "winapi")
    set(DECODE_BACKEND "winapi")
elseif (UNIX OR APPLE)
    set(STACKTRACE_BACKEND "execinfo")
    set(DECODE_BACKEND "libbacktrace")
endif()

if (DEFINED USE_STACKTRACE_BACKEND)
    set(IS_STACKTRACE_DEFAULT OFF)
    set(STACKTRACE_BACKEND ${USE_STACKTRACE_BACKEND})
endif()

if (DEFINED USE_DECODE_BACKEND)
    set(IS_DECODE_DEFAULT OFF)
    set(DECODE_BACKEND ${USE_DECODE_BACKEND})
endif()

string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "stacktrace")
string(JSON COMPAT_CHECK GET ${COMPAT_CHECK} ${_OS_NAME})
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${STACKTRACE_BACKEND})

if(NOT _ERROR_CODE STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Nonexistent stacktrace generation backend '${STACKTRACE_BACKEND}'")
elseif(COMPAT_CHECK EQUAL OFF)
    message(FATAL_ERROR "Backend '${STACKTRACE_BACKEND}' is incompatibile OS '${_OS_NAME}'")
endif()

if(IS_STACKTRACE_DEFAULT)
    message(WARNING "Backend for stacktrace generation not supplied by user using default backend for '${_OS_NAME}'")
endif()

# check compat for decode
string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "decode")
string(JSON COMPAT_CHECK GET ${COMPAT_CHECK} ${_OS_NAME})
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${DECODE_BACKEND})

if(NOT _ERROR_CODE STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Nonexistent stacktrace decode backend '${DECODE_BACKEND}'")
elseif(COMPAT_CHECK EQUAL OFF)
    message(FATAL_ERROR "Backend '${DECODE_BACKEND}' is incompatibile OS '${_OS_NAME}'")
endif()

if(IS_DECODE_DEFAULT)
    message(WARNING "Backend for stacktrace decode not supplied by user using default backend for '${_OS_NAME}'")
endif()

string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "libs")
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${STACKTRACE_BACKEND})

if(_ERROR_CODE STREQUAL "NOTFOUND")
    set(BACKEND_LINK_LIBS "${BACKEND_LINK_LIBS} ${COMPAT_CHECK}")
endif()

string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "libs")
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${DECODE_BACKEND})

if(_ERROR_CODE STREQUAL "NOTFOUND")
    set(BACKEND_LINK_LIBS "${BACKEND_LINK_LIBS} ${COMPAT_CHECK}")
endif()

separate_arguments(BACKEND_LINK_LIBS)

message("Using backends '${STACKTRACE_BACKEND}' for stack generation and '${DECODE_BACKEND}' for decode")

set(STACKTRACE_BACKEND "\"detail/stack/${STACKTRACE_BACKEND}.h\"")
set(DECODE_BACKEND "\"detail/decode/${DECODE_BACKEND}.h\"")

# check for ABI compat
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("CC: clang, using abi::__cxx_demangle!")
    set(DEMANGLE_BACKEND "\"detail/demangle/cxxabi.h\"")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message("CC: g++, using abi::__cxx_demangle!")
    set(DEMANGLE_BACKEND "\"detail/demangle/cxxabi.h\"")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message("CC: msvc, using noop demangle!")
    set(DEMANGLE_BACKEND "\"detail/demangle/noop.h\"")
endif()

configure_file(include/config.h.in config.h)
target_include_directories(stacktrace INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(stacktrace INTERFACE ${BACKEND_LINK_LIBS})


add_executable(stacktrace-test examples/main.cpp)
add_dependencies(stacktrace-test stacktrace)

add_executable(stacktrace-signal examples/signal.cpp)
add_dependencies(stacktrace-signal stacktrace)

target_include_directories(stacktrace-test PRIVATE include)
target_include_directories(stacktrace-signal PRIVATE include)

target_link_libraries(stacktrace-test stacktrace)
target_link_libraries(stacktrace-signal stacktrace)

set_target_properties(stacktrace-test PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(stacktrace-signal PROPERTIES EXCLUDE_FROM_ALL TRUE)
