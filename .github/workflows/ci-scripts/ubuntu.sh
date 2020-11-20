#!/bin/bash

export BRANCH="$1"
export IMAGE="$2"
export NLS="$3"
export TOOL="$4"
export CC="$5"
export CXX="$6"
export CXX_STD="$7"
export CFG="$8"
export LTO="$9"
export CACHE_DIR="/home/wesnoth-travis/build"

echo "Using linux:"
echo "BRANCH: $BRANCH"
echo "IMAGE: $IMAGE"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXSTD: $CXXSTD"
echo "CFG: $CFG"
echo "LTO: $LTO"
echo "CACHE_DIR: $CACHE_DIR"

echo FROM wesnoth/wesnoth:"$IMAGE"-"$BRANCH" > utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
echo COPY ./ /home/wesnoth-travis/ >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
echo WORKDIR /home/wesnoth-travis >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"

docker build -t wesnoth-repo:"$IMAGE"-"$BRANCH" -f utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH" .

docker run --cap-add=ALL --privileged \
    --volume ~/build-cache:"$CACHE_DIR" \
    --env BRANCH --env IMAGE --env NLS --env TOOL --env CC --env CXX \
    --env CXX_STD --env CFG --env LTO --env CACHE_DIR \
    wesnoth-repo:"$IMAGE"-"$BRANCH" ./.github/workflows/ci-scripts/docker.sh
