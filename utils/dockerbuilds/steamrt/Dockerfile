FROM wesnoth/wesnoth:steamrt-sniper
ENV DEBIAN_FRONTEND=noninteractive

COPY start.sh /staging/start.sh

ENTRYPOINT mkdir -p /build && cd /build && scons -j `nproc` ctool=gcc-10 cxxtool=g++-10 enable_lto=true boostdir=/usr/local/include boostlibdir=/usr/local/lib extra_flags_config=-lrt -Y /wesnoth && cp /build/wesnoth /output/ && cp /build/wesnothd /output/ && cp -r /staging/* /output/ && mkdir -p /output/lib && cp /usr/local/lib/x86_64-linux-gnu/libpango* /output/lib/
