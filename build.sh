#!/bin/sh
#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2022 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

STARTPATH=$(pwd)

#exit on error
set -e

#exit on undefined variable
set -u

#set umask
umask 0022

#get mygpiod version
VERSION=$(grep CPACK_PACKAGE_VERSION_ CMakeLists.txt | cut -d\" -f2 | tr '\n' '.' | sed 's/\.$//')

#check for command
check_cmd() {
  for DEPENDENCY in "$@"
  do
    if ! check_cmd_silent "$@"
    then
      echo "ERROR: ${DEPENDENCY} not found"
      return 1
    fi
  done
  return 0
}

check_cmd_silent() {
  for DEPENDENCY in "$@"
  do
    if ! command -v "${DEPENDENCY}" > /dev/null
    then
      return 1
    fi
  done
  return 0
}

setversion() {
  echo "Setting version to ${VERSION}"
  export LC_TIME="en_GB.UTF-8"
  
  sed -e "s/__VERSION__/${VERSION}/g" contrib/packaging/alpine/APKBUILD.in > contrib/packaging/alpine/APKBUILD
  sed -e "s/__VERSION__/${VERSION}/g" contrib/packaging/arch/PKGBUILD.in > contrib/packaging/arch/PKGBUILD
  DATE=$(date +"%a %b %d %Y")
  sed -e "s/__VERSION__/${VERSION}/g" -e "s/__DATE__/$DATE/g" \
  	contrib/packaging/rpm/mygpiod.spec.in > contrib/packaging/rpm/mygpiod.spec
  DATE=$(date +"%a, %d %b %Y %H:%m:%S %z")
  sed -e "s/__VERSION__/${VERSION}/g" -e "s/__DATE__/$DATE/g" \
  	contrib/packaging/debian/changelog.in > contrib/packaging/debian/changelog
}

buildrelease() {
  echo "Compiling myGPIOd"
  install -d release
  cd release || exit 1
  #set INSTALL_PREFIX and build mygpiod
  export INSTALL_PREFIX="${MYGPIOD_INSTALL_PREFIX:-/usr}"
  cmake -DCMAKE_INSTALL_PREFIX:PATH="$INSTALL_PREFIX" -DCMAKE_BUILD_TYPE=RELEASE ..
  make
}

addmygpioduser() {
  echo "Checking status of mygpiod system user"
  if ! getent passwd mympd > /dev/null
  then
    if check_cmd_silent useradd
    then
      useradd -r -g gpio -s /bin/false -d /var/lib/mygpiod mygpiod
    elif check_cmd_silent adduser
    then
      #alpine
      adduser -S -D -H -h /var/lib/mygpiod -s /sbin/nologin -G gpio -g myGPIOd mygpiod
    else
      echo "Can not add user mygpiod"
      return 1
    fi
  fi
  return 0
}

installrelease() {
  echo "Installing myGPIOd"
  cd release || exit 1  
  [ -z "${DESTDIR+x}" ] && DESTDIR=""
  make install DESTDIR="$DESTDIR"
  addmygpioduser
  echo "mygpiod installed"
  echo "Modify mygpiod.conf to suit your needs."
}

builddebug() {
  MEMCHECK=$1
  echo "Compiling myGPIOd"
  install -d debug
  cd debug || exit 1
  cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=DEBUG -DMEMCHECK="$MEMCHECK" \
  	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
  make VERBOSE=1
  echo "Linking compilation database"
  sed -e 's/\\t/ /g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' -e 's/-static-libasan//g' compile_commands.json > ../src/compile_commands.json
}

cleanup() {
  #build directories
  rm -rf release
  rm -rf debug
  rm -rf package
  
  #tmp files
  find ./ -name \*~ -delete
  
  #compilation database
  rm -f src/compile_commands.json
  #clang tidy
  rm -f clang-tidy.out
}

cleanuposc() {
  rm -rf osc
}

check() {
  if check_cmd cppcheck
  then
    echo "Running cppcheck"
    [ -z "${CPPCHECKOPTS+z}" ] && CPPCHECKOPTS="--enable=warning"
    cppcheck $CPPCHECKOPTS src/*.c src/*.h
  else
    echo "cppcheck not found"
  fi

  if check_cmd flawfinder
  then
    echo "Running flawfinder"
    [ -z "${FLAWFINDEROPTS+z}" ] && FLAWFINDEROPTS="-m3"
    flawfinder $FLAWFINDEROPTS src
  else
    echo "flawfinder not found"
  fi

  if [ ! -f src/compile_commands.json ]
  then
    echo "src/compile_commands.json not found"
    echo "run: ./build.sh debug"
    exit 1
  fi

  if check_cmd clang-tidy
  then
    echo "Running clang-tidy, output goes to clang-tidy.out"
    rm -f clang-tidy.out
    cd src || exit 1
	find ./ -name '*.c' -exec clang-tidy \
    	--checks="*,-hicpp-signed-bitwise,-cert-env33-c,-clang-diagnostic-format-nonliteral,-llvm-header-guard" \
    	-header-filter='.*' {}  \; >> ../clang-tidy.out
  else
    echo "clang-tidy not found"  
  fi
}

prepare() {
  cleanup
  SRC=$(ls -d "$PWD"/* -1)
  mkdir -p package/build
  cd package/build || exit 1
  for F in $SRC
  do
    [ "$F" = "$STARTPATH/osc" ] && continue
    cp -av "$F" .
  done
}

pkgdebian() {
  check_cmd dpkg-buildpackage
  prepare
  cp -a contrib/packaging/debian .
  export LC_TIME="en_GB.UTF-8"
  tar -czf "../mygpiod_${VERSION}.orig.tar.gz" -- *
  
  SIGNOPT="--no-sign"
  if [ -n "${SIGN+x}" ] && [ "$SIGN" = "TRUE" ]
  then
    SIGNOPT="--sign-key=$GPGKEYID"  
  else
    echo "Package would not be signed"
  fi
  #shellcheck disable=SC2086
  dpkg-buildpackage -rfakeroot $SIGNOPT

  #get created package name
  PACKAGE=$(ls ../mygpiod_"${VERSION}"-1_*.deb)
  if [ "$PACKAGE" = "" ]
  then
    echo "Can't find package"
  fi

  if check_cmd lintian
  then
    echo "Checking package with lintian"
    lintian "$PACKAGE"
  else
    echo "WARNING: lintian not found, can't check package"
  fi
}

pkgalpine() {
  if [ -z "${1+x}" ]
  then
    TARONLY=""
  else
    TARONLY=$1
  fi
  check_cmd abuild
  prepare
  tar -czf "mygpiod_${VERSION}.orig.tar.gz" -- *
  [ "$TARONLY" = "taronly" ] && return 0
  cp contrib/packaging/alpine/* .
  abuild checksum
  abuild -r
}

pkgrpm() {
  if [ -z "${1+x}" ]
  then
    TARONLY=""
  else
    TARONLY=$1
  fi
  check_cmd rpmbuild
  prepare
  SRC=$(ls)
  mkdir "mygpiod-${VERSION}"
  for F in $SRC
  do
    mv "$F" "mygpiod-${VERSION}"
  done
  tar -czf "mygpiod-${VERSION}.tar.gz" "mygpiod-${VERSION}"
  [ "$TARONLY" = "taronly" ] && return 0
  install -d "$HOME/rpmbuild/SOURCES"
  mv "mygpiod-${VERSION}.tar.gz" ~/rpmbuild/SOURCES/
  cp ../../contrib/packaging/rpm/mygpiod.spec .
  rpmbuild -ba mygpiod.spec
  if check_cmd rpmlint
  then
    echo "Checking package with rpmlint"
    ARCH=$(uname -p)
    rpmlint "$HOME/rpmbuild/RPMS/${ARCH}/mygpiod-${VERSION}-0.${ARCH}.rpm"
  else
    echo "WARNING: rpmlint not found, can't check package"
  fi
}

pkgarch() {
  check_cmd makepkg
  prepare
  tar -czf "mygpiod_${VERSION}.orig.tar.gz" -- *
  cp contrib/packaging/arch/* .
  makepkg
  if [ -n "${SIGN+x}" ] && [ "$SIGN" = "TRUE" ]
  then
    KEYARG=""
    [ -z "${GPGKEYID+x}" ] || KEYARG="--key $PGPGKEYID"
    makepkg --sign "$KEYARG" mygpiod-*.pkg.tar.xz
  fi
  if check_cmd namcap
  then
    echo "Checking package with namcap"
    namcap PKGBUILD
    namcap mygpiod-*.pkg.tar.xz
  else
    echo "WARNING: namcap not found, can't check package"
  fi
}

pkgosc() {
  check_cmd osc  
  cleanup
  cleanuposc
  if [ -z "${OSC_REPO+x}" ]
  then
    if [ -f .git/HEAD ] && grep -q "master" .git/HEAD
    then
      OSC_REPO="home:jcorporation/myGPIOd"
    else
      OSC_REPO="home:jcorporation/myGPIOd-devel"
    fi
  fi
  
  mkdir osc
  cd osc || exit 1  
  osc checkout "$OSC_REPO"
  rm -f "$OSC_REPO"/*
  
  cd "$STARTPATH" || exit 1
  pkgrpm taronly

  cd "$STARTPATH" || exit 1
  cp "package/build/mygpiod-${VERSION}.tar.gz" "osc/$OSC_REPO/"
  
  if [ -f /etc/debian_version ]
  then
    pkgdebian
  else
    pkgalpine taronly
    rm -f "$OSC_REPO"/debian.*
  fi

  cd "$STARTPATH/osc" || exit 1
  cp "../package/mygpiod_${VERSION}.orig.tar.gz" "$OSC_REPO/"
  if [ -f /etc/debian_version ]
  then
    cp "../package/mygpiod_${VERSION}-1.dsc" "$OSC_REPO/"
    cp "../package/mygpiod_${VERSION}-1.debian.tar.xz"  "$OSC_REPO/"
  fi
  cp ../contrib/packaging/rpm/mygpiod.spec "$OSC_REPO/"
  cp ../contrib/packaging/arch/PKGBUILD "$OSC_REPO/"
  cp ../contrib/packaging/arch/archlinux.install "$OSC_REPO/"

  cd "$OSC_REPO" || exit 1
  osc addremove
  osc st
  osc vc
  osc commit
}

installdeps() {
  echo "Platform: $(uname -m)"
  if [ -f /etc/debian_version ]
  then
    #debian
    apt-get update
    apt-get install -y --no-install-recommends gcc cmake build-essential libgpiod-dev
  elif [ -f /etc/arch-release ]
  then
    #arch
    pacman -S gcc cmake libgpiod
  elif [ -f /etc/alpine-release ]
  then
    #alpine
    apk add cmake alpine-sdk linux-headers libgpiod-dev
  elif [ -f /etc/SuSE-release ]
  then
    #suse
    zypper install gcc cmake unzip libgpiod-devel
  elif [ -f /etc/redhat-release ]
  then  
    #fedora 	
    yum install gcc cmake unzip libgpiod-devel
  else 
    echo "Unsupported distribution detected."
    echo "You should manually install:"
    echo "  - gcc"
    echo "  - cmake"
  fi
}

# Also deletes stale installations in other locations.
#
uninstall() {
  # cmake does not provide an uninstall target,
  # instead its manifest is of use at least for
  # the binaries
  if [ -f release/install_manifest.txt ]
  then
    xargs rm < release/install_manifest.txt
  fi

  #MYGPIOD_INSTALL_PREFIX="/usr"
  rm -f "$DESTDIR/usr/bin/mygpiod"
   #MYGPIOD_INSTALL_PREFIX="/usr/local"
  rm -f "$DESTDIR/usr/local/bin/mygpiod"
   #MYGPIOD_INSTALL_PREFIX="/opt/mygpiod/"
  rm -rf "$DESTDIR/opt/mygpiod"
  #systemd
  rm -f "$DESTDIR/usr/lib/systemd/system/mygpiod.service"
  rm -f "$DESTDIR/lib/systemd/system/mygpiod.service"
  #sysVinit, open-rc
  if [ -z "$DESTDIR" ] && [ -f "/etc/init.d/mygpiod" ]
  then
    echo "SysVinit / OpenRC-script /etc/init.d/mygpiod found."
    echo "Make sure it isn't part of any runlevel and delete by yourself"
    echo "or invoke with purge instead of uninstall."
  fi
}

purge() {
  #MYGPIOD_INSTALL_PREFIX="/usr"
  rm -rf "$DESTDIR/var/lib/mygpiod"
  rm -f "$DESTDIR/etc/mygpiod.conf"
  rm -f "$DESTDIR/etc/mygpiod.conf.dist"
  rm -f "$DESTDIR/etc/init.d/mygpiod"
  #MYGPIOD_INSTALL_PREFIX="/usr/local"
  rm -f "$DESTDIR/usr/local/etc/mygpiod.conf"
  rm -f "$DESTDIR/usr/local/etc/mygpiod.conf.dist"
  #MYGPIOD_INSTALL_PREFIX="/opt/mygpiod/"
  rm -rf "$DESTDIR/var/opt/mygpiod"
  rm -rf "$DESTDIR/etc/opt/mygpiod"
  #remove user
  if getent passwd mygpiod > /dev/null
  then
  	if check_cmd_silent userdel
    then
	  userdel mygpiod
	elif check_cmd_silent deluser
	then
	  deluser mygpiod
	else
	  echo "Can not del user mygpiod"
	  return 1
	fi
  fi
  return 0
}

#get action
if [ -z "${1+x}" ]
then
  ACTION=""
else
  ACTION="$1"
fi

case "$ACTION" in
	release)
	  buildrelease
	;;
	install)
	  installrelease
	;;
	releaseinstall)
	  buildrelease
	  cd .. || exit 1
	  installrelease
	;;
	debug)
	  builddebug "FALSE"
	;;
	memcheck)
	  builddebug "TRUE"
	;;
	installdeps)
	  installdeps
	;;
	cleanup)
	  cleanup
	  cleanuposc
	;;
	check)
	  check
	;;
	pkgdebian)
	  pkgdebian
	;;
	pkgalpine)
	  pkgalpine
	;;
	pkgrpm)
	  pkgrpm
	;;
	pkgarch)
	  pkgarch
	;;
	setversion)
	  setversion
	;;
	pkgosc)
	  pkgosc
	;;
	addmygpioduser)
	  addmygpioduser
	;;
	uninstall)
	  uninstall
	;;
	purge)
	  uninstall
	  purge
	;;
	*)
	  echo "Usage: $0 <option>"
	  echo "Version: ${VERSION}"
	  echo ""
	  echo "Build options:"
	  echo "  release:          build release files in directory release"
	  echo "  install:          installs release files from directory release"
	  echo "                    following environment variables are respected"
	  echo "                      - DESTDIR=\"\""
	  echo "  releaseinstall:   calls release and install afterwards"
	  echo "  debug:            builds debug files in directory debug,"
	  echo "                    linked with libasan3, uses assets in htdocs"
	  echo "  memcheck:         builds debug files in directory debug"
	  echo "                    for use with valgrind, uses assets in htdocs"
	  echo "  check:            runs cppcheck and flawfinder on source files"
	  echo "                    following environment variables are respected"
	  echo "                      - CPPCHECKOPTS=\"--enable=warning\""
	  echo "                      - FLAWFINDEROPTS=\"-m3\""
	  echo "  installdeps:      installs build and run dependencies"
	  echo ""
	  echo "Cleanup options:"
	  echo "  cleanup:          cleanup source tree"
	  echo "  cleanupdist:      cleanup dist directory, forces release to build new assets"
	  echo "  uninstall:        removes mygpiod files, leaves configuration in place "
	  echo "                    following environment variables are respected"
	  echo "                      - DESTDIR=\"\""
	  echo "  purge:            removes all mygpiod files, also your init scripts"
	  echo "                    following environment variables are respected"
	  echo "                      - DESTDIR=\"\""
	  echo ""
	  echo "Packaging options:"
	  echo "  pkgalpine:        creates the alpine package"
	  echo "  pkgarch:          creates the arch package"
	  echo "                    following environment variables are respected"
	  echo "                      - SIGN=\"FALSE\""
	  echo "                      - GPGKEYID=\"\""
	  echo "  pkgdebian:        creates the debian package"
	  echo "                    following environment variables are respected"
	  echo "                      - SIGN=\"FALSE\""
	  echo "                      - GPGKEYID=\"\""
	  echo "  pkgrpm:           creates the rpm package"
	  echo "  pkgosc:           updates the open build service repository"
	  echo "                    following environment variables are respected"
	  echo "                      - OSC_REPO=\"home:jcorporation/myGPIOd\""
	  echo ""
	  echo "Misc options:"
	  echo "  setversion:       sets version and date in packaging files from CMakeLists.txt"
	  echo "  addmygpioduser:     adds mygpiod group and user"
	  echo ""
	  echo "Environment variables for building"
	  echo "  - MYGPIOD_INSTALL_PREFIX=\"/usr\""
	  echo ""
	  exit 1
	;;
esac

exit 0
