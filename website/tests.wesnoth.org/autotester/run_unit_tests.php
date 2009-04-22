<?php
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

if ($argc != 2)
{
	echo "Wrong number of parameters given";
	die;
}
$root_dir = $argv[1];

require_once $root_dir . '/../include/settup.php';

// used to show all queries to db when debuging;
//$db->debug = true;
// Upgrade the database format to the newest
$creator = new DBCreator();
$creator->checkDB();

// do svn up
$svn = new SVNUpdater();

if ($svn->getRevision() === false)
{
	trigger_error("SVN is down", E_USER_ERROR);
}

$build = new Build($svn->getRevision());

if (!$build->Exists())
{
	// Only run tests if build doesn't exists
	if ($build->compile($svn->getRevision()))
	{
		$test_runner = new TestRunner();
		$test_runner->run($build);
	}
} 

$build->pruneOldBuilds();

$config = new DBConfig('last_autotest_run_time');
$config->set($db->DBTimeStamp(time()));
?>
