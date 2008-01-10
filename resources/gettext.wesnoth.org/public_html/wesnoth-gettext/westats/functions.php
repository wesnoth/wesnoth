<?php

////////////////////////////////////////////////////////////////////////////
// $Id: grab-functions.php,v 1.4 2004/02/21 00:45:53 claudiuc Exp $
//
// Description: Common functions for GUI statistics grabbing program
//
////////////////////////////////////////////////////////////////////////////

//
// create a lock file
//
function create_lock() {
  global $prog;
  @touch("/tmp/$prog.lock");
}

//
// remove the lock file
//
function remove_lock() {
  global $prog;
  @unlink("/tmp/$prog.lock");
}

//
// check if the lock file exist
//
function is_locked() {
  global $prog;
  if (@is_file("/tmp/$prog.lock")) {
    return 1;
  } else {
    return 0;
  }
}

function update($basedir, $lang, $package){
	$pofile=$basedir . "/po/" . $lang . "/" . $package . ".po";
	$potfile=$basedir . "/po/" . $package . ".pot";
	echo "msgmerge --update " . $pofile . " " . $potfile;
	@exec("msgmerge --update " . $pofile . " " . $potfile);
}

//
// get statistics from PO file
//
function getstats($file) {
  global $msgfmt;
  
  $translated=0;
  $untranslated=0;
  $fuzzy=0;
  $error=0;
  
  $escfile=escapeshellarg($file);
  @exec("$msgfmt -o /dev/null --statistics $escfile 2>&1",$output,$ret);

  if ($ret==0) {
    // new version of msgfmt make life harder :-/
    if (preg_match("/^\s*(\d+)\s*translated[^\d]+(\d+)\s*fuzzy[^\d]+(\d+)\s*untranslated/",$output[0],$m)) {
    } else if (preg_match("/^\s*(\d+)\s*translated[^\d]+(\d+)\s*fuzzy[^\d]/",$output[0],$m)) {
    } else if (preg_match("/^\s*(\d+)\s*translated[^\d]+(\d+)\s*untranslated[^\d]/",$output[0],$m)) {
      $m[3]=$m[2];
      $m[2]=0;
    } else if (preg_match("/^\s*(\d+)\s*translated[^\d]+/",$output[0],$m)) {
    } else {
      return array(1,0,0,0);
    }
    
    $translated = $m[1]+0;
    $fuzzy = $m[2]+0;
    $untranslated = $m[3]+0;
  } else {
    $error=1;
  }

  return array($error,$translated,$fuzzy,$untranslated);
}

function getdomain($string) {
	return "wesnoth-" . str_replace("-po","",$string);
}

function getpackage($string) {
	return str_replace("wesnoth-","",$string) . "-po";
}

?>
