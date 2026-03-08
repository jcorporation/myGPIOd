myGPIOd HTTP-API
================

The default HTTP-API port is ``8081``. In addition to a REST API and a long poll endpoint, it offers a simple web interface for managing the configured GPIOs.

REST-API
--------

- `OpenAPI-Documentation <https://raw.githubusercontent.com/jcorporation/myGPIOd/refs/heads/master/openapi.yml>`__

+----------------------------------------------------------------------------+---------+-----------------------+
| Path                                                                       | Method  | Command               |
+============================================================================+=========+=======================+
| ``/api/v1/gpio``                                                           | GET     | gpiolist              |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/gpio/{gpionumber}``                                              | GET     | gpioget               |
+------------+---------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/gpio/{gpionumber}``                                              | OPTIONS | gpioinfo              |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/gpio/{gpio number}/blink?interval={interval}&timeout={timeout}`` | PATCH   | gpioblink             |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/gpio/{gpio number}/set/?value={active,inactive}``                | PATCH   | gpioset               |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/gpio/{gpio number}/toggle``                                      | PATCH   | gpiotoggle            |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/timerev``                                                        | GET     | timerevlist           |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/vcio``                                                           | GET     | Gets all vcio values. |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/vcio/temp``                                                      | GET     | vciotemp              |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/vcio/volts``                                                     | GET     | vciovolts             |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/vcio/clock``                                                     | GET     | vcioclock             |
+----------------------------------------------------------------------------+---------+-----------------------+
| ``/api/v1/vcio/throttled``                                                 | GET     | vciothrottled         |
+----------------------------------------------------------------------------+---------+-----------------------+

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

Webhook
-------

External programs can trigger myGPIOd actions via a webhook.

URI: ``/hook/<name>``
