#!/bin/bash
# Install isutf8 program (from package "moreutils" at least in linux mint)
# in order to use this script
#
# This script assumes that the current working directory is the root of the
# wesnoth repository.
set -e
find src/ -type f -print0 | xargs -0 isutf8 --
find data/ -not -name "*.png" -not -name "*.ogg" -not -name "*.jpg" -not -name "*.wav" -not -name "*.gif" -not -name "*.xcf" -type f -print0 | xargs -0 isutf8 --
find po/ -type f -print0 | xargs -0 isutf8 --

