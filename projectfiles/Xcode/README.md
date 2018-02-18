# Building with Xcode

## Types of builds and requirements
Currently there are two types of builds:

* **local builds** - useful for regular builds which should work mainly on actual system and hardware. They are easiest to reproduce.
* **package builds** - useful (only) for packagers. These builds are trying to be as compatible as possible. They depends on `MacCompileStuff` which contains precompiled universal but sometimes outdated libraries.

### Requirements for local builds
 * Xcode 4.6.3 or higher...
 * Mac OS X 10.7.5 or higher...
 * `scons` and `gettext` (if you want to compile translations)
 * Homebrew from https://brew.sh/

### Requirements for package builds
 * Xcode 4.6.3 or higher...
 * Mac OS X 10.7.5 or higher...
 * `scons` and `gettext` (if you want to compile translations)
 * The Headers and lib folders, which can be found in the newest zip here:  
   https://sourceforge.net/downloads/wesnoth/unofficial/Mac%20Compile%20Stuff/

## Targets
* **Wesnoth**:
Builds the actual game, depends on wesnothd. If you don't want to build wesnothd, get info on the Wesnoth target, go to the General tab, and remove wesnothd from its dependencies.

* **wesnothd**:
Builds the multiplayer server. The MP server is needed for hosting a local MP server, not for connecting to the official one.

* **unit_tests**:
Builds the unit tests. This is mainly useful when trying to fix a failing Travis build.

* **campaignd**:
Builds the addons server. This doesn't work currently.

* **liblua**:
Builds the lua library for Wesnoth.


## Configurations
* **Release**:
Builds for maximum (runtime) speed and compatibility; it builds for 32-bit and 64-bit, with the latest SDK, but targets 10.7. You do not however need 10.7 SDK to build it. This is what's used for official releases.

* **Debug**:
Builds for maximum compiling speed, and uses the current OS as the SDK. If you just want to compile for testing things yourself, this is the way to go.

## Step by step building
1. Obtain sources from github using `git clone https://github.com/wesnoth/wesnoth` or use your favourite git program
2. Use `Finder` to navigate into `wesnoth/projectfiles/Xcode` . And now, there will be 2 options.

    3. If you are going to build **local build** you can just double click on `Fix_Xcode_Dependencies` script and wait until it will be done.
    4. If you are going to build **package build** you must download `MacCompileStuff` and extract it here.
5. (Optional) Compile translations as it is described in **Translations** section.
6. Now you can open `Wesnoth.xcodeproj` file in Xcode.

## Translations
To compile translations you need `gettext-tools` and `scons`. You can obtain these tools using `brew install gettext scons`. In the Terminal, `cd` to the Wesnoth root directory, and run `scons translations`. This will compile all the translations into a translations directory.

## Packaging
When compiling Wesnoth for an official release, the following steps should be taken:

 * You must use **package build**
 * Update version numbers in Info.plist (if not already by the release manager)
 * Update the changelog in SDLMain.nib with player_changelog
 * Rebuild translations
 * Rebuild all in XCode (clean all, then build). Be sure to set it to release configuration first!
 * Download old `.dmg` release.
 * Convert it using Disk Utility to Read and Write disk image and mount it.
 * Delete old `Wesnoth.app` and copy new `Wesnoth.app`.
 * Rename disk image to match new release version.
 * Unmount it and convert it back using `hdiutil convert /PATH/TO/IMAGE.dmg -format UDBZ -o /PATH/TO/NEW/IMAGE.dmg` command.
 * Create SHA-256 checksum using `shasum -a 256 /PATH/TO/NEW/IMAGE.dmg` command.
 * Done! You can release it now.
