# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  TinyHttp
  VERSION 0.1
  DESCRIPTION "TinyHttp (Arduino) Library"
  LANGUAGES CXX)

  include(FetchContent)

  add_definitions(-DPLATFORM_LINUX)

  # Build with Linux Arduino Emulator
  FetchContent_Declare(arduino_emulator GIT_REPOSITORY "https://github.com/pschatzmann/Arduino-Emulator.git" GIT_TAG main )
  FetchContent_GetProperties(arduino_emulator)
  if(NOT arduino_emulator_POPULATED)
      FetchContent_Populate(arduino_emulator)
      add_subdirectory(${arduino_emulator_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/emulator)
  endif()



  # The compiled library code is here
  add_subdirectory(src)

  # The executable code is here
  add_subdirectory(tests)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
#if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
#   AND BUILD_TESTING)
#  add_subdirectory(tests)
#endif()
