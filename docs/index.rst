myGPIOd Documentation
=====================

myGPIOd is a lightweight GPIO controlling framework. It is written in C and depends on libgpiod version 2.x.x.

It consists of a daemon, a client library and a command line tool. It is designed to run on Raspberry PIs and similar devices.

I wrote this tool primarily for myMPDos and myMPD.

myGPIOd can communicate natively with MPD and also integrates nicely with all HTTP APIs.

- `GitHub repository <https://github.com/jcorporation/myGPIOd>`__

.. include:: _includes/version

.. toctree::
   :glob:
   :hidden:

   installation.rst
   mygpio-configuration.rst
   mygpiod-http-api.rst
   mygpiod-protocol.rst
   mygpio-client.rst
   mygpio-library.rst
   test.rst
