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
| ``local rc, value = gpioGet({GPIO})``                            || Returns the GPIO state:                            |
|                                                                  || ``active``, ``inactive`` or ``error``              |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = gpioSet({GPIO}, {active|inactive})``                | Sets the state of an output GPIO.                   |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = gpioToggle({GPIO})``                                | Toggles the state of an output GPIO.                |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = http({method}, {uri}, {content-type}, {postdata})`` | Submits a HTTP request in a new child process.      |
|                                                                  | Set ``content-type`` and ``postdata`` to ``nil`` if |
|                                                                  | no body should be sent.                             |
|                                                                  | This is an async function.                          |
|                                                                  | Valid HTTP methods are: DELETE, GET, HEAD,          |
|                                                                  | OPTIONS, PATCH, POST, PUT                           |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = mpc({mpd protocol command})``                       | Runs a mpd protocol command.                        |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = mympd({uri}, {partition}, {script})``               | Calls the myGPIOd api to execute a script in a      |
|                                                                  | new child process. This is an async function.       |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc = system({command})``                                 | Executes an executable or script in a new           |
|                                                                  | child process. This is an async function.           |
+------------------------------------------------------------------+-----------------------------------------------------+

Example lua file
----------------

.. code:: lua

   function changeMPDvolume()
     -- This example function can be used to change the MPD volume with a rotary encoder
     -- Get value of the GPIOs
     local _, clk, dt
     _, clk = gpioGet(4)
     _, dt = gpioGet(5)
     -- Check rotation direction
     if clk == dt then
       mpc("volume 5")
     else
       mpc("volume -5")
     end
   end