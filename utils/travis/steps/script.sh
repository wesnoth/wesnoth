#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        export PATH="/usr/local/opt/ccache/libexec:$PWD/utils/travis:$PATH"
        export CC=ccache-clang
        export CXX=ccache-clang++

        cd ./projectfiles/Xcode

        export CCACHE_MAXSIZE=2G
        export CCACHE_COMPILERCHECK=content

        xcodebuild -project Wesnoth.xcodeproj -target Wesnoth
        
        BUILD_RET=$?
        
        ccache -s
        ccache -z
        
        exit $BUILD_RET
    else
        ln -s build-cache/ build
        ./utils/travis/check_utf8.sh
        ./utils/travis/utf8_bom_dog.sh
        "$CXX" --version
        scons wesnoth wesnothd campaignd boost_unit_tests build=release ctool="$CC" cxxtool="$CXX" --debug=time extra_flags_config="-pipe" extra_flags_release="$EXTRA_FLAGS_RELEASE" strict=true cxx_std="$CXXSTD" nls="$NLS" jobs=2 enable_lto=false
    fi
else
    export DISPLAY=:99.0
    /sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_99.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :99 -ac -screen 0 1024x768x24

    docker run -v "$PWD"/build-cache:/home/wesnoth-travis/build -v "$HOME"/.ccache:/root/.ccache wesnoth-repo:16.04 bash -c './utils/travis/docker_run.sh "$@"' bash "$NLS" "$TOOL" "$CC" "$CXX" "$CXXSTD" "$EXTRA_FLAGS_RELEASE" "$WML_TESTS" "$WML_TEST_TIME" "$PLAY_TEST" "$MP_TEST" "$BOOST_TEST"
fi
