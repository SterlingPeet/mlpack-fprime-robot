####
# This sets up the build system for the 'MarsRobot' project, including
# components and deployments from project.cmake. In addition, it imports
# the core F Prime components.
####

cmake_minimum_required(VERSION 3.13)
project(MarsRobot C CXX)
# mlpack requires C++17.
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###
# F' Core Setup
# This includes all of the F prime core components, and imports the make-system.
###
include("${CMAKE_CURRENT_LIST_DIR}/fprime/cmake/FPrime.cmake")
# NOTE: register custom targets between these two lines
fprime_setup_included_code()

include("${CMAKE_CURRENT_LIST_DIR}/CMake/SearchOpenBLAS.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/CMake/mlpack.cmake")
# Automatically download dependencies and only build OpenBLAS if we are
# cross-compiling.  This CMake configuration only supports cross-compiling
# specifically to an RPi 3B+.  Modify the OPENBLAS_TARGET below if not.
set(OPENBLAS_TARGET "CORTEXA53")
set(OPENBLAS_BINARY "64")
fetch_mlpack(${CMAKE_CROSSCOMPILING})
include_directories(${MLPACK_INCLUDE_DIRS})


# This includes project-wide objects
include("${CMAKE_CURRENT_LIST_DIR}/project.cmake")
