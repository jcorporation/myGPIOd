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

%description 
myGPIOd is a lightweight gpio controlling daemon.

%if 0%{?disturl:1}
  # build debug package in obs
  %debug_package
%endif

%package devel
Summary: Development package for %{name}

%description devel
Files for development with %{name}.

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
%license LICENSE.md
%doc LICENSE.md
%doc README.md
%doc CHANGELOG.md
%doc PROTOCOL.md
%{_bindir}/mygpiod
%{_bindir}/mygpioc
%{_libdir}/libmygpio.so*
/usr/lib/systemd/system/mygpiod.service
%config(noreplace) /etc/mygpiod.conf
%config() /etc/mygpiod.d/*
%{_mandir}/man1/mygpio*

%files devel
%{_datadir}/pkgconfig/libmygpio.pc
%{_includedir}/libmygpio/*
%{_mandir}/man3/libmygpio_*

%changelog
* Tue Dec 19 2023 Juergen Mang <mail@jcgames.de> 0.4.0-0
- Version from master
