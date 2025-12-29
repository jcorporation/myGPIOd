# SPDX-License-Identifier: GPL-3.0-or-later
# myGPIOd (c) 2020-2025 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/mygpiod

FROM alpine:latest AS build
COPY . /myGPIOd/
WORKDIR /
RUN <<EOF
set +ex
apk add --no-cache lua5.4-dev libmpdclient-dev curl-dev linux-headers alpine-sdk cmake libgpiod-dev linux-headers samurai
cmake -B "myGPIOd/release" -G Ninja -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release "myGPIOd"
cmake --build "myGPIOd/release"
EOF

FROM alpine:latest
RUN apk add --no-cache libmpdclient curl lua5.4 libgpiod
COPY --from=build /myGPIOd/release/bin/mygpiod /usr/bin/
COPY --from=build /myGPIOd/release/bin/mygpioc /usr/bin/
COPY --from=build /myGPIOd/release/libmygpio/libmygpio.so /usr/lib/
COPY --from=build /myGPIOd/release/libmygpio/libmygpio.so.0 /usr/lib/

ENTRYPOINT ["/usr/bin/mygpiod"]
