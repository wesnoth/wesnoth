#!/bin/bash

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

hdiutil create -volname "Wesnoth_${CFG}" -fs 'HFS+' -srcfolder "build/$CFG" -ov -format UDBZ "Wesnoth_${CFG}.dmg"

ccache -s
ccache -z

if [ $EXIT_VAL == 0 ] && [ "$CFG" == "Release" ]; then
		cd ../..
		./run_wml_tests -g -c -t 20 -p "./projectfiles/Xcode/build/$CFG/The Battle for Wesnoth.app/Contents/MacOS/The Battle for Wesnoth"
		EXIT_VAL=$?
fi

exit $EXIT_VAL
