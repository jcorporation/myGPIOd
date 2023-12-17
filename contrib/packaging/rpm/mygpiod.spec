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
BuildRequires:  libgpiod-devel
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
cmake -B release -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo  ..
make -c release

%install
cd release || exit 1
make install DESTDIR=%{buildroot}

%post
echo "Checking status of mygpiod system user and group"
getent group mygpiod > /dev/null || groupadd -r mygpiod
getent passwd mygpiod > /dev/null || useradd -r -g mygpiod -s /bin/false -d /var/lib/mygpiod mygpiod
echo "myGPIOd installed"
echo "Modify /etc/mygpiod.conf to suit your needs"
true

%files 
%defattr(-,root,root,-)
%doc README.md LICENSE.md
/usr/bin/mygpiod
/usr/bin/mygpioc
/usr/include/libmygpio/connection.h
/usr/include/libmygpio/gpio.h
/usr/include/libmygpio/idle.h
/usr/include/libmygpio/libmygpio.h
/usr/include/libmygpio/protocol.h
/usr/lib/libmygpio.so
/usr/lib/libmygpio.so.0
/usr/lib/systemd/system/mygpiod.service
%config(noreplace) /etc/mygpiod.conf
%config() /etc/mygpiod.d/gpio-in.example
%config() /etc/mygpiod.d/gpio-out.example
%{_mandir}/man1/mygpiod.1.gz
%{_mandir}/man1/mygpioc.1.gz
%{_defaultdocdir}/mygpiod/CHANGELOG.md
%{_defaultdocdir}/mygpiod/LICENSE.md
%{_defaultdocdir}/mygpiod/README.md
%{_defaultdocdir}/mygpiod/PROTOCOL.md
%license LICENSE.md

%changelog
* Sun Dec 17 2023 Juergen Mang <mail@jcgames.de> 0.4.0-0
- Version from master
