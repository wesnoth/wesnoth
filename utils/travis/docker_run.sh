#!/bin/bash

red=$(tput setaf 1)
reset=$(tput sgr0)
# print given message in red
error() { printf '%s%s%s\n' "$red" "$*" "$reset"; }
# print given message and exit
die() { error "$*"; exit 1; }

# set the fake display for unit tests
export DISPLAY=:99.0
/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_99.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :99 -ac -screen 0 1024x768x24

if [ "$CFG" = "debug" ] || [ "$CFG" = "Debug" ]; then
    STRICT="true"
    build_timeout=35
else
    STRICT="false"
    build_timeout=40
fi

if [ "$CXXSTD" == "17" ]; then
    STRICT="true"
fi

echo "Using configuration:"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXSTD: $CXXSTD"
echo "CFG: $CFG"
echo "WML_TESTS: $WML_TESTS"
echo "WML_TEST_TIME: $WML_TEST_TIME"
echo "PLAY_TEST: $PLAY_TEST"
echo "MP_TEST: $MP_TEST"
echo "BOOST_TEST: $BOOST_TEST"
echo "LTO: $LTO"
echo "SAN: $SAN"
echo "VALIDATE: $VALIDATE"
echo "IMAGE: $IMAGE"
echo "TRAVIS_COMMIT: $TRAVIS_COMMIT"
echo "BRANCH: $BRANCH"
echo "UPLOAD_ID: $UPLOAD_ID"
echo "TRAVIS_PULL_REQUEST: $TRAVIS_PULL_REQUEST"
echo "TRAVIS_TAG: $TRAVIS_TAG"

echo "STRICT: $STRICT"
echo "build_timeout(mins): $build_timeout"

$CXX --version

if [ "$NLS" == "only" ]; then
    export LANGUAGE=en_US.UTF-8
    export LANG=en_US.UTF-8
    export LC_ALL=en_US.UTF-8

    ./utils/travis/check_utf8.sh || exit 1
    ./utils/travis/utf8_bom_dog.sh || exit 1

    cmake -DENABLE_NLS=true -DENABLE_GAME=false -DENABLE_SERVER=false -DENABLE_CAMPAIGN_SERVER=false -DENABLE_TESTS=false -DENABLE_POT_UPDATE_TARGET=TRUE
    make update-po4a-man || exit 1
    make update-po4a-manual || exit 1
    make pot-update || exit 1
    make mo-update || exit 1
    make clean

    scons translations build=release --debug=time nls=true jobs=2 || exit 1

    scons pot-update update-po4a manual
elif [ "$IMAGE" == "flatpak" ]; then
# docker's --volume means the directory is on a separate filesystem
# flatpak-builder doesn't support this
# therefore manually move stuff between where flatpak needs it and where travis' caching can see it

    rm -R .flatpak-builder/*
    cp -R flatpak-cache/. .flatpak-builder/
    jq '.modules[2].sources[0]={"type":"dir","path":"/home/wesnoth-travis"} | ."build-options".env.FLATPAK_BUILDER_N_JOBS="2"' packaging/flatpak/org.wesnoth.Wesnoth.json > utils/dockerbuilds/travis/org.wesnoth.Wesnoth.json
    flatpak-builder --ccache --force-clean --disable-rofiles-fuse wesnoth-app utils/dockerbuilds/travis/org.wesnoth.Wesnoth.json
    BUILD_RET=$?
    rm -R flatpak-cache/*
    cp -R .flatpak-builder/. flatpak-cache/
    chmod -R 777 flatpak-cache/
    exit $BUILD_RET
elif [ "$IMAGE" == "mingw" ]; then
    scons wesnoth wesnothd build="$CFG" \
        cxx_std=$CXXSTD strict="$STRICT" \
        nls=false enable_lto="$LTO" sanitize="$SAN" jobs=2 --debug=time \
        arch=x86-64 prefix=/windows/mingw64 gtkdir=/windows/mingw64 host=x86_64-w64-mingw32 || exit 1

    if [ "$UPLOAD_ID" != "" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
        ./utils/travis/sftp wesnoth.exe wesnothd.exe
    fi

    if [ "$TRAVIS_TAG" != "" ]; then
        echo "Creating Windows installer for tag: $TRAVIS_TAG"
        scons translations build=release --debug=time nls=true jobs=2 || exit 1
        python3 ./utils/dockerbuilds/mingw/get_dlls.py || exit 1
        scons windows-installer arch=x86-64 prefix=/windows/mingw64 gtkdir=/windows/mingw64 host=x86_64-w64-mingw32 || exit 1
        ./utils/travis/sftp "$(find . -type f -regex '.*win64.*')"
        echo "Creating .tar.bz2 for tag: $TRAVIS_TAG"
        git archive --format=tar --prefix="wesnoth-$TRAVIS_TAG/" $TRAVIS_TAG > "wesnoth-$TRAVIS_TAG.tar" || exit 1
        bzip2 wesnoth-$TRAVIS_TAG.tar || exit 1
        ./utils/travis/sftp wesnoth-$TRAVIS_TAG.tar.bz2 || exit 1
    fi
elif [ "$IMAGE" == "steamrt" ]; then
    scons ctool=$CC cxxtool=$CXX boostdir=/usr/local/include boostlibdir=/usr/local/lib extra_flags_config=-lrt \
        cxx_std=$CXXSTD strict="$STRICT" nls="$NLS" enable_lto="$LTO" sanitize="$SAN" jobs=2 --debug=time \
        build="$CFG"
else
    SECONDS=0

    if [ "$TOOL" == "cmake" ]; then
        export CCACHE_MAXSIZE=3000M
        export CCACHE_COMPILERCHECK=content

        cmake -DCMAKE_BUILD_TYPE="$CFG" -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS="$NLS" \
              -DEXTRA_FLAGS_CONFIG="-pipe" -DENABLE_STRICT_COMPILATION="$STRICT" -DENABLE_LTO="$LTO" -DLTO_JOBS=2 -DENABLE_MYSQL=true -DSANITIZE="$SAN" \
              -DCXX_STD="$CXXSTD" -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
              make VERBOSE=1 -j2
        BUILD_RET=$?

        ccache -s
        ccache -z
# remove once 1804 isn't used anymore
    elif [ "$IMAGE" == "1804" ]; then
        scons wesnoth wesnothd campaignd boost_unit_tests build="$CFG" \
              ctool=$CC cxxtool=$CXX cxx_std=$CXXSTD \
              extra_flags_config="-pipe" strict="$STRICT" forum_user_handler=false \
              nls="$NLS" enable_lto="$LTO" sanitize="$SAN" jobs=2 --debug=time
        BUILD_RET=$?
    else
        scons wesnoth wesnothd campaignd boost_unit_tests build="$CFG" \
              ctool=$CC cxxtool=$CXX cxx_std=$CXXSTD \
              extra_flags_config="-pipe" strict="$STRICT" forum_user_handler=true \
              nls="$NLS" enable_lto="$LTO" sanitize="$SAN" jobs=2 --debug=time
        BUILD_RET=$?
    fi

    if [ $BUILD_RET != 0 ]; then
        exit $BUILD_RET
    fi

# rename debug executables to what the tests expect
    if [ "$CFG" == "debug" ]; then
        mv wesnoth-debug wesnoth
        mv wesnothd-debug wesnothd
        mv campaignd-debug campaignd
        mv boost_unit_tests-debug boost_unit_tests
    fi

    if [ "$UPLOAD_ID" != "" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
        bzip2 -9 -k wesnoth
        bzip2 -9 -k wesnothd
        ./utils/travis/sftp wesnoth.bz2 wesnothd.bz2
    fi

    if (( SECONDS > 60*build_timeout )); then
        die "Insufficient time remaining to execute unit tests. Exiting now to allow caching to occur. Please restart the job."
    fi

# needed since bash returns the exit code of the final command executed, so a failure needs to be returned if any unit tests fail
    EXIT_VAL=0

# print given message ($1) and execute given command; sets EXIT_VAL on failure
    execute() {
        local message=$1; shift
        printf 'Executing %s\n' "$message"
        if "$@"; then
            : # success
        else
            EXIT_VAL=$?
            error "$message failed! ($*)"
        fi
    }

    if [ "$VALIDATE" = "true" ]; then
        execute "WML validation" ./utils/travis/schema_validation.sh
    fi

    if [ "$WML_TESTS" = "true" ]; then
        execute "WML tests" ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
    fi

    if [ "$PLAY_TEST" = "true" ]; then
        execute "Play tests" ./utils/travis/play_test_executor.sh
    fi

    if [ "$MP_TEST" = "true" ]; then
        execute "MP tests" ./utils/travis/mp_test_executor.sh
    fi

    if [ "$BOOST_TEST" = "true" ]; then
        execute "Boost unit tests" ./utils/travis/test_executor.sh
    fi

    if [ -f "errors.log" ]; then
        error $'\n*** \n*\n* Errors reported in wml unit tests, here is errors.log...\n*\n*** \n'
        cat errors.log
    fi

    exit $EXIT_VAL
fi
