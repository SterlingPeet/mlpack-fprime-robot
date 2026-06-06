# Taken from mlpack's ConfigureCrossCompile.cmake.

macro(search_openblas version)
  set(BLA_STATIC ON)
  find_package(BLAS)
  if (NOT BLAS_FOUND OR (NOT BLAS_LIBRARIES))
    if(NOT OPENBLAS_TARGET)
      message(FATAL_ERROR "Cannot compile OpenBLAS: OPENBLAS_TARGET is not set.  Either set that variable, or set BOARD_NAME correctly!")
    endif()
    get_deps(https://github.com/xianyi/OpenBLAS/releases/download/v${version}/OpenBLAS-${version}.tar.gz OpenBLAS OpenBLAS-${version}.tar.gz)
    if (NOT MSVC)
      if (NOT EXISTS "${CMAKE_BINARY_DIR}/deps/OpenBLAS-${version}/libopenblas.a")
        set(OLD_CC $ENV{CC})
        set(ENV{COMMON_OPT} "${CMAKE_OPENBLAS_FLAGS}") # Pass our flags to OpenBLAS
        set(ENV{CC} "${CMAKE_C_COMPILER}")
        if (CCACHE_PROGRAM)
          set (ENV{CC} "${CCACHE_PROGRAM} $ENV{CC}")
        endif ()
        # Turn OPENBLAS_EXTRA_ARGS into a list so that we can use them as
        # parameters to make.
        separate_arguments(ARG_LIST NATIVE_COMMAND ${OPENBLAS_EXTRA_ARGS})

        # Now run the OpenBLAS build with all of the configuration variables set.
        execute_process(
            COMMAND make NO_SHARED=1 HOSTCC=gcc TARGET=${OPENBLAS_TARGET} BINARY=${OPENBLAS_BINARY} ${ARG_LIST}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/deps/OpenBLAS-${version})

        # Don't leak the environment variables back into the rest of the
        # configuration.
        unset(ENV{COMMON_OPT})
        set(ENV{CC} ${OLD_CC})
      endif()
      file(GLOB OPENBLAS_LIBRARIES "${CMAKE_BINARY_DIR}/deps/OpenBLAS-${version}/libopenblas.a")
      set(BLAS_openblas_LIBRARY ${OPENBLAS_LIBRARIES})
      set(LAPACK_openblas_LIBRARY ${OPENBLAS_LIBRARIES})
      set(BLA_VENDOR OpenBLAS)
      set(BLAS_FOUND ON)
    endif()
  endif()
  find_library(GFORTRAN NAMES libgfortran.a)
  if (GFORTRAN)
    set(CROSS_COMPILE_SUPPORT_LIBRARIES ${CROSS_COMPILE_SUPPORT_LIBRARIES} ${GFORTRAN})
  endif ()
  find_library(PTHREAD NAMES libpthread.a)
  if (PTHREAD)
    set(CROSS_COMPILE_SUPPORT_LIBRARIES ${CROSS_COMPILE_SUPPORT_LIBRARIES} ${PTHREAD})
  endif ()
endmacro()
