# SPDX-License-Identifier: GPL-3.0-or-later
# myMPD (c) 2018-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_executable(mygpiod "")

if(CMAKE_BUILD_TYPE MATCHES "(Release|Debug)")
  target_compile_options(mygpiod
    PRIVATE
      "-Wconversion"
      "-Wformat=2"
  )
endif()

target_include_directories(mygpiod
  PRIVATE 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_sources(mygpiod PRIVATE
  main.c
  actions/system.c
  actions/gpioset.c
  event_loop/event_loop.c
  event_loop/signal_handler.c
  gpio/action.c
  gpio/chip.c
  gpio/event.c
  gpio/input.c
  gpio/output.c
  gpio/timer.c
  lib/action.c
  lib/config.c
  lib/events.c 
  lib/list.c
  lib/log.c
  lib/timer.c
  lib/util.c
  server/gpio.c
  server/idle.c
  server/protocol.c
  server/response.c
  server/socket.c
)

target_link_libraries(mygpiod
  gpiod
  sds
  mygpio-common
)

install(TARGETS mygpiod DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
