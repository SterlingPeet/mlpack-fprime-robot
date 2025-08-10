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

set(SOURCE_FILES
  "${CMAKE_CURRENT_LIST_DIR}/RomiHWDriver.fpp"
  "${CMAKE_CURRENT_LIST_DIR}/RomiHWDriver.cpp"
)

# Uncomment and add any modules that this component depends on, else
# they might not be available when cmake tries to build this component.
#
set(MOD_DEPS
  Components_RomiTypes
  Components_RomiPorts
)

register_fprime_module()


### Unit Tests ###
# set(UT_SOURCE_FILES
#   "${CMAKE_CURRENT_LIST_DIR}/RomiHWDriver.fpp"
#   "${CMAKE_CURRENT_LIST_DIR}/test/ut/RomiHWDriverTestMain.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/test/ut/RomiHWDriverTester.cpp"
# )
# set(UT_MOD_DEPS
#   STest
# )
# set(UT_AUTO_HELPERS ON)
# register_fprime_ut()
