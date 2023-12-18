#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#
# Maintainer: Juergen Mang <mail@jcgames.de>
#

Name:           mygpiod
Version:        0.4.0
Release:        0 
License:        GPL-3.0-or-later
Group:          Hardware/Other
Summary:        A lightweight gpio controlling daemon.
Url:            https://jcorporation.github.io/myGPIOd/
Source:         mygpiod-%{version}.tar.gz
BuildRequires:  gcc
BuildRequires:  cmake
BuildRequires:  unzip
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%global debug_package %{nil}

%description 
myGPIOd is a lightweight gpio controlling daemon.

%if 0%{?disturl:1}
  # build debug package in obs
  %debug_package
%endif

%prep 
%setup -q -n %{name}-%{version}

%build
cmake -B release -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo  .
make -C release

%install
make -C release install DESTDIR=%{buildroot}

%post
echo "Checking status of mygpiod system user and group"
getent group mygpiod > /dev/null || groupadd -r mygpiod
getent passwd mygpiod > /dev/null || useradd -r -g mygpiod -s /bin/false -d /var/lib/mygpiod mygpiod
echo "myGPIOd installed"
echo "Modify /etc/mygpiod.conf to suit your needs"
true

%files 
%defattr(-,root,root,-)
%{_bindir}/mygpiod
%{_bindir}/mygpioc
%{_includedir}/libmygpio/libmygpio_connection.h
%{_includedir}/libmygpio/libmygpio_gpio.h
%{_includedir}/libmygpio/libmygpio_gpioinfo.h
%{_includedir}/libmygpio/libmygpio_gpiolist.h
%{_includedir}/libmygpio/libmygpio_gpio_struct.h
%{_includedir}/libmygpio/libmygpio.h
%{_includedir}/libmygpio/libmygpio_idle.h
%{_includedir}/libmygpio/libmygpio_parser.h
%{_includedir}/libmygpio/libmygpio_protocol.h
%{_libdir}/libmygpio.so
%{_libdir}/libmygpio.so.0
%{_datadir}/pkgconfig/libmygpio.pc
/usr/lib/systemd/system/mygpiod.service
%config(noreplace) /etc/mygpiod.conf
%config() /etc/mygpiod.d/gpio-in.example
%config() /etc/mygpiod.d/gpio-out.example
%{_mandir}/man1/mygpiod.1.gz
%{_mandir}/man1/mygpioc.1.gz
%{_mandir}/man3/libmygpio_connection.3.gz
%{_mandir}/man3/libmygpio_connection.h.3.gz
%{_mandir}/man3/libmygpio_gpio_functions.3.gz
%{_mandir}/man3/libmygpio_gpio.h.3.gz
%{_mandir}/man3/libmygpio_gpioinfo.3.gz
%{_mandir}/man3/libmygpio_gpioinfo.h.3.gz
%{_mandir}/man3/libmygpio_gpiolist.3.gz
%{_mandir}/man3/libmygpio_gpiolist.h.3.gz
%{_mandir}/man3/libmygpio_gpio_settings.3.gz
%{_mandir}/man3/libmygpio_gpio_struct.h.3.gz
%{_mandir}/man3/libmygpio_idle_event.3.gz
%{_mandir}/man3/libmygpio_idle.h.3.gz
%{_mandir}/man3/libmygpio_parser.3.gz
%{_mandir}/man3/libmygpio_parser.h.3.gz
%{_mandir}/man3/libmygpio_protocol.3.gz
%{_mandir}/man3/libmygpio_protocol.h.3.gz
%{_mandir}/man3/libmygpio_t_mygpio_connection.3.gz
%{_mandir}/man3/libmygpio_t_mygpio_gpio.3.gz
%{_mandir}/man3/libmygpio_t_mygpio_idle_event.3.gz
%{_defaultdocdir}/mygpiod/CHANGELOG.md
%{_defaultdocdir}/mygpiod/LICENSE.md
%{_defaultdocdir}/mygpiod/README.md
%{_defaultdocdir}/mygpiod/PROTOCOL.md

%changelog
* Sun Dec 17 2023 Juergen Mang <mail@jcgames.de> 0.4.0-0
- Version from master
