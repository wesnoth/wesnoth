/*
   AI Batch testing suite
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


REQUIRES:
 Wesnoth
 Postgresql database for storing data
 PHP-enabled web server for web frontend

INSTALLATION:

1. Create a role, two users and a database - one for uploader script with INSERT priv (and ability to use the sequence used for generating IDs), and another for web frontend with SELECT priv.
---------------------
CREATE ROLE wesnoth_ai_test_viewer
  NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE;

CREATE ROLE wesnoth_ai_test_user LOGIN
  PASSWORD 'YOUR_PASSWORD_FOR_TEST_USER'
  NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE;
GRANT wesnoth_ai_test_viewer TO wesnoth_ai_test_user;

CREATE ROLE wesnoth_ai_test_viewer_impl LOGIN
  PASSWORD 'YOUR_PASSWORD_FOR_WEB_FRONTEND_USER'
  NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE;
GRANT wesnoth_ai_test_viewer TO wesnoth_ai_test_viewer_impl;

CREATE DATABASE "org.wesnoth.ai.test"
  WITH OWNER = wesnoth_ai_test_user
       ENCODING = 'UTF8';
---------------------
2. Restore ai_test_db.backup to that newly created DB to create the DB schema and set privilegies.

3. place wesnoth_ai_test.php at a php-enabled web server

4. Modify passwords/paths in ai_test.cfg and wesnoth_ai_test.php



