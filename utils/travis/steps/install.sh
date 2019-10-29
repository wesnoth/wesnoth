#!/bin/bash

date

export EXTRA_FLAGS_RELEASE=""
export WML_TESTS=true
export PLAY_TEST=true
export MP_TEST=true
export WML_TEST_TIME=15
export BOOST_TEST=true
export LTO=false

if [ "$OPT" = "-O0" ]; then
    export EXTRA_FLAGS_RELEASE="-O0"
    export PLAY_TEST=false
    export MP_TEST=false
    export WML_TEST_TIME=20
else
    # change to true to enable LTO on optimized, non-xcode builds
    export LTO=false
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache
        travis_wait ./projectfiles/Xcode/Fix_Xcode_Dependencies
    else
        travis_wait ./utils/travis/install_deps.sh
    fi
else
    docker build -t wesnoth-repo:"$LTS"-"$BRANCH" -f docker/Dockerfile-travis-"$LTS"-"$BRANCH" .
fi
