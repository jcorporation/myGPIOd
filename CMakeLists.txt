#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

# minimal cmake version needed for new option handling
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0003 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  cmake_policy(SET CMP0135 NEW)
endif()

# myGPIO is written in C
# supported compilers: gcc, clang
project (mygpiod
  VERSION 0.7.0
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
option(MYGPIOD_ENABLE_ACTION_MPC "Enables the mpc action, requires libmpdclient, default ON" "ON")
option(MYGPIOD_ENABLE_ACTION_HTTP "Enables the http action, requires libcurl, default ON" "ON")
option(MYGPIOD_ENABLE_ACTION_LUA "Enables the lua action, requires lua, default ON" "ON")
option(MYGPIOD_DOC "Installs documentation, default ON" "ON")
option(MYGPIOD_MANPAGES "Creates and installs manpages" "ON")
option(MYGPIOD_STARTUP_SCRIPT "Installs the startup script, default ON" "ON")
option(MYGPIOD_CLIENT "Builds the myGPIOd client, default ON" "ON")
option(MYGPIOD_DAEMON "Builds the myGPIOd daemon, default ON" "ON")
option(MYGPIOD_HEADER "Installs the myGPIOd headers, default ON" "ON")
option(MYGPIOD_LIBRARY "Builds the myGPIOd library, default ON" "ON")
option(MYGPIOD_STATIC_LIBGPIOD "Builds the libgpiod v2 as static library, default OFF" "OFF")
# sanitizer options
option(MYGPIOD_ENABLE_ASAN "Enables build with address sanitizer, default OFF" "OFF")
option(MYGPIOD_ENABLE_TSAN "Enables build with thread sanitizer, default OFF" "OFF")
option(MYGPIOD_ENABLE_UBSAN "Enables build with undefined behavior sanitizer, default OFF" "OFF")

# cmake modules
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckIPOSupported)
include(GNUInstallDirs)

# custom cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# find required dependencies
if(MYGPIOD_DAEMON)
  if(NOT MYGPIOD_STATIC_LIBGPIOD)
    find_package(LIBGPIOD)
    if (LIBGPIOD_FOUND)
      message("Found libgpiod version is: ${LIBGPIOD_VERSION}")
      if(NOT LIBGPIOD_VERSION VERSION_GREATER_EQUAL "2.0.0")
        message("Required libgpiod version >= 2.0.0")
        set(LIBGPIOD_FOUND "OFF")
      endif()
    endif()
  endif()
  if (NOT LIBGPIOD_FOUND)
    include("cmake/InstallLIBGPIOD.cmake")
  endif()
endif()

# find optional dependencies
if(MYGPIOD_ENABLE_ACTION_MPC)
  find_package(LIBMPDCLIENT)
  if (NOT LIBMPDCLIENT_FOUND)
    message("MPC action is disabled because libmpdclient was not found")
    set(MYGPIOD_ENABLE_ACTION_MPC "OFF")
  endif()
else()
  message("MPC action is disabled by user")
endif()

if(MYGPIOD_ENABLE_ACTION_HTTP)
  find_package(CURL)
  if (NOT CURL_FOUND)
    message("HTTP action is disabled because curl was not found")
    set(MYGPIOD_ENABLE_ACTION_HTTP "OFF")
  endif()
else()
  message("HTTP action is disabled by user")
endif()

if(MYGPIOD_ENABLE_ACTION_LUA)
  if(EXISTS "/etc/alpine-release")
    set(ENV{LUA_DIR} "/usr/lib/lua5.4")
  endif()
  message("Searching for lua")
  find_package(Lua)
  if(LUA_FOUND)
    if(NOT LUA_VERSION_STRING VERSION_GREATER_EQUAL "5.3.0")
      message("Lua is disabled because a version lower than 5.3.0 was found")
      set(MYGPIOD_ENABLE_ACTION_LUA "OFF")
    endif()
  else()
    message("Lua is disabled because it was not found")
    set(MYGPIOD_ENABLE_ACTION_LUA "OFF")
  endif()
endif()

# configure some files - version and path information
configure_file(mygpio-common/compile_time.h.in ${PROJECT_BINARY_DIR}/compile_time.h)
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
  set(MYGPIOD_DEBUG "ON")
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Debug|Release|RelWithDebInfo|MinSizeRel)")
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
    "-Wformat"
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

  if(NOT MYGPIOD_ENABLE_ASAN AND NOT MYGPIOD_ENABLE_UBSAN)
    # incompatible with address sanitizers
    if(NOT CMAKE_C_FLAGS MATCHES "_FORTIFY_SOURCE")
      add_compile_options("-D_FORTIFY_SOURCE=2")
    endif()
  endif()
else()
  # if CMAKE_BUILD_TYPE is neither Release nor Debug,
  # do not alter compile options
endif()

# sanitizers
set(ASAN_FLAGS
  "-fsanitize=address"
  "-fsanitize=leak"
  "-latomic"
)

set(UBSAN_FLAGS
  "-fsanitize=undefined"
  "-latomic"
)

set(TSAN_FLAGS
  "-fsanitize=thread"
  "-latomic"
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

# the main mygpio targets
add_subdirectory("mygpio-common")
if(MYGPIOD_LIBRARY OR MYGPIOD_CLIENT)
  add_subdirectory("libmygpio")
endif()
if(MYGPIOD_CLIENT)
  add_subdirectory("mygpioc")
endif()
if(MYGPIOD_DAEMON)
  add_subdirectory("mygpiod")
endif()

# dist targets
add_subdirectory(dist)

# install
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/Install.cmake)
