# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mygpiod

# Try to find LIBMHD
#
# LIBGMHTD_FOUND
# LIBGMHTD_INCLUDE_DIRS
# LIBMHD_LIBRARIES

find_package(PkgConfig)
pkg_check_modules(PC_LIBMHD QUIET libmicrohttpd)

# Look for the header file
find_path(LIBMHD_INCLUDE_DIR
    NAMES gpiod.h
    HINTS ${PC_LIBMHD_INCLUDEDIR} ${PC_LIBMHD_INCLUDE_DIRS}
)

# Look for the library
find_library(LIBMHD_LIBRARY
    NAMES microhttpd
    HINTS ${PC_LIBMHD_LIBDIR} ${PC_LIBMHD_LIBRARY_DIRS}
)

set(LIBMHD_VERSION ${PC_LIBMHD_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBMHD
    FOUND_VAR LIBMHD_FOUND
    REQUIRED_VARS
        LIBMHD_LIBRARY
        LIBMHD_INCLUDE_DIR
    VERSION_VAR LIBMHD_VERSION
)

# Copy the results to the output variables
if(LIBMHD_FOUND)
    set(LIBMHD_LIBRARIES ${LIBMHD_LIBRARY})
    set(LIBMHD_INCLUDE_DIRS ${LIBMHD_INCLUDE_DIR})
else()
    set(LIBMHD_LIBRARIES)
    set(LIBMHD_INCLUDE_DIRS)
endif()

mark_as_advanced(LIBMHD_INCLUDE_DIRS LIBMHD_LIBRARIES)
