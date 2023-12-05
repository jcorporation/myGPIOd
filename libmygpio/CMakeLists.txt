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
  PUBLIC
    include/libmygpio.h
)

install(TARGETS mygpio DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
