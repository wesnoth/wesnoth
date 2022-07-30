# Building with Xcode
This README describes the way to create the wesnoth release packages for macOS.

### Requirements for building Wesnoth
 * Xcode 8.2.1 or higher...
 * Mac OS X 10.11.6 or higher...
 * `git`
 * `scons` and `gettext` (if you want to compile translations)

## Targets
* **The Battle for Wesnoth**:
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
Builds for maximum (runtime) speed and compatibility; it builds for 64-bit, with the latest SDK, but targets 10.11. You do not however need 10.11 SDK to build it. This is what's used for official releases.

* **Debug**:
Builds for maximum compiling speed, and uses the current OS as the SDK. If you just want to compile for testing things yourself, this is the way to go.

## Translations
To compile translations you need `gettext-tools` and `scons`. You can obtain these tools using `brew install gettext scons`. You also have to force-link Homebrew's `gettext` tools using `brew link --force gettext` command. In the Terminal, `cd /PATH/TO/PROJECT` and run `scons translations`. This will compile all the translations into a translations directory.

## Packaging
When compiling Wesnoth for an official release, the following steps should be taken. Packaging is separated to the 4 subchapters:
1. In first chapter we will prepare package for all 3 distribution sources.
2. In second chapter we will notarize Steam package and we will release it to the SteamStore.
3. In third chapter we will notarize SourceForge package, we will create `.dmg` image and we will release it to SourceForge.
4. In fourth chapter we will release package to the Mac AppStore.

### Packaging - Packages preparation
 * Obtain sources from github using `git clone -b BRANCH --depth 10 https://github.com/wesnoth/wesnoth /PATH/TO/PROJECT` or use your favourite git program.
 * Checkout to the latest tag using `cd /PATH/TO/PROJECT ; git checkout TAG` or use your favourite git program.
 * Use `Finder` to navigate into `/PATH/TO/PROJECT/projectfiles/Xcode`.
 * Double click on the `Fix_Xcode_Dependencies` script and wait until it finishes.
 * Compile translations as it is described in **Translations** section.
 * Now you can open `The Battle for Wesnoth.xcodeproj` file in Xcode.
 * Update version numbers in Info.plist (if not already done by the release manager).
 * Create `dist` file using `touch /PATH/TO/PROJECT/data/dist`.
 * You must increment the build number before creating Mac AppStore package. If you don't know the previous build number, the best way is to proceed with steps for one of the packages using build number 1 and wait until you get an error message that the notarization step failed. The current build number is contained there.

### Packaging - SteamStore
 * Find and edit `dist` file in path `/PATH/TO/PROJECT/data/dist`. For Steam it must contain `Steam`.
 * Now you can hit `Product` > `Archive` from the menubar.
 * After archivation is done, you can select correct archive in Xcode Organizer, click on `Distribute App`, select `Developer ID` and select `Upload`.
 * Now you must wait. After successful notarization you should get notification via Xcode.
 * Now click on `Distribute App` again, select `Developer ID` and select export location.
 * Your final package will be saved in the location you selected.
 * You can now continue to releasing using partner steam tools.

### Packaging - SourceForge
 * Find and edit `dist` file in path `/PATH/TO/PROJECT/data/dist`. For SourceForge it must contain `SourceForge`.
 * Now you can hit `Product` > `Archive` from the menubar.
 * After archivation is done, you can select correct archive in Xcode Organizer, click on `Distribute App`, select `Developer ID` and select `Upload`.
 * Now you must wait. After successful notarization you should get notification via Xcode.
 * Now click on `Distribute App` again, select `Developer ID` and select export location.
 * Your final package will be saved in the location you selected.
 * Now copy dmg template from `/PATH/TO/PROJECT/packaging/macOS/Wesnoth_dmg_packaging_template.dmg`
 * Convert this template to the R/W image using `hdiutil convert /PATH/TO/TEMPLATE.dmg -format UDRW -o /PATH/TO/NEW/RW_IMAGE.dmg` and mount it.
 * Copy new wesnoth package using `cp -R /PATH/TO/THE/EXPORTED/PACKAGE.app "/Volumes/The Battle for Wesnoth/The Battle for Wesnoth.app"`.
 * Unmount R/W image and convert it to final ro image using `hdiutil convert /PATH/TO/RW_IMAGE.dmg -format ULFO -o /PATH/TO/NEW/IMAGE.dmg` command.
 * Rename disk image to match new release version. Example: `Wesnoth_1.15.2.dmg`
 * Sign newly created ro image using `codesign -s "Developer ID Application: Wesnoth, Inc (N5CYW96P9T)" /PATH/TO/NEW/IMAGE.dmg`. (You must have Wesnoth's signing certificate.)
 * Verify that you signed `.dmg` properly using `spctl -a -t open --context context:primary-signature -v /PATH/TO/NEW/IMAGE.dmg`.
 * Now you have to notarize the whole `.dmg` image. You must have app specific password for your AppleID prepared.
 * Execute the `xcrun altool --notarize-app -f /PATH/TO/NEW/IMAGE.dmg --primary-bundle-id org.wesnoth.Wesnoth -u YOUR_APPLE_ID_EMAIL -p YOUR_APPLE_ID_APP_SPECIFIC_PASSWORD --asc-provider N5CYW96P9T` and wait. After successful execution it should give you RequestUUID.
 * You can check notarization status using `xcrun altool --notarization-info REQUEST_UUID -u YOUR_APPLE_ID_EMAIL -p YOUR_APPLE_ID_APP_SPECIFIC_PASSWORD` command. Wait until it returns the `Package Approved` status message.
 * Create SHA-256 checksum using `shasum -a 256 /PATH/TO/NEW/IMAGE.dmg > Wesnoth_x.x.x.dmg.sha256` command.
 * Done! You can release it to SourceForge now.

### Packaging - Mac AppStore
 * First you have to enable app sandbox for `wesnothd`.
 * In Xcode window click on `The Battle for Wesnoth` project in the left sidebar.
 * Select `wesnothd` target.
 * Select `Signing & Capabilities` from the top bar.
 * Click `+ Capability` while in the `All` tab.
 * Select `App Sandbox`.
 * And in newly created section check both `Incoming` and `Outgoing` connections.
 * Find and edit `dist` file in path `/PATH/TO/PROJECT/data/dist`. For Mac AppStore it must contain `macOS App Store`.
 * Now you can hit `Product` > `Archive` from the menubar.
 * After archivation is done, you can select correct archive in Xcode Organizer, click on `Distribute App`, select `App Store Connect` and proceed with all steps by clicking `Next`.
 * After successful uploading you must go to the https://appstoreconnect.apple.com/ and continue with releasing there.
