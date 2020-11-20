#!/bin/bash

BRANCH="$1"
IMAGE="$2"
NLS="$3"
TOOL="$4"
CC="$5"
CXX="$6"
CXXSTD="$7"
CFG="$8"
LTO="$9"

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

date

echo FROM wesnoth/wesnoth:"$IMAGE"-"$BRANCH" > utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
echo COPY ./ /home/wesnoth-travis/ >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
echo WORKDIR /home/wesnoth-travis >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"

docker build -t wesnoth-repo:"$IMAGE"-"$BRANCH" -f utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH" .

docker run --cap-add=ALL --privileged \
    --volume ~/build-cache:/home/wesnoth-travis/build \
    --env BRANCH --env IMAGE --env NLS --env TOOL --env CC --env CXX \
    --env CXX_STD --env CFG --env LTO \
    wesnoth-repo:"$IMAGE"-"$BRANCH" ./.github/workflows/ci-scripts/docker.sh
