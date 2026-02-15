Run
===

myGPIOd needs read-write access to the gpio chip device (e. g. ``/dev/gpiochip0``).

.. code:: sh

   /usr/bin/mygpiod [/etc/mygpiod.conf]

The cmake install script creates a startup script for systemd, openrc or sysVinit.

Systemd
-------

You must enable and start the service manually. Use ``systemctl enable mygpiod`` to enable myGPIOd at startup and ``systemctl start mygpiod`` to start myGPIOd now.

myGPIOd logs to STDERR, you can see the live logs with ``journalctl -fu mygpiod``.

The default myGPIOd service unit uses the ``DynamicUser=`` directive, therefore no static mygpiod user is created. If you want to change the group membership of this dynamic user, you must add an override.

**Example: add the mygpiod user to the gpio group**

.. code:: sh

   mkdir /etc/systemd/system/mygpiod.service.d
   echo -e '[Service]\nSupplementaryGroups=gpio input' > /etc/systemd/system/mygpiod.service.d/gpio-group.conf

Docker
------

Example docker compose file to start myGPIOd.

.. code:: yaml

   services:
     mygpiod:
       image: ghcr.io/jcorporation/mygpiod/mygpiod
       container_name: mygpiod
       restart: unless-stopped
       environment:
         TZ: Europe/Berlin
         MPD_HOST: 192.168.1.1
       user: 1000:1000
       group_add:
         - gpio
         - video
         - input
       devices:
         - /dev/gpiochip0:/dev/gpiochip0
         - /dev/vcio:/dev/vcio
         - /dev/input/event0:/dev/input/event0
       volumes:
         - /etc/mygpiod.conf:/etc/mygpiod.conf
         - /etc/mygpiod.d/:/etc/mygpiod.d/

To run ``mygpioc`` in the already running myGPIOd container:

.. code:: sh

   docker exec -it mygpiod mygpioc gpiolist
