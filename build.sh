#!/bin/sh
#
# SPDX-License-Identifier: GPL-2.0-or-later
# myGPIOd (c) 2020 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

STARTPATH=$(pwd)

#set umask
umask 0022

#get mygpiod version
VERSION=$(grep CPACK_PACKAGE_VERSION_ CMakeLists.txt | cut -d\" -f2 | tr '\n' '.' | sed 's/\.$//')

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
  mv contrib/packaging/gentoo/mygpiod-*.ebuild "contrib/packaging/gentoo/mygpiod-${VERSION}.ebuild"
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
  echo "Checking status of mygpiod system user and group"
  getent group mygpiod > /dev/null || groupadd -r mygpiod
  getent passwd mygpiod > /dev/null || useradd -r -g mygpiod -s /bin/false -d /var/lib/mygpiod mygpiod
}

installrelease() {
  echo "Installing myGPIOd"
  cd release || exit 1  
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

buildtest() {
  install -d test/build
  cd test/build || exit 1
  cmake ..
  make VERBOSE=1
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
  CPPCHECKBIN=$(command -v cppcheck)
  [ "$CPPCHECKOPTS" = "" ] && CPPCHECKOPTS="--enable=warning"
  if [ "$CPPCHECKBIN" != "" ]
  then
    echo "Running cppcheck"
    $CPPCHECKBIN $CPPCHECKOPTS src/*.c src/*.h
    $CPPCHECKBIN $CPPCHECKOPTS src/mpd_client/*.c src/mpd_client/*.h
    $CPPCHECKBIN $CPPCHECKOPTS src/mygpiod_api/*.c src/mygpiod_api/*.h
    $CPPCHECKBIN $CPPCHECKOPTS src/web_server/*.c src/web_server/*.h
    $CPPCHECKBIN $CPPCHECKOPTS cli_tools/*.c
  else
    echo "cppcheck not found"
  fi
  
  FLAWFINDERBIN=$(command -v flawfinder)
  [ "$FLAWFINDEROPTS" = "" ] && FLAWFINDEROPTS="-m3"
  if [ "$FLAWFINDERBIN" != "" ]
  then
    echo "Running flawfinder"
    $FLAWFINDERBIN $FLAWFINDEROPTS src
  else
    echo "flawfinder not found"
  fi

  if [ ! -f src/compile_commands.json ]
  then
    echo "src/compile_commands.json not found"
    echo "run: ./build.sh debug"
    exit 1
  fi
  
  CLANGTIDYBIN=$(command -v clang-tidy)
  if [ "$CLANGTIDYBIN" != "" ]
  then
    echo "Running clang-tidy, output goes to clang-tidy.out"
    rm -f clang-tidy.out
    cd src || exit 1
    find ./ -name '*.c' -exec clang-tidy \
    	--checks="*,-readability-isolate-declaration,-hicpp-multiway-paths-covered,-readability-uppercase-literal-suffix,-hicpp-uppercase-literal-suffix,-cert-msc51-cpp,-cert-msc32-c,-hicpp-no-assembler,-android*,-cert-env33-c,-cert-msc50-cpp,-bugprone-branch-clone,-misc-misplaced-const,-readability-non-const-parameter,-cert-msc30-c,-hicpp-signed-bitwise,-readability-magic-numbers,-readability-avoid-const-params-in-decls,-llvm-include-order,-bugprone-macro-parentheses,-modernize*,-cppcoreguidelines*,-llvm-header-guard,-clang-analyzer-optin.performance.Padding,-clang-diagnostic-embedded-directive" \
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
    cp -a "$F" .
  done
  rm -r dist/buildtools
}

pkgdebian() {
  prepare
  cp -a contrib/packaging/debian .
  export LC_TIME="en_GB.UTF-8"
  tar -czf "../mygpiod_${VERSION}.orig.tar.gz" -- *
  dpkg-buildpackage -rfakeroot

  #get created package name
  PACKAGE=$(ls ../mygpiod_"${VERSION}"-1_*.deb)
  if [ "$PACKAGE" = "" ]
  then
    echo "Can't find package"
  fi

  if [ "$SIGN" = "TRUE" ]
  then  
    DPKGSIG=$(command -v dpkg-sig)
    if [ "$DPKGSIG" != "" ]
    then
      if [ "$GPGKEYID" != "" ]
      then
        echo "Signing package with key $GPGKEYID"
        dpkg-sig -k "$GPGKEYID" --sign builder "$PACKAGE"
      else
        echo "WARNING: GPGKEYID not set, can't sign package"
      fi
    else
      echo "WARNING: dpkg-sig not found, can't sign package"
    fi
  fi
  
  LINTIAN=$(command -v lintian)
  if [ "$LINTIAN" != "" ]
  then
    echo "Checking package with lintian"
    $LINTIAN "$PACKAGE"
  else
    echo "WARNING: lintian not found, can't check package"
  fi
}

pkgalpine() {
  prepare
  tar -czf "mygpiod_${VERSION}.orig.tar.gz" -- *
  [ "$1" = "taronly" ] && return 0
  cp contrib/packaging/alpine/* .
  abuild checksum
  abuild -r
}

pkgrpm() {
  prepare
  SRC=$(ls)
  mkdir "mygpiod-${VERSION}"
  for F in $SRC
  do
    mv "$F" "mygpiod-${VERSION}"
  done
  tar -czf "mygpiod-${VERSION}.tar.gz" "mygpiod-${VERSION}"
  [ "$1" = "taronly" ] && return 0
  install -d "$HOME/rpmbuild/SOURCES"
  mv "mygpiod-${VERSION}.tar.gz" ~/rpmbuild/SOURCES/
  cp ../../contrib/packaging/rpm/mygpiod.spec .
  rpmbuild -ba mygpiod.spec
  RPMLINT=$(command -v rpmlint)
  if [ "$RPMLINT" != "" ]
  then
    echo "Checking package with rpmlint"
    ARCH=$(uname -p)
    $RPMLINT "$HOME/rpmbuild/RPMS/${ARCH}/mygpiod-${VERSION}-0.${ARCH}.rpm"
  else
    echo "WARNING: rpmlint not found, can't check package"
  fi
}

pkgarch() {
  prepare
  tar -czf "mygpiod_${VERSION}.orig.tar.gz" -- *
  cp contrib/packaging/arch/* .
  makepkg
  if [ "$SIGN" = "TRUE" ]
  then
    KEYARG=""
    [ "$GPGKEYID" != "" ] && KEYARG="--key $PGPGKEYID"
    makepkg --sign "$KEYARG" mygpiod-*.pkg.tar.xz
  fi
  NAMCAP=$(command -v namcap)
  if [ "$NAMCAP" != "" ]
  then
    echo "Checking package with namcap"
    $NAMCAP PKGBUILD
    $NAMCAP mygpiod-*.pkg.tar.xz
  else
    echo "WARNING: namcap not found, can't check package"
  fi
}

pkgosc() {
  OSCBIN=$(command -v osc)
  if [ "$OSCBIN" = "" ]
  then
    echo "ERROR: osc not found"
    exit 1
  fi
  
  cleanup
  cleanuposc
  [ "$OSC_REPO" = "" ] && OSC_REPO="home:jcorporation/mygpiod"
  
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
    apt-get install -y --no-install-recommends gcc cmake build-essential
  elif [ -f /etc/arch-release ]
  then
    #arch
    pacman -S gcc cmake
  elif [ -f /etc/alpine-release ]
  then
    #alpine
    apk add cmake alpine-sdk linux-headers
  elif [ -f /etc/SuSE-release ]
  then
    #suse
    zypper install gcc cmake unzip
  elif [ -f /etc/redhat-release ]
  then  
    #fedora 	
    yum install gcc cmake unzip
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
  getent passwd mygpiod > /dev/null && userdel mygpiod
  getent group mygpiod > /dev/null && groupdel -f mygpiod
}

case "$1" in
	release)
	  buildrelease
	;;
	install)
	  cleanupoldinstall
	  installrelease
	;;
	releaseinstall)
	  buildrelease
	  cd .. || exit 1
	  cleanupoldinstall
	  installrelease
	;;
	cleanupoldinst)
	  cleanupoldinstall
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
	  echo "  cleanupoldinst:   removes deprecated files"
	  echo "  uninstall:        removes mygpiod files, leaves configuration and "
	  echo "                    state files in place"
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
	  echo "                      - OSC_REPO=\"home:jcorporation/mygpiod\""
	  echo ""
	  echo "Misc options:"
	  echo "  setversion:       sets version and date in packaging files from CMakeLists.txt"
	  echo "  addmygpioduser:     adds mygpiod group and user"
	  echo ""
	  echo "Environment variables for building"
	  echo "  - MYGPIOD_INSTALL_PREFIX=\"/usr\""
	  echo ""
	;;
esac
