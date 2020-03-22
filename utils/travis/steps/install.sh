#!/bin/bash

date

export WML_TESTS=true
export PLAY_TEST=true
export MP_TEST=true
export WML_TEST_TIME=15
export BOOST_TEST=true

if [ "$CFG" = "debug" ] || [ "$CFG" = "Debug" ]; then
    export PLAY_TEST=false
    export MP_TEST=false
    export WML_TEST_TIME=20
fi

if [ "$LTO" == "" ]; then
    export LTO=false
fi

if [ "$UPLOAD" == "true" ]; then
    if [ "$TRAVIS_OS_NAME" != "osx" ]; then
        export UPLOAD_ID="${TRAVIS_OS_NAME}-${IMAGE}-${TOOL}-${CFG}"
    else
        export UPLOAD_ID="${TRAVIS_OS_NAME}-${TOOL}-${CFG}"
    fi
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache scons
    yes | pip3 install paramiko
    export PATH="/usr/local/opt/gettext/bin:/usr/local/opt/ccache/libexec:$PWD/utils/travis:$PATH"
    export CC=ccache-clang
    export CXX=ccache-clang++
    export CCACHE_MAXSIZE=3000M
    export CCACHE_COMPILERCHECK=content
    travis_wait ./projectfiles/Xcode/Fix_Xcode_Dependencies
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    if [ "$IMAGE" == "VC16" ]; then
        choco install visualstudio2019community --package-parameters "--includeRecommended --includeOptional"
        choco install visualstudio2019-workload-vctools
    fi

    if [[ "$IMAGE" == "VC14" || ( "$IMAGE" == "VC16" && -d "$HOME/vcpkg/installed" ) ]]; then
        choco install sqlite
        choco install python --version=3.6.8
        cd /c/Python36
        ln -s python.exe python3.exe
        export PATH="/c/Python36:"$PATH":/c/Python36/Scripts/"
        yes | pip3 install paramiko
        if [ "$(which python3)" == "" ] || [ "$(which sqlite3)" == "" ]; then
            echo "Failed to retrieve dependencies!"
            exit 1
        else
            echo "Dependencies retrieved and installed!"
        fi

        cd $TRAVIS_BUILD_DIR
        ./utils/travis/windows-file-hasher.sh "projectfiles/$IMAGE/$CFG/filehashes.sqlite"
    fi
else
    # if not doing translations, save a bit of time by not copying them into the docker image
    # otherwise, if this is the mingw job, the .git directory is needed for running the git archive command
    if [ "$NLS" == "false" ]; then
        echo "po/" >> .dockerignore
    elif [ "$IMAGE" == "mingw" ]; then
        rm .dockerignore
    fi

    echo "FROM wesnoth/wesnoth:$IMAGE-$BRANCH" > utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
    echo "COPY ./ /home/wesnoth-travis/" >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"
    echo "WORKDIR /home/wesnoth-travis" >> utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH"

    docker build -t wesnoth-repo:"$IMAGE"-"$BRANCH" -f utils/dockerbuilds/travis/Dockerfile-travis-"$IMAGE"-"$BRANCH" .
fi
