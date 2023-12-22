# myGPIOd

myGPIOd is a lightweight GPIO controlling framework. It is written in C and has no hard dependencies but the libgpiod2 library version 2.

It consists of a daemon, a client library and a command line tool. It is designed to run on Raspberry PIs and similar devices.

I wrote this tool primarily for [myMPDos](https://github.com/jcorporation/myMPDos) and [myMPD](https://github.com/jcorporation/myMPD).

myGPIOd can communicate natively with MPD and also integrates nicely with all HTTP APIs.

## Features

- **mygpiod - the daemon component**
  - Call actions on GPIO events
    - rising
    - falling
    - long press (with optional interval)
  - Set various GPIO attributes (bias, debounce, ...)
  - Set the output value of GPIOs
  - Provides a unix socket with a simple line-based text protocol
    - List GPIO configuration
    - Set and get GPIO values
    - Get notifications of GPIO events
- **libmygpio - the client library**
  - Simple C client library
  - High level API
  - Integration in fd based event loops
- **mygpioc - the command line client**
  - Connects to the mygpiod socket to control the various functions.

## Build

Building myGPIOd is straight forward.

### Dependencies

- C build environment
- cmake >= 3.13
- libgpiod-dev >= 2.0.0
- Optional:
  - libcurl
  - libmpdclient2

Only the current Fedora release packages the version 2 of libgpiod. For all other distributions, you must compile libgpiod yourself.

### Build libgpiod

Install it in `/usr/local` to avoid conflicts with an already installed libgpiod version.

1. Get latest release tarball from [kernel.org git](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/)
2. Extract myGPIOd tarball and change path to this directory
3. Run autotools: `./autogen.sh --enable-tools=yes --prefix=/usr/local`
4. Build: `make`
5. Install (as root): `make install`

### Build myGPIOd

This builds and installs the `mygpiod` daemon, `mygpioc` command line tool, the shared library `libmygpio`, the associated header files for development and the documentation.

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Run cmake: `cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .`
4. Build: `make -C build`
5. Install (as root): `make -C build install`

## Run

myGPIOd needs rw access to the gpio chip device (e. g. `/dev/gpiochip0`).

### Configuration steps

- Adapt the configuration file `/etc/mygpiod.conf` to your needs.
- Create GPIO configuration files in the directory `/etc/mygpiod.d`. There are example configuration files for input and output configuration. myGPIOd only accesses GPIOs configured in this files.

```sh
/usr/bin/mygpiod [/etc/mygpiod.conf]
```

The cmake install script creates a startup script for systemd, openrc or sysVinit.

## Events and actions

Each event can have multiple actions. Actions and its arguments are delimited by a colon, arguments are delimited by space.

| ACTION | ARGUMENTS | DESCRIPTION |
| ------ | --------- | ----------- |
| gpioblink | `<gpio>` `<timeout>` `<interval>` | Toggle the value of the GPIO in given timeout and interval. Set interval to 0 to blink only once. |
| gpioset | `<gpio>` `<active\|inactive>` | Sets the value of a GPIO. |
| gpiotoggle | `<gpio>` | Toggles the value of a GPIO. |
| http | `{GET\|POST}` `{uri}` [`{content-type}` `{postdata}`] | Submits a HTTP request in a new child process. If `postdata` starts with `<</`, the string after the `<<` is interpreted as an absolute filepath from which the postdata is read. Requires libcurl. |
| mpc | `{mpd command}` [`{option1}` `{option2}` ...] | Connects to MPD and issues the command with options. It uses the default connection settings from libmpdclient. A maximum of 10 options are supported. Requires libmpdclient.|
| mympd | `{uri}` `{partition}` `{script}` | Calls the myMPD api to execute a script. Requires libcurl. |
| system | `<command>` | Executes an executable or script in a new child process. No arguments for the command are allowed. |

myGPIOd can take actions on rising, falling and long_press events. Long press is triggered by a falling or rising event and does not disable the triggering event, but the release event. To use a button for normal press and long_press request both events and use one event for long and the other for short press. The example below illustrates this.

## Example configuration

This example configuration does the following:

- Configures GPIO 3 as input:
  - Enables the pull-up resistor on start
  - Sets GPIO 6 to active on falling event
  - Calls `/usr/local/bin/reboot.sh` after a button press (falling) of 2 seconds length
  - Toggles the value of GPIO 6 on long_press
  - Calls `/usr/local/bin/poweroff.sh` on a short press (rising)
- Configures GPIO 4 as input:
  - Enables the pull-up resistor on start
  - Runs the mpd `next` command on a short press (falling)
- Configures GPIO 5 as output:
  - Sets the value to active on start
- Configures GPIO 6 as output:
  - Sets the value to inactive on start
- Configures GPIO 7 as input:
  - Enables the pull-up resistor on start
  - Enabled the long press action for falling event
  - Increases the mpd volume by 5 % after 100 ms and each 500 ms as long the button is pressed

**/etc/mygpiod.conf**
```
chip = /dev/gpiochip0
loglevel = info
syslog = 0
gpio_dir = /etc/mygpiod.d
```

**/etc/mygpiod.d/3.in**
```
# Request falling and rising events
event_request = both

# Active is high
active_low = false

# Enable the internal pull-up resistor
bias = pull-up

# Short press does a poweroff
# The rising event is not triggered if GPIO 6 is pressed longer than 2000 ms.
action_rising = system:/usr/local/bin/poweroff.sh

# Reboot on long press and activate a LED for maximal 2s while GPIO 6 is pressed
# Set GPIO 6 active on falling
action_falling = gpioset:6 active

# Enable long press for falling
long_press_event = falling
long_press_timeout = 2000
long_press_interval = 0
long_press_action = gpiotoggle:6
long_press_action = system:/usr/local/bin/reboot.sh
```

**/etc/mygpiod.d/4.in**
```
# Request the falling event
event_request = falling

# Enable the internal pull-up resistor
bias = pull-up

# Use libmpdclient to connect to mpd and send the command `next`
action_falling = mpc:next
```

**/etc/mygpiod.d/5.out**
```
# Set the GPIO to active
value = active
```

**/etc/mygpiod.d/6.out**
```
# Set the GPIO to inactive
value = inactive
```

**/etc/mygpiod.d/7.in**
```
# Request the falling event
event_request = falling

# Enable the internal pull-up resistor
bias = pull-up

# Enable long press action for the falling event
long_press_event = falling

# Set initial long press timeout to 100 ms
long_press_timeout = 100

# Set interval to 500 ms, action is repeated in this interval until the value changes again
long_press_interval = 500

# Use libmpdclient to connect to mpd and send the command `volume +5`
# This increases the volume by 5 percent
long_press_action = mpc:volume +5
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

You can find a usage example [here](libmygpio/example/main.c).

The `mygpioc` command line client is also based on this library.

## Copyright

2020-2023 Juergen Mang <mail@jcgames.de>
