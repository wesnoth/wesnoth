#!/bin/bash

HOMEBREW_NO_AUTO_UPDATE=1 brew install scons
export PATH="/usr/local/opt/gettext/bin:$PWD/utils/CI:$PATH"
./projectfiles/Xcode/Fix_Xcode_Dependencies

scons translations build=release --debug=time nls=true jobs=2 || exit 1

cd ./projectfiles/Xcode

xcodebuild ARCHS=x86_64 -project "The Battle for Wesnoth.xcodeproj" -target "The Battle for Wesnoth" -target "unit_tests" -configuration "$CFG"
EXIT_VAL=$?

hdiutil create -volname "Wesnoth_${CFG}" -fs 'HFS+' -srcfolder "build/$CFG" -ov -format UDBZ "Wesnoth_${CFG}.dmg"

if [ $EXIT_VAL == 0 ] && [ "$CFG" == "Release" ]; then
		cd ../..
		./run_wml_tests -g -c -t 30 -p "./projectfiles/Xcode/build/$CFG/The Battle for Wesnoth.app/Contents/MacOS/The Battle for Wesnoth"
		EXIT_VAL=$?
fi

if [ "$CFG" == "Debug" ]; then
	if [ $? == 0 ] && [ "$CFG" == "Debug" ]; then
		echo "Entering 'build'" ; cd build ; ls
		echo "Entering '$CFG'" ; cd $CFG ; ls
		echo "Entering '.app'" ; cd "The Battle for Wesnoth.app" ; ls
		echo "Entering 'Content'" ; cd Content ; ls
		echo "Entering 'Resources'" ; cd Resources ; ls
		#cd "build/$CFG/The Battle for Wesnoth.app/Content/Resources"
		../../../unit_tests --color_output --log_level=test_suite
		EXIT_VAL=$?
	fi
fi

exit $EXIT_VAL
