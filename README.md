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
    - long press release
  - Integrated actions
    - GPIO settings
    - HTTP requests
    - Lua scripting
    - MPD client
    - myMPD client
    - System commands
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

## Installation

- Build it yourself.
- Use the [docker image](https://github.com/jcorporation?tab=packages&repo_name=myGPIOd).
- Use [prebuild packages](https://download.opensuse.org/repositories/home:/jcorporation/).

## Build

Building myGPIOd is straight forward.

### Dependencies

- C build environment
- cmake >= 3.13
- libgpiod-dev >= 2.0.0
- Optional:
  - libcurl
  - libmpdclient2
  - lua >= 5.3.0

Only the current Fedora release packages the version 2 of libgpiod. If libgpiod version 2 is not found, the cmake script compiles it as a static library.

### Build myGPIOd

This builds and installs the `mygpiod` daemon, `mygpioc` command line tool, the shared library `libmygpio`, the associated header files for development and the documentation.

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Run cmake: `cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .`
4. Build: `make -C build`
5. Install (as root): `make -C build install`

## Run

myGPIOd needs rw access to the gpio chip device (e. g. `/dev/gpiochip0`).

```sh
/usr/bin/mygpiod [/etc/mygpiod.conf]
```

The cmake install script creates a startup script for systemd, openrc or sysVinit.

### Systemd

You must enable and start the service manually. Use `systemctl enable mygpiod` to enable myGPIOd at startup and `systemctl start mygpiod` to start myGPIOd now.

myGPIOd logs to STDERR, you can see the live logs with `journalctl -fu mygpiod`.

The default myGPIOd service unit uses the `DynamicUser=` directive, therefore no static mygpiod user is created. If you want to change the group membership of this dynamic user, you must add an override.

**Example: add the mygpiod user to the gpio group**

```sh
mkdir /etc/systemd/system/mygpiod.service.d
echo -e '[Service]\nSupplementaryGroups=gpio' > /etc/systemd/system/mygpiod.service.d/gpio-group.conf
```

- **Note:** The default systemd service unit supports only systemd v235 and above.

### Docker

Example docker compose file to start myGPIOd.

```yml
---
version: "3.x"
services:
  mygpiod:
    image: ghcr.io/jcorporation/mygpiod/mygpiod
    container_name: mygpiod
    network_mode: "host"
    user: 1000:1000
    environment:
      - MPD_HOST=localhost
    volumes:
      - /dev/gpiochip0:/dev/gpiochip0
      - /etc/mygpiod.conf:/etc/mygpiod.conf
      - /etc/mygpiod.d/:/etc/mygpiod.d/
    restart: unless-stopped
```

To run `mygpioc` in the already running mygpiod container:

```sh
docker exec mygpiod mygpioc gpiolist
```

## Configuration steps

- Adapt the configuration file `/etc/mygpiod.conf` to your needs. All options are documented in the file.
- Create GPIO configuration files in the directory `/etc/mygpiod.d`. There are documented example configuration files for input and output configuration. myGPIOd only accesses GPIOs configured in this files.
- GPIO configuration file names: `<gpio number>.<direction>`
  - `<gpio number>`: This is the line number of the GPIO.
  - `<direction>`: Configures the GPIO line direction, `<in>` for input and `<out>` for output.

## Events

Events are triggered through changes of input GPIO values.

| EVENT | DESCRIPTION |
| ----- | ----------- |
| falling | State of GPIO has changed from active to inactive. |
| rising | State of GPIO has changed from inactive to active. |
| long_press | GPIO was pressed long. Event is triggered after configurable delay. |
| long_press_release | GPIO changing it's state after a long press event. |

## Actions

Each event can have multiple actions. Actions and its arguments are delimited by a colon, arguments are delimited by space.

| ACTION | ARGUMENTS | DESCRIPTION |
| ------ | --------- | ----------- |
| gpioblink | `<gpio>` `<timeout>` `<interval>` | Toggle the value of the GPIO in given timeout and interval. Set interval to 0 to blink only once. |
| gpioset | `<gpio>` `<active\|inactive>` | Sets the value of a GPIO. |
| gpiotoggle | `<gpio>` | Toggles the value of a GPIO. |
| http | `{GET\|POST}` `{uri}` [`{content-type}` `{postdata}`] | Submits a HTTP request in a new child process. If `postdata` starts with `<</`, the string after the `<<` is interpreted as an absolute filepath from which the postdata is read. Requires libcurl. |
| lua | `{lua function}` [`{option1}` `{option2}` ...] | Calls a user defined lua function. |
| mpc | `{mpd command}` [`{option1}` `{option2}` ...] | Connects to MPD and issues the command with options. It uses the default connection settings from libmpdclient. A maximum of 10 options are supported. Requires libmpdclient.|
| mympd | `{uri}` `{partition}` `{script}` | Calls the myMPD api to execute a script in a new child process. Requires libcurl. |
| system | `{command}` | Executes an executable or script in a new child process. No arguments are allowed. |

myGPIOd can take actions on rising, falling and long_press events. Long press is triggered by a falling or rising event and does not disable the triggering event, but the release event. To use a button for normal press and long_press request both events and use one event for long and the other for short press. The example below illustrates this.

### Lua

The lua action calls user defined lua functions. The lua script itself is loaded on startup of myGPIOd (`lua_file` configuration setting). All lua functions in this file are registered and can be called with the lua action.

myGPIOd registers custom lua functions to provide access to the actions.

| Lua function | Description |
| ------------ | ----------- |
| `gpioBlink({GPIO}, {timeout}, {interval})` | Toggle the value of the GPIO in given timeout and interval. |
| `gpioGet({GPIO})` | Returns the GPIO state: 1 = active, 0 = inactive |
| `gpioSet({GPIO}, {1\|0})` | Sets the state of an output GPIO: 1 = active, 0 = inactive |
| `gpioToggle({GPIO})` | Toggles the state of an output GPIO: 1 = active, 0 = inactive |
| `http({GET\|POST}, {uri}, {content-type}, {postdata})` | Submits a HTTP request in a new child process. |
| `mpc({mpd protocol command})` | Runs a mpd protocol command |
| `mympd({uri}, {partition}, {script})` | Calls the myMPD api to execute a script in a new child process. |
| `system({command})` | Executes an executable or script in a new child process. |

**Example gpio config**
```
# Call the lua function `testFunc` with the argument `testArg` on rising event.
action_rising = lua:testFunc testArg
```

**Example lua file**
```lua
function testFunc(arg1)
  -- get the argument
  print("Arg1:"..arg1)
  -- get value of GPIO 4
  v = gpioGet(4)
  -- set value of GPIO 5 to high
  gpioSet(5, 1)
end
```

## Example configuration

This example configuration does the following:

- Configures GPIO 3 as input:
  - Enables the pull-up resistor on start
  - Sets GPIO 6 to active on falling event
  - Calls `/usr/local/bin/reboot.sh` after a button press (falling) of 2 seconds length
  - Toggles the value of GPIO 6 on release event of the long press event
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
- Configures GPIO 8 as input:
  - Enables the pull-up resistor on start
  - On falling event:
    - Set GPIO 6 for 1000 ms to active
    - Execute the Jukebox script through the myMPD API

**/etc/mygpiod.conf**
```
# GPIO chip to use
chip = /dev/gpiochip0

# The loglevel
loglevel = info

# Log to stdout
syslog = 0

# Directory for GPIO configuration files
gpio_dir = /etc/mygpiod.d
```

**/etc/mygpiod.d/3.in**
```
# Configuration file for GPIO 3 as input
# Request falling and rising events
event_request = both

# Active is high
active_low = false

# Enable the internal pull-up resistor
bias = pull-up

# Short press does a poweroff
# The rising event is not triggered if GPIO 6 is pressed longer than 2000 ms.
action_rising = system:/usr/local/bin/poweroff.sh

# Reboot on long press and activate a LED for maximal 2s while GPIO 6 is pressed.

# Set GPIO 6 active on falling
action_falling = gpioset:6 active

# Enable long press for falling.
long_press_event = falling

# Set the long press timeout to 2000 ms.
long_press_timeout = 2000

# Disable the long press interval.
long_press_interval = 0

# Action for long press is to run a script.
long_press_action = system:/usr/local/bin/reboot.sh

# Action for releasing the button after a long press is to toggle the value of GPIO 6.
long_press_release_action = gpiotoggle:6
```

**/etc/mygpiod.d/4.in**
```
# Configuration file for GPIO 4 as input
# Request the falling event
event_request = falling

# Enable the internal pull-up resistor
bias = pull-up

# Use libmpdclient to connect to mpd and send the command `next`
action_falling = mpc:next
```

**/etc/mygpiod.d/5.out**
```
# Configuration file for GPIO 5 as output
# Set the GPIO to active
value = active
```

**/etc/mygpiod.d/6.out**
```
# Configuration file for GPIO 6 as output
# Set the GPIO to inactive
value = inactive
```

**/etc/mygpiod.d/7.in**
```
# Configuration file for GPIO 7 as input
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

**/etc/mygpiod.d/8.in**
```
# Configuration file for GPIO 8 as input
# Request the falling event
event_request = falling

# Enable the internal pull-up resistor
bias = pull-up

# Execute the Jukebox script through the myMPD API
action_falling = gpioblink:6 1000 0
action_falling = mympd:https://127.0.0.1 default Jukebox
```

## Protocol

myGPIOd can be controlled and queried through a simple line-based text protocol.

- [Protocol specification](PROTOCOL.md)

```sh
socat unix-client:/run/mygpiod/socket stdio
```

## Command line client

The `mygpioc` command line client connects to the socket `/run/mygpiod/socket` to control myGPIOd.

```sh
mygpioc -h
```

## Client library

The client library is documented in the header files. You can use doxygen to create the html documentation locally.

You can find a usage example [here](libmygpio/example/main.c).

The `mygpioc` command line client is also based on this library.

## Copyright

2020-2024 Juergen Mang <mail@jcgames.de>
