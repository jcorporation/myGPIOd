myGPIOd Configuration
=====================

Configuration steps
-------------------

- Adapt the configuration file ``/etc/mygpiod.conf`` to your needs. All
  options are documented in the file.
- Create GPIO configuration files in the directory ``/etc/mygpiod.d``.
  There are documented example configuration files for input and output
  configuration. myGPIOd only accesses GPIOs configured in this files.
- GPIO configuration file names: ``<gpio number>.<direction>``

  - ``<gpio number>``: This is the line number of the GPIO.
  - ``<direction>``: Configures the GPIO line direction, ``in`` for input and ``out`` for output.

Events
------

Events are triggered through changes of input GPIO values.

+-----------------------------+-----------------------------------------------------------------------------+
| Event                       | Description and options                                                     |
+=============================+=============================================================================+
| ``gpio_falling``            || State of GPIO has changed from active to inactive.                         |
|                             || ``action_falling``: Action to execute.                                     |
+-----------------------------+-----------------------------------------------------------------------------+
| ``gpio_rising``             || State of GPIO has changed from inactive to active.                         |
|                             || ``action_rising``: Action to execute.                                      |
+-----------------------------+-----------------------------------------------------------------------------+
| ``gpio_long_press``         || GPIO was pressed long.                                                     |
|                             || ``long_press_event``: Event for long press ``falling`` or ``rising``.      |
|                             || ``long_press_timeout``: Timeout a button is considered as long pressed.    |
|                             || ``long_press_action``: Action for long press.                              |
|                             || ``long_press_interval``: Action is repeated in this interval until button  |
|                             | is released, set to ``0`` to disable.                                       |
+-----------------------------+-----------------------------------------------------------------------------+
| ``gpio_long_press_release`` || GPIO changing it's state after a long press event.                         |
|                             || ``long_press_release_action``: Action executed when button was released.   |
+-----------------------------+-----------------------------------------------------------------------------+
| ``input``                   || An input event has occurred.                                               |
+-----------------------------+-----------------------------------------------------------------------------+

GPIO events
-----------

myGPIOD can read edge events from GPIO lines and execute configured actions. Only one GPIO device can be configured.

.. code:: ini

  # GPIO chip device
  chip = /dev/gpiochip0

Which GPIO lines to use are configured with one file per line in the directory ``/etc/mygpiod.d``.

Input events
------------

myGPIOd read events from `/dev/input/...` devices and execute configured actions. Which input device to use and which is the correct event type, code and value can easily determined with the `evtest` utility.

.. code:: ini

  # Inputs must be defined before input events
  input = /dev/input/event0
  #input = /dev/input/event3

  #input_ev = device:type:code:value:action:options
  # To ignore the carried event value use UINT_MAX for the value
  #input_ev = device:type:code:UINT_MAX:action:options
  input_ev = /dev/input/event0:EV_KEY:KEY_POWER:1:system:/home/juergen/projekte/myGPIOd/etc/command.sh

Supported event types: EV_KEY, EV_REL, EV_ABS, EV_SW

Actions
-------

Each event can have multiple actions. Actions and its arguments are delimited by a colon, arguments are delimited by space.

.. code:: ini

  action_falling = {action}:{arg1} {arg2} ...
  action_rising = {action}:{arg1} {arg2} ...
  long_press_action = {action}:{arg1} {arg2} ...
  long_press_release_action = {action}:{arg1} {arg2} ...

+----------------+-----------------------+---------------------------------------------------------+
| Action         | Arguments             | Description                                             |
+================+=======================+=========================================================+
| ``gpioblink``  | ``<gpio>``            | Toggle the value of the GPIO after given timeout        |
|                | ``<timeout_ms>``      | (initial delay) and interval. Set interval to ``0`` to  |
|                | ``<interval_ms>``     | blink only once.                                        |
+----------------+-----------------------+---------------------------------------------------------+
| ``gpioset``    | ``<gpio>``            | Sets the value of a GPIO.                               |
|                | ``<active|inactive>`` |                                                         |
+----------------+-----------------------+---------------------------------------------------------+
| ``gpiotoggle`` | ``<gpio>``            | Toggles the value of a GPIO.                            |
+----------------+-----------------------+---------------------------------------------------------+
| ``http``       | ``{method}``          | Submits a HTTP request in a new child process. If       |
|                | ``{uri}``             | ``postdata`` starts with ``<</``, the string after      |
|                | [``{content-type}``   | the ``<<`` is interpreted as an absolute filepath       |
|                | ``{postdata}``]       | from which the postdata is read. Requires libcurl.      |
|                |                       | Valid HTTP methods are: DELETE, GET, HEAD, OPTIONS,     |
|                |                       | PATCH, POST, PUT                                        |
+----------------+-----------------------+---------------------------------------------------------+
| ``lua``        | ``{lua function}``    | Calls a user defined :doc:`lua function <lua-scripts>`. |
|                | [``{option}`` ...]    |                                                         |
+----------------+-----------------------+---------------------------------------------------------+
| ``mpc``        | ``{mpd command}``     | Connects to MPD and issues the command with options.    |
|                | [``{option}`` ...]    | It uses the default connection settings from            |
|                |                       | libmpdclient. A maximum of 10 options are supported.    |
|                |                       | Requires libmpdclient.                                  |
+----------------+-----------------------+---------------------------------------------------------+
| ``mympd``      | ``{uri}``             | Calls the myMPD api in a new child process to           |
|                | ``{partition}``       | execute a myMPD script. Requires libcurl.               |
|                | ``{script}``          |                                                         |
+----------------+-----------------------+---------------------------------------------------------+
| ``system``     | ``{command}``         | Executes an executable or script in a new child         |
|                |                       | process. No arguments are allowed.                      |
+----------------+-----------------------+---------------------------------------------------------+

myGPIOd can take actions on rising, falling and long_press events. Long
press is triggered by a falling or rising event and does not disable the
triggering event, but the release event. To use a button for normal
press and long_press request both events and use one event for long and
the other for short press. The example below illustrates this.

Example actions
~~~~~~~~~~~~~~~

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

Example configuration
---------------------

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

  # File with user defined lua functions
  #lua_file = /etc/mygpiod.lua

GPIO configuration
~~~~~~~~~~~~~~~~~~

- Configures GPIO 3 as input:

  - Enables the pull-up resistor on start
  - Sets GPIO 6 to active on falling event
  - Calls ``/usr/local/bin/reboot.sh`` after a button press (falling) of 2 seconds length
  - Toggles the value of GPIO 6 on release event of the long press event
  - Calls ``/usr/local/bin/poweroff.sh`` on a short press (rising)

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
    # The rising event is not triggered if GPIO 3 is pressed longer than 2000 ms.
    action_rising = system:/usr/local/bin/poweroff.sh

    # Reboot on long press and activate a LED for maximal 2s while GPIO 3 is pressed.

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

- Configures GPIO 4 as input:

  - Enables the pull-up resistor on start
  - Runs the mpd ``next`` command on a short press (falling)
  
  **/etc/mygpiod.d/4.in**

  .. code:: ini

    # Configuration file for GPIO 4 as input
    # Request the falling event
    event_request = falling

    # Enable the internal pull-up resistor
    bias = pull-up

    # Use libmpdclient to connect to mpd and send the command `next`
    action_falling = mpc:next

- Configures GPIO 5 as output:

  - Sets the value to active on start
  
  **/etc/mygpiod.d/5.out**

  .. code:: ini

    # Configuration file for GPIO 5 as output
    # Set the GPIO to active
    value = active

- Configures GPIO 6 as output:

  - Sets the value to inactive on start

  **/etc/mygpiod.d/6.out**

  .. code:: ini

    # Configuration file for GPIO 6 as output
    # Set the GPIO to inactive
    value = inactive

- Configures GPIO 7 as input:

  - Enables the pull-up resistor on start
  - Enabled the long press action for falling event
  - Increases the mpd volume by 5 % after 100 ms and each 500 ms as long the button is pressed

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

- Configures GPIO 8 as input:

  - Enables the pull-up resistor on start
  - On falling event:

    - Set GPIO 6 for 1000 ms to active
    - Execute the Jukebox script through the myGPIOd API

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

- Rotary Encoder configuration:

  - Configure GPIO 5 as input and connect it to the CLX pin
  - Configure GPIO 6 as input and connect it to the DT pin
  - Disable internal resistors
  - On rising event of GPIO 5 execute a Lua script that reads the value of GPIO 6

  **/etc/mygpiod.d/5.in**

  .. code:: ini

    event_request = rising
    bias = disable
    debounce = 5000
    action_rising = lua:rotaryEncoder

  **/etc/mygpiod.d/5.in**

  .. code:: ini

    event_request = none
    bias = disable
    debounce = 5000

  **/etc/mygpiod.lua**

  .. code:: lua

    function rotaryEncoder()
      -- Get value of the DT pin
      local _, dt  = gpioGet(6)
      -- Determine rotation direction
      if dt == "active" then
          print("counter-clockwise")
      else
          print("clockwise")
      end
    end
