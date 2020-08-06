#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    scons translations build=release --debug=time nls=true jobs=2 || exit 1

    cd ./projectfiles/Xcode

    xcodebuild CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project "The Battle for Wesnoth.xcodeproj" -target "The Battle for Wesnoth" -configuration "$CFG"
    BUILD_RET=$?

    if [ "$UPLOAD_ID" != "" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
        hdiutil create -volname "Wesnoth_${CFG}" -fs 'HFS+' -srcfolder "build/$CFG" -ov -format UDBZ "Wesnoth_${CFG}.dmg"
        ./../../utils/travis/sftp "Wesnoth_${CFG}.dmg"
    fi

    ccache -s
    ccache -z

    exit $BUILD_RET
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    if [ ! -d "$HOME/vcpkg/installed" ]; then
        cd "$HOME"
        git clone --depth=1 https://github.com/microsoft/vcpkg.git vcpkg
        cd vcpkg
        cmd.exe //C bootstrap-vcpkg.bat
        cmd.exe //C 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat' amd64 '&&' vcpkg integrate install
        alias make="make -j4"
        ./vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-image[libjpeg-turbo]:x64-windows sdl2-mixer:x64-windows sdl2-ttf:x64-windows bzip2:x64-windows zlib:x64-windows pango:x64-windows cairo:x64-windows fontconfig:x64-windows libvorbis:x64-windows libogg:x64-windows boost-filesystem:x64-windows boost-iostreams:x64-windows boost-locale:x64-windows boost-random:x64-windows boost-regex:x64-windows boost-asio:x64-windows boost-program-options:x64-windows boost-system:x64-windows boost-thread:x64-windows boost-bimap:x64-windows boost-multi-array:x64-windows boost-ptr-container:x64-windows boost-logic:x64-windows boost-format:x64-windows &
        waitforpid=$!
        while kill -0 $waitforpid
        do
            echo "vcpkg install in progress with pid $waitforpid ..."
            sleep 60
        done
        rm -R downloads
        rm -R buildtrees
        echo "Built dependencies, exiting now to cache them. Please restart the job."
        exit 1
    else
        cd "$HOME/vcpkg"
        cmd.exe //C 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat' amd64 '&&' vcpkg integrate install
    fi

    cd $TRAVIS_BUILD_DIR
    cmd.exe //C 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat' amd64 '&&' MSBuild.exe projectfiles/$IMAGE/wesnoth.sln -p:Configuration=$CFG -p:Platform=Win64
    BUILD_RET=$?

    if [ "$UPLOAD_ID" != "" ] && [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
        ./utils/travis/sftp wesnoth.exe wesnothd.exe
        if [ "$CFG" == "Debug" ]; then
            ./utils/travis/sftp wesnoth.pdb wesnothd.pdb
        fi
    fi

    if [ "$BUILD_RET" != "0" ]; then
        sqlite3 "projectfiles/$IMAGE/$CFG/filehashes.sqlite" "update FILES set MD5 = OLD_MD5, OLD_MD5 = '-' where OLD_MD5 != '-'"
    else
        sqlite3 "projectfiles/$IMAGE/$CFG/filehashes.sqlite" "update FILES set OLD_MD5 = '-' where OLD_MD5 != '-'"
    fi

    if [ "$CFG" == "Release" ] && [ "$BUILD_RET" == "0" ]; then
        ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
        BUILD_RET=$?
    fi

    exit $BUILD_RET
else
# additional permissions required due to flatpak's use of bubblewrap
# enabling tty/using unbuffer causes po4a-translate to hang during the translation job
    if [ "$NLS" != "only" ]; then
        docker run --cap-add=ALL --privileged \
               --env SFTP_PASSWORD --env IMAGE --env TRAVIS_COMMIT --env BRANCH --env UPLOAD_ID --env TRAVIS_PULL_REQUEST --env NLS --env CC --env CXX --env TOOL \
               --env CXXSTD --env CFG --env WML_TESTS --env WML_TEST_TIME --env PLAY_TEST --env MP_TEST --env BOOST_TEST --env LTO --env SAN --env VALIDATE \
               --env TRAVIS_TAG \
               --volume "$HOME"/build-cache:/home/wesnoth-travis/build \
               --volume "$HOME"/flatpak-cache:/home/wesnoth-travis/flatpak-cache \
               --volume "$HOME"/.ccache:/root/.ccache \
               --tty wesnoth-repo:"$IMAGE"-"$BRANCH" \
               unbuffer ./utils/travis/docker_run.sh
    else
        docker run --cap-add=ALL --privileged \
               --env SFTP_PASSWORD --env IMAGE --env TRAVIS_COMMIT --env BRANCH --env UPLOAD_ID --env TRAVIS_PULL_REQUEST --env NLS --env CC --env CXX --env TOOL \
               --env CXXSTD --env CFG --env WML_TESTS --env WML_TEST_TIME --env PLAY_TEST --env MP_TEST --env BOOST_TEST --env LTO --env SAN --env VALIDATE \
               --env TRAVIS_TAG \
               --volume "$HOME"/build-cache:/home/wesnoth-travis/build \
               --volume "$HOME"/flatpak-cache:/home/wesnoth-travis/flatpak-cache \
               --volume "$HOME"/.ccache:/root/.ccache \
               wesnoth-repo:"$IMAGE"-"$BRANCH" \
               ./utils/travis/docker_run.sh
    fi
fi
