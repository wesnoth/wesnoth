#!/bin/bash

echo "Using docker:"
echo "BRANCH: $BRANCH"
echo "IMAGE: $IMAGE"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXX_STD: $CXX_STD"
echo "CFG: $CFG"
echo "LTO: $LTO"

# set the fake display for unit tests
export DISPLAY=:99.0
/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_99.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :99 -ac -screen 0 1024x768x24

red=$(tput setaf 1)
blue=$(tput bold; tput setaf 4)
reset=$(tput sgr0)
print() { printf '%s%s%s\n' "$blue" "$*" "$reset"; }
# print given message in red
error() { printf '%s%s%s\n' "$red" "$*" "$reset"; }
# print given message and exit
die() { error "$*"; exit 1; }

# print given message ($1) and execute given command; sets EXIT_VAL on failure
execute() {
    local message=$1; shift
    echo
    print " -~=+=~-  ${message//?/-}"
    print "Executing $message"
    print " -~=+=~-  ${message//?/-}"
    echo
    if "$@"; then
        : # success
    else
        EXIT_VAL=$?
        error '********** !FAILURE! **********'
        error '********** !FAILURE! **********'
        error '********** !FAILURE! **********'
        error '********** !FAILURE! **********'
        error '********** !FAILURE! **********'
        echo
        error "$message failed! ($*)"
        echo
    fi
}

# in order:
# check for proper indentation of WML
# check for trailing whitespace in hpp|cpp files
# check for trailing whitespace in lua files
checkindent() {
    ./utils/CI/fix_whitespace.sh
    git status
    (( $(git status --short | wc -l) == 0 ))
}

EXIT_VAL=-1

if [ "$NLS" == "only" ]; then
    export LANGUAGE=en_US.UTF-8
    export LANG=en_US.UTF-8
    export LC_ALL=en_US.UTF-8

    ./utils/CI/check_utf8.sh || exit 1
    ./utils/CI/utf8_bom_dog.sh || exit 1
    echo "Checked for invalid characters"

    cmake -DENABLE_NLS=true -DENABLE_GAME=false -DENABLE_SERVER=false -DENABLE_CAMPAIGN_SERVER=false -DENABLE_TESTS=false -DENABLE_POT_UPDATE_TARGET=TRUE .
    make update-po4a-man || exit 1
    echo "Ran cmake pdate-po4a-man"
    make update-po4a-manual || exit 1
    echo "Ran make update-po4a-manual"
    make pot-update || exit 1
    echo "Ran make pot-update"
    make mo-update || exit 1
    echo "Ran make mo-update"
    make clean

    scons translations build=release --debug=time nls=true jobs=2 || exit 1
    echo "Ran scons translations"
    scons pot-update || exit 1
    echo "Ran scons pot-update"
    scons update-po4a || exit 1
    echo "Ran scons update-po4a"
    scons manual || exit 1
    exit 0
elif [ "$IMAGE" == "flatpak" ]; then
# docker's --volume means the directory is on a separate filesystem
# flatpak-builder doesn't support this
# therefore manually move stuff between where flatpak needs it and where CI caching can see it
    rm -R .flatpak-builder/*
    jq '.modules[2].sources[0]={"type":"dir","path":"/home/wesnoth-CI"} | ."build-options".env.FLATPAK_BUILDER_N_JOBS="2"' packaging/flatpak/org.wesnoth.Wesnoth.json > utils/dockerbuilds/CI/org.wesnoth.Wesnoth.json
    flatpak-builder --force-clean --disable-rofiles-fuse wesnoth-app utils/dockerbuilds/CI/org.wesnoth.Wesnoth.json
    EXIT_VAL=$?
    exit $EXIT_VAL
else
    if [ "$TOOL" == "cmake" ]; then
        cmake -DCMAKE_BUILD_TYPE="$CFG" -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS="$NLS" \
              -DEXTRA_FLAGS_CONFIG="-pipe" -DENABLE_STRICT_COMPILATION=true -DENABLE_LTO="$LTO" -DLTO_JOBS=2 -DENABLE_MYSQL=true \
              -DFORCE_COLOR_OUTPUT=true -DCXX_STD="$CXX_STD" . || exit 1
        make conftests || exit 1
        make VERBOSE=1 -j2
        EXIT_VAL=$?
    else
        scons wesnoth wesnothd campaignd boost_unit_tests build="$CFG" \
            ctool="$CC" cxxtool="$CXX" cxx_std="$CXX_STD" \
            extra_flags_config="-pipe" strict=true forum_user_handler=true \
            nls="$NLS" enable_lto="$LTO" force_color=true jobs=2 --debug=time
        EXIT_VAL=$?
    fi
fi

if [ $EXIT_VAL != 0 ]; then
    exit $EXIT_VAL
fi

# rename debug executables to what the tests expect
if [ "$CFG" == "debug" ]; then
    mv wesnoth-debug wesnoth
    mv wesnothd-debug wesnothd
    mv campaignd-debug campaignd
    mv boost_unit_tests-debug boost_unit_tests
fi

execute "WML validation" ./utils/CI/schema_validation.sh
execute "Luacheck linting" luacheck .
execute "Whitespace and WML indentation check" checkindent
execute "Doxygen check" ./utils/CI/doxygen-check.sh
execute "WML tests" ./run_wml_tests -g -c -t 20
execute "Play tests" ./utils/CI/play_test_executor.sh
execute "MP tests" ./utils/CI/mp_test_executor.sh
execute "Boost unit tests" ./utils/CI/test_executor.sh

if [ -f "errors.log" ]; then
    echo
    error '***'
    error '*'
    error '* Errors reported in wml unit tests, here is errors.log...'
    error '*'
    error '***'
    cat errors.log
fi

exit $EXIT_VAL
