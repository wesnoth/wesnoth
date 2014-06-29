#!/bin/bash
# Install isutf8 program (from package "moreutils" at least in linux mint)
# in order to use this script
find src/ -type f -exec isutf8 {} \;
find data/ -not -name "*.png" -not -name "*.ogg" -not -name "*.jpg" -not -name "*.wav" -not -name "*.gif" -not -name "*.xcf" -type f -exec isutf8 {} \;
