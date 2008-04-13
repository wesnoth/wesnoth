<?php

include("config.php");
include("functions.php");
include("langs.php");

global $langs;
global $branch;

function cmp_translated($a, $b){
	if($a[1]==$b[1]){
		if($a[2]==$b[2]){
			return 0;
		}
		return ($a[2] < $b[2]) ? 1 : -1;
	}
	return ($a[1] < $b[1]) ? 1 : -1;
}

function cmp_alpha($a, $b){
	return strcmp($langs[$a],$langs[$b]);
}

$existing_packs = explode(" ", $packages);
$existing_corepacks = explode(" ", $corepackages);
$existing_extra_packs_t = explode(" ", $extratpackages);
$existing_extra_packs_b = explode(" ", $extrabpackages);
$stats = array();

if(!isset($_GET['version'])){
        $version = 'trunk';
}else{
	$version = $_GET['version'];
}

if(!isset($_GET['lang'])){
        $lang = '';
}else{
	$lang = $_GET['lang'];
}

if($lang != "") {
	$j = 0;
	for($i = 0; $i < 2; $i++){
		if($i==0){
			$packs = $existing_packs;
		}else{
			$packs = ($version == 'trunk') ? $existing_extra_packs_t : $existing_extra_packs_b;
		}
		foreach($packs as $pack){
			if($i==1) $pack = getdomain($pack);
			$statsfile = $version . "stats";
			if (!file_exists("stats/" . $pack . "/" . $statsfile)) {
				continue;
			}
			$serialized = file_get_contents("stats/" . $pack . "/" . $statsfile);
			$tmpstats = array();
			$tmpstats = unserialize($serialized);
			$stat = $tmpstats[$lang];
			$stats[$j] = array();
			$stats[$j][0]=$stat[0];	//errors
			$stats[$j][1]=$stat[1];	//translated
			$stats[$j][2]=$stat[2];	//fuzzy
			$stats[$j][3]=$stat[3];	//untranslated
			$stats[$j][4]=$pack;	//package name
			$stats[$j][5]=$tmpstats["_pot"][1]+$tmpstats["_pot"][2]+$tmpstats["_pot"][3];
			$stats[$j][6]=$i;	//official

			$j++;
		}
	}
	$nostats = 0;
} else {
	$nostats = 1;
	unset($lang);
}
$firstpack = $existing_packs[0];
$statsfile = $version . "stats";
$filestat = stat("stats/" . $firstpack . "/" . $statsfile);
$date = $filestat[9];
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
	<meta http-equiv="content-type" content="text/xhtml; charset=utf-8" />
	<link rel="shortcut icon" type="image/png" href="http://www.wesnoth.org/mw/skins/glamdrol/ico.png" />
	<style type="text/css">@import url('http://www.wesnoth.org/mw/skins/glamdrol/main.css');</style>
	<link rel="stylesheet" type="text/css" href="styles/old.css" />
	<title>Battle for Wesnoth</title>
</head>
<body>
<div id="global">
<div id="header">
<div id="logo">
<a href="http://www.wesnoth.org/"><img alt="Wesnoth logo" src="http://www.wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg" /></a>
</div>
</div>

<div id="nav">
	<ul>
	<li><a href="http://www.wesnoth.org/">Home</a></li>
	<li><a href="http://www.wesnoth.org/wiki/Play">Play</a></li>
	<li><a href="http://www.wesnoth.org/wiki/Create">Create</a></li>
	<li><a href="http://www.wesnoth.org/forum/">Forums</a></li>
	<li><a href="http://www.wesnoth.org/wiki/Support">Support</a></li>
	<li><a href="http://www.wesnoth.org/wiki/Project">Project</a></li>
	<li><a href="http://www.wesnoth.org/wiki/Credits">Credits</a></li>
	</ul>
</div>

<h2 style="display:inline">Wesnoth translation stats</h2>
(last update: <strong><? echo date("r", $date); ?></strong>)

<table class="main" cellpadding="1" cellspacing="0" border="0" width="100%"><tr><td>
<table class="title" cellpadding="0" cellspacing="0" border="0" width="100%"><tr><td>
<table cellpadding="2" cellspacing="0" border="0" width="100%">
<tr>
<td align="left">
Version:
<? if($version=='trunk'){ ?>
<strong>Development</strong> || <a href="?version=branch&amp;package=<?=$package?>&amp;lang=<?=$lang?>"><?=$branch?></a>
<? }else{ ?>
<a href="?version=trunk&amp;package=<?=$package?>&amp;lang=<?=$lang?>">Development</a>  || <strong><?=$branch?></strong>
<? } ?>
</td>
</tr>
<tr>
<td align="left">
Show:
<a href="index.php?package=alloff&amp;order=trans">Official packages</a>
|| <a href="index.php?package=allcore&amp;order=trans">Official core packages</a>
|| <a href="index.php?package=all&amp;order=trans">All packages</a>
|| <a href="index.php?package=allun&amp;order=trans">All unofficial packages</a>
|| <strong>By language</strong>
<br/>
Language:
<?
	$first=true;
	foreach($langs as $code => $langname){
		if($first){
			$first = false;
		}else{
			echo "||";
		}

		if($code==$lang){ 
?>
			<strong><?=$langname?></strong>
<? 		}else{ ?>
			<a href="?lang=<?=$code?>&amp;version=<?=$version?>"><?=$langname?></a> <?
		}
	}
?>
</td>
</tr>
</table>
</td></tr></table>
</td></tr></table>
<div> <br/> </div>
<? if(!$nostats){ ?>
<table class="main" cellspacing="0" cellpadding="0" border="0" width="100%"><tr><td>
<table cellspacing="1" cellpadding="2" border="0" width="100%">
<tr class="header">
<td class="title">package</td>
<td class="translated">translated</td>
<td class="translated">%</td>
<td class="fuzzy"><strong>fuzzy</strong></td>
<td class="fuzzy"><strong>%</strong></td>
<td class="untranslated"><strong>untranslated</strong></td>
<td class="untranslated"><strong>%</strong></td>
<td class="title">total</td>
<td class="title">graph</td>
</tr>
<?
$i=0;
$sumstat[1]=0;
$sumstat[2]=0;
$sumstat[3]=0;
$sumstat[5]=0;

$oldofficial = 0;	//0 == official, 1 == not official

foreach($stats as $stat){
	$oldofficial = $official;
	$official = $stat[6];

	$sumstat[1]+=$stat[1];
	$sumstat[2]+=$stat[2];
	$sumstat[3]+=$stat[3];
	$sumstat[5]+=$stat[5];

	$total = $stat[1] + $stat[2] + $stat[3];

	$class="-" . ($i%2);
	if($oldofficial != $official) {
?>
	<tr>
	<td></td>
	</tr>
<?
	}
?>
<tr class="row<?=$class?>">
    <td>
<? 
	if($official == 0){
		if($version == 'trunk') {
			$repo = 'trunk';
		}else{
			$repo = "branches/$branch";
		}
		echo "<strong><a href='http://svn.gna.org/viewcvs/*checkout*/wesnoth/$repo/po/" . $stat[4]. "/" . $lang . ".po'>" . $stat[4] . "</a></strong>";
	}else{
		$repo = ($version == 'trunk') ? 'trunk' : "branches/$branch";
		echo "<strong><a href='http://svn.berlios.de/viewcvs/*checkout*/wescamp-i18n/$repo/" . getpackage($stat[4]) . "/po/" . $lang . ".po'>" . $stat[4] . "</a></strong>";
	}

?>
	</td>
<? if(($stat[0]==1) || ($total == 0)){ ?>
	<td colspan="8">Error in <? echo $stat[4] ; ?> translation files</td>
<? }else{ ?>
    <td align="right"><? echo $stat[1]; ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", ($stat[1]*100)/$stat[5]); ?></td>
    <td align="right"><? echo $stat[2]; ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", ($stat[2]*100)/$stat[5]); ?></td>
    <td align="right"><? echo ($stat[5] - $stat[1] - $stat[2]); ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", (($stat[5]-$stat[1]-$stat[2])*100)/$stat[5]); ?></td>
    <td align="right"><? echo $total; ?></td>
    <? $trans = sprintf("%d", ($stat[1]*200)/$stat[5]);?>
    <? $fuzzy = sprintf("%d", ($stat[2]*200)/$stat[5]);?>
    <? $untrans = 200 - $trans - $fuzzy;?>
    <td><img src="images/green.png" height="15" width="<?=$trans?>" alt="translated"/><img src="images/blue.png" height="15" width="<?=$fuzzy?>" alt="fuzzy"/><img src="images/red.png" height="15" width="<?=$untrans?>" alt="untranslated"/></td>
<? } ?>
		    </tr>
<?
	$i++;
}

?>
<tr class="title">
</td>
    <td>Total</td>
    <td align="right"><? echo $sumstat[1]; ?></td>
    <td></td>
    <td align="right"><? echo $sumstat[2]; ?></td>
    <td></td>
    <td align="right"><? echo $sumstat[3]; ?></td>
    <td></td>
    <td align="right"><? echo $sumstat[5]; ?></td>
    <td></td>
</tr>
</table>
</td>
</tr>
</table>
<? }else{
	if(isset($lang)){
?>
<h2>No available stats for lang <?=$lang?></h2>
<?
	}
} ?>
<div> <br/> </div>
<div id="footer">
<div id="footnote">
&copy; 2003-2005 The Battle for Wesnoth<br/>
<br/>
<a href="http://validator.w3.org/check?uri=referer"><img
src="http://www.w3.org/Icons/valid-xhtml10"
alt="Valid XHTML 1.0!" height="31" width="88" /></a>
</div>
</div>

</body>
</html>
