# Lua for Wesnoth

This document describes the process used to install Lua 5.4.4 for Wesnoth. The external repository pointed to by the lua submodule is located https://github.com/lua/lua.

The goal of this process was, as much as possible, install clean, unchanged sources.
Traditionally, Wesnoth maintainers have made changes directly to the Lua source kit.
__This is no longer allowed, to remain compatible with using system copies of Lua.__
Future maintainers should strive, as much as possible, to follow a similar process.

The process, below, involves a number of separate commits; this makes the upgrade history easier to follow.

Future maintainers are expected to update this document as appropriate.

## 1) Before you begin

Be sure you are using a copy of the current master.
And be sure you are working in a private branch.

        $ cd ~/wesnoth
        $ git checkout master
        $ git pull --rebase upstream master
        $ git checkout -b Upgrade_to_Lua-5.4.4

## 2) Update CMake Modules

Download new versions of <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindPackageMessage.cmake> and <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindPackageHandleStandardArgs.cmake> to cmake/.

Commit the changes:

        $ git add cmake/
        $ git commit -m 'Update FindLua.cmake and dependencies'

Update cmake/FindLua.cmake from <https://gitlab.kitware.com/cmake/cmake/-/raw/master/Modules/FindLua.cmake> and cherry pick the Wesnoth commit "Modify FindLua.cmake to find Lua compiled as C++".

## 3) Update Lua Source

From the repository root, change directory to the lua submodule, and checkout version 5.4.4:

        $ cd src/modules/lua
        $ git checkout tags/v5.4.4

## 4) Update SCons and CMake

In CMakeLists.txt, update `set(Lua_FIND_VERSION_MAJOR 5)` and `set(Lua_FIND_VERSION_MINOR 4)`. In SConstruct, update `lua_ver = "5.4"` (i.e. "MAJOR.MINOR").

Remember to review the source kit for added and removed files, and change the SCons and CMake configuration, as needed.
Both build systems' build lists are in `~/wesnoth/source_lists/lua`.
Verify the files listed match the C source files in the submodule; order is not important, headers are not listed.

**DO NOT** add any of the following files to the lua source_list:
- lua.c
- luac.c
- linit.c
- onelua.c
- ltests.c

We do not need, or want, the Lua command line interpreter or the Lua compiler.

We initialize the Lua runtime and do not want to allow the provided initialization routine.

We do not need to run lua's test suite.

Updating the project files for other target platforms is optional at this point.

## 5) Commit the changes

        $ cd ~/wesnoth
        $ git add .
        $ git commit -m 'Upgrade to Lua 5.4.4'

## 6) Update Wesnoth Lua runtime support

Sometimes Lua adds and removes libraries.
`~/src/scripting/lua_kernel_base.cpp` contains the table to add libraries to the Lua run-time.
Remove any deleted libraries from the table.
Review each new library and, if it does not access the outside environment (primarily the filesystem), and seems safe to allow, add the library to the table.

Note that some libraries are only partially supported.
`lua_kernel_base.cpp` adds those libraries, then removes functions which are not to be exposed through the Lua runtime.
Carefully review the functions, paying particular attention to those listed which are __not__ removed.

__Apply these changes as a separate commit.__

## 7) Build Wesnoth

Run a test build.
Often, when upgrading Lua, there are changes to the API.
Be sure to carefully check the build for errors and warnings about missing Lua functions.
Make any adjustments necessary.

__Separately commit these adjustments.__

## 8) Check Mainline

Finally, read the Lua manual section on incompatibilities with older versions.
Be sure to check all mainline Lua modules to ensure any deprecated or removed Lua functions are updated.

__Commit any changes needed.__

## 9) Create a Pull Request

Even if you have direct access to the Wesnoth master repository, you should __never upgrade Lua immediately__.
Push your local branch up to GitHub and create a Pull Request.

Don't forget to monitor Travis/CI for your pull request to ensure a clean test run.

Have someone else review your changes and merge them when all issues have been addressed.
