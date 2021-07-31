cmake_minimum_required(VERSION 3.9)

project(stacktrace VERSION 1.0 DESCRIPTION "stacktrace" LANGUAGES CXX)

add_library(stacktrace INTERFACE)

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

# check for os + cc
if (WIN32)
    message("OS: windows, using WINAPI impl")
    option(USE_WINAPI_STACKTRACE "Use winapi for stacktrace" ON)
    option(USE_EXECINFO_STACKTRACE "Use execinfo for stacktrace" OFF)
    option(USE_LIBBFD_DECODE "Use libbdf for symbols" OFF)
elseif(UNIX)
    message("OS: linux, using execinfo.h and libbfd impl")
    option(USE_EXECINFO_STACKTRACE "Use execinfo for stacktrace" ON)
    option(USE_WINAPI_STACKTRACE "Use winapi for stacktrace" OFF)
    option(USE_LIBBFD_DECODE "Use libbdf for symbols" ON)
elseif(APPLE)
    message("OS: apple, using execinfo.h and libbfd impl")
    option(USE_EXECINFO_STACKTRACE "Use execinfo for stacktrace" ON)
    option(USE_WINAPI_STACKTRACE "Use winapi for stacktrace" OFF)
    option(USE_LIBBFD_DECODE "Use libbdf for symbols" ON)
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

if(USE_LIBBFD_DECODE)
    target_link_libraries(stacktrace INTERFACE dl bfd)
endif()

configure_file("${PROJECT_SOURCE_DIR}/include/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h")

# TESTS

add_executable(stacktrace-test "${PROJECT_SOURCE_DIR}/examples/main.cpp")
add_dependencies(stacktrace-test stacktrace)

include_directories("${PROJECT_BINARY_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
set(CMAKE_BUILD_TYPE Debug)
                      
target_link_libraries(stacktrace-test stacktrace)