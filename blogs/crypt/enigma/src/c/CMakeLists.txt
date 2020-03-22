cmake_minimum_required(VERSION 3.0.2)
project(enigma)

# for vim plugin - YCM
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(MUGGLE_BUILD_TRACE "build with trace info in source codes" OFF)

file(GLOB src_c ${CMAKE_CURRENT_LIST_DIR}/*.c)
file(GLOB src_h ${CMAKE_CURRENT_LIST_DIR}/*.h)

if (WIN32)
add_executable(enigma ${src_c} ${src_h})
else()
add_executable(enigma ${src_c})
endif()
if (${MUGGLE_BUILD_TRACE})
	target_compile_definitions(enigma PUBLIC MUGGLE_BUILD_TRACE)
endif()

