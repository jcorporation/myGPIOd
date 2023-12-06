# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2018-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_library(mygpio SHARED)

target_include_directories(mygpio
  PRIVATE
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_sources(mygpio
  PRIVATE
    src/connection.c
)

set_target_properties(mygpio PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(mygpio PROPERTIES PUBLIC_HEADER include/libmygpio.h)

# Install shared library and public header
install(TARGETS mygpio
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# pkg-config file
configure_file(libmygpio.pc.in libmygpio.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libmygpio.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
