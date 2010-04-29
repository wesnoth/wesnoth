<?php

////////////////////////////////////////////////////////////////////////////
// $Id: grab-stats.php,v 1.3 2004/01/29 23:32:37 claudiuc Exp $
//
// Description: Grab GUI messages statistics
//
////////////////////////////////////////////////////////////////////////////

include("functions.php");
include("config.php");

// ************* PRELIMINARY TASKS, DATA INIT *************
set_time_limit(0);

// data initialization
$currdate = date("Y-m-d",mktime());

// ************* DATA PROCESSING *************

// check and then aquire lock to prevent two simultaneous running
 if (is_locked()) {
  exit();
} else {
  register_shutdown_function("remove_lock");
  create_lock();
}

$packs = explode(" ", $packages);
$extratpacks = explode(" ", $extratpackages);
$extrabpacks = explode(" ", $extrabpackages);

function grab_stats ($tob, $official, $packs) // trunk or branch, official (1) or extras (0), package array
{
	//these are defined in config.php
	global $trunkbasedir;
	global $branchbasedir;
	global $extratbasedir;
	global $extrabbasedir;
	global $ignore_langs;

	foreach($packs as $package){
		$stats = array();
		if ($official)
		{
			$basedir = ($tob == "trunk") ? $trunkbasedir : $branchbasedir;
			$po_dir = $basedir . "/po/" . $package . "/";
			$domain = $package;
		} else { // wescamp
			$basedir = ($tob == "trunk") ? $extratbasedir : $extrabbasedir;
			$po_dir = $basedir . "/" . $package . "/po/";
			$domain = getdomain($package);
		}
		if (file_exists($po_dir . "/" . $domain . ".pot")) // it can happen that the translation is broken in wescamp, this mainly happens when there is no po/ folder with the file TEXTDOMAIN.pot
		{
			$languages = file_get_contents($po_dir . "/LINGUAS");
			$languages = substr($languages, 0, strlen($languages)-1);
			$langs = explode(" ", $languages);
			echo "Getting stats for package $package\n";
			$stats["_pot"] = getstats("$po_dir/" . $domain . ".pot");
			if (!file_exists("stats/" . $domain))
			{
				system("mkdir stats/" . $domain);
			}
			foreach ($langs as $lang)
			{
				//echo "Getting stats for lang $lang\n";
				if (!in_array($lang, $ignore_langs))
				{
					$pofile = $po_dir . "/" . $lang . ".po";
					$stats[$lang] = getstats($pofile);
				}
			}
		
			$serialized = serialize($stats);
			$file = fopen("stats/" . $domain . "/" . $tob . "stats", "wb");
			fwrite($file, $serialized);
			fclose($file);
		}
	}
}

echo "Getting stats for trunk\n";
grab_stats("trunk", 1, $packs);
grab_stats("trunk", 0, $extratpacks);

echo "Getting stats for branch ($branch)\n";
grab_stats("branch", 1, $packs);
grab_stats("branch", 0, $extrabpacks);

?>
