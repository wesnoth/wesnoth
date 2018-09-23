# Building with Xcode

### Requirements for building Wesnoth
 * Xcode 5.1.1 or higher...
 * Mac OS X 10.8.5 or higher...
 * `git`
 * `scons` and `gettext` (if you want to compile translations)

## Targets
* **Wesnoth**:
Builds the actual game, depends on wesnothd. If you don't want to build wesnothd, get info on the Wesnoth target, go to the General tab, and remove wesnothd from its dependencies.

* **wesnothd**:
Builds the multiplayer server. The MP server is needed for hosting a local MP server, not for connecting to the official one.

* **unit_tests**:
Builds the unit tests. This is mainly useful when trying to fix a failing Travis build.

* **campaignd**:
Builds the addons server.

* **liblua**:
Builds the lua library for Wesnoth.


## Configurations
* **Release**:
Builds for maximum (runtime) speed and compatibility; it builds for 64-bit, with the latest SDK, but targets 10.8. You do not however need 10.8 SDK to build it. This is what's used for official releases.

* **Debug**:
Builds for maximum compiling speed, and uses the current OS as the SDK. If you just want to compile for testing things yourself, this is the way to go.

## Step by step building
1. Obtain sources from github using `git clone https://github.com/wesnoth/wesnoth` or use your favourite git program
2. Use `Finder` to navigate into `wesnoth/projectfiles/Xcode` . And now, there will be 2 options.
3. Double click on `Fix_Xcode_Dependencies` script and wait until it will be done.
5. (Optional) Compile translations as it is described in **Translations** section.
6. Now you can open `Wesnoth.xcodeproj` file in Xcode.

## Translations
To compile translations you need `gettext-tools` and `scons`. You can obtain these tools using `brew install gettext scons`. In the Terminal, `cd` to the Wesnoth root directory, and run `scons translations`. This will compile all the translations into a translations directory.

## Packaging
When compiling Wesnoth for an official release, the following steps should be taken:

 * Update version numbers in Info.plist (if not already by the release manager).
 * Update the changelog in SDLMain.nib with `changelog.md`.
 * Rebuild translations `scons translations`.
 * Rebuild all in XCode (clean all, then build). Be sure to set it to RELEASE configuration first!
 * Download old `.dmg` release.
 * Convert it using Disk Utility to Read and Write disk image and mount it.
 * Delete old `Wesnoth.app` and copy new `Wesnoth.app`.
 * CodeSign `Wesnoth.app` inside Read and Write disk image using `codesign --deep --force -s "Wesnoth, Inc" Wesnoth.app`. You must have Wesnoth's signing certificate.
 * Verify that you signed `.app` propertly using `spctl -a -t exec -vv Wesnoth.app`.
 * Rename disk image to match new release version.
 * Unmount it and convert it back using `hdiutil convert /PATH/TO/IMAGE.dmg -format UDBZ -o /PATH/TO/NEW/IMAGE.dmg` command.
 * Sign newly created `.dmg` using `codesign -s "Wesnoth, Inc" /PATH/TO/NEW/IMAGE.dmg`. You must have Wesnoth's signing certificate.
 * Verify that you signed `.dmg` propertly using `spctl -a -t open --context context:primary-signature -v /PATH/TO/NEW/IMAGE.dmg`.
 * Create SHA-256 checksum using `shasum -a 256 /PATH/TO/NEW/IMAGE.dmg` command.
 * Done! You can release it now.


