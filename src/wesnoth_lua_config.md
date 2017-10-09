# Lua for Wesnoth

This document describes the process used to install Lua 5.3.4 for Wesnoth.

The goal of this process was, as much as possible, install clean, unchanged sources.
Traditionally, Wesnoth maintainers have made changes directly to the Lua source kit.
__This is strongly discouraged.__
Future maintainers should strive, as much as possible, to follow a similar process.

The process, below, involves a number of separate commits; this makes the upgrade history easier to follow.
The primary commit, after replacing the sources, is usually unreadable since it contains thousands of changes.
Separating the supporting commits makes them easier to find, and read.

Future maintainers are expected to update this document as appropriate.

## 1) Before you begin

Be sure you are using a copy of the current master.
And be sure you are working in a private branch.

        $ cd ~/wesnoth
        $ git checkout master
        $ git pull --rebase upstream master
        $ git checkout -b Upgrade_to_Lua-5.3.4

## 2) Update Lua Source

Download the current source kit from [the Lua maintainers](http://www.lua.org).
For Lua 5.3.4, this was <http://www.lua.org/ftp/lua-5.3.4.tag.gz>.
The following presumes you are working on Unix.
Windows is a bit more work, but generally follows the same process.

        $ cd ~
        $ wget http://www.lua.org/ftp/lua-5.3.4.tar.gz

Unpack into your home folder.

        $ tar -zxf lua-5.3.4.tar.gz

Change into the Lua source folder.

        $ cd ~/lua-5.3.4/src

We do not need, or want, the Lua command line interpreter, the Lua compiler or the Makefile, so delete them.
We compile using C++ so cannot allow the use of "C" linkage from the provided header.
And we initialize the Lua runtime and do not want to allow the provided initialization routine.

        $ rm lua.c luac.c Makefile lua.hpp linit.c

Wesnoth requires all Lua sources to be compiled using C++.
To ensure  this, rather than depend upon compiler flags, rename the files.

        $ for FILENAME in *.c; do mv "$FILENAME" "`basename $FILENAME .c`.cpp"; done

Delete the entire Lua source from Wesnoth.
Be careful not to delete the `COPYRIGHT` file.
After removing the old sources, these should be the only files remaining.

        $ rm ~/wesnoth/src/lua/*.h
        $ rm ~/wesnoth/src/lua/*.hpp
        $ rm ~/wesnoth/src/lua/*.cpp

Copy in the new Lua source kit.

        $ cp *.h ~/wesnoth/src/lua/
        $ cp *.hpp ~/wesnoth/src/lua/
        $ cp *.cpp ~/wesnoth/src/lua/

## 3) Change the COPYRIGHT file

The official copyright notice appears at the end of the `lua.h` header.
Replace the copyright notice in the `COPYRIGHT` file.

## 4) Update SCons and CMake

Remember to review the source kit for added and removed files, and change the SCons and CMake configuration, as needed.
Both build systems' build lists are in `~/wesnoth/source_lists/lua`.
Verify the files listed match the C++ source files just copied in; order is not important, headers are not listed.

Updating the project files for other target platforms is optional at this point.

## 5) Commit the changes

        $ cd ~/wesnoth
        $ git add .
        $ git commit -m 'Upgrade to Lua 5.3.4'

## 6) Apply official patches

Often there will be patches available against the Lua release.
Generally, these can be applied using `patch`, but obtaining clean patch files takes some looking.
Hand-applying the patches may be easier.
The patches can be [viewed online](http://www.lua.org/bugs.html).

__Apply each patch as a separate commit.__

## 7) Update Wesnoth Lua runtime support

Sometimes Lua adds and removes libraries.
`~/src/scripting/lua_kernel_base.cpp` contains the table to add libraries to the Lua run-time.
Remove any deleted libraries from the table.
Review each new library and, if it does not access the outside environment (primarily the filesystem), and seems safe to allow, add the library to the table.

Note that some libraries are only partially supported.
`lua_kernel_base.cpp` adds those libraries, then removes functions which are not to be exposed through the Lua runtime.
Carefully review the functions, paying particular attention to those listed which are __not__ removed.

__Apply these changes as a separate commit.__

## 8) Update Wesnoth Lua runtime options

Review `~/wesnoth/src/wesnoth_lua_config.h` to enable optional features.
Note that these features only effect the Lua run-time.
Optional features will __NOT__ be available to the Wesnoth C++ source.

__Apply these changes as a separate commit.__

## 9) Build Wesnoth

Run a test build.
Often, when upgrading Lua, there are changes to the API.
Be sure to carefully check the build for errors and warnings about missing Lua functions.
Make any adjustments necessary.

__Separately commit these adjustments.__

## 10) Check Mainline

Finally, read the Lua manual section on incompatibilities with older versions.
Be sure to check all mainline Lua modules to ensure any deprecated or removed Lua functions are updated.

__Commit any changes needed.__

## 11) Create a Pull Request

Even if you have direct access to the Wesnoth master repository, you should __never upgrade Lua immediately__.
Push your local branch up to GitHub and create a Pull Request.

Don't forget to monitor Travis/CI for your pull request to ensure a clean test run.

Have someone else review your changes and merge them when all issues have been addressed.
