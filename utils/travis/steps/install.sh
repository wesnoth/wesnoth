#!/bin/bash

date

export WML_TESTS=true
export PLAY_TEST=true
export MP_TEST=true
export WML_TEST_TIME=15
export BOOST_TEST=true

if [ "$OPT" = "-O0" ]; then
    export PLAY_TEST=false
    export MP_TEST=false
    export WML_TEST_TIME=20
fi

if [ "$LTO" == "" ]; then
    export LTO=false
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$TOOL" = "xcodebuild" ]; then
        HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache
        travis_wait ./projectfiles/Xcode/Fix_Xcode_Dependencies
    else
        HOMEBREW_NO_AUTO_UPDATE=1 brew install scons cairo pango moreutils sdl2_image sdl2_ttf sdl2_mixer glew ccache
        HOMEBREW_NO_AUTO_UPDATE=1 brew reinstall libffi
    fi
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    start=`pwd`
    choco install sqlite
    choco install python --version=3.6.8
    cd /c/Python36
    ln -s python.exe python3.exe
    cd $start
    cd ..
    wget https://github.com/aquileia/external/archive/VC15.zip -O VC15.zip
    7z x VC15.zip
    mv external-VC15 external
    cd $start
    export PATH="/c/Python36:"$PATH":/c/Program Files (x86)/Microsoft Visual Studio/2017/BuildTools/MSBuild/15.0/Bin/amd64:$start/../external/dll"
    if [ "$(which python3)" == "" ] || [ "$(which sqlite3)" == "" ] || [ ! -d "../external" ]; then
      echo "Failed to retrieve dependencies!"
      exit 1
    else
      echo "Dependencies retrieved and installed!"
    fi

    ./utils/travis/windows-file-hasher.sh "projectfiles/VC14/$OPT/filehashes.sqlite"
else
    if [ "$NLS" != "true" ]; then
        echo "po/" >> .dockerignore
    fi

    echo "FROM wesnoth/wesnoth:$LTS-$BRANCH" > utils/dockerbuilds/travis/Dockerfile-travis-"$LTS"-"$BRANCH"
    echo "COPY ./ /home/wesnoth-travis/" >> utils/dockerbuilds/travis/Dockerfile-travis-"$LTS"-"$BRANCH"
    echo "WORKDIR /home/wesnoth-travis" >> utils/dockerbuilds/travis/Dockerfile-travis-"$LTS"-"$BRANCH"

    docker build -t wesnoth-repo:"$LTS"-"$BRANCH" -f utils/dockerbuilds/travis/Dockerfile-travis-"$LTS"-"$BRANCH" .
fi
