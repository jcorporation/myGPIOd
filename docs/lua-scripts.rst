Lua scripts
===========

myGPIOd reads on startup the file defined by the ``lua_file`` configuration setting and starts a Lua VM that compiles the file.
All lua functions in this file are registered and can be called with the lua action.

.. warning:: Lua scripts are executed in the main thread, therefor lua scripts can block it.

.. note:: The lua functions should not return any value.

Custom lua functions
--------------------

myGPIOd registers custom lua functions to provide access to the actions. The functions return ``0`` on success, else ``1``.

+---------------------------------------------------------+-------------------------------------------------+
| Lua function                                            | Description                                     |
+=========================================================+=================================================+
| ``gpioBlink({GPIO}, {timeout_ms}, {interval_ms})``      | Toggle the value of the GPIO in given timeout   |
|                                                         | and interval.                                   |
+---------------------------------------------------------+-------------------------------------------------+
| ``gpioGet({GPIO})``                                     || Returns the GPIO state:                        |
|                                                         || 1 = active, 0 = inactive                       |
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
|                                                         | child process. This is an async function.       |
+---------------------------------------------------------+-------------------------------------------------+

Example lua file
----------------

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