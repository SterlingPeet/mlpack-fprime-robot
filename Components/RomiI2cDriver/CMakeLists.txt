####
# FPrime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/documentation/reference
####

# Add the project root so that #include "config/RomiCfg.hpp" resolves
# regardless of which build directory is active.
include_directories("${CMAKE_CURRENT_LIST_DIR}/../../")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND NOT FPRIME_USE_STUBBED_DRIVERS)
  # Real implementation: only available on Linux (requires linux/i2c-dev.h)
  set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/RomiI2cDriver.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/RomiI2cDriver.cpp"
  )
else()
  # Stub: used on non-Linux hosts (e.g. macOS dev machines) or when
  # FPRIME_USE_STUBBED_DRIVERS=ON is passed to cmake.
  add_definitions(-DSTUBBED_ROMI_I2C_DRIVER)
  set(SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/RomiI2cDriver.fpp"
    "${CMAKE_CURRENT_LIST_DIR}/RomiI2cDriverStub.cpp"
  )
endif()

register_fprime_module()
