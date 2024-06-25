# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#
# Maintainer: Juergen Mang <mail@jcgames.de>

Name:           mygpiod
Version:        0.7.0
Release:        0 
License:        GPL-3.0-or-later
Group:          Hardware/Other
Summary:        A lightweight gpio controlling daemon.
Url:            https://jcorporation.github.io/myGPIOd/
Source:         mygpiod-%{version}.tar.gz
BuildRequires:  gcc
BuildRequires:  cmake
BuildRequires:  unzip
BuildRequires:  libmpdclient-devel
BuildRequires:  libcurl-devel
BuildRequires:  autoconf-archive
BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  lua-devel
%if 0%{?fedora} >= 39
BuildRequires:  libgpiod-devel
%else
BuildRequires:  git
%endif
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description 
myGPIOd is a lightweight gpio controlling daemon.

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
if [ "%{_defaultdocdir}" == "/usr/share/doc/packages" ]
then
  install -d "%{buildroot}%{_defaultdocdir}"
  mv -v "%{buildroot}/usr/share/doc/mygpiod" "%{buildroot}%{_defaultdocdir}/mygpiod"
fi

%post
/sbin/ldconfig
echo "myGPIOd installed"
echo "Modify /etc/mygpiod.conf to suit your needs"
true

%postun
/sbin/ldconfig

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
%config() /etc/mygpiod.d
%config() /etc/mygpiod.d/*
%{_mandir}/man1/mygpio*
%{_defaultdocdir}/mygpiod/*

%files devel
%{_datadir}/pkgconfig/libmygpio.pc
%{_includedir}/libmygpio
%{_includedir}/libmygpio/*
%{_mandir}/man3/libmygpio_*

%changelog
* Sun Jan 28 2024 Juergen Mang <mail@jcgames.de> 0.7.0-0
- Version from master
