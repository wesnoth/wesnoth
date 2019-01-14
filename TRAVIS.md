To create the base image with Wesnoth's dependencies:

  docker build -t wesnoth:16.04 -f docker/Dockerfile-base docker

To create the image that travis will run during a build:

  docker build -t wesnoth-repo:16.04 -f docker/Dockerfile-travis .

To push a new base image to Docker Hub

  docker login
  Username: wesnoth

  docker tag <Image ID from `docker images`> wesnoth/wesnoth:16.04
  docker push wesnoth/wesnoth:16.04
