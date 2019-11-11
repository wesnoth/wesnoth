FROM steamrt_scout_amd64
ENV DEBIAN_FRONTEND=noninteractive
ARG BOOST_VERSION=1.66.0
ARG BOOST_VERSION_=1_66_0

RUN apt-get update && apt-get -y dist-upgrade && apt-get install -y scons libicu-dev libreadline-dev

RUN wget --max-redirect 3 https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_}.tar.gz
RUN mkdir -p /src/boost && tar zxf boost_${BOOST_VERSION_}.tar.gz -C /src/boost --strip-components=1
RUN cd /src/boost && ./bootstrap.sh --with-libraries=iostreams,regex,system,filesystem,program_options,random,locale && ./b2 toolset=gcc-5 --layout=system link=static variant=release cxxflags='-fPIE -fstack-protector-strong' define=_FORTIFY_SOURCE=2 install

COPY start.sh /staging/start.sh
RUN mkdir /staging/lib64 && cp /lib/libicu*.so.* /lib/x86_64-linux-gnu/libhistory.so.6 /staging/lib64/

ENTRYPOINT mkdir /build && cd /build && scons -j `nproc` ctool=gcc-5 cxxtool=g++-5 boostdir=/usr/local/include boostlibdir=/usr/local/lib extra_flags_config=-lrt -Y /wesnoth && cp /build/wesnoth /output/ && cp /build/wesnothd /output/ && cp -r /staging/* /output/
