Intended usage:
* modify setup-toolchains.py with correct path to your ndk and the desired api level
* run ./build-android-deps.sh

Builds will be done in /tmp/android-build. Resulting builds will be in /tmp/android-prefix. They can be passed to wesnoth's scons script with prefix option, e.g. prefix=/tmp/android-prefix/armeabi-v7a
