<?php

////////////////////////////////////////////////////////////////////////////
// $Id: grab-config.php,v 1.4 2003/08/12 20:01:30 claudiuc Exp $
//
// Description: Configuration file for GUI statistics grabbing program
//
////////////////////////////////////////////////////////////////////////////

// the msgfmt program path
$msgfmt="/usr/bin/msgfmt";

$branch="1.4";

$trunkbasedir="/public/vhost/w/wesnoth/source/trunk/";
$branchbasedir="/public/vhost/w/wesnoth/source/" . $branch . "/";
//$extrabasedir="/public/vhost/w/wesnoth/source/wescamp-i18n/";

//$packages = file_get_contents($basedir . "/po/DOMAINS");
//$packages = substr($packages, 0, strlen($packages)-1);
$corepackages = "wesnoth wesnoth-lib wesnoth-units wesnoth-multiplayer wesnoth-httt wesnoth-tutorial";
//$packages = trim(system("grep ^SUBDIRS " . $basedir . "/po/Makefile.am | cut -d= -f2"));
$packages = "wesnoth wesnoth-lib wesnoth-editor wesnoth-units wesnoth-multiplayer wesnoth-tutorial wesnoth-manpages wesnoth-manual wesnoth-aoi wesnoth-did wesnoth-ei wesnoth-httt wesnoth-l wesnoth-nr wesnoth-sof wesnoth-sotbe wesnoth-tb wesnoth-thot wesnoth-trow wesnoth-tsg wesnoth-utbs";

//get unofficial packages
// TODO:
// reactivate when stuff adjusted for new infrastructure
// deactivated for not
//$dir = opendir($extrabasedir);
//
//$extra_packages = "";
//while (false !== ($file = readdir($dir))) { 
//	if($file[0] != '.'){
//		$packarray[] = $file;
//	}
//}
//sort($packarray);
//$extrapackages = implode(" ", $packarray);


//$languages = file_get_contents($basedir . "/po/LINGUAS");
//$languages = substr($languages, 0, strlen($languages)-1);

$prog="grab-stats";

?>
