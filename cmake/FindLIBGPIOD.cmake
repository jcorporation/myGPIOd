# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mygpiod

# Try to find libgpiod
#
# LIBGPIOD_FOUND
# LIBGPIOD_INCLUDE_DIRS
# LIBGPIOD_LIBRARIES

find_package(PkgConfig)
pkg_check_modules(PC_LIBGPIOD QUIET libgpiod)

# Look for the header file
find_path(LIBGPIOD_INCLUDE_DIR
    NAMES gpiod.h
    HINTS ${PC_LIBGPIOD_INCLUDEDIR} ${PC_LIBGPIOD_INCLUDE_DIRS}
)

# Look for the library
find_library(LIBGPIOD_LIBRARY
    NAMES gpiod
    HINTS ${PC_LIBGPIOD_LIBDIR} ${PC_LIBGPIOD_LIBRARY_DIRS}
)

set(LIBGPIOD_VERSION ${PC_LIBGPIOD_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBGPIOD
    FOUND_VAR LIBGPIOD_FOUND
    REQUIRED_VARS
        LIBGPIOD_LIBRARY
        LIBGPIOD_INCLUDE_DIR
    VERSION_VAR LIBGPIOD_VERSION
)

# Copy the results to the output variables
if(LIBGPIOD_FOUND)
    set(LIBGPIOD_LIBRARIES ${LIBGPIOD_LIBRARY})
    set(LIBGPIOD_INCLUDE_DIRS ${LIBGPIOD_INCLUDE_DIR})
else()
    set(LIBGPIOD_LIBRARIES)
    set(LIBGPIOD_INCLUDE_DIRS)
endif()

mark_as_advanced(LIBGPIOD_INCLUDE_DIRS LIBGPIOD_LIBRARIES)
