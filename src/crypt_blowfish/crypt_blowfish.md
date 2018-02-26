# Blowfish Password Hashing (bcrypt)

This document describes the process used to install or upgrade Blowfish Password Hashing (bcrypt) for Wesnoth.

The goal of this process is to install clean, unchanged sources.
Traditionally, Wesnoth maintainers are tempted to make changes directly to these source files.
__This is strongly discouraged.__
Future maintainers should strive, as much as possible, to follow a similar process.

Future maintainers are expected to update this document as appropriate.

## 1) Before you begin

Be sure you are using a copy of the current master.
And be sure you are working in a private branch.

        cd ~/wesnoth
        git checkout master
        git pull --rebase upstream master
        git checkout -b Upgrade_to_PHP_5.6.27_bcrypt

## 2) Update Blowfish Password Hashing Sources from PHP

        cd ~/wesnoth/src/crypt_blowfish
        wget https://raw.githubusercontent.com/php/php-src/master/ext/standard/crypt_blowfish.c
        wget https://raw.githubusercontent.com/php/php-src/master/ext/standard/crypt_blowfish.h

## 3) Commit the changes

        cd ~/wesnoth
        git add .
        git commit -m 'Upgrade to PHP 5.6.27 bcrypt'

## 4) Build Wesnoth

Run a test build.
Rarely, when upgrading Blowfish Password Hashing (bcrypt), there are changes to the API.
Be sure to carefully check the build for errors and warnings.
Make any adjustments necessary.

__Separately commit these adjustments.__

## 5) Create a Pull Request

Even if you have direct access to the Wesnoth master repository, you should __never upgrade Blowfish Password Hashing (bcrypt) immediately__.
Push your local branch up to GitHub and create a Pull Request.

Don't forget to monitor Travis/CI for your pull request to ensure a clean test run.

Have someone else review your changes and merge them when all issues have been addressed.
