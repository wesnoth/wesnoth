FROM wesnoth/wesnoth:steamrt-master
ENV DEBIAN_FRONTEND=noninteractive

COPY start.sh /staging/start.sh

ENTRYPOINT mkdir -p /build && cd /build && scons -j `nproc` ctool=gcc-5 cxxtool=g++-5 boostdir=/usr/local/include boostlibdir=/usr/local/lib extra_flags_config=-lrt -Y /wesnoth && cp /build/wesnoth /output/ && cp /build/wesnothd /output/ && cp -r /staging/* /output/
