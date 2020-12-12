#!/bin/bash

echo "Using linux:"
echo "BRANCH: $BRANCH"
echo "IMAGE: $IMAGE"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXX_STD: $CXX_STD"
echo "CFG: $CFG"
echo "LTO: $LTO"
echo "CACHE_DIR: $CACHE_DIR"

echo FROM wesnoth/wesnoth:"$IMAGE"-"$BRANCH" > utils/dockerbuilds/CI/Dockerfile-CI-"$IMAGE"-"$BRANCH"
echo COPY ./ /home/wesnoth-CI/ >> utils/dockerbuilds/CI/Dockerfile-CI-"$IMAGE"-"$BRANCH"
echo WORKDIR /home/wesnoth-CI >> utils/dockerbuilds/CI/Dockerfile-CI-"$IMAGE"-"$BRANCH"

docker build -t wesnoth-repo:"$IMAGE"-"$BRANCH" -f utils/dockerbuilds/CI/Dockerfile-CI-"$IMAGE"-"$BRANCH" .

docker run --cap-add=ALL --privileged \
    --volume ~/build-cache:"$CACHE_DIR" \
    --env BRANCH --env IMAGE --env NLS --env TOOL --env CC --env CXX \
    --env CXX_STD --env CFG --env LTO --env CACHE_DIR \
    wesnoth-repo:"$IMAGE"-"$BRANCH" ./.github/workflows/ci-scripts/docker.sh
