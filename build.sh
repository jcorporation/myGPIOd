#!/bin/sh
#
# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2024 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/myGPIOd
#

#save script path and change to it
STARTPATH=$(dirname "$(realpath "$0")")
cd "$STARTPATH" || exit 1

#exit on error
set -e

#exit on undefined variable
set -u

#set umask
umask 0022

#get mygpiod version
VERSION=$(grep "  VERSION" CMakeLists.txt | sed 's/  VERSION //')

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
  TS=$(stat -c%Y CMakeLists.txt)
  export LC_TIME="en_GB.UTF-8"
  DATE_F1=$(date --date=@"${TS}" +"%a %b %d %Y")
  DATE_F2=$(date --date=@"${TS}" +"%a, %d %b %Y %H:%m:%S %z")
  DATE_F3=$(date --date=@"${TS}" +"%d %b %Y")

  for F in contrib/packaging/alpine/APKBUILD contrib/packaging/arch/PKGBUILD \
      contrib/packaging/rpm/mygpiod.spec contrib/packaging/debian/changelog \
      contrib/man/man1/mygpiod.1 contrib/man/man1/mygpioc.1
  do
    echo "$F"
    sed -e "s/__VERSION__/${VERSION}/g" -e "s/__DATE_F1__/$DATE_F1/g" -e "s/__DATE_F2__/$DATE_F2/g" \
        -e "s/__DATE_F3__/$DATE_F3/g" "$F.in" > "$F"
  done
  echo "Generating man pages"
  rm -f contrib/man/man3/*.3
  doxygen
  mv contrib/man/man3/t_mygpio_connection.3 contrib/man/man3/libmygpio_t_mygpio_connection.3 
  mv contrib/man/man3/t_mygpio_gpio.3 contrib/man/man3/libmygpio_t_mygpio_gpio.3 
  mv contrib/man/man3/t_mygpio_idle_event.3 contrib/man/man3/libmygpio_t_mygpio_idle_event.3 
}

buildrelease() {
  BUILD_TYPE=$1
  echo "Compiling myGPIOd v${VERSION}"
  cmake -B release \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    .
  make -j4 -C release
}

addmygpioduser() {
  echo "Checking status of mygpiod system user"
  if ! getent passwd mympd > /dev/null
  then
    if check_cmd_silent useradd
    then
      groupadd -r gpio || true
      useradd -r -g gpio -s /bin/false -d /var/lib/mygpiod mygpiod
    elif check_cmd_silent adduser
    then
      #alpine
      addgroup -S gpio || true
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
  echo "Compiling myGPIOd v${VERSION}"
  CMAKE_SANITIZER_OPTIONS=""
  case "$ACTION" in
    asan)  CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_ASAN=ON" ;;
    tsan)  CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_TSAN=ON" ;;
    ubsan) CMAKE_SANITIZER_OPTIONS="-DMYGPIOD_ENABLE_UBSAN=ON" ;;
  esac

  cmake -B debug \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    $CMAKE_SANITIZER_OPTIONS \
    .
  make -j4 -C debug VERBOSE=1
  echo "Linking compilation database"
  sed -e 's/\\t/ /g' -e 's/-Wformat-overflow=2//g' -e 's/-fsanitize=bounds-strict//g' debug/compile_commands.json > mygpiod/compile_commands.json
  cp mygpiod/compile_commands.json libmygpio
  cp mygpiod/compile_commands.json mygpioc
  cp mygpiod/compile_commands.json mygpio-common
}

cleanup() {
  #build directories
  rm -rf release
  rm -rf debug
  rm -rf package
  rm -rf docs/html
  
  #tmp files
  find ./ -name \*~ -delete
  
  #compilation database
  rm -f mygpiod/compile_commands.json
  rm -f mygpioc/compile_commands.json
  rm -f libmygpio/compile_commands.json
  #clang tidy
  rm -f clang-tidy.out
}

cleanuposc() {
  rm -rf osc
}

check() {
  if [ ! -f mygpiod/compile_commands.json ]
  then
    echo "mygpiod/compile_commands.json not found"
    echo "run: ./build.sh debug"
    exit 1
  fi

  if check_cmd clang-tidy
  then
    echo "Running clang-tidy"
    rm -f clang-tidy.out
    cd mygpiod || exit 1
    find ./ -name '*.c' -exec clang-tidy \
        --config-file="$STARTPATH/.clang-tidy" {} \; >> ../clang-tidy.out  2>/dev/null
    cd ../mygpioc || exit 1
    find ./ -name '*.c' -exec clang-tidy \
        --config-file="$STARTPATH/.clang-tidy" {} \; >> ../clang-tidy.out  2>/dev/null
    cd ../libmygpio || exit 1
    find ./ -name '*.c' -exec clang-tidy \
        --config-file="$STARTPATH/.clang-tidy" {} \; >> ../clang-tidy.out  2>/dev/null
    cat ../clang-tidy.out
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
}

pkgdebian() {
  check_cmd dpkg-buildpackage
  prepare
  cp -a contrib/packaging/debian .
  install -d debian/mygpiod/DEBIAN
  cp debian/triggers debian/mygpiod/DEBIAN
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
    lintian "$PACKAGE" || true
  else
    echo "WARNING: lintian not found, can't check package"
  fi
}

pkgalpine() {
  check_cmd abuild
  prepare
  tar -czf "mygpiod_${VERSION}.orig.tar.gz" -- *
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
    [ -z "${GPGKEYID+x}" ] || KEYARG="--key $GPGKEYID"
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
  fi

  cd "$STARTPATH/osc" || exit 1
  if [ -f /etc/debian_version ]
  then
    cp "../package/mygpiod_${VERSION}.orig.tar.gz" "$OSC_REPO/"
    cp "../package/mygpiod_${VERSION}-1.dsc" "$OSC_REPO/"
    cp "../package/mygpiod_${VERSION}-1.debian.tar.xz"  "$OSC_REPO/"
  fi
  cp ../contrib/packaging/rpm/mygpiod.spec "$OSC_REPO/"
  cp ../contrib/packaging/arch/PKGBUILD "$OSC_REPO/"
  cp ../contrib/packaging/arch/archlinux.install "$OSC_REPO/"
  cp ../libgpiod-2.1.tar.gz "$OSC_REPO/"

  cd "$OSC_REPO" || exit 1
  osc addremove
  osc st
  osc vc -m "Update"
  osc commit -m "Update"
}

pkgdocker() {
  check_cmd docker
  prepare
  docker build --rm -t mygpiod -f "contrib/packaging/docker/Dockerfile" .
}

uninstall() {
  [ -z "${DESTDIR+x}" ] && DESTDIR=""
  # cmake does not provide an uninstall target,
  # instead its manifest is of use at least for
  # the binaries
  if [ -f release/install_manifest.txt ]
  then
    xargs rm -v -f < release/install_manifest.txt
  fi

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
  [ -z "${DESTDIR+x}" ] && DESTDIR=""
  rm -f "$DESTDIR/etc/init.d/mygpiod"
  #CMAKE_INSTALL_PREFIX="/usr"
  rm -f "$DESTDIR/etc/mygpiod.conf"
  rm -f "$DESTDIR/etc/mygpiod.conf.dist"
  rm -fr "$DESTDIR/etc/mygpiod.d"
  #CMAKE_INSTALL_PREFIX="/usr/local"
  rm -f "$DESTDIR/usr/local/etc/mygpiod.conf"
  rm -f "$DESTDIR/usr/local/etc/mygpiod.conf.dist"
  rm -fr "$DESTDIR/usr/local/etc/mygpiod.d"
  #CMAKE_INSTALL_PREFIX="/opt/mygpiod/"
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
      echo "Can not remove user mygpiod"
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
  release|MinSizeRel)
    buildrelease "Release"
  ;;
  RelWithDebInfo)
    buildrelease "RelWithDebInfo"
  ;;
  install)
    installrelease
  ;;
  releaseinstall)
    buildrelease "Release"
    installrelease
  ;;
  debug|asan|tsan|ubsan)
    builddebug
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
  pkgdocker)
    pkgdocker
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
    echo "  release:          build release files in directory release (stripped)"
    echo "  RelWithDebInfo:   build release files in directory release (with debug info)"
    echo "  install:          installs release files from directory release"
    echo "                    following environment variables are respected"
    echo "                      - DESTDIR=\"\""
    echo "  releaseinstall:   calls release and install afterwards"
    echo "  debug:            builds debug files in directory debug,"
    echo "  asan|tsan|ubsan:  builds debug files in directory debug"
    echo "                    linked with the sanitizer"
    echo "  check:            runs clang-tidy on source files"
    echo ""
    echo "Cleanup options:"
    echo "  cleanup:          cleanup source tree"
    echo "  uninstall:        removes mygpiod files, leaves configuration in place "
    echo "                    following environment variables are respected"
    echo "                      - DESTDIR=\"\""
    echo "  purge:            removes all mygpiod files, also your init scripts and configuration"
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
    echo "  pkgdocker:        creates the docker image"
    echo ""
    echo "Misc options:"
    echo "  setversion:       sets version and date in packaging files from CMakeLists.txt"
    echo "  addmygpioduser:   adds mygpiod group and user"
    echo ""
    exit 1
  ;;
esac

exit 0
