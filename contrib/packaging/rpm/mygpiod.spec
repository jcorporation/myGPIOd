#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2023 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#
# Maintainer: Juergen Mang <mail@jcgames.de>
#

Name:           mygpiod
Version:        0.3.1
Release:        0 
License:        GPL-3.0-or-later
Group:          Hardware/Other
Summary:        A small daemon to call scripts on GPIO events. 
Url:            https://jcorporation.github.io/myGPIOd/
Source:         mygpiod-%{version}.tar.gz
BuildRequires:  gcc
BuildRequires:  cmake
BuildRequires:  unzip
BuildRequires:  libgpiod-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%global debug_package %{nil}

%description 
myGPIOd is a small daemon to call scripts on GPIO events. 

%if 0%{?disturl:1}
  # build debug package in obs
  %debug_package
%endif

%prep 
%setup -q -n %{name}-%{version}

%build
mkdir release
cd release || exit 1
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release -DMYGPIOD_STRIP_BINARY=OFF ..
make

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
/usr/lib/systemd/system/mygpiod.service
%config(noreplace) /etc/mygpiod.conf

%changelog
* Wed Dec 13 2023 Juergen Mang <mail@jcgames.de> 0.3.1-0
- Version from master
