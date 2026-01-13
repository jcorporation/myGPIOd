Configuration
=============

Configuration steps
-------------------

- Adapt the configuration file ``/etc/mygpiod.conf`` to your needs. All
  options are documented in the file.
- Create GPIO configuration files in the directory ``/etc/mygpiod.d``.
  There are documented example configuration files for input and output
  configuration. myGPIOd only accesses GPIOs configured in this files.
- GPIO configuration file names: ``<gpio number>.<direction>``
  - ``<gpio number>``: This is the line number of the GPIO.
  - ``<direction>``: Configures the GPIO line direction, ``<in>`` for input and ``<out>`` for output.

Events
------

Events are triggered through changes of input GPIO values.

+------------------------+---------------------------------------------------------------------+
| Event                  | Description                                                         |
+========================+=====================================================================+
| ``falling``            | State of GPIO has changed from active to inactive.                  |
+------------------------+---------------------------------------------------------------------+
| ``rising``             | State of GPIO has changed from inactive to active.                  |
+------------------------+---------------------------------------------------------------------+
| ``long_press``         | GPIO was pressed long. Event is triggered after configurable delay. |
+------------------------+---------------------------------------------------------------------+
| ``long_press_release`` | GPIO changing it's state after a long press event.                  |
+------------------------+---------------------------------------------------------------------+

Actions
-------

Each event can have multiple actions. Actions and its arguments are delimited by a colon, arguments are delimited by space.

+----------------+-----------------------+------------------------------------------------------+
| Action         | Arguments             | Description                                          |
+================+=======================+======================================================+
| ``gpioblink``  | ``<gpio>``            | Toggle the value of the GPIO in given timeout and    |
|                | ``<timeout_ms>``      | interval. Set interval to 0 to blink only once.      |
|                | ``<interval_ms>``     |                                                      |
+----------------+-----------------------+------------------------------------------------------+
| ``gpioset``    | ``<gpio>``            | Sets the value of a GPIO.                            |
|                | ``<active|inactive>`` |                                                      |
+----------------+-----------------------+------------------------------------------------------+
| ``gpiotoggle`` | ``<gpio>``            | Toggles the value of a GPIO.                         |
+----------------+-----------------------+------------------------------------------------------+
| ``http``       | ``{method}``          | Submits a HTTP request in a new child process. If    |
|                | ``{uri}``             | ``postdata`` starts with ``<</``, the string after   |
|                | [``{content-type}``   | the ``<<`` is interpreted as an absolute filepath    |
|                | ``{postdata}``]       | from which the postdata is read. Requires libcurl.   |
|                |                       | Valid HTTP methods are: DELETE, GET, HEAD, OPTIONS,  |
|                |                       | PATCH, POST, PUT                                     |
+----------------+-----------------------+------------------------------------------------------+
| ``lua``        | ``{lua function}``    | Calls a user defined lua function.                   |
|                | [``{option}`` ...]    |                                                      |
+----------------+-----------------------+------------------------------------------------------+
| ``mpc``        | ``{mpd command}``     | Connects to MPD and issues the command with options. |
|                | [``{option}`` ...]    | It uses the default connection settings from         |
|                |                       | libmpdclient. A maximum of 10 options are supported. |
|                |                       | Requires libmpdclient.                               |
+----------------+-----------------------+------------------------------------------------------+
| ``mympd``      | ``{uri}``             | Calls the myMPD api in a new child process to        |
|                | ``{partition}``       | execute a myMPD script. Requires libcurl.            |
|                | ``{script}``          |                                                      |
+----------------+-----------------------+------------------------------------------------------+
| ``system``     | ``{command}``         | Executes an executable or script in a new child      |
|                |                       | process. No arguments are allowed.                   |
+----------------+-----------------------+------------------------------------------------------+

myGPIOd can take actions on rising, falling and long_press events. Long
press is triggered by a falling or rising event and does not disable the
triggering event, but the release event. To use a button for normal
press and long_press request both events and use one event for long and
the other for short press. The example below illustrates this.

Examples
~~~~~~~~

.. code:: ini

   # GPIO actions
   gpioblink:5 1000 2000
   gpioset:5 active
   gpiotoggle:5

   # Execute HTTP actions
   http:GET http://server.lan/webhook1
   http:PATCH http://server.lan/webhook1 application/json '{"value": 1}'
   http:POST http://server.lan/webhook2 text/plain <</tmp/postdata

   # Execute a custom lua function
   lua:my_lua_func arg1 arg2

   # Execute mpd commands
   mpc:next
   mpc:volume 5

   # Start a script in myMPD
   mympd:http://localhost:8443 default script1

   # Execute a system command
   system:/bin/true

Lua
~~~

The lua action calls user defined lua functions. The lua script itself
is loaded on startup of myGPIOd (``lua_file`` configuration setting).
All lua functions in this file are registered and can be called with the
lua action.

- Lua scripts are executed in the main thread, therefor lua scripts can block it.
- The lua functions should not return any value.

myGPIOd registers custom lua functions to provide access to the actions.
The functions return ``0`` on success, else ``1``.

+---------------------------------------------------------+-------------------------------------------------+
| Lua function                                            | Description                                     |
+=========================================================+=================================================+
| ``gpioBlink({GPIO}, {timeout_ms}, {interval_ms})``      | Toggle the value of the GPIO in given timeout   |
|                                                         | and interval.                                   |
+---------------------------------------------------------+-------------------------------------------------+
| ``gpioGet({GPIO})``                                     | Returns the GPIO state:                         |
|                                                         | 1 = active, 0 = inactive                        |
+---------------------------------------------------------+-------------------------------------------------+
| ``gpioSet({GPIO}, {1\|0})``                             || Sets the state of an output GPIO:              |
|                                                         || 1 = active, 0 = inactive                       |
+---------------------------------------------------------+-------------------------------------------------+
| ``gpioToggle({GPIO})``                                  | Toggles the state of an output GPIO.            |
+---------------------------------------------------------+-------------------------------------------------+
| ``http({method}, {uri}, {content-type}, {postdata})``   | Submits a HTTP request in a new child process.  |
|                                                         | ``content-type`` and ``postdata`` are optional. |
|                                                         | This is an async function.                      |
|                                                         | Valid HTTP methods are: DELETE, GET, HEAD,      |
|                                                         | OPTIONS, PATCH, POST, PUT                       |
+---------------------------------------------------------+-------------------------------------------------+
| ``mpc({mpd protocol command})``                         | Runs a mpd protocol command.                    |
+---------------------------------------------------------+-------------------------------------------------+
| ``mympd({uri}, {partition}, {script})``                 | Calls the myGPIOd api to execute a script in a  |
|                                                         | new child process. This is an async function.   |
+---------------------------------------------------------+-------------------------------------------------+
| ``system({command})``                                   | Executes an executable or script in a new       |
|                                                         |  child process. This is an async function.      |
+---------------------------------------------------------+-------------------------------------------------+

**Example gpio config**

.. code:: ini

   # Call the lua function `testFunc` with the argument `testArg` on rising event.
   action_rising = lua:testFunc testArg

**Example lua file**

.. code:: lua

   function changeMPDvolume()
     -- This example function can be used to change the MPD volume with a rotary encoder
     -- Get value of the GPIOs
     local clk = gpioGet(4)
     local dt = gpioGet(5)
     -- Check rotation direction
     if clk == dt then
       mpc("volume 5")
     else
       mpc("volume -5")
     end
   end

Example configuration
---------------------

This example configuration does the following:

- Configures GPIO 3 as input:

  - Enables the pull-up resistor on start
  - Sets GPIO 6 to active on falling event
  - Calls ``/usr/local/bin/reboot.sh`` after a button press (falling) of 2 seconds length
  - Toggles the value of GPIO 6 on release event of the long press event
  - Calls ``/usr/local/bin/poweroff.sh`` on a short press (rising)

- Configures GPIO 4 as input:

  - Enables the pull-up resistor on start
  - Runs the mpd ``next`` command on a short press (falling)

- Configures GPIO 5 as output:

  - Sets the value to active on start

- Configures GPIO 6 as output:

  - Sets the value to inactive on start

- Configures GPIO 7 as input:

  - Enables the pull-up resistor on start
  - Enabled the long press action for falling event
  - Increases the mpd volume by 5 % after 100 ms and each 500 ms as long the button is pressed

- Configures GPIO 8 as input:

  - Enables the pull-up resistor on start
  - On falling event:

    - Set GPIO 6 for 1000 ms to active
    - Execute the Jukebox script through the myGPIOd API

**/etc/mygpiod.conf**

.. code:: ini

   # GPIO chip to use
   chip = /dev/gpiochip0

   # The loglevel
   loglevel = info

   # Log to stdout
   syslog = 0

   # Directory for GPIO configuration files
   gpio_dir = /etc/mygpiod.d

**/etc/mygpiod.d/3.in**

.. code:: ini

   # Configuration file for GPIO 3 as input
   # Request falling and rising events
   event_request = both

   # Active is high
   active_low = false

   # Enable the internal pull-up resistor
   bias = pull-up

   # Short press does a poweroff
   # The rising event is not triggered if GPIO 6 is pressed longer than 2000 ms.
   action_rising = system:/usr/local/bin/poweroff.sh

   # Reboot on long press and activate a LED for maximal 2s while GPIO 6 is pressed.

   # Set GPIO 6 active on falling
   action_falling = gpioset:6 active

   # Enable long press for falling.
   long_press_event = falling

   # Set the long press timeout to 2000 ms.
   long_press_timeout = 2000

   # Disable the long press interval.
   long_press_interval = 0

   # Action for long press is to run a script.
   long_press_action = system:/usr/local/bin/reboot.sh

   # Action for releasing the button after a long press is to toggle the value of GPIO 6.
   long_press_release_action = gpiotoggle:6

**/etc/mygpiod.d/4.in**

.. code:: ini

   # Configuration file for GPIO 4 as input
   # Request the falling event
   event_request = falling

   # Enable the internal pull-up resistor
   bias = pull-up

   # Use libmpdclient to connect to mpd and send the command `next`
   action_falling = mpc:next

**/etc/mygpiod.d/5.out**

.. code:: ini

   # Configuration file for GPIO 5 as output
   # Set the GPIO to active
   value = active

**/etc/mygpiod.d/6.out**

.. code:: ini

   # Configuration file for GPIO 6 as output
   # Set the GPIO to inactive
   value = inactive

**/etc/mygpiod.d/7.in**

.. code:: ini

   # Configuration file for GPIO 7 as input
   # Request the falling event
   event_request = falling

   # Enable the internal pull-up resistor
   bias = pull-up

   # Enable long press action for the falling event
   long_press_event = falling

   # Set initial long press timeout to 100 ms
   long_press_timeout = 100

   # Set interval to 500 ms, action is repeated in this interval until the value changes again
   long_press_interval = 500

   # Use libmpdclient to connect to mpd and send the command `volume +5`
   # This increases the volume by 5 percent
   long_press_action = mpc:volume +5

**/etc/mygpiod.d/8.in**

.. code:: ini

   # Configuration file for GPIO 8 as input
   # Request the falling event
   event_request = falling

   # Enable the internal pull-up resistor
   bias = pull-up

   # Execute the Jukebox script through the myGPIOd API
   action_falling = gpioblink:6 1000 0
   action_falling = mympd:https://127.0.0.1 default Jukebox
