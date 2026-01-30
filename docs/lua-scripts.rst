Lua scripts
===========

myGPIOd reads on startup the file defined by the ``lua_file`` configuration setting and starts a Lua VM that compiles the file.
All lua functions in this file are registered and can be called with the lua action.

.. warning:: Lua scripts are executed in the main thread, therefor lua scripts can block it.

.. note:: The lua functions should not return any value.

Custom lua functions
--------------------

myGPIOd registers custom lua functions to provide access to the actions. The functions return ``true`` on success, else ``false``.

+------------------------------------------------------------------+-----------------------------------------------------+
| Lua function                                                     | Description                                         |
+==================================================================+=====================================================+
| ``local rc = gpioBlink({GPIO}, {timeout_ms}, {interval_ms})``    | Toggle the value of the GPIO in given timeout       |
|                                                                  | and interval.                                       |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc, value = gpioGet({GPIO})``                            | Returns the GPIO state:                             |
|                                                                  | ``active``, ``inactive`` or ``error``               |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = gpioSet({GPIO}, {active|inactive})``                | Sets the state of an output GPIO.                   |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = gpioToggle({GPIO})``                                | Toggles the state of an output GPIO.                |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = http({method}, {uri}, {content-type}, {postdata})`` | Submits a HTTP request in a new child process.      |
|                                                                  | Set ``content-type`` and ``postdata`` to ``nil`` if |
|                                                                  | no body should be sent. This is an async function   |
|                                                                  | and you can not get the HTTP response.              |
|                                                                  | Valid HTTP methods are: DELETE, GET, HEAD,          |
|                                                                  | OPTIONS, PATCH, POST, PUT                           |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = mpc({mpd protocol command})``                       | Runs a mpd protocol command.                        |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = mympd({uri}, {partition}, {script})``               | Calls the myGPIOd api to execute a script in a      |
|                                                                  | new child process. This is an async function and    |
|                                                                  | you can not get the HTTP response.                  |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = system({command})``                                 | Executes an executable or script in a new           |
|                                                                  | child process. This is an async function and you    |
|                                                                  | can not get the output of the command.              |
+------------------------------------------------------------------+-----------------------------------------------------+

Example lua file
----------------

This Lua file registers two functions: ``btnPress`` and ``rotaryEncoder``.

.. code:: lua

  function btnPress(arg1)
      -- This is printed in the myGPIOd log
      print("btnPress " .. arg1)

      -- HTTP request with no body
      http("GET", "http://nas.lan/btnpress", nil, nil)

      -- myMPD API request to execute a script
      mympd("https://nas.lan/mympd", "default", "test")

      -- Execute a local shell script
      system("/home/juergen/projekte/myGPIOd/etc/command.sh")

      -- Send commands to MPD
      mpc("volume 5")
      mpc("play")

      -- Blink GPIO 3
      gpioBlink(3, 1000, 2000)

      -- Set GPIO 4 to active
      gpioSet(4, "active")

      -- Toggle the state of GPIO 4
      gpioToggle(4)

      -- Get the value of GPIO 1
      local _, val
      _, val = gpioGet(1)
      print("Value of GPIO1: " .. val)
  end

  -- Add this action to the CLX pin for the rising event
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
