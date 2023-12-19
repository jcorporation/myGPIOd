# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2018-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_executable(mygpioc "")

target_include_directories(mygpioc
  PRIVATE
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_sources(mygpioc
  PRIVATE
    main.c
    gpio.c
    idle.c
    options.c
    util.c
)

target_link_directories(mygpioc
  PRIVATE
    "${PROJECT_BINARY_DIR}/libmygpio"
)

target_link_libraries(mygpioc
  PRIVATE
    mygpio
    mygpio-common
)

install(TARGETS mygpioc DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
