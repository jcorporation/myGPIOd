# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_library(mygpio-common "")

target_sources(mygpio-common
  PRIVATE
    util.c
)

target_include_directories(mygpio-common
  PRIVATE
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)
