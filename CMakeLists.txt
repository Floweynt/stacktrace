cmake_minimum_required(VERSION 3.19)

# --------------------------- 
# Basic cmake for install
# ---------------------------

project(stacktrace VERSION 1.0 DESCRIPTION "stacktrace" LANGUAGES CXX)

add_library(stacktrace INTERFACE)
set(MAX_CAPTURE_FRAMES "100")

# install stuff
target_include_directories(stacktrace INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/stacktrace_config_version.cmake"
    VERSION ${VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS stacktrace
    EXPORT stacktrace_targets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/stacktrace_config.cmake.in" 
    "${PROJECT_BINARY_DIR}/stacktrace_config.cmake" 
    INSTALL_DESTINATION lib/cmake/stacktrace
)

install(EXPORT stacktrace_targets DESTINATION lib/cmake/stacktrace)
install(FILES "${PROJECT_BINARY_DIR}/stacktrace_config_version.cmake" "${PROJECT_BINARY_DIR}/stacktrace_config.cmake" 
    DESTINATION lib/cmake/stacktrace
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

# --------------------------- 
# Compat checking
# ---------------------------

# set defaults
set(IS_STACKTRACE_DEFAULT ON)
set(IS_DECODE_DEFAULT ON)

if (WIN32)
    set(STACKTRACE_BACKEND "winapi")
    set(DECODE_BACKEND "winapi")
elseif (UNIX OR APPLE)
    set(STACKTRACE_BACKEND "execinfo")
    set(DECODE_BACKEND "libbfd")
endif()

# read config
file(READ "compat-cfg.json" _COMPATIBILITY_TABLE_JSON)

# set some flags
if (DEFINED USE_STACKTRACE_BACKEND)
    set(IS_STACKTRACE_DEFAULT OFF)
    set(STACKTRACE_BACKEND ${USE_STACKTRACE_BACKEND})
endif()

if (DEFINED USE_DECODE_BACKEND)
    set(IS_DECODE_DEFAULT OFF)
    set(DECODE_BACKEND ${USE_DECODE_BACKEND})
endif()

# check for compatibility
if (WIN32)
    set(_OS_NAME "win")
elseif (UNIX)
    set(_OS_NAME "linux")
elseif (APPLE)
    set(_OS_NAME "mac")
endif()

# check compat for stacktrace
string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "stacktrace")
string(JSON COMPAT_CHECK GET ${COMPAT_CHECK} ${_OS_NAME})
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${STACKTRACE_BACKEND})

if(_ERROR_CODE EQUAL NOTFOUND)
    message(FATAL_ERROR "Nonexistent stacktrace generation backend '${STACkTRACE_BACKEND}'")
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

if(_ERROR_CODE EQUAL NOTFOUND)
    message(FATAL_ERROR "Nonexistent stacktrace decode backend '${DECODE_BACKEND}'")
elseif(COMPAT_CHECK EQUAL OFF)
    message(FATAL_ERROR "Backend '${DECODE_BACKEND}' is incompatibile OS '${_OS_NAME}'")
endif()

if(IS_DECODE_DEFAULT)
    message(WARNING "Backend for stacktrace decode not supplied by user using default backend for '${_OS_NAME}'")
endif()


# linking voodoo
string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "libs")
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${STACKTRACE_BACKEND})

if(NOT _ERROR_CODE)
    set(BACKEND_LINK_LIBS "${BACKEND_LINK_LIBS} ${COMPAT_CHECK}")
endif()

string(JSON COMPAT_CHECK GET ${_COMPATIBILITY_TABLE_JSON} "libs")
string(JSON COMPAT_CHECK ERROR_VARIABLE _ERROR_CODE GET ${COMPAT_CHECK} ${DECODE_BACKEND})

if(NOT _ERROR_CODE)
    set(BACKEND_LINK_LIBS "${BACKEND_LINK_LIBS} ${COMPAT_CHECK}")
endif()

separate_arguments(BACKEND_LINK_LIBS)
target_link_libraries(stacktrace INTERFACE ${BACKEND_LINK_LIBS})


# set macros
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

configure_file("${PROJECT_SOURCE_DIR}/include/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h" @ONLY)

# TESTS

add_executable(stacktrace-test "${PROJECT_SOURCE_DIR}/examples/main.cpp")
add_dependencies(stacktrace-test stacktrace)

include_directories("${PROJECT_BINARY_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(stacktrace-test stacktrace)
