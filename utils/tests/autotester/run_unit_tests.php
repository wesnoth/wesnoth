<?php
if ($argc != 2)
{
	echo "Wrong number of parameters given";
	die;
}
$root_dir = $argv[1];

require_once $root_dir . 'include/config.php';

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
	$db->CompleteTrans();
} 

$config = new Config('last_autotest_run_time');
$config->set(time());
?>
