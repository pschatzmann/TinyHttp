# ---------------------------------------------------------------------------------
# CMake definition for stand alone linux build
#
# Copyright (c) 2020 SSID


# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.h")
file(GLOB SRC_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
# set(HEADER_LIST "${TinyPPP_SOURCE_DIR}/include/modern/lib.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(tinyhttp_library ${SRC_LIST} ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(tinyhttp_library PUBLIC ../src)

# This depends on (header only) boost
# target_link_libraries(tinyhttp_library PRIVATE Boost::boost)

# All users of this library will need at least C++11
target_compile_features(tinyhttp_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/src"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
