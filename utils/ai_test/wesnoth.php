<?
$sqlitefile = 'RELATIVE PATH TO SQLITE-FILE';
?>
<html>
<head>
<title>Wesnoth AI Testing Statistics</title>
<link rel="stylesheet" type="text/css" href="http://wiki.wesnoth.org/skins/glamdrol/main.css">
<style>
thead th, thead td {
  text-align: center;
}
tbody th, tbody td {
  text-align: center;
}
tfoot th, tfoot td {
  text-align: center;
}
table {
  margin: auto;
}
div {
  text-align:center;
}
</style>
</head>
<body>
<div id="main">
<?php
$db = new SQLite3($sqlitefile);
if (!$db) {
    print("Connection Failed.");
    exit;
} else {
#    print("Connection Ok!");
}
?>
<h2>Ai-Test Suite - Test results</h2>
<table>
<tr>
<th>title</th>
<th>ai_ident1</th>
<th>ai_ident2</th>
<th>games</th>
<th>1 won</th>
<th>2 won</th>
<th>draws</th>
</tr>
<?
$query = "SELECT test_id, title, ai_config1, ai_config2, ai_ident1, ai_ident2, map, tests.faction1, tests.faction2, time,
count(games.id) as total_games,
sum(case when winner = 1 then 1 else 0 end) as side1_won,
sum(case when winner = 2 then 1 else 0 end) as side2_won,
sum(case when winner = 0 then 1 else 0 end) as draw,
sum(case when winner = 1 then 1 else 0 end) * 100.0 / count(games.id) as side1_won_p,
sum(case when winner = 2 then 1 else 0 end) * 100.0 / count(games.id) as side2_won_p,
sum(case when winner = 0 then 1 else 0 end) * 100.0 / count(games.id) as draw_p,
avg(games.end_turn) as avg_end_turn
FROM tests, games
WHERE tests.id = games.test_id
GROUP BY test_id
ORDER BY test_id DESC";
$result = $db->query($query) or die('Query failed');
while ($row = $result->fetchArray())
{
        $title = (strlen($row['title']) > 30) ? substr($row['title'], 0, 30) . "..." : $row['title'];
	echo "<tr>";
        echo "<td><a href=wesnoth_test.php?id=" . $row['test_id'] . ">{$title}</a></td>";
	echo "<td>{$row['ai_ident1']}</td>";
	echo "<td>{$row['ai_ident2']}</td>";
	echo "<td>{$row['total_games']}</td>";
	echo "<td>" . round($row['side1_won_p'], 2) . "%</td>";
	echo "<td>" . round($row['side2_won_p'], 2) . "%</td>";
	echo "<td>" . round($row['draw_p'], 2) . "%</td>";
	echo "</tr>";
}
?>
</table>
</div>
</body>
</html>
