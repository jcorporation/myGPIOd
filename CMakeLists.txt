#
# SPDX-License-Identifier: GPL-2.0-or-later
# myGPIOd (c) 2020 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

cmake_minimum_required(VERSION 3.4)

include(GNUInstallDirs)

project (mygpiod C)
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
  set(DEBUG "OFF")
else()
  set(DEBUG "ON")
endif()

configure_file(src/config.h.in ${PROJECT_BINARY_DIR}/config.h)
configure_file(cmake/CopyConfig.cmake.in cmake/CopyConfig.cmake @ONLY)
configure_file(contrib/initscripts/mygpiod.service.in contrib/initscripts/mygpiod.service @ONLY)
configure_file(contrib/initscripts/mygpiod.sysVinit.in contrib/initscripts/mygpiod.sysVinit @ONLY)
configure_file(contrib/initscripts/mygpiod.openrc.in contrib/initscripts/mygpiod.openrc @ONLY)
configure_file(contrib/etc/mygpiod.conf.in contrib/etc/mygpiod.conf @ONLY)

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
include(CheckCSourceCompiles)

if(${MEMCHECK} MATCHES "TRUE")
  #for use with valgrind
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -Werror -Wvla -Winit-self -Wuninitialized -Werror=array-bounds")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O1 -Wall -Werror -Wvla -Winit-self -Wuninitialized -Werror=array-bounds \
	-fstack-protector-strong -D_FORTIFY_SOURCE=2 -pie -fPIE \
	-Wextra -pedantic -Wformat=2 -Wunused-parameter -Wshadow -Wwrite-strings \
	-Wformat=2  -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -fsanitize=address \
	-fsanitize=undefined -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound \
	-fsanitize=null -fsanitize=return -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=bounds-strict \
	-fsanitize=alignment -fsanitize=object-size -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow \
	-fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=bool -fsanitize=enum -fsanitize=vptr -static-libasan")
endif()

#linker flags
set (CMAKE_EXE_LINKER_FLAGS "-Wl,-z,relro -Wl,-z,now")

set(SOURCES
  src/main.c
  src/config.c
  src/log.c
)


add_executable(mygpiod ${SOURCES})
target_link_libraries(mygpiod gpiod)

install(TARGETS mygpiod DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/cmake/CopyConfig.cmake)
