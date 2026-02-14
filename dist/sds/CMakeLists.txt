# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_library(sds "")

target_sources(sds
  PRIVATE
    sds.c
    sdsalloc.h
  PUBLIC
    sds.h
)

target_include_directories(sds
  PRIVATE
    ${PROJECT_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE MATCHES "(Debug|Release|RelWithDebInfo|MinSizeRel)")
  target_compile_options(sds
    PRIVATE
      "-Wno-conversion"
      "-Wno-format-nonliteral"
  )
endif()
