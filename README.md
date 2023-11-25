# myGPIOd

myGPIOd is a very lightweight daemon to call scripts on GPIO events. It has no dependencies but the libgpiod2 library.

It is based on the gpiomon tool from [libgpiod](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/)

## Features

- Calls scripts on GPIO events
  - raising
  - falling
  - long press
- Supports pull-up and pull-down bias

## Build Dependencies

- cmake >= 3.13
- libgpiod-dev >= 1.5.0 and < 2.0.0

## Quick Build Instructions

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Install dependencies (as root): `./build.sh installdeps`
4. Build: `./build.sh release`
5. Install (as root): `./build.sh install`

The `build.sh` script is only a wrapper for cmake. You can use the default cmake workflow to compile myGPIOd.

```
cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make -C build
```

## Run

myGPIOd needs rw access to the gpio chip device (e. g. /dev/gpiochip0).

The `./build.sh` script creates a mygpiod user with the group gpio. The GPIO group has on many systems sufficient privileges, do not run myGPIOd as root.

Adapt the configuration file `/etc/mygpiod.conf` to your needs.

```
runuser -u mygpiod -g gpio -- /usr/bin/mygpiod [/etc/mygpiod.conf]
```

The `./build.sh` script installs a startup script for systemd, openrc or sysVinit.

## Scripts

Scripts are executed in a separate process with the `execve` function.

myGPIOd sets following environment variables:

- MYGPIOD_GPIO
- MYGPIOD_EDGE
- MYGPIOD_LONG_PRESS

## Example configuration

This example configuration does the following:

- Enables the pull-up resistor for gpio number 3 on start
- Calls `/usr/local/bin/reboot.sh` after a button press of 2 seconds length
- Calls `/usr/local/bin/poweroff.sh` on a short press

```
chip=0
edge=both
active_low=true
loglevel=4
syslog=0
bias=pull-up
#gpio,edge,long_press,cmd
3,rising,0,/usr/local/bin/poweroff.sh
3,falling,2,/usr/local/bin/reboot.sh
```

## Copyright

2020-2023 Juergen Mang <mail@jcgames.de>
