cmake_minimum_required(VERSION 3.9)

project(stacktrace VERSION 1.0 DESCRIPTION "stacktrace" LANGUAGES CXX)

add_library(stacktrace INTERFACE)
set(MAX_CAPTURE_FRAMES "10000")

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

# libbfd and backtrace are mutually exclusive
if (USE_LIBBFD_DECODE AND USE_BACKTRACE_DECODE)
    message(FATAL_ERROR "cannot use both libbfd and libbacktrace")
endif()

# check for os + cc
if (WIN32)
    message("OS: windows, using WINAPI impl")
    option(USE_WINAPI_STACKTRACE "Use winapi for stacktrace" ON)
    option(USE_EXECINFO_STACKTRACE "Use execinfo for stacktrace" OFF)
    option(USE_LIBBFD_DECODE "Use libbfd for symbols" OFF)
    option(USE_BACKTRACE_DECODE "Use backtrace for symbols" OFF)
    if (USE_LIBBFD_DECODE OR USE_BACKTRACE_DECODE)
        message(WARNING "cannot use libbfd or backtrace on windows, overriding to use WINAPI")
    endif()
elseif (UNIX OR APPLE)
    message("OS: linux, using execinfo.h impl")
    option(USE_EXECINFO_STACKTRACE "Use execinfo for stacktrace" ON)
    option(USE_WINAPI_STACKTRACE "Use winapi for stacktrace" OFF)
    if (NOT(USE_LIBBFD_DECODE OR USE_BACKTRACE_DECODE))
        message(WARNING "neither use libbfd or backtrace was specified, using libbfd")
        option(USE_LIBBFD_DECODE "use libbfd for symbols" ON)
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("CC: clang, using abi::__cxx_demangle!")
    option(USE_ABI_CXX_DEMANGLE "Use abi::__cxx_demangle for demangle" ON)
    option(USE_NOOP_DEMANGLE "Use no-op for demangle" OFF)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message("CC: g++, using abi::__cxx_demangle!")
    option(USE_ABI_CXX_DEMANGLE "Use abi::__cxx_demangle for demangle" ON)
    option(USE_NOOP_DEMANGLE "Use no-op for demangle" OFF)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message("CC: clang, using abi::__cxx_demangle!")
    option(USE_ABI_CXX_DEMANGLE "Use abi::__cxx_demangle for demangle" OFF)
    option(USE_NOOP_DEMANGLE "Use no-op for demangle" ON)
endif()

# same as has ABI (g++/clang)
if(USE_BACKTRACE_DECODE AND NOT USE_ABI_CXX_DEMANGLE)
    message(ERROR "Unable to use libbacktrace if there is no ABI")
endif()

if(USE_LIBBFD_DECODE)
    message("Backend: using libbfd")
    target_link_libraries(stacktrace INTERFACE dl bfd)
elseif(USE_BACKTRACE_DECODE)
    message("Backend: using libbacktrace")
    target_link_libraries(stacktrace INTERFACE backtrace)
endif()

configure_file("${PROJECT_SOURCE_DIR}/include/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h" @ONLY)

# TESTS

add_executable(stacktrace-test "${PROJECT_SOURCE_DIR}/examples/main.cpp")
add_dependencies(stacktrace-test stacktrace)

include_directories("${PROJECT_BINARY_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(stacktrace-test stacktrace)
