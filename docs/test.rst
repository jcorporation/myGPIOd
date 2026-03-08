Testing
=======

Simulate GPIOs
--------------

A GPIO chip can be simulated with the gpio-sim kernel module.

The ``build.sh`` script has an option to simplify the creation of a simulated GPIO device. It must be run as root.

.. code:: sh

    sudo ./build.sh gpiosim

After that you should have a ``/dev/gpiochip0`` device and 8 lines. You can access the lines through the sysfs filesystem.
The path is ``/sys/devices/platform/gpio-sim.0/``.

References:

- https://github.com/warthog618/gpiosim-rs/blob/master/examples/basic_sim.sh
- https://www.kernel.org/doc/html/latest/admin-guide/gpio/gpio-sim.html

Simulate GPIO events
--------------------

The debug build of myGPIOd provides an extra protocol command for simulating GPIO events: ``event {gpio} {falling,rising}``

.. code:: sh

    socat - UNIX-CONNECT:/tmp/mygpiod.socket <<< "event 1 falling"

