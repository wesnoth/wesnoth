FROM emscripten/emsdk:5.0.7

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

RUN mv /usr/bin/pkg-config /usr/bin/pkg-config.real \
    && printf '%s\n' \
        '#!/bin/sh' \
        'output=$(/usr/bin/pkg-config.real "$@")' \
        'rc=$?' \
        'if [ "${WESNOTH_STRIP_PTHREAD_PKG_CONFIG:-0}" = "1" ] && [ -n "${PKG_CONFIG_PATH:-}" ]; then' \
        '    case "$PKG_CONFIG_PATH" in' \
        '        *wasm32-emscripten*)' \
        '            printf "%s\n" "$output" | sed "s/-pthread//g; s/-sPTHREAD_POOL_SIZE=[0-9]*//g"' \
        '            exit "$rc"' \
        '            ;;' \
        '    esac' \
        'fi' \
        'printf "%s\n" "$output"' \
        'exit "$rc"' \
        > /usr/bin/pkg-config \
    && chmod +x /usr/bin/pkg-config

WORKDIR /wesnoth

COPY build-web.sh /usr/local/bin/wesnoth-web-build
RUN chmod +x /usr/local/bin/wesnoth-web-build

ENTRYPOINT ["/usr/local/bin/wesnoth-web-build"]
