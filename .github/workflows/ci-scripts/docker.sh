#!/bin/bash

scons wesnoth wesnothd campaignd boost_unit_tests build=release \
    ctool=gcc cxxtool=g++ cxx_std=17 \
    extra_flags_config="-pipe" strict=true forum_user_handler=true \
    nls=false enable_lto=true sanitize="" jobs=2 --debug=time

ls -Al
