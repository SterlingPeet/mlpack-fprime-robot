####
# FPrime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
# UT_SOURCE_FILES: list of source files for unit tests
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/documentation/reference
#
####

# Add the project root so that #include "config/RomiCfg.hpp" resolves
# regardless of which build directory is active.
include_directories("${CMAKE_CURRENT_LIST_DIR}/../../")

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/RomiImu.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/RomiImu.cpp"
)

set(MOD_DEPS
  Components_RomiTypes
)

register_fprime_module()


### Unit Tests ###
# set(UT_SOURCE_FILES
#   "${CMAKE_CURRENT_LIST_DIR}/RomiImu.fpp"
#   "${CMAKE_CURRENT_LIST_DIR}/test/ut/RomiImuTestMain.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/test/ut/RomiImuTester.cpp"
# )
# set(UT_MOD_DEPS
#   STest
# )
# set(UT_AUTO_HELPERS ON)
# register_fprime_ut()
