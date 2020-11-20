#!/bin/bash

date

echo FROM wesnoth/wesnoth:2004-master > utils/dockerbuilds/travis/Dockerfile-travis-2004-master
echo COPY ./ /home/wesnoth-travis/ >> utils/dockerbuilds/travis/Dockerfile-travis-2004-master
echo WORKDIR /home/wesnoth-travis >> utils/dockerbuilds/travis/Dockerfile-travis-2004-master

docker build -t wesnoth-repo:2004-master -f utils/dockerbuilds/travis/Dockerfile-travis-2004-master .

docker image ls


