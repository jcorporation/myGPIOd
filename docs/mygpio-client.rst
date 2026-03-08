myGPIO Client: mygpioc
======================

The ``mygpioc`` command line client connects to the socket ``/run/mygpiod/socket`` to control myGPIOd.

.. code:: shell

    Usage: mygpioc [options] <command> [<arguments>]

    myGPIOc 0.9.0
    (c) 2020-2026 Juergen Mang <mail@jcgames.de>
    https://github.com/jcorporation/myGPIOd

    Options:
    -h, --help                               Displays this help
    -s, --socket                             Path to myGPIOd socket
    -t, --timeout                            Connection timeout in milliseconds
    -v, --verbose                            Verbose output

    Commands:
    gpioget <number>                         Gets the current value of a gpio
    gpioblink <number> <timeout> <interval>  Toggles the value of an output gpio in the given timeout and interval
    gpioinfo <number>                        Gets the settings of a gpio
    gpiolist                                 Lists all configured gpios with its mode and value
    gpioset <number> <active|inactive>       Sets the value of an output gpio
    gpiotoggle <number>                      Toggles the value of an output gpio
    hook <name>                              Trigger a hook
    idle [<timeout>]                         Waits for idle events, timeout is in milliseconds
    vciotemp                                 Gets the temperature from /dev/vcio
    vciovolts                                Gets the core voltage from /dev/vcio
    vcioclock                                Gets the core clock from /dev/vcio
    vciothrottled                            Gets the throttled mask from /dev/vcio
