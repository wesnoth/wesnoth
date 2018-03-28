#!/bin/bash

date

export WML_TESTS=true
export PLAY_TEST=true
export MP_TEST=true
export WML_TEST_TIME=15
export BOOST_TEST=true

if [ "$OPT" = "-O0" ]; then
    export PLAY_TEST=false
    export MP_TEST=false
    export WML_TEST_TIME=20
fi

if [ "$LTO" == "" ]; then
    export LTO=false
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        brew install ccache
        travis_wait ./projectfiles/Xcode/Fix_Xcode_Dependencies
    else
        brew install scons cairo pango moreutils sdl2_image sdl2_mixer openssl glew
        export CXXFLAGS="-I/usr/local/opt/openssl/include $CFLAGS"
        export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
    fi
else
    if [ "$NLS" != "true" ]; then
        echo "po/" >> .dockerignore
    fi

    echo "FROM wesnoth/wesnoth:$LTS-$BRANCH" > docker/Dockerfile-travis-"$LTS"-"$BRANCH"
    echo "COPY ./ /home/wesnoth-travis/" >> docker/Dockerfile-travis-"$LTS"-"$BRANCH"
    echo "WORKDIR /home/wesnoth-travis" >> docker/Dockerfile-travis-"$LTS"-"$BRANCH"

    docker build -t wesnoth-repo:"$LTS"-"$BRANCH" -f docker/Dockerfile-travis-"$LTS"-"$BRANCH" .
fi
