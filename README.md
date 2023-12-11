# myGPIOd

myGPIOd is a lightweight GPIO controlling daemon. It is written in C and has no dependencies but the libgpiod2 library version 2.

It consists of a daemon, a client library and a command line tool.

## Features

- **mygpiod - the daemon component**
  - Call executables on GPIO events
    - rising
    - falling
    - long press
  - Set various gpio attributes (bias, debounce, ...)
  - Set the output value of gpios
  - Provides a unix socket with simple line-based text protocol
    - List gpio configuration
    - Set and get GPIO values
    - Get notifications of GPIO events
- **libmygpio - the client library**
  - Simple C client library
  - High level API
- **mygpioc - the command line client**
  - Connects to the mygpiod socket to control the various functions.

## Build Dependencies

- cmake >= 3.13
- libgpiod-dev >= 2.0.0

## Quick Build Instructions

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Install dependencies (as root): `./build.sh installdeps`
4. Build: `./build.sh release`
5. Install (as root): `./build.sh install`

The `build.sh` script is only a wrapper for cmake. You can use the default cmake workflow to compile myGPIOd.

```
cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .
make -C build
```

## Run

myGPIOd needs rw access to the gpio chip device (e. g. /dev/gpiochip0).

The `./build.sh` script and the packages are creating a mygpiod system user with the group gpio. The GPIO group has on many systems sufficient privileges, do not run myGPIOd as root.

Adapt the configuration file `/etc/mygpiod.conf` to your needs.

```
runuser -u mygpiod -g gpio -- /usr/bin/mygpiod [/etc/mygpiod.conf]
```

The `./build.sh` script installs a startup script for systemd, openrc or sysVinit.

## Actions

Each event can have multiple actions. Actions and its arguments are delimited by a colon, arguments are delimited by space.

| ACTION | ARGUMENTS | DESCRIPTION |
| ------ | --------- | ----------- |
| gpioset | `<gpio>` `<value>` | Sets the value of a GPIO. |
| system | `<command>` | Executes an executable or script in a new child process. |

## Example configuration

This example configuration does the following:

- Configures gpio 3 as input:
  - Enables the pull-up resistor on start
  - Sets GPIO 6 to low on rising event
  - Sets GPIO 6 to high on falling event
  - Calls `/usr/local/bin/reboot.sh` after a button press (falling) of 2 seconds length
  - Calls `/usr/local/bin/poweroff.sh` on a short press (rising)
- Configures gpio 4 as input:
  - Enables the pull-up resistor on start
  - Calls `/usr/local/bin/poweroff.sh` on a short press (falling)
- Configures gpio 5 as output:
  - Sets the value to high on start

**/etc/mygpiod.conf**
```
chip = /dev/gpiochip0
loglevel = info
syslog = 0
gpio_dir = /etc/mygpiod.d
```

**/etc/mygpiod.d/3.in**
```
request_event = both
active_low = false
bias = pull-up
action_rising = gpioset:6 low
action_rising = system:/usr/local/bin/poweroff.sh

action_falling = gpioset:6 high

long_press_event = falling
long_press_timeout = 2
long_press_action = system:/usr/local/bin/reboot.sh
```

**/etc/mygpiod.d/4.in**
```
request_event = falling
active_low = false
bias = pull-up
action_falling = system:/usr/local/bin/poweroff.sh
```

**/etc/mygpiod.d/5.out**
```
value = high
```

**/etc/mygpiod.d/6.out**
```
value = low
```

## Protocol

myGPIOd can be controlled and queried through a simple line-based text protocol.

- [Protocol specification](PROTOCOL.md)

```sh
socat unix-client:/run/mygpiod/socket stdio
```

## Command line client

The `mygpioc` command line client connects to the socket to control myGPIOd.

```sh
mygpioc -h
```

## Client library

The client library is documented in the header files. You can use doxygen to create the html documentation locally.

You can find a usage example [here](libmygpio/example/main.c)

## Copyright

2020-2023 Juergen Mang <mail@jcgames.de>
