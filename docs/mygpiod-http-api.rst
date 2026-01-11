myGPIOd HTTP-API
================

The default HTTP-API port is ``8081``. In addition to a REST API and a long poll endpoint, it offers a simple web interface for managing the configured GPIOs.

REST-API
--------

- `OpenAPI-Documentation <https://raw.githubusercontent.com/jcorporation/myGPIOd/refs/heads/master/openapi.yml>`__

+-------------------------------------------------------------------------+---------+-----------------------+
| Path                                                                    | Method  | Command               |
+=========================================================================+=========+=======================+
| ``/api/gpio``                                                           | GET     | gpiolist              |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/gpio/{gpionumber}``                                              | GET     | gpioget               |
+------------+------------------------------------------------------------+---------+-----------------------+
| ``/api/gpio/{gpionumber}``                                              | OPTIONS | gpioinfo              |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/gpio/{gpio number}/blink?interval={interval}&timeout={timeout}`` | PATCH   | gpioblink             |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/gpio/{gpio number}/set/?value={active,inactive}``                | PATCH   | gpioset               |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/gpio/{gpio number}/toggle``                                      | PATCH   | gpiotoggle            |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/vcio``                                                           | GET     | Gets all vcio values. |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/vcio/temp``                                                      | GET     | vciotemp              |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/vcio/volts``                                                     | GET     | vciovolts             |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/vcio/clock``                                                     | GET     | vcioclock             |
+-------------------------------------------------------------------------+---------+-----------------------+
| ``/api/vcio/throttled``                                                 | GET     | vciothrottled         |
+-------------------------------------------------------------------------+---------+-----------------------+

Long poll endpoint
------------------

This endpoint can be used to poll for GPIO events. It responds as soon an event occurs.

URI: ``/poll``

.. code:: sh

   curl -s http://172.0.0.1:8081/poll | jq '.'
   {
     "gpio": 15,
     "event": "rising",
     "timestamp_ms": 1768080661383
   }
