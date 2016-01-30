/*
   AI Batch testing suite
   Copyright (C) 2009 - 2013 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


REQUIRES:
 Wesnoth
 Python
 (optional) PHP-enabled web server for web frontend

USAGE:
1. Edit ai_test.cfg. (Read comments there for explanations)
2. Run "python ai_test.pyi [-p]" to run the tests.

USAGE WITH SQLITE DATABASE:
1. Copy ai_test_empty.sqlite to somewhere outside the git repo.
2. Give ai_test_empty.sqlite write access.
3. place wesnoth.php and wesnoth_test.php at a php-enabled web server.
4. Edit BOTH php files and change the variable $sqlitefile.
5. Edit ai_test.cfg. (Don't forget to change 'sqlite_file' there).
6. Run "python ai_test.py [-p]".

TIPPS:
- Before you start testing use the "-p" parameter
  to play a test-game with gui. Then you may want to use
  :inspect to see if the ais are set up correctly.
- 100 tests are nothing, 500 tests are good to see some
  trends. I recommend to run at least 1000 tests.
- You can make use of your multicores and run multiple
  tests at the same time. In Linux the 'System Monitor'
  is your friend. You can set the process priority there
  to 'Very Low' or pause the process and continue it later.
- In linux you can pause the tests by pressing Ctrl + z.
  You can continue the process by running 'fg'.
