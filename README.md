# myGPIOd

myGPIOd is a small daemon to set modes of Raspberry Pi GPIOs and to call scripts on GPIO events. 
It uses the GPIO Sysfs interface from the linux kernel. myGPIOd has no dependencies beside the standard c libraries.

## Build Dependencies

- cmake >= 3.4
- libasan3: for debug builds only

## Quick Build Instructions

1. Get myGPIOd tarball from [GitHub](https://github.com/jcorporation/myGPIOd/releases/latest)
2. Extract myGPIOd tarball and change path to this directory
3. Install dependencies (as root): `./build.sh installdeps`
4. Build: `./build.sh release`
5. Install(as root): `./build.sh install`

## Run

myGPIOd needs rw access to the `/sys/class/gpio/` directory. The `./build.sh` script creates a mygpiod user with the group gpio. 
The GPIO group has on many systems sufficient privileges, do not run myGPIOd as root.

Adapt the configuration file `/etc/mygpiod.conf` to your needs.

```
runuser -u mygpiod -g gpio /usr/bin/mygpiod
```

The `./build.sh` script installs a startup script for systemd, openrc or sysVinit.

## Example configuration

This example configuration calls `poweroff` if GPIO 3 is falling from high (1) to low (0).

```
#gpio,direction,edge,active_low,cmd
3,in,falling,1,/usr/sbin/poweroff 
```

## Copyright

2020 Juergen Mang <mail@jcgames.de>
