<?php

////////////////////////////////////////////////////////////////////////////
// $Id: grab-config.php,v 1.4 2003/08/12 20:01:30 claudiuc Exp $
//
// Description: Configuration file for GUI statistics grabbing program
//
////////////////////////////////////////////////////////////////////////////

// the msgfmt program path
$msgfmt="/usr/bin/msgfmt";

$branch="1.2";

$trunkbasedir="/home/isaac/wesnoth/wesnoth/";
$branchbasedir="/home/isaac/wesnoth/wesnoth-" . $branch . "/";
$extrabasedir="/home/isaac/wesnoth/wescamp-i18n/";

//$packages = file_get_contents($basedir . "/po/DOMAINS");
//$packages = substr($packages, 0, strlen($packages)-1);
$corepackages = "wesnoth wesnoth-lib wesnoth-httt wesnoth-tutorial";
//$packages = trim(system("grep ^SUBDIRS " . $basedir . "/po/Makefile.am | cut -d= -f2"));
$packages = "wesnoth wesnoth-editor wesnoth-lib wesnoth-units wesnoth-multiplayer wesnoth-tutorial wesnoth-manpages wesnoth-manual wesnoth-httt wesnoth-did wesnoth-ei wesnoth-l wesnoth-nr wesnoth-sof wesnoth-sotbe wesnoth-trow wesnoth-tb wesnoth-tsg wesnoth-utbs wesnoth-aoi wesnoth-thot";
//get unofficial packages
$dir = opendir($extrabasedir);

$extra_packages = "";
while (false !== ($file = readdir($dir))) { 
	if($file[0] != '.'){
		$packarray[] = $file;
	}
}
sort($packarray);
$extrapackages = implode(" ", $packarray);

//$languages = file_get_contents($basedir . "/po/LINGUAS");
//$languages = substr($languages, 0, strlen($languages)-1);

$prog="grab-stats";

?>
