# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mygpiod

FROM alpine:latest as build
COPY . /myGPIOd/
WORKDIR /myGPIOd
RUN apk add --no-cache lua5.4-dev libmpdclient-dev curl-dev linux-headers alpine-sdk cmake autoconf automake libtool linux-headers autoconf-archive
RUN cmake -B "release" -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release -DMYGPIOD_MANPAGES=OFF -DMYGPIOD_DOC=OFF .
RUN make -j4 -C "release"

FROM alpine:latest
RUN apk add --no-cache libmpdclient curl lua5.4
COPY --from=build /myGPIOd/release/bin/mygpiod /usr/bin/
COPY --from=build /myGPIOd/release/bin/mygpioc /usr/bin/
COPY --from=build /myGPIOd/release/libmygpio/libmygpio.so /usr/lib/
COPY --from=build /myGPIOd/release/libmygpio/libmygpio.so.0 /usr/lib/

ENTRYPOINT ["/usr/bin/mygpiod"]
