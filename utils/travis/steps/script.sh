#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    export PATH="/usr/local/opt/ccache/libexec:$PWD/utils/travis:$PATH"
    export CC=ccache-clang
    export CXX=ccache-clang++

    cd ./projectfiles/Xcode

    export CCACHE_MAXSIZE=500M
    export CCACHE_COMPILERCHECK=content

    xcodebuild GCC_GENERATE_DEBUGGING_SYMBOLS=NO -project "The Battle for Wesnoth.xcodeproj" -target "The Battle for Wesnoth" -configuration "$OPT"

    BUILD_RET=$?

    ccache -s
    ccache -z

    exit $BUILD_RET
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    powershell "MSBuild.exe projectfiles/VC14/wesnoth.sln -p:PlatformToolset=v141 -p:Configuration=$OPT"
    BUILD_RET=$?

    if [ "$UPLOAD_ID" != "" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
        ./utils/travis/sftp wesnoth.exe wesnothd.exe
    fi

    if [ "$BUILD_RET" != "0" ]; then
        sqlite3 "projectfiles/VC14/$OPT/filehashes.sqlite" "update FILES set MD5 = OLD_MD5, OLD_MD5 = '-' where OLD_MD5 != '-'"
    else
        sqlite3 "projectfiles/VC14/$OPT/filehashes.sqlite" "update FILES set OLD_MD5 = '-' where OLD_MD5 != '-'"
    fi

    if [ "$OPT" == "Release" ] && [ "$BUILD_RET" == "0" ]; then
        ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
        BUILD_RET=$?
    fi

    exit $BUILD_RET
else
# additional permissions required due to flatpak's use of bubblewrap
    docker run --cap-add=ALL --privileged \
               --env SFTP_PASSWORD --env LTS --env TRAVIS_COMMIT --env BRANCH --env UPLOAD_ID --env TRAVIS_PULL_REQUEST --env NLS --env CC --env CXX --env TOOL \
               --env CXXSTD --env OPT --env WML_TESTS --env WML_TEST_TIME --env PLAY_TEST --env MP_TEST --env BOOST_TEST --env LTO --env SAN --env VALIDATE \
               --env TRAVIS_TAG \
               --volume "$HOME"/build-cache:/home/wesnoth-travis/build \
               --volume "$HOME"/flatpak-cache:/home/wesnoth-travis/flatpak-cache \
               --volume "$HOME"/.ccache:/root/.ccache \
               --tty wesnoth-repo:"$LTS"-"$BRANCH" \
               unbuffer ./utils/travis/docker_run.sh
fi
