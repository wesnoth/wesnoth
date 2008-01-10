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
$extrapacks = explode(" ", $extrapackages);

echo "<h1>Getting stats for trunk</h1>";

#Get trunk stats
foreach($packs as $package){
	$stats = array();
	$languages = file_get_contents($trunkbasedir . "/po/" . $package . "/LINGUAS");
	$languages = substr($languages, 0, strlen($languages)-1);
	$langs = explode(" ", $languages);
	echo "<h2>Getting stats for package $package</h2>";
	$stats["_pot"]=getstats($trunkbasedir . "/po/" . $package . "/" . $package . ".pot");
	if(!file_exists("stats/" . $package)){
		system("mkdir stats/" . $package);
	}
	foreach($langs as $lang){
		echo "Getting stats for lang $lang<br/>";
		$pofile = $trunkbasedir . "/po/" . $package . "/" . $lang . ".po";
		$stats[$lang]=getstats($pofile);
	}

	$serialized = serialize($stats);
	$file = fopen("stats/" . $package . "/trunkstats", "wb");
	fwrite($file, $serialized);
	fclose($file); 
}

echo "<h1>Getting stats for branch</h1>";

#Get branch stats
foreach($packs as $package){
	$stats = array();
	$languages = file_get_contents($branchbasedir . "/po/" . $package . "/LINGUAS");
	$languages = substr($languages, 0, strlen($languages)-1);
	$langs = explode(" ", $languages);
	echo "<h2>Getting stats for package $package</h2>";
	$stats["_pot"]=getstats($branchbasedir . "/po/" . $package . "/" . $package . ".pot");
	if(!file_exists("stats/" . $package)){
		system("mkdir stats/" . $package);
	}
	foreach($langs as $lang){
		echo "Getting stats for lang $lang<br/>";
		$pofile = $branchbasedir . "/po/" . $package . "/" . $lang . ".po";
		$stats[$lang]=getstats($pofile);
	}

	$serialized = serialize($stats);
	$file = fopen("stats/" . $package . "/branchstats", "wb");
	fwrite($file, $serialized);
	fclose($file); 
}

foreach($extrapacks as $package){
	$stats = array();
	$domain = getdomain($package);
	$languages = file_get_contents($extrabasedir . "/" . $package . "/po/LINGUAS");
	$languages = substr($languages, 0, strlen($languages)-1);
	$langs = explode(" ", $languages);
	echo "<h2>Getting stats for package $package</h2>";
	$stats["_pot"]=getstats($extrabasedir . "/" . $package . "/po/" . $domain . ".pot");
	if(!file_exists("stats/" . $domain)){
		system("mkdir stats/" . $domain);
	}
	foreach($langs as $lang){
		echo "Getting stats for lang $lang<br/>";
		$pofile = $extrabasedir . "/" . $package . "/po/" . $lang . ".po";
		$stats[$lang]=getstats($pofile);
	}
	$serialized = serialize($stats);
	$file = fopen("stats/" . $domain . "/stats", "wb");
	fwrite($file, $serialized);
	fclose($file); 
}
?>
