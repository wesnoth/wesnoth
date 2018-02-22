# Blowfish Cryptography

This document describes the process used to install Blowfish Cryptography 1.3 for Wesnoth.

The goal of this process was, as much as possible, install clean, unchanged sources.
Traditionally, Wesnoth maintainers are tempted to make changes directly to the Blowfish source kit.
__This is strongly discouraged.__
Future maintainers should strive, as much as possible, to follow a similar process.

Future maintainers are expected to update this document as appropriate.

## 1) Before you begin

Be sure you are using a copy of the current master.
And be sure you are working in a private branch.

        $ cd ~/wesnoth
        $ git checkout master
        $ git pull --rebase upstream master
        $ git checkout -b Upgrade_to_Blowfish_1.3

## 2) Update Blowfish Source

Download the current source kit from [the maintainers](http://www.openwall.com/crypt/).
For Blowfish Cryptography 1.3, this was <http://www.openwall.com/crypt/crypt_blowfish-1.3.tar.gz>.
The following presumes you are working on Unix.
Windows is a bit more work, but generally follows the same process.

        $ cd ~
        $ wget http://www.openwall.com/crypt/crypt_blowfish-1.3.tar.gz

Unpack into your home folder.
Note that, while the filename implies the file is compressed using GNU zip, it is actually just a plain tarball.

        $ tar -xf crypt_blowfish-1.3.tar.gz

Change into the Blowfish Cryptography folder.

        $ cd ~/crypt_blowfish-1.3

We do not need, or want, the GNU libc patch files, the man page, or the x86 Assember source file, so delete them.

        $ rm *.diff crypt.3 x86.S

Next, delete the current copy of Blowfish Cryptography in Wesnoth.

        $ cd ~/wesnoth/src/bcrypt
        $ rm -fR crypt_blowfish

Finally, move the new sources into place.

        $ mv ~/crypt_blowfish-1.3 crypt_blowfish

## 3) Update SCons and CMake

Remember to review the source kit for added and removed files, and change the SCons and CMake configuration, as needed.
Both build systems' build lists are in `~/wesnoth/source_lists/libwesnoth_core`.
Verify the files listed match the C source files just copied in; order is not important, headers are not listed.
The source list lists many files; you may have to search a bit.
The Blowfish Cryptography files, however, should all have names beginning with `bcrypt/crypt_blowfish/`.

Updating the project files for other target platforms is optional at this point.

## 4) Commit the changes

        $ cd ~/wesnoth
        $ git add .
        $ git commit -m 'Upgrade to Blowfish Cryptography 1.3'

## 5) Build Wesnoth

Run a test build.
Rarely, when upgrading Blwofish Cryptography, there are changes to the API.
Be sure to carefully check the build for errors and warnings about changed or missing Blowfish cryptography functions.
Make any adjustments necessary.
Generally, if needed, changes will appear in `~/wesnoth/src/bcrypt/bcrypt.c`.

__Separately commit these adjustments.__

## 6) Create a Pull Request

Even if you have direct access to the Wesnoth master repository, you should __never upgrade Blowfish Cryptography immediately__.
Push your local branch up to GitHub and create a Pull Request.

Don't forget to monitor Travis/CI for your pull request to ensure a clean test run.

Have someone else review your changes and merge them when all issues have been addressed.
