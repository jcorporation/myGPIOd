#
# spec file for package myGPIOd
#
# (c) 2020Juergen Mang <mail@jcgames.de>

Name:           mygpiod
Version:        0.1.0
Release:        0 
License:        GPL-2.0-or-later
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

%prep 
%setup -q -n %{name}-%{version}

%build
mkdir release
cd release || exit 1
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RELEASE ..
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

%postun
if [ "$1" = "0" ]
then
  echo "Please purge /var/lib/mygpiod manually"
fi

%files 
%defattr(-,root,root,-)
%doc README.md LICENSE
/usr/bin/mygpiod
/usr/lib/systemd/system/mygpiod.service
%config(noreplace) /etc/mygpiod.conf

%changelog
* Thu Nov 12 2020 Juergen Mang <mail@jcgames.de> 0.1.0-0
- Version from master
