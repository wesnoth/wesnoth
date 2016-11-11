#!/bin/bash

if [ "$CXXSTD" == "1y" ]; then
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y;
fi

travis_wait sudo apt-get update -qq
travis_wait sudo apt-get install -qq libboost-filesystem-dev libboost-iostreams-dev libboost-random-dev libboost-program-options-dev libboost-regex-dev libboost-system-dev libboost-test-dev libboost-locale-dev libboost-thread-dev libcairo2-dev libfribidi-dev libpango1.0-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libvorbis-dev gdb moreutils xvfb

if [ "$USE_CMAKE" = true  ]; then
	travis_wait sudo apt-get install -qq cmake;
else
	travis_wait sudo apt-get install -qq scons;
fi

if [ "$CXXSTD" == "1y" ]; then
	sudo apt-get install -qq g++-5;
	export CXX=g++-5;
	export CC=gcc-5;
fi
