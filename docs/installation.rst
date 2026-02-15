Installation
============

- Build it yourself.
- Use the `docker image <https://github.com/jcorporation?tab=packages&repo_name=myGPIOd>`__.
- Use `prebuild packages <https://download.opensuse.org/repositories/home:/jcorporation/>`__.

Build
-----

Building myGPIOd is straight forward.

Dependencies
~~~~~~~~~~~~

- C build environment
- cmake >= 3.13
- ninja
- pkgconf
- libgpiod >= 2.0.0
- Optional:
  - libcurl
  - libmicrohttpd >= 1.0.0
  - libmpdclient2
  - lua >= 5.4.0

Build myGPIOd
~~~~~~~~~~~~~

This builds and installs the ``mygpiod`` daemon, ``mygpioc`` command
line tool, the shared library ``libmygpio``, the associated header files
for development and the documentation.

1. Get myGPIOd tarball from `GitHub <https://github.com/jcorporation/myGPIOd/releases/latest>`__
2. Extract myGPIOd tarball and change path to this directory
3. Run cmake: ``cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release .``
4. Build: ``cmake --build build``
5. Install (as root): ``cmake --install build``


myMPD specific cmake options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-----------------------------+---------+------------------------------------------------+
| OPTION                      | DEFAULT | DESCRIPTION                                    |
+=============================+=========+================================================+
| MYGPIOD_ENABLE_ACTION_MPC   | ON      | Enables the mpc action, requires libmpdclient. |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_ENABLE_ACTION_HTTP  | ON      | Enables the http action, requires libcurl.     |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_ENABLE_ACTION_LUA   | ON      | Enables the lua action, requires lua.          |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_ENABLE_HTTPD        | ON      | Enables the integrated http server.            |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_DOC                 | ON      | Installs documentation.                        |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_MANPAGES            | ON      | Creates and installs manpages.                 |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_STARTUP_SCRIPT      | ON      | Installs the startup script.                   |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_CLIENT              | ON      | Builds the myGPIOd client.                     |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_DAEMON              | ON      | Builds the myGPIOd daemon.                     |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_HEADER              | ON      | Installs the myGPIOd headers.                  |
+-----------------------------+---------+------------------------------------------------+
| MYGPIOD_LIBRARY             | ON      | Builds the myGPIOd library.                    |
+-----------------------------+---------+------------------------------------------------+

cmake build types
~~~~~~~~~~~~~~~~~

- **Release, MinSizeRel**

  - Uses predefined compile and link options for a release build
  - No debug output
  - Strips binary

- **RelWithDebInfo**

  - Uses predefined compile and link options for a release build
  - No debug output
  - Included Debug info

- **Debug**

  - Uses predefined compile and link options for a debug build
  - Debug output
  - Included Debug info

- **None**

  - Use this option to set your own compile and link options
