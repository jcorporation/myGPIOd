# Protocol

The myGPIOd protocol exchanges line-based text messages between client and server over a local unix socket.

When the client connects, the server responds with:

```
OK
version:{myGPIOd version}
END
```

Default socket: `/run/mygpiod/socket`

## Requests

```
COMMAND [ARG...]
```

## Responses

- The first line of the response is `OK` on completion or `ERROR` on failure.
- After that any number of `key:value` pairs can follow.
- A final `END` denotes the end of the response.

## General commands

### close

Closes the connection.

### idle

Enables the idle mode for the connection. In this mode the client waits for gpio events. This command disables the connection timeout.

The command returns as soon as an event occurs. It returns immediately if there are events occurred while the client was not in idle mode.

Only the `noidle` command is allowed while the client is in idle mode.

```
OK
gpio:{gpio number}
event:{event1}
time:{nanoseconds}
gpio:{gpio number}
event:{event2}
time:{nanoseconds}
END
```

### noidle

Exits the idle mode and allows the client to send commands. It responds with the accumulated events.

myGPIOD stores only the last 10 events while not in idle mode.

## Events

Events are triggered by monitored gpios. Valid events are:

- falling
- rising
- long_press

## GPIO commands

### gpiolist

Lists all configured gpios.

**Response**

```
OK
gpio:{gpio number}
mode:{in|out}
...
END
```

### gpioget {gpio number}

Gets the current value of an output gpio.

**Response**

```
OK
gpio:{gpio number}
value:{0|1}
END
```

### gpioset {gpio number} {value}

Sets the value of an output gpio. Valid values are:

- 0 = low
- 1 = high
