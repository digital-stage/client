# - Try to find FFTW
# Once done, this will define
#
#  FFTW_FOUND - system has FFTW
#  FFTW_INCLUDE_DIRS - the FFTWinclude directories
#  FFTW_LIBRARIES - link these to use FFTW

if (FFTW_LIBRARIES AND FFTW_INCLUDE_DIRS)
    set(FFTW_FOUND TRUE)
else ()
    # Use pkg-config to get hints about paths
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(FFTW_PKGCONF fftw3)
    endif (PKG_CONFIG_FOUND)

    # Include dir
    find_path(FFTW_INCLUDE_DIR
            NAMES fftw3.h
            PATHS ${FFTW_PKGCONF_INCLUDE_DIRS}
            )

    # Library
    find_library(FFTW_LIBRARY
            NAMES fftw libfftw3f-3 fftw3f
            PATHS ${FFTW_PKGCONF_LIBRARY_DIRS}
            )

    find_package(PackageHandleStandardArgs)
    find_package_handle_standard_args(FFTW DEFAULT_MSG FFTW_LIBRARY FFTW_INCLUDE_DIR)

    if (FFTW_FOUND)
        set(FFTW_LIBRARIES ${FFTW_LIBRARY})
        set(FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})
    endif (FFTW_FOUND)

    mark_as_advanced(FFTW_LIBRARY FFTW_INCLUDE_DIR)
endif ()