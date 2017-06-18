# this one is important
SET (CMAKE_SYSTEM_NAME Linux)
# this one not so much
SET (CMAKE_SYSTEM_VERSION 1)

# Check for Raspberry Pi Tools and bail out if they don't have it
if(DEFINED ENV{PI_TOOLS_HOME})
  message("Using Raspberry Pi Tools found in $ENV{PI_TOOLS_HOME}")
else()
  message("PI_TOOLS_HOME is not set; You must tell CMake where to find Raspberry Pi Tools (cross-compiler)")
  return()
endif()

SET (PiToolsDir arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian)

# specify the cross compiler
SET (CMAKE_C_COMPILER $ENV{PI_TOOLS_HOME}/${PiToolsDir}/bin/arm-linux-gnueabihf-gcc)
SET (CMAKE_CXX_COMPILER $ENV{PI_TOOLS_HOME}/${PiToolsDir}/bin/arm-linux-gnueabihf-g++)

# where is the target environment
SET (CMAKE_FIND_ROOT_PATH $ENV{PI_TOOLS_HOME}/${PiToolsDir})

# search for programs in the build host directories
SET (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)