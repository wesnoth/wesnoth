#!/bin/bash

export EXTRA_FLAGS_RELEASE=""
export WML_TESTS=true
export PLAY_TEST=true
export MP_TEST=true
export WML_TEST_TIME=15
export BOOST_TEST=true

if [ "$OPT" = "-O0" ]; then
    export EXTRA_FLAGS_RELEASE="-O0"
    export PLAY_TEST=false
    export MP_TEST=false
    export WML_TEST_TIME=20
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        brew install ccache
        travis_wait ./projectfiles/Xcode/Fix_Xcode_Dependencies
    else
        travis_wait ./utils/travis/install_deps.sh
        export CXXFLAGS="-I/usr/local/opt/openssl/include $CFLAGS"
        export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
    fi
else
    docker build -t wesnoth-repo:16.04 -f docker/Dockerfile-travis .
fi
