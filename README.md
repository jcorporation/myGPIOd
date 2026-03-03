# myGPIOd

myGPIOd is a lightweight GPIO controlling framework. It is written in C and depends on [libgpiod version 2.x.x](https://libgpiod.readthedocs.io/).

It consists of a daemon, a client library and a command line tool. It is designed to run on Raspberry PIs and similar devices.

I wrote this tool primarily for [myMPDos](https://github.com/jcorporation/myGPIOdos) and [myMPD](https://github.com/jcorporation/myMPD).

myGPIOd can communicate natively with MPD and also integrates nicely with all HTTP APIs.

Since version 2.0.0 myGPIOd can also read events from generic input devices (`/dev/input/..`).

## Features

- **mygpiod - the daemon component**
  - Call actions on GPIO events
    - rising
    - falling
    - long press (with optional interval)
    - long press release
  - Call actions on input events (`/dev/input/..`)
  - Timer based events
  - Integrated actions
    - GPIO settings
    - HTTP requests
    - Lua scripting
    - MPD client
    - myMPD client
    - System commands
  - Set various GPIO attributes (bias, debounce, ...)
  - Set the output value of GPIOs
  - Get info from Raspberry PI Video Core (`/dev/vcio`)
  - Provides a Unix socket with a simple line-based text protocol
    - List GPIO configuration
    - Set and get GPIO values
    - Get notifications of GPIO events
  - Provides a REST-API endpoint
    - List GPIO configuration
    - Set and get GPIO values
  - Provides a long poll endpoint to receive GPIO and input events
- **libmygpio - the client library**
  - Simple C client library
  - High level API
  - Integration in poll based event loops
- **mygpioc - the command line client**
  - Connects to the mygpiod socket to control the various functions.

## Documentation

For information on installation and configuration, see the [myGPIOd documentation](https://jcorporation.github.io/myGPIOd/).

## Support

Please read the [documentation](https://jcorporation.github.io/myGPIOd/) before asking for help. Bugs should be reported through [issues](https://github.com/jcorporation/myGPIOd/issues). For all other question and general feedback use the [discussions](https://github.com/jcorporation/myGPIOd/discussions).

You can follow me at [mastodon](https://mastodon.social/@jcorporation) to get news about myGPIOd.

## Contribution

myGPIOd is in active development. If you like it, you can help to improve it (no programming skills are required).

- Star this repository to make it more popular.
- [Discuss usage of myGPIOd](https://github.com/jcorporation/myGPIOd/discussions).
- Consider [donating](https://jcorporation.github.io/donate) a coffee to this project.

## Copyright

2020-2026 Juergen Mang <mail@jcgames.de>
