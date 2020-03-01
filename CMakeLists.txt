cmake_minimum_required(VERSION 3.12)

project(pcaudiolib-example)

set(TARGET_NAME ${PROJECT_NAME})
add_executable(${TARGET_NAME} "main.cpp" "boni.hpp")

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME})

#
# ### Macros and compiler options

set_target_properties(
  ${TARGET_NAME} PROPERTIES CXX_STANDARD "17" CXX_STANDARD_REQUIRED ON
)

# ### Link dependencies

# The `pcaudiolib` is not usually built using CMake.
# Provide a fallback for the user to give explicit paths.
find_package(pcaudiolib QUIET)
if(pcaudiolib_FOUND)
  target_link_libraries(${TARGET_NAME} pcaudiolib::pcaudiolib)
else()
  find_library(
    pcaudiolib_LIBRARY
    NAMES pcaudiolib
    PATH_SUFFIXES lib
    DOC "pcaudiolib library"
  )
  target_link_libraries(${TARGET_NAME} "${pcaudiolib_LIBRARY}")
  find_path(
    pcaudiolib_INCLUDE_DIR
    NAMES "pcaudiolib/audio.h"
    PATH_SUFFIXES include
    DOC "pcaudiolib include directory"
  )
  target_include_directories(
    ${TARGET_NAME} PRIVATE "${pcaudiolib_INCLUDE_DIR}"
  )
endif()

#
# ### Enable warnings and silence known warnings

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  target_compile_definitions(
    ${TARGET_NAME} PRIVATE "_CRT_SECURE_NO_WARNINGS"
  )
endif()

#
### Documentation

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  set(DOXYGEN_EXTRACT_ALL TRUE)
  set(DOXYGEN_EXTRACT_STATIC TRUE)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
  set(
    DOXYGEN_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_BINARY_DIR}/doc/${PROJECT_NAME}"
  )
  doxygen_add_docs(${TARGET_NAME}-doc "README.md" "main.cpp" "boni.hpp")
  install(DIRECTORY "${DOXYGEN_OUTPUT_DIRECTORY}" TYPE "DOC" OPTIONAL)
endif()
