--- Building with Xcode ---

-- Requirements --
 * Xcode 3.0+
 * Mac OS X 10.5+ (required by Xcode 3.0)
 * scons and gettext-tools (if you want to compile translations)
 * DMG Canvas (if you want to make a fancy .dmg)
 * The Headers and lib folders, which can be found in the newest zip here:
   https://sourceforge.net/downloads/wesnoth/unofficial/Mac%20Compile%20Stuff/

Once you download the zip, unzip it and move "Headers" and "lib" into the same folder as the Xcode project.


-- Targets --

- Wesnoth:
Builds the actual game, depends on wesnothd. If you don't want to build wesnothd, get info on the Wesnoth target, go to the General tab, and remove wesnothd from its dependencies.

- wesnothd:
Builds the multiplayer server. The MP server is needed for hosting a local MP server, not for connecting to the official one.

- unit_tests:
Builds the unit tests, but probably doesn't work currently. If it does build, there's still no good way to run it.


-- Configurations --

- Release:
Builds for maximum (runtime) speed and compatibility; it builds for PPC and i386 (Intel), and with the 10.4 SDK, and targets 10.4. Consequently, you need 10.4 to build it. This is what's used for official releases.

- Debug:
Builds for maximum compiling speed, and uses the current OS as the SDK. If you just want to compile for testing things yourself, this is the way to go.


-- Translations --

To compile translations you need gettext-tools and scons. In the Terminal, cd to the Wesnoth root directory, and run "scons translations". This will compile all the translations into a translations directory, and then you can move this into Wesnoth.app/Contents/Resources/ to use it.


-- Packaging --

 * Update version numbers in Info.plist
 * Update the changelog in SDLMain.nib with player_changelog
 * Rebuild all in XCode (clean all, then build)
 * Rebuild translations, and move to Resources (see above)
 * Make dmg with Wesnoth.dmgCanvas (also included in the zip), after updating the volume name with the version number


~ Ben Anderman / crimson_penguin <ben@happyspork.com>, August 12 2010 --
