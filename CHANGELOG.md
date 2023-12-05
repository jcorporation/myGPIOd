# myGPIOd Changelog

https://github.com/jcorporation/myGPIOd

***

## myGPIOd v0.4.0 (not yet released)

This is the first larger release after the initial release.

### Notes

- The configuration file format has changes significantly.
- libgpiod v2 is required

### Changelog

- Feat: Add a server socket and define a client protocol to retrieve gpio events and control gpio outputs
- Feat: Rework build process to support the standard cmake build types
- Feat: Distinct actions for gpio events
- Feat: Long press detection
- Feat: Set gpio output states
- Upd: Restructure and document the source code
- Upd: Improved documentation

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
