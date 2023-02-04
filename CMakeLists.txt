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
  VERSION 0.2.2
  LANGUAGES C
)

# set debug define for source
if(NOT DEFINED MYGPIOD_DEBUG)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MYGPIOD_DEBUG "ON")
  else()
    set(MYGPIOD_DEBUG "OFF")
  endif()
endif()

# available options
option(MYGPIOD_DEBUG "Enables myGPIOd debug mode, default OFF, ON for Debug" "OFF")
option(MYGPIOD_ENABLE_LIBASAN "Enables build with libasan, default OFF" "OFF")
option(MYGPIOD_STRIP_BINARY "Enables stripping the binaries for Release, default ON" "ON")

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
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_DEBUG "")

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
configure_file(cmake/CopyConfig.cmake.in cmake/CopyConfig.cmake @ONLY)
configure_file(contrib/initscripts/mygpiod.service.in contrib/initscripts/mygpiod.service @ONLY)
configure_file(contrib/initscripts/mygpiod.sysVinit.in contrib/initscripts/mygpiod.sysVinit @ONLY)
configure_file(contrib/initscripts/mygpiod.openrc.in contrib/initscripts/mygpiod.openrc @ONLY)
configure_file(contrib/etc/mygpiod.conf.in contrib/etc/mygpiod.conf @ONLY)

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

if(CMAKE_BUILD_TYPE MATCHES "(Release|Debug)")
  # set strict global compile flags
  add_compile_options(
    "-fdata-sections"
    "-ffunction-sections"
    "-fstack-protector-strong"
    "-pedantic"
    "-Wall"
    "-Wconversion"
    "-Werror"
    "-Wextra"
    "-Wformat=2"
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

  if(NOT MYGPIOD_ENABLE_LIBASAN)
    #incompatible with libasan
    add_compile_options("-D_FORTIFY_SOURCE=2")
  endif()
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter compile options
endif()

# libasan memory checker
# https://github.com/google/sanitizers/wiki/AddressSanitizer#faq
if(MYGPIOD_ENABLE_LIBASAN)
  message("Compiling with libasan")
  set(LIBASAN_FLAGS
    "-fsanitize=address"
    "-fsanitize=alignment"
    "-fsanitize=bool"
    "-fsanitize=bounds"
    "-fsanitize=bounds-strict"
    "-fsanitize=enum"
    "-fsanitize=float-cast-overflow"
    "-fsanitize=float-divide-by-zero"
    "-fsanitize=integer-divide-by-zero"
    "-fsanitize=nonnull-attribute"
    "-fsanitize=null"
    "-fsanitize=object-size"
    "-fsanitize=return"
    "-fsanitize=returns-nonnull-attribute"
    "-fsanitize=shift"
    "-fsanitize=signed-integer-overflow"
    "-fsanitize=undefined"
    "-fsanitize=unreachable"
    "-fsanitize=vla-bound"
    "-fsanitize=vptr"
  )
  add_compile_options(
    ${LIBASAN_FLAGS}
    "-fno-omit-frame-pointer"
  )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(
    "-fPIE"
    "-O2"
    "-DNDEBUG"
  )
  if(NOT MYGPIOD_STRIP_BINARY)
    message("Generating binary with debug symbols")
    add_compile_options("-g")
  endif()
  # IPO/LTO support
  check_ipo_supported()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(
    "-ggdb"
    "-Og"
  )
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter compile options
endif()

# linker flags
if(MYGPIOD_ENABLE_LIBASAN)
  add_link_options(${LIBASAN_FLAGS})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_link_options(
    "-pie"
    "-Wl,-z,relro,-z,now,--gc-sections,--as-needed"
  )
  if(MYGPIOD_STRIP_BINARY)
    message("Generating stripped binary")
    add_link_options("-s")
  endif()
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter link options
endif()

# set sources
set(SOURCES
  src/main.c
  src/config.c
  src/log.c
)

add_executable(mygpiod ${SOURCES})
target_link_libraries(mygpiod gpiod)

install(TARGETS mygpiod DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/CopyConfig.cmake)
