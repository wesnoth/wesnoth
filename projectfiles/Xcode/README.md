# Building with Xcode
This README describes the way to create the wesnoth release packages for macOS.

## iOS status
The default Xcode dependency workflow in this directory remains macOS-oriented.

For the iOS simulator dependency bootstrap and audit workflow, use:

- `projectfiles/Xcode/ios/build_ios_deps.sh`
- `projectfiles/Xcode/ios/audit_ios_linkage.sh`

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
When compiling Wesnoth for an official release, the following steps should be taken. Packaging is separated into subchapters, one per distribution channel:
1. First, we prepare the sources and Xcode project for all distribution channels.
2. Steam: notarize the package and release to the Steam Store.
3. SourceForge: notarize the package, build a `.dmg`, and release to SourceForge.
4. itch.io: notarize the package and release to itch.io.
5. Mac AppStore: sandbox `wesnothd`, archive, and upload via App Store Connect.

The release channel is recorded inside the archive in `Contents/Resources/data/dist` (a plain text file). Valid values are `Steam`, `SourceForge`, `itch`, and `macOS App Store`. Any other value is treated as an unknown / default channel by the game, and by the `Check_xcarchive` validation script.

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

### Packaging - itch.io
 * Find and edit `dist` file in path `/PATH/TO/PROJECT/data/dist`. For itch.io it must contain `itch`.
 * Now you can hit `Product` > `Archive` from the menubar.
 * After archivation is done, select the archive in Xcode Organizer, click on `Distribute App`, select `Developer ID` and select `Upload` (to notarize).
 * Wait for successful notarization.
 * Click `Distribute App` again, select `Developer ID` and select an export location.
 * Upload the exported `.app` to itch.io using `butler`.

### Packaging - Mac AppStore
 * First you have to enable app sandbox for `wesnothd` by swapping its entitlements file. The easiest way is to double click on the `Prepare_for_MAS_release` script in `/PATH/TO/PROJECT/projectfiles/Xcode` — it flips the `Code Signing Entitlements` build setting from `Resources/wesnothd-nosandbox.entitlements` to `Resources/wesnothd-mas.entitlements` for all three `wesnothd` configurations. If the Xcode project is open, close it first (or at least close the `wesnothd` target editor) so Xcode picks up the change.
 * If you prefer to do it manually in Xcode: click on the `The Battle for Wesnoth` project in the left sidebar, select the `wesnothd` target, open the `Build Settings` tab (with `All` and `Combined` selected), find `Code Signing Entitlements`, and change it from `Resources/wesnothd-nosandbox.entitlements` to `Resources/wesnothd-mas.entitlements`. Do NOT use the `Signing & Capabilities` tab to toggle App Sandbox — it can leave the build in an inconsistent state.
 * Remember to run `Prepare_for_nosandbox_release` (or revert the build setting manually) after the MAS archive is uploaded, before building any non-MAS package.
 * Find and edit `dist` file in path `/PATH/TO/PROJECT/data/dist`. For Mac AppStore it must contain `macOS App Store`.
 * Now you can hit `Product` > `Archive` from the menubar.
 * After archivation is done, you can select correct archive in Xcode Organizer, click on `Distribute App`, select `App Store Connect` and proceed with all steps by clicking `Next`.
 * After successful uploading you must go to the https://appstoreconnect.apple.com/ and continue with releasing there.

### Packaging - Verifying an archive

Before distributing any archive, you can validate it against its intended distribution channel (reads the `dist` file inside the bundled `.app`, then checks `wesnothd`'s code signing entitlements to confirm the sandbox state matches):

 * Double-click `Check_xcarchive` in `/PATH/TO/PROJECT/projectfiles/Xcode` and drag one or more `.xcarchive` bundles (or a folder containing them) from Finder onto the Terminal window.
 * Or from a shell: `./Check_xcarchive /path/to/archive.xcarchive` (also accepts a directory of archives).

The script reports PASS / FAIL / WARN per archive. A MAS archive must have `wesnothd` sandboxed; Steam, SourceForge, and itch.io archives must not. In every case `wesnothd` must not carry `com.apple.application-identifier` (that entitlement on a nested binary triggers App Store rejection ITMS-90288 / 90286 / 90885).
