FROM emscripten/emsdk:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    autoconf-archive \
    automake \
    bash \
    bison \
    ca-certificates \
    ccache \
    cmake \
    file \
    flex \
    git \
    libtool \
    ninja-build \
    pkg-config \
    python3 \
    rsync \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /wesnoth

COPY build-web.sh /usr/local/bin/wesnoth-web-build
RUN chmod +x /usr/local/bin/wesnoth-web-build

ENTRYPOINT ["/usr/local/bin/wesnoth-web-build"]
