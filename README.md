# myGPIOd

myGPIOd is a small daemon to call scripts on GPIO events.

It is based on the gpiomon tool from [libgpiod](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/about/)

## Build Dependencies

- cmake >= 3.13
- libgpiod-dev >= 1.5.0
- libasan3: for debug builds only

## Quick Build Instructions

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Install dependencies (as root): `./build.sh installdeps`
4. Build: `./build.sh release`
5. Install (as root): `./build.sh install`

## Dependencies

- libgpiod2

## Run

myGPIOd needs rw access to the gpio chip device (e. g. /dev/gpiochip0).

The `./build.sh` script creates a mygpiod user with the group gpio. The GPIO group has on many systems sufficient privileges, do not run myGPIOd as root.

Adapt the configuration file `/etc/mygpiod.conf` to your needs.

```
runuser -u mygpiod -g gpio -- /usr/bin/mygpiod [/etc/mygpiod.conf]
```

The `./build.sh` script installs a startup script for systemd, openrc or sysVinit.

## Example configuration

This example configuration calls `poweroff` if GPIO 3 is falling from high (1) to low (0).

```
chip=0
edge=falling
active_low=true
loglevel=4
syslog=0
#gpio,edge,cmd
3,falling,sudo /usr/sbin/poweroff 
```

## Copyright

2020-2023 Juergen Mang <mail@jcgames.de>
