# Compiling Wesnoth on Windows using Visual Studio

The current minimum Visual Studio version required for compiling Wesnoth is Visual Studio 2013 (VC12).
Compilation is also supported with Visual Studio 2015 (VC14) and later versions. However, since we keep
the project files in the Git repository targeted at the minimum version, it is recommended you duplicate
the `wesnoth/projectfiles/VC12` directory and rename it after the Visual Studio version with which you
wish to build.

## Prerequisites

We maintain a handy [GitHub repository](https://github.com/aquileia/external) with all the external
libraries (see [INSTALL.md](https://github.com/wesnoth/wesnoth/blob/master/INSTALL.md)) Wesnoth requires.
It has various branches corresponding to the Visual Studio version you are building with. Be sure to use
the libraries from the appropriate branch!

## What to do

1. Clone or download a snapshot of the aforementioned dependency repository. If you do the latter, be sure
you've downloaded the snapshot for the correct branch, **not** `master`! Either way, it should be
cloned/unpacked into the same directory as Wesnoth. If you have Wesnoth cloned in `Documents/wesnoth`, for
example, the dependency pack should be at `Documents/external`. Note that `external` is the name of the
resulting directory by default.  **DO NOT RENAME IT!**

2. If you cloned the repository, switch to the the version-appropriate branch now.

3. Return to `wesnoth/projectfiles` and duplicate the VC12 folder if applicable, as mentioned above.

4. Open `projectfiles/VCXX/wesnoth.sln` in Visual Studio. At this point, it may prompt you to re-target the
projectfiles for your current VS and Windows versions. Do so; the build will likely fail if you do not.

5. **Optional:** by default, Wesnoth's WML unit tests are run after each build. This can be quite annoying
and/or time-consuming if you build regularly. They can be disabled by heading to the `wesnoth` projectfile's
properties, under `Build Events → Post-Build Event`. Delete the value in the "Command Line" field.

6. **Optional:** By default, the Debug configuration is selected. This type of build is only good if you
intend to work on the game's engine or wish to get a stacktrace for a bug report. If you're only interested
in playing the game, a Release build is faster. You can switch the configurations in the toolbar at the top
of the screen.

7. That's it! You can now go ahead and start the build (`Build → Build Solution`).

8. **Important!** After building, copy all the files from `external/dll` into the same directory (`wesnoth/`
by default) as the newly built executable . The game will not start if you fail to do this.

## Manually updating the external dependencies

We do our best to keep the build dependency repository up-to-date with the latest versions of the libraries
within, as well as synced with any build requirement changes. If you want to build with a different version
of a certain library, however, you can fetch the relevant files at the links below:

* [**Boost:**](http://www.boost.org/users/download) Do note that you will need to build the necessary Boost
libraries yourself. See the [instructions](https://github.com/aquileia/external/blob/master/README.md#updating-boost-libraries)
in the dependency repository for details.

* [**SDL 2:**](https://www.libsdl.org/download-2.0.php) You'll want the "Visual C++ 32/64-bit" Development
Libraries.

* [**SDL_Image:**](https://www.libsdl.org/projects/SDL_image) Again, you'll want the "Visual C++ 32/64-bit"
Development Libraries.

* [**SDL_Mixer:**](https://www.libsdl.org/projects/SDL_mixer) Again, you'll want the "Visual C++ 32/64-bit"
Development Libraries.

The other libraries require complicated compilation procedures too in-depth to document here.
