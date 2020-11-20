#!/bin/bash

scons wesnoth wesnothd campaignd boost_unit_tests build=release \
    ctool=gcc cxxtool=g++ cxx_std=17 \
    extra_flags_config="-pipe" strict=true forum_user_handler=true \
    nls=false enable_lto=true sanitize="" jobs=2 --debug=time

ls -Al

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

checkindent() {
    make -C data/tools reindent &&
    git diff-index --quiet HEAD
}

execute "WML validation" ./utils/travis/schema_validation.sh
execute "WML indentation check" checkindent
execute "WML tests" ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
execute "Play tests" ./utils/travis/play_test_executor.sh
execute "Boost unit tests" ./utils/travis/test_executor.sh

if [ -f "errors.log" ]; then
    error $'\n*** \n*\n* Errors reported in wml unit tests, here is errors.log...\n*\n*** \n'
    cat errors.log
fi
