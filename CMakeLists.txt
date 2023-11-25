#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

# minimal cmake version needed for new option handling
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0003 NEW)

# myGPIOd is written in C
# supported compilers: gcc, clang
project (mygpiod
  VERSION 0.4.0
  LANGUAGES C
)

# output binaries in bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("Cmake version: ${CMAKE_VERSION}")
message("Cmake src dir: ${PROJECT_SOURCE_DIR}")
message("Cmake build dir: ${CMAKE_CURRENT_BINARY_DIR}")
message("Cmake build type: ${CMAKE_BUILD_TYPE}")
message("Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# reset cmake default Release and Debug flags
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "")
set(CMAKE_C_FLAGS_MINSIZEREL "")

# available options
option(MYGPIOD_STARTUP_SCRIPT "Installs the startup script, default ON" "ON")
# sanitizer options
option(MYGPIOD_ENABLE_ASAN "Enables build with address sanitizer, default OFF" "OFF")
option(MYGPIOD_ENABLE_TSAN "Enables build with thread sanitizer, default OFF" "OFF")
option(MYGPIOD_ENABLE_UBSAN "Enables build with undefined behavior sanitizer, default OFF" "OFF")

# cmake modules
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckIPOSupported)
include(GNUInstallDirs)

# set debug define for source
if(NOT DEFINED MYGPIOD_DEBUG)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MYGPIOD_DEBUG "ON")
  else()
    set(MYGPIOD_DEBUG "OFF")
  endif()
endif()

# configure some files - version and path information
configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/config.h)
configure_file(cmake/Install.cmake.in cmake/Install.cmake @ONLY)
configure_file(contrib/initscripts/mygpiod.service.in contrib/initscripts/mygpiod.service @ONLY)
configure_file(contrib/initscripts/mygpiod.sysVinit.in contrib/initscripts/mygpiod.sysVinit @ONLY)
configure_file(contrib/initscripts/mygpiod.openrc.in contrib/initscripts/mygpiod.openrc @ONLY)
configure_file(contrib/etc/mygpiod.conf.in contrib/etc/mygpiod.conf @ONLY)

# set myGPIOD specific debug define
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    "-DMYGPIOD_DEBUG=ON"
  )
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Debug|Release|RelWithDebInfo|MinSizeRel)")
  # set strict global compile flags
  add_compile_options(
    "-fdata-sections"
    "-ffunction-sections"
    "-fstack-protector-strong"
    "-pedantic"
    "-Wall"
    "-Werror"
    "-Wextra"
    "-Wformat"
    "-Wformat-security"
    "-Winit-self"
    "-Wmissing-include-dirs"
    "-Wnested-externs"
    "-Wold-style-definition"
    "-Wredundant-decls"
    "-Wshadow"
    "-Wsign-compare"
    "-Wstrict-prototypes"
    "-Wundef"
    "-Wuninitialized"
    "-Wunused-parameter"
    "-Wvla"
    "-Wwrite-strings"
  )

  # check for supported compiler flags
  foreach(FLAG IN ITEMS "-std=gnu17" "-fstack-clash-protection" "-fcf-protection" "-fno-plt")
    message("Checking for compiler flag ${FLAG}")
    unset(COMPILER_SUPPORTS_FLAG CACHE)
    unset(COMPILER_SUPPORTS_FLAG)
    check_c_compiler_flag("${FLAG}" COMPILER_SUPPORTS_FLAG)
    if(COMPILER_SUPPORTS_FLAG)
      add_compile_options("${FLAG}")
    endif()
  endforeach()

  if(NOT MYGPIOD_ENABLE_ASAN AND NOT MYGPIOD_ENABLE_UBSAN)
    # incompatible with address sanitizers
    add_compile_options("-D_FORTIFY_SOURCE=2")
  endif()
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter compile options
endif()

# sanitizers
set(ASAN_FLAGS
  "-fsanitize=address"
  "-fsanitize=leak"
)

set(UBSAN_FLAGS
  "-fsanitize=undefined"
)

set(TSAN_FLAGS
  "-fsanitize=thread"
)

if(MYGPIOD_ENABLE_ASAN)
  message("Compiling with address sanitizer")
  add_compile_options(
    ${ASAN_FLAGS}
    "-fno-omit-frame-pointer"
  )
elseif(MYGPIOD_ENABLE_UBSAN)
  message("Compiling with undefined behavior sanitizer")
  add_compile_options(
    ${UBSAN_FLAGS}
    "-fno-omit-frame-pointer"
  )
elseif(MYGPIOD_ENABLE_TSAN)
  message("Compiling with thread sanitizer")
  add_compile_options(
    ${TSAN_FLAGS}
    "-fno-omit-frame-pointer"
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    "-ggdb"
    "-Og"
  )
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
elseif(CMAKE_BUILD_TYPE MATCHES "(Release|RelWithDebInfo|MinSizeRel)")
  add_compile_options(
    "-fPIE"
    "-O2"
  )
  if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message("Generating binary with debug symbols")
    add_compile_options("-g")
  endif()
  # IPO/LTO support
  check_ipo_supported()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  # if CMAKE_BUILD_TYPE is not Debug, Release, RelWithDebInfo or MinSizeRel
  # do not alter compile options
endif()

# linker flags
if(MYGPIOD_ENABLE_ASAN)
  add_link_options(${ASAN_FLAGS})
elseif(MYGPIOD_ENABLE_UBSAN)
  add_link_options(${UBSAN_FLAGS})
elseif(MYGPIOD_ENABLE_TSAN)
  add_link_options(${TSAN_FLAGS})
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Release|RelWithDebInfo|MinSizeRel)")
  add_link_options(
    "-pie"
    "-Wl,-z,relro,-z,now,--gc-sections,--as-needed"
  )
  if(CMAKE_BUILD_TYPE MATCHES "(Release|MinSizeRel)")
    message("Generating stripped binary")
    add_link_options("-s")
  endif()
else()
  # if CMAKE_BUILD_TYPE is not Release, RelWithDebInfo or MinSizeRel
  # do not alter link options
endif()

# the main mygpiod target
add_subdirectory("src")

target_link_libraries(mygpiod gpiod)

install(TARGETS mygpiod DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/Install.cmake)