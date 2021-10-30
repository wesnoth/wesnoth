#!/bin/bash

echo ~
echo $PWD

HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache scons
export PATH="/usr/local/opt/gettext/bin:/usr/local/opt/ccache/libexec:$PWD/utils/CI:$PATH"
export CC=ccache-clang
export CXX=ccache-clang++
export CCACHE_MAXSIZE=3000M
export CCACHE_COMPILERCHECK=content
export CCACHE_DIR="$CACHE_DIR"
./projectfiles/Xcode/Fix_Xcode_Dependencies

scons translations build=release --debug=time nls=true jobs=2 || exit 1

cd ./projectfiles/Xcode

xcodebuild CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO -project "The Battle for Wesnoth.xcodeproj" -target "The Battle for Wesnoth" -configuration "$CFG"
EXIT_VAL=$?

ccache -s
ccache -z

hdiutil create -volname "Wesnoth_${CFG}" -fs 'HFS+' -srcfolder "build/$CFG" -ov -format UDBZ "Wesnoth_${CFG}.dmg"

exit $EXIT_VAL
