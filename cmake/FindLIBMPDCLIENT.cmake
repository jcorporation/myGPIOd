# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mygpiod

# Try to find libmpdclient
#
# LIBMPDCLIENT_FOUND
# LIBMPDCLIENT_INCLUDE_DIRS
# LIBMPDCLIENT_LIBRARIES

find_package(PkgConfig)
pkg_check_modules(PC_LIBMPDCLIENT QUIET libmpdclient)

# Look for the header file
find_path(LIBMPDCLIENT_INCLUDE_DIR
    NAMES mpd/client.h
    HINTS ${PC_LIBMPDCLIENT_INCLUDEDIR} ${PC_LIBMPDCLIENT_INCLUDE_DIRS}
)

# Look for the library
find_library(LIBMPDCLIENT_LIBRARY
    NAMES mpdclient
    HINTS ${PC_LIBMPDCLIENT_LIBDIR} ${PC_LIBMPDCLIENT_LIBRARY_DIRS}
)

set(LIBMPDCLIENT_VERSION ${PC_LIBMPDCLIENT_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBMPDCLIENT
    FOUND_VAR LIBMPDCLIENT_FOUND
    REQUIRED_VARS
        LIBMPDCLIENT_LIBRARY
        LIBMPDCLIENT_INCLUDE_DIR
    VERSION_VAR LIBMPDCLIENT_VERSION
)

# Copy the results to the output variables
if(LIBMPDCLIENT_FOUND)
    set(LIBMPDCLIENT_LIBRARIES ${LIBMPDCLIENT_LIBRARY})
    set(LIBMPDCLIENT_INCLUDE_DIRS ${LIBMPDCLIENT_INCLUDE_DIR})
else()
    set(LIBMPDCLIENT_LIBRARIES)
    set(LIBMPDCLIENT_INCLUDE_DIRS)
endif()

mark_as_advanced(LIBMPDCLIENT_INCLUDE_DIRS LIBMPDCLIENT_LIBRARIES)
