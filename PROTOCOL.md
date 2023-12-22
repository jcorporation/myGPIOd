# Protocol

The myGPIOd protocol exchanges line-based text messages between client and server over a local unix socket.

When the client connects, the server responds with:

```
OK
version:{major}.{minor}.{patch}
END
```

Default socket: `/run/mygpiod/socket`

## Requests

```
COMMAND [ARG...]
```

## Responses

- The first line of the response is `OK` on completion or `ERROR:<message>` on failure.
- After an `OK`:
  - Any number of `key:value` pairs can follow.
  - A final `END` denotes the end of the server response.

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
event:{falling|rising|long_press}
timestamp_ms:{milliseconds}
gpio:{gpio number}
event:{falling|rising|long_press}
timestamp_ms:{milliseconds}
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
direction:{in|out}
value:{active|inactive}
END
```

### gpioinfo {gpio number}

Gets the current settings of a configured input or output gpio.

**Response for an input gpio**

```
OK
direction:in
value:{active|inactive}
active_low:{true|false}
bias:{as-is|disable|pull-down|pull-up}
event_request:{both|falling|rising}
is_debounced:{true|false}
debounce_period_us:{microseconds}
event_clock:{monotonic|realtime|hte}
END
```

**Response for an output gpio**

```
OK
direction:out
value:{active|inactive}
drive:{push-pull|open-drain|open-source}
END
```

### gpioget {gpio number}

Gets the current value of a configured input or output gpio.

**Response**

```
OK
value:{active|inactive}
END
```

### gpioblink {gpio number} {timeout} {interval}

Toggles the value of a configured output gpio at given timeout and interval. Set Interval to 0 to blink only once.

Use the gpioset or gpiotoggle commands to disable blinking.

### gpioset {gpio number} {active|inactive}

Sets the value of a configured output gpio.

### gpiotoggle {gpio number}

Toggles the value of a configured output gpio.
