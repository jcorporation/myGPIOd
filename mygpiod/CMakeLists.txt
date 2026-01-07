# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mympd

add_executable(mygpiod "")

target_include_directories(mygpiod
  PRIVATE 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    "${LIBGPIOD_INCLUDE_DIRS}"
    "${LIBMHD_INCLUDE_DIRS}"
)

target_sources(mygpiod
  PRIVATE
    main.c
    actions/gpio.c
    actions/system.c
    event_loop/event_loop.c
    event_loop/signal_handler.c
    gpio/action.c
    gpio/chip.c
    gpio/event.c
    gpio/gpio.c
    gpio/input.c
    gpio/output.c
    gpio/timer.c
    gpio/util.c
    lib/action.c
    lib/config.c
    lib/events.c
    lib/json_print.c
    lib/list.c
    lib/log.c
    lib/sds_extras.c
    lib/timer.c
    raspberry/vcgencmd.c
    server_http/httpd.c
    server_http/rest_api_gpio.c
    server_http/rest_api_raspberry.c
    server_http/rest_api.c
    server_http/util.c
    server_http/webui.c
    server_socket/gpio.c
    server_socket/idle.c
    server_socket/protocol.c
    server_socket/raspberry.c
    server_socket/response.c
    server_socket/socket.c
)

if (MYGPIOD_DEBUG)
  target_sources(mygpiod
    PRIVATE
      server_socket/event.c
  )
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Release|RelWithDebInfo|MinSizeRel)")
  target_link_options(mygpiod
    PUBLIC
      "-pie"
  )
endif()

target_link_libraries(mygpiod
  "${LIBGPIOD_LIBRARIES}"
  "${LIBMHD_LIBRARIES}"
  sds
  mygpio-common
)

if (MYGPIOD_ENABLE_ACTION_MPC)
  target_include_directories(mygpiod
    PRIVATE 
      "${LIBMPDCLIENT_INCLUDE_DIRS}"
  )
  target_sources(mygpiod PRIVATE
    actions/mpc.c
  )
  target_link_libraries(mygpiod
    "${LIBMPDCLIENT_LIBRARIES}"
  )
endif()

if (MYGPIOD_ENABLE_ACTION_HTTP)
  target_include_directories(mygpiod
    PRIVATE 
      "${LIBCURL_INCLUDE_DIRS}"
  )
  target_sources(mygpiod
    PRIVATE
      actions/http.c
      actions/mympd.c
  )
  target_link_libraries(mygpiod
    "${CURL_LIBRARIES}"
  )
endif()

if(MYGPIOD_ENABLE_ACTION_LUA)
  target_include_directories(mygpiod
    PRIVATE 
      "${LUA_INCLUDE_DIR}"
  )
  target_sources(mygpiod
    PRIVATE
      actions/lua.c
  )
  target_link_libraries(mygpiod
    "${LUA_LIBRARIES}"
  )
endif()

install(TARGETS mygpiod DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
