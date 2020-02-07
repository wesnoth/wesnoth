#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        export PATH="/usr/local/opt/ccache/libexec:$PWD/utils/travis:$PATH"
        export CC=ccache-clang
        export CXX=ccache-clang++

        cd ./projectfiles/Xcode

        export CCACHE_MAXSIZE=200M
        export CCACHE_COMPILERCHECK=content

        xcodebuild GCC_GENERATE_DEBUGGING_SYMBOLS=NO -project "The Battle for Wesnoth.xcodeproj" -target "The Battle for Wesnoth" -configuration Debug

        BUILD_RET=$?

        ccache -s
        ccache -z

        exit $BUILD_RET
    else
        ./utils/travis/check_utf8.sh || exit 1
        ./utils/travis/utf8_bom_dog.sh || exit 1

        "$CXX" --version
        if [ "$TOOL" = "scons" ]; then
            ln -s $HOME/build-cache/ build
            export PKG_CONFIG_PATH="/usr/local/opt/libffi/lib/pkgconfig"

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
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    powershell "MSBuild.exe projectfiles/VC14/wesnoth.sln -p:PlatformToolset=v141 -p:Configuration=$OPT"
    BUILD_RET=$?

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
               --volume "$HOME"/build-cache:/home/wesnoth-travis/build \
               --volume "$HOME"/flatpak-cache:/home/wesnoth-travis/flatpak-cache \
               --volume "$HOME"/.ccache:/root/.ccache \
               --tty wesnoth-repo:"$LTS"-"$BRANCH" \
               unbuffer ./utils/travis/docker_run.sh
fi
