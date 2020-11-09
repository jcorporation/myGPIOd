# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit eutils user cmake systemd

MY_PN="myGPIOd"
S="${WORKDIR}/${MY_PN}-${PV}"

DESCRIPTION="A daemon for interacting with raspberry GPIOs."
HOMEPAGE="https://jcorporation.github.io/myGPIOd"
SRC_URI="https://github.com/jcorporation/${MY_PN}/archive/v${PV}.tar.gz -> ${PN}-${PV}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64 ~x86 ~arm ~arm64"
IUSE="systemd"

BDEPEND="
        >=dev-util/cmake-3.4
"

RDEPEND="
        systemd? ( sys-apps/systemd )
"

QA_PRESTRIPPED="
	usr/bin/mygpiod
"

src_compile() {
    default
    ./build.sh release || die
}

src_install() {
    cd release
    dobin mygpiod
    newinitd "contrib/initscripts/mygpiod.openrc" "${PN}"
    if use systemd; then
        systemd_newunit contrib/initscripts/mygpiod.service mygpiod.service
    fi
    dodoc ${S}/README.md
}

pkg_postinst() {
    elog "Checking status of mygpiod system user and group"
    getent group gpio > /dev/null || enewgroup gpio
    getent passwd mygpiod > /dev/null || enewuser mygpiod -1 -1 -1 gpio

    elog
    elog "myGPIOd installed"
    elog "Modify /etc/mygpiod.conf to suit your needs."
    elog
}
