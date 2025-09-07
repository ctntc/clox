cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS ON)

enable_testing()

if (POLICY CMP0141 AND MSVC)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("clox")

find_package(spdlog CONFIG REQUIRED)

add_subdirectory("core")
add_subdirectory("tests")

add_executable("lox" "app/main.cpp")
target_include_directories("lox" PRIVATE "core")
target_link_libraries("lox" PRIVATE "loxc_core" spdlog::spdlog)
