<html>
<head>
<title>Wesnoth AI Testing Statistics</title>
</head>
<body>
<?php
  $database = pg_connect("host=127.0.0.1 dbname=org.wesnoth.ai.test user=wesnoth_ai_test_viewer_impl password=PASSWORD");
  if (!$database) {
    print("Connection Failed.");
    exit;
  } else {
#    print("Connection Ok!");
  }
?>
<h2>Latest svn AI wins % graph:</h2>
<?php
$query  = "select ai_ident_me,avg_from(sum(win)*100,count(*))::bigint as win_percent from games_side where svn_release=(select max(svn_release) from games_side) group by ai_ident_me order by ai_ident_me;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
 $a = "";
 $b = "";
 $i = true;
 while($myrow = pg_fetch_assoc($result)) {
     if ($i) {
       $a=$a.$myrow['ai_ident_me'];
       $b=$b.$myrow['win_percent'];
       $i=false;
     } else {
       $a=$a."|".$myrow['ai_ident_me'];
       $b=$b.",".$myrow['win_percent'];
     }
 }

printf("<img src=\"http://chart.apis.google.com/chart?cht=p3&amp;chd=t:$b&amp;chs=500x200&amp;chl=$a\" alt=\"win percentages\" title=\"win percentages\"");
?>

<h2>By AI:</h2> 
<table border=1>
<tr>
<th>AI</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, svn_release order by ai_ident_me, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>
<h2>By AI and side</h2>
<table border=1>
<tr>
<th>AI</th>
<th>Side</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, my_side, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, my_side, svn_release order by ai_ident_me, my_side, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['my_side'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>
<h2>By AI and map</h2>
<table border=1>
<tr>
<th>AI</th>
<th>Map</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, map, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, map, svn_release order by ai_ident_me, map, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['map'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>
<h2>By AI and own faction</h2>
<table border=1>
<tr>
<th>AI</th>
<th>Own faction</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, faction_me, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, faction_me, svn_release order by ai_ident_me, faction_me, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['faction_me'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>
<h2>By AI and enemy faction</h2>
<table border=1>
<tr>
<th>AI</th>
<th>Enemy faction</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, faction_enemy, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, faction_enemy, svn_release order by ai_ident_me, faction_enemy, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['faction_enemy'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>
<h2>By AI and factions</h2>
<table border=1>
<tr>
<th>AI</th>
<th>Own faction</th>
<th>Enemy faction</th>
<th>SVN Release</th>
<th>Win %</th>
<th>Games</th>
<th>Wins</th>
<th>Losses</th>
<th>Avg. turns to win</th>
<th>Avg. turns to lose</th>
</tr>
<?php
$query  = "select ai_ident_me, faction_me, faction_enemy, svn_release, avg_from(sum(win)*100,count(*)) as win_percent, count(*) as games, sum(win) as wins, sum(draw) as draws, sum(loss) as losses, avg_from(sum(win_turns),sum(win)) as avg_win_turns, avg_from(sum(loss_turns),sum(loss)) as avg_loss_turns from games_side group by ai_ident_me, faction_me, faction_enemy ,svn_release order by ai_ident_me, faction_me, faction_enemy, svn_release desc;";
 $result = pg_query($query);
 if (!$result) {
            echo pg_last_error();
            exit();
        }
	
while($myrow = pg_fetch_assoc($result)) {
	printf ("<tr><td>%s</td><td>%s</td><td>%s</td><td>%d</td><td>%.1f</td><td>%d</td><td>%d</td><td>%d</td><td>%.1f</td><td>%.1f</td></tr>",  
	$myrow['ai_ident_me'],$myrow['faction_me'],$myrow['faction_enemy'],$myrow['svn_release'],$myrow['win_percent'],$myrow['games'],$myrow['wins'],$myrow['losses'],$myrow['avg_win_turns'],$myrow['avg_loss_turns']);
}
  ?>
</table>

</body>
