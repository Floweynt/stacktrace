cmake_minimum_required(VERSION 3.9)

project(stacktrace VERSION 1.0 DESCRIPTION "stacktrace" LANGUAGES C CXX)
file(GLOB SRC_FILES "${PROJECT_SOURCE_DIR}/include/*.h" "${PROJECT_SOURCE_DIR}/source/stacktrace.cpp")
add_library(stacktrace STATIC ${SRC_FILES})

add_executable(stacktracetest "main.cpp")
add_dependencies(stacktracetest stacktrace)

include_directories("${PROJECT_BINARY_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
set_target_properties(stacktrace PROPERTIES LINKER_LANGUAGE CXX)
set(CMAKE_BUILD_TYPE Debug)

target_include_directories(stacktrace PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/MathFunctions"
                          )
                      
target_link_libraries(stacktracetest stacktrace)

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
endif()

if(USE_LIBBFD_DECODE)
    target_link_libraries(stacktrace dl bfd)
endif()

configure_file("${PROJECT_SOURCE_DIR}/include/config.h.in" "${PROJECT_BINARY_DIR}/include/config.h")
