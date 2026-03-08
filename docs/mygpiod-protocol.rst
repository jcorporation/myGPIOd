myGPIOd Protocol
================

myGPIOd can be controlled and queried through a simple line-based text protocol over a local Unix socket.

Default socket: ``/run/mygpiod/socket``

You can test the protocol with:

.. code:: shell

   socat unix-client:/run/mygpiod/socket stdio

When the client connects, the server responds with:

::

   OK
   version:{major}.{minor}.{patch}
   END

Requests
--------

::

   COMMAND [ARG...]

Responses
---------

- The first line of the response is ``OK`` on completion or
  ``ERROR:<message>`` on failure.
- After an ``OK``:

  - Any number of ``key:value`` pairs can follow.
  - A final ``END`` denotes the end of the server response.

General commands
----------------

close
~~~~~

Closes the connection.

idle
~~~~

Enables the idle mode for the connection. In this mode the client waits
for gpio events. This command disables the connection timeout.

The command returns as soon as an event occurs. It returns immediately
if there are events occurred while the client was not in idle mode.
myGPIOd stores only the last 10 events while not in idle mode.

Only the ``noidle`` command is allowed while the client is in idle mode.

**Response for GPIO events**

::

   event:{gpio_falling|gpio_rising|gpio_long_press|input}
   timestamp_ms:{milliseconds}
   gpio:{gpio number}
   event:{gpio_falling|gpio_rising|gpio_long_press|input}
   timestamp_ms:{milliseconds}
   gpio:{gpio number}
   END

**Response for input events**

::
   OK
   event:gpio_input
   timestamp_ms:1771101110
   device:/dev/input/event0
   type:EV_KEY
   code:KEY_POWER
   value:1

noidle
~~~~~~

Exits the idle mode and allows the client to send commands. It responds
with the accumulated events.

**Response for GPIO events**

::

   OK
   event:{gpio_falling|gpio_rising|gpio_long_press|input}
   timestamp_ms:{milliseconds}
   gpio:{gpio number}
   event:{gpio_falling|gpio_rising|gpio_long_press|input}
   timestamp_ms:{milliseconds}
   gpio:{gpio number}
   END

**Response for input events**

::
   OK
   event:gpio_input
   timestamp_ms:1771101110
   device:/dev/input/event0
   type:EV_KEY
   code:KEY_POWER
   value:1

Events
------

Events are triggered by monitored gpios. Valid events are:

- falling
- rising
- long_press

GPIO commands
-------------

gpiolist
~~~~~~~~

Lists all configured gpios.

**Response**

::

   OK
   gpio:{gpio number}
   direction:{in|out}
   value:{active|inactive}
   name:{name}
   gpio:{gpio number}
   direction:{in|out}
   value:{active|inactive}
   name:{name}
   END

gpioinfo {gpio number}
~~~~~~~~~~~~~~~~~~~~~~

Gets the current settings of a configured input or output gpio.

**Response for an input gpio**

::

   OK
   direction:in
   value:{active|inactive}
   active_low:{true|false}
   bias:{as-is|disable|pull-down|pull-up}
   event_request:{both|falling|rising}
   is_debounced:{true|false}
   debounce_period_us:{microseconds}
   event_clock:{monotonic|realtime|hte}
   name:{name}
   END

**Response for an output gpio**

::

   OK
   direction:out
   value:{active|inactive}
   drive:{push-pull|open-drain|open-source}
   name:{name}
   END

gpioget {gpio number}
~~~~~~~~~~~~~~~~~~~~~

Gets the current value of a configured input or output gpio.

**Response**

::

   OK
   value:{active|inactive}
   END

gpioblink {gpio number} {timeout} {interval}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toggles the value of a configured output gpio at given timeout and
interval. Set Interval to 0 to blink only once.

Use the ``gpioset`` or ``gpiotoggle`` commands to disable blinking.

gpioset {gpio number} {active|inactive}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sets the value of a configured output gpio.

gpiotoggle {gpio number}
~~~~~~~~~~~~~~~~~~~~~~~~

Toggles the value of a configured output gpio.

VCIO commands
-------------

vciotemp
~~~~~~~~

Gets the temperature from ``/dev/vcio``.

**Response**

::

   OK
   Value: 36.7'C
   END

vciovolts
~~~~~~~~~

Gets the core voltage from ``/dev/vcio``.

**Response**

::

   OK
   Value: 0.7500V
   END

vcioclock
~~~~~~~~~

Gets the clock speed from ``/dev/vcio``.

**Response**

::

   OK
   Value: 1600017024
   END

vciothrottled
~~~~~~~~~~~~~

Gets the throttled mask from ``/dev/vcio``.

**Response**

::

   OK
   Value: 0x0
   END

Hook commands
-------------

hook <name>
~~~~~~~~~~~

Triggers a hook.

Timer event comands
-------------------

timerevlist
~~~~~~~~~~~

Lists all timers with next triggering unix timestamp.

**Response**

::

   OK
   name:test
   next:1772988599
   END
