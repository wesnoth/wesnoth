#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        export PATH="/usr/local/opt/ccache/libexec:$PWD/utils/travis:$PATH"
        export CC=ccache-clang
        export CXX=ccache-clang++

        cd ./projectfiles/Xcode

        export CCACHE_MAXSIZE=200M
        export CCACHE_COMPILERCHECK=content

        xcodebuild GCC_GENERATE_DEBUGGING_SYMBOLS=NO -project Wesnoth.xcodeproj -target Wesnoth -configuration Debug

        BUILD_RET=$?

        ccache -s
        ccache -z

        exit $BUILD_RET
    else
        ./utils/travis/check_utf8.sh || exit 1
        ./utils/travis/utf8_bom_dog.sh || exit 1

        "$CXX" --version
        if [ "$TOOL" == "scons" ]; then
            ln -s $HOME/build-cache/ build

            scons wesnoth wesnothd campaignd boost_unit_tests build=release \
                  ctool="$CC" cxxtool="$CXX" cxx_std="$CXXSTD" \
                  extra_flags_config="-pipe" opt="$OPT" strict=true \
                  nls="$NLS" enable_lto="$LTO" jobs=2 --debug=time
        else
            cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS=false \
                  -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCXX_STD="$CXXSTD"\
                  -DEXTRA_FLAGS_CONFIG="-pipe" -DOPT="$OPT" -DENABLE_STRICT_COMPILATION="$STRICT" && \
                  make VERBOSE=1 -j2
        fi
    fi
else
    docker run --volume "$HOME"/build-cache:/home/wesnoth-travis/build \
               --volume "$HOME"/.ccache:/root/.ccache \
               wesnoth-repo:"$LTS"-"$BRANCH" \
               bash -c './utils/travis/docker_run.sh "$@"' \
               bash "$NLS" "$TOOL" "$CC" "$CXX" "$CXXSTD" "$OPT" "$WML_TESTS" "$WML_TEST_TIME" "$PLAY_TEST" "$MP_TEST" "$BOOST_TEST" "$LTO" "$SAN"
fi
