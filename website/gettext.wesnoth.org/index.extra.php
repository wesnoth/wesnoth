<?php

include("config.php");
include("functions.php");
include("langs.php");

global $langs;

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

$official = true;

$existing_packs = explode(" ", $packages);
$existing_extra_packs = explode(" ", $extrapackages);
$firstpack = $existing_packs[0];
$stats = array();
if(!isset($_GET['package'])){
	$package = 'alloff';
}else{
	$package = $_GET['package'];
}

if(!isset($_GET['order']) || $_GET['order'] != 'alpha'){
	$order='trans';
}else{
	$order='alpha';
}

if($package=='alloff'){
	$packs = $existing_packs;
	foreach($packs as $pack){
		if (!file_exists("stats/" . $pack . "/stats")) {
			continue;
		}
		$serialized = file_get_contents("stats/" . $pack . "/stats");
		$tmpstats = array();
		$tmpstats = unserialize($serialized);
		foreach($tmpstats as $lang => $stat){
			if(isset($stats[$lang])){
				$stats[$lang][0]+=$stat[0];
				$stats[$lang][1]+=$stat[1];
				$stats[$lang][2]+=$stat[2];
				$stats[$lang][3]+=$stat[3];
			}else{
				$stats[$lang] = array();
				$stats[$lang][0]=$stat[0];
				$stats[$lang][1]=$stat[1];
				$stats[$lang][2]=$stat[2];
				$stats[$lang][3]=$stat[3];
			}
		}
	}
}elseif($package=='all'){
	for($i = 0; $i < 2; $i++){
		if($i==0){
			$packs = $existing_packs;
		}else{
			$packs = $existing_extra_packs;
		}
		foreach($packs as $pack){
			if($i==1){
				$pack = getdomain($pack);
			}
			if (!file_exists("stats/" . $pack . "/stats")) {
				continue;
			}
			$serialized = file_get_contents("stats/" . $pack . "/stats");
			$tmpstats = array();
			$tmpstats = unserialize($serialized);
			foreach($tmpstats as $lang => $stat){
				if(isset($stats[$lang])){
					$stats[$lang][0]+=$stat[0];
					$stats[$lang][1]+=$stat[1];
					$stats[$lang][2]+=$stat[2];
					$stats[$lang][3]+=$stat[3];
				}else{
					$stats[$lang] = array();
					$stats[$lang][0]=$stat[0];
					$stats[$lang][1]=$stat[1];
					$stats[$lang][2]=$stat[2];
					$stats[$lang][3]=$stat[3];
				}
			}
		}
	}
}else{
	$package = $_GET['package'];
	if (!file_exists("stats/" . $package . "/stats")) {
		$nostats=true;
	}else{
		$serialized = file_get_contents("stats/" . $package . "/stats");
		$stats = unserialize($serialized);
	}
}

if(!$nostats){
	//get total number of strings
	$main_total=$stats["_pot"][1]+$stats["_pot"][2]+$stats["_pot"][3];
	unset($stats["_pot"]);


	$filestat = stat("stats/" . $firstpack ."/stats");
	$date = $filestat[9];

	if($order=='trans'){
		uasort($stats,"cmp_translated");	
	}else{
		uksort($stats,"cmp_alpha");
	}
}
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

<div id="menu">
  <a href="http://www.wesnoth.org/index.htm">Home</a> | <a href="http://www.wesnoth.org/project.htm">Project</a> | <a href="http://www.wesnoth.org/downloads.htm">Downloads</a> | <a href="http://www.wesnoth.org/sshots.htm">Screenshots</a> | <a href="http://www.wesnoth.org/faq.htm">FAQ</a> | <a href="http://wesnoth.slack.it/?WesnothManual">Manual</a> | <a href="http://wiki.wesnoth.org/">Wiki</a> | <a href="http://www.wesnoth.org/forum/">Forum</a> | <a href="http://www.wesnoth.org/wiki/Credits">Credits</a>
  </div>

<h2>Wesnoth translation stats</h2>

<table class="main" cellpadding="1" cellspacing="0" border="0" width="100%"><tr><td>
<table class="title" cellpadding="0" cellspacing="0" border="0" width="100%"><tr><td>
<table cellpadding="2" cellspacing="0" border="0" width="100%">
<tr>
<td align="left">
Order by:
<? if($order=='trans'){ ?>
<strong># of translated strings</strong> || <a href="?order=alpha&amp;package=<?=$package?>">Team name</a>
<? }else{ ?>
<a href="?order=trans&amp;package=<?=$package?>"># of translated strings</a>  || <strong>Team name</strong>
<? } ?>
</td>
<? if(!$nostats){ ?>
<td align="right">
last update: <strong><? echo date("r", $date); ?></strong> &nbsp;
</td>
<? } ?>
</tr>
<tr>
<td align="left">
Show:
<? if($package=='alloff'){ ?>
<strong>All official packages</strong>
<? }else{ ?>
<a href="?package=alloff&amp;order=<?=$order?>">All official packages</a>
<? }
echo "||";
   if($package=='all'){ ?>
<strong>All packages</strong>
<? }else{ ?>
<a href="?package=all&amp;order=<?=$order?>">All packages</a>
<? }
	for($i = 0; $i < 2; $i++){
		if($i==0){
			$packs = $existing_packs;
			echo "<br/>Official: ";
		}else{
			$packs = $existing_extra_packs;
			echo "<br/>Unofficial: ";
		}
		$first=true;
		foreach($packs as $pack){
			if($first){
				$first = false;
			}else{
				echo "||";
			}

			if($i==1){
				$pack = getdomain($pack);
			}
			if($pack==$package){ 
				if($i==1){
					$official=false;
				}
			?>
				<strong><?=$pack?></strong>
			<? }else{ ?>
			<a href="?package=<?=$pack?>&amp;order=<?=$order?>"><?=$pack?></a> <?
			}
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
<? if($order=='trans'){ ?>
<td class="title">position</td>
<? } ?>
<td class="title">team name</td>
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
$pos=1;
$oldstat[0]=0;
$oldstat[1]=0;
$oldstat[2]=0;

foreach($stats as $lang => $stat){
	$total = $stat[1] + $stat[2] + $stat[3];

	$class="-" . ($i%2);
	if(cmp_translated($stat, $oldstat)!=0){
		$pos=$i+1;
	}
?>
<tr class="row<?=$class?>">
<?
	if($order=='trans'){ ?>
<td align="right"><?=($pos)?></td>
<?	}
?>
    <td>
<? 
if ($package=='alloff' || $package=='allun' || $package=='all' || $package=='allcore'){
	echo "<strong>" . $langs[$lang] . "</strong> (" . $lang . ")";
}else{
	$repo = ($version == 'trunk') ? 'trunk' : "branches/$branch";
	if($official){
		echo "<a href='http://svn.gna.org/viewcvs/*checkout*/wesnoth/$repo/po/" . $package . "/" . $lang . ".po?view=markup'>" . $langs[$lang] . "</a> (" .$lang . ")";
	}else{
		echo "<a href='http://svn.berlios.de/viewcvs/wescamp-i18n/$repo/" . getpackage($package) . "/po/" . $lang . ".po?view=markup'>" . $langs[$lang] . "</a> (" . $lang . ")";
	}
} ?>
	</td>
<? if(($stat[0]==1) || ($total == 0)){ ?>
	<td colspan="8">Error in <? echo $langs[$lang]; ?> translation files</td>
<? }else{ ?>
    <td align="right"><? echo $stat[1]; ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", ($stat[1]*100)/$main_total); ?></td>
    <td align="right"><? echo $stat[2]; ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", ($stat[2]*100)/$main_total); ?></td>
    <td align="right"><? echo ($main_total - $stat[1] - $stat[2]); ?></td>
    <td class="percentage<?=$class?>" align="right"><? printf("%0.2f", (($main_total-$stat[1]-$stat[2])*100)/$main_total); ?></td>
    <td align="right"><? echo $total; ?></td>
    <? $trans = sprintf("%d", ($stat[1]*200)/$main_total);?>
    <? $fuzzy = sprintf("%d", ($stat[2]*200)/$main_total);?>
    <? $untrans = 200 - $trans - $fuzzy;?>
    <td><img src="images/green.png" height="15" width="<?=$trans?>" alt="translated"/><img src="images/blue.png" height="15" width="<?=$fuzzy?>" alt="fuzzy"/><img src="images/red.png" height="15" width="<?=$untrans?>" alt="untranslated"/></td>
<? } ?>
		    </tr>
<?
	$i++;
	$oldstat = $stat;
}

?>
<tr class="title">
<?
	if($order=='trans'){ ?>
<td align="right"></td>
<?	}
?>
    <td>
<?
if ($package=='alloff' || $package=='allun' || $package=='all' || $package=='allcore'){
	echo "<strong>Template catalog</strong>";
}else{
	$repo = ($version == 'trunk') ? 'trunk' : "branches/$branch";
	if($official){
		echo "<a href='http://svn.gna.org/viewcvs/*checkout*/wesnoth/$repo/po/" . $package . "/" . $package . ".pot?view=markup'>Template catalog</a>";
	}else{
		echo "<a href='http://svn.berlios.de/viewcvs/wescamp-i18n/$repo/" . getpackage($package) . "/po/" . $package . ".pot?view=markup'>Template catalog</a>";
	}
}
?></td>
    <td align="right"></td>
    <td align="right"></td>
    <td align="right"></td>
    <td></td>
    <td align="right"></td>
    <td></td>
    <td align="right"><? echo $main_total; ?></td>
    <td></td>
		    </tr>
</table>
</td>
</tr>
</table>
<? }else{ ?>
<h2>No available stats for package <?=$package?></h2>
<? } ?>
<div> <br/> </div>
<div id="footer">
<div id="footnote">
&copy; 2003-2008 The Battle for Wesnoth<br/>
<br/>
<a href="http://validator.w3.org/check?uri=referer"><img
src="http://www.w3.org/Icons/valid-xhtml10"
alt="Valid XHTML 1.0!" height="31" width="88" /></a>
</div>
</div>

</body>
</html>
