Asynchronous Lua scripts
========================

myGPIOd reads on startup the folder defined by ``lua_async_dir``. Each file with the extension ``.lua`` is loaded and registered as Lua file.
This files can be called with the ``lua_async`` action.

.. note::
   
   These Lua scripts are executed in a new thread and therefore do not have access to data structures from the main thread.
   Use this type of scripts for longer running actions.

Custom lua functions
--------------------

myGPIOd registers custom lua functions to provide access to the actions. The functions return ``true`` on success, else ``false`` as first value.

+------------------------------------------------------------------+-----------------------------------------------------+
| Lua function                                                     | Description                                         |
+==================================================================+=====================================================+
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc, resp_header, resp_body =``                           | Submits a HTTP request and waits for the response.  |
| ``http({method}, {uri}, {content-type}, {postdata})``            | Set ``content-type`` and ``postdata`` to ``nil`` if |
|                                                                  | no body should be sent.                             |
|                                                                  | Valid HTTP methods are: DELETE, GET, HEAD,          |
|                                                                  | OPTIONS, PATCH, POST, PUT                           |
+------------------------------------------------------------------+-----------------------------------------------------+
| ``local rc, resp_header, resp_body =``                           | Calls the myGPIOd api to execute a script and waits |
| ``mympd({uri}, {partition}, {script})``                          | for the response.                                   |
+------------------------------------------------------------------+-----------------------------------------------------+

Example lua file
----------------

.. code:: lua

  local rc, resp_header, resp_body = http("GET", "http://test.lan/", nil, nil)
  print(resp_body)
