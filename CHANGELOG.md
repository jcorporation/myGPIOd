# myGPIOd Changelog

https://github.com/jcorporation/myGPIOd

***

## myGPIOd v0.8.1 (not yet released)

This is a small maintenance release.

### Changelog

- Fix: Build for Debian Testing and Unstable

***

## myGPIOd v0.8.0 (2024-11-03)

This is a small maintenance release.

### Changelog

- Upd: libgpiod v2.2
- Fix: relax security configuration of systemd service unit to allow loading shared libraries

***

## myGPIOd v0.7.0 (2023-01-28)

This release adds lua scripting functionality for actions. The new lua action calls preloaded lua functions.

### Changelog

- Feat: Add lua action
- Feat: Harden systemd unit #13
- Upd: Add systemd usage
- Fix: Event loop polling
- Fix: Timestamp calculation

***

## myGPIOd v0.6.0 (2023-01-03)

This release improves mainly the packaging of myGPIOd.

### Changelog

- Feat: Add long_press_release event for input GPIOs.
- Feat: Include a static version of libgpiod if shared library is not found.
- Feat: Improve packaging for various distributions
- Feat: Add docker image

***

## myGPIOd v0.5.0 (2023-12-24)

This release adds more actions.

### Changelog

- Feat: Add long press interval for input GPIOs.
- Feat: Add blink interval for output GPIOs.
- Feat: New MPD action controls MPD with the help of libmpdclient.
- Feat: New HTTP action submits arbitrary GET and POST requests.
- Feat: New myGPIOd action triggers myGPIOd scripts through the myGPIOd api.
- Upd: Improved build system.
- Fix: Memory leak in GPIO set actions.
- Fix: Lookup correct event request.

***

## myGPIOd v0.4.0 (2023-12-19)

This is a complete rewrite of myGPIOd. It is now based on libgpiod v2.

This release ships in addition to the myGPIOd daemon a simple C client library and a command line tool.

The daemon can be controlled through a simple text based protocol accessible by a unix socket.

### Notes

- The configuration file format has changes significantly.
- libgpiod v2 is required

### Changelog

- Feat: Add a server socket and define a client protocol to retrieve gpio events and control gpio outputs
- Feat: Simple C client library
- Feat: Command line client
- Feat: Rework build process to support the standard cmake build types
- Feat: Distinct actions for gpio events
- Feat: Long press detection
- Feat: Set gpio output states
- Upd: Restructure and document the source code
- Upd: Improved documentation

***

## myGPIOd v0.3.1 (2023-12-13)

This release fixes a bug in the Alpine Linux package.

***

## myGPIOd v0.3.0 (2023-10-09)

This release adds bias support and requires therefor gpiod >= 1.5.0.

***

## myGPIOd v0.2.1 (2021-03-09)

This release fixes a logging bug and improves the documentation of the configuration file.

***

## myGPIOd v0.2.0 (2020-02-21)

This release improves the build script and adds syslog functionality.

***

## myGPIOd v0.1.0 (2020-11-17)

This is the initial release of myGPIOd.
