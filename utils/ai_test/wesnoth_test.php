<?
$sqlitefile = 'RELATIVE PATH TO SQLITE-FILE';
$db = new SQLite3($sqlitefile);
if (!$db) {
    print("Connection Failed.");
    exit;
} else {
#    print("Connection Ok!");
}
$id = $_GET["id"];
$query = "SELECT * FROM tests WHERE id = $id";
$result = $db->query($query) or die('Query failed');
$row = $result->fetchArray();
$title = $row["title"];
$ai_config1 = $row["ai_config1"];
$ai_config2 = $row["ai_config2"];
$ai_ident1 = $row["ai_ident1"];
$ai_ident2 = $row["ai_ident2"];
$map = $row["map"];
$version = $row["version"];
$faction1 = $row["faction1"];
$faction2 = $row["faction2"];
$time = $row["time"];

$query = 'SELECT faction1, faction2,
count(test_id) as total_games,
sum(case when winner = 1 then 1 else 0 end) as side1_won,
sum(case when winner = 2 then 1 else 0 end) as side2_won,
sum(case when winner = 0 then 1 else 0 end) as draw,
sum(case when winner = 1 then 1 else 0 end) * 100. / count(test_id) as side1_won_p,
sum(case when winner = 2 then 1 else 0 end) * 100. / count(test_id) as side2_won_p,
sum(case when winner = 0 then 1 else 0 end) * 100. / count(test_id) as draw_p,
avg(end_turn) as avg_end_turn
FROM "games"
WHERE test_id = ' . $id . '
GROUP BY faction1, faction2';
$result = $db->query($query) or die('Query failed');
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
<h2><?echo $title;?></h2>
<script type='text/javascript' src='https://www.google.com/jsapi'></script>
    <script type='text/javascript'>
      google.load('visualization', '1', {packages:['table']});
      google.setOnLoadCallback(drawTable);
      function drawTable() {
        var data = new google.visualization.DataTable();
        data.addColumn('string', 'Side1 / Side2');
        data.addColumn('number', 'Drakes');
        data.addColumn('number', 'Rebels');
        data.addColumn('number', 'Undead');
        data.addColumn('number', 'Northerners');
        data.addColumn('number', 'Knalgan Alliance');
        data.addColumn('number', 'Loyalists');
        data.addRow(['Drakes', null, null, null, null, null, null]);
        data.addRow(['Rebels', null, null, null, null, null, null]);
        data.addRow(['Undead', null, null, null, null, null, null]);
        data.addRow(['Northerners', null, null, null, null, null, null]);
        data.addRow(['Knalgan Alliance', null, null, null, null, null, null]);
        data.addRow(['Loyalists', null, null, null, null, null, null]);
        <?
        while ($row = $result->fetchArray()) {
          echo "data.setCell(toId('".$row['faction1']."')-1, toId('".$row['faction2']."'), ".$row['side1_won_p'].", '".round($row['side1_won_p'],2)."% / ".round($row['side2_won_p'],2)."% / ".$row['total_games']."');";
        }
        ?>
        var table = new google.visualization.Table(document.getElementById('table_div'));
        var formatter = new google.visualization.ColorFormat();
        formatter.addGradientRange(-0.001, 100.0001, 'black', '#FF3333', '#33FF33');
        formatter.format(data, 1);
        formatter.format(data, 2);
        formatter.format(data, 3);
        formatter.format(data, 4);
        formatter.format(data, 5);
        formatter.format(data, 6);
        table.draw(data, {showRowNumber: false, allowHtml: true});
      }

      function toId(faction) {
        switch(faction) {
        case 'Drakes': return 1;
        case 'Rebels': return 2;
        case 'Undead': return 3;
        case 'Northerners': return 4;
        case 'Knalgan Alliance': return 5;
        case 'Loyalists': return 6;
        }
      }
    </script>
<div id="table_div"> </div>
</div>



<?
$query = "SELECT test_id,
count(games.id) as total_games,
sum(case when winner = 1 then 1 else 0 end) as side1_won,
sum(case when winner = 2 then 1 else 0 end) as side2_won,
sum(case when winner = 0 then 1 else 0 end) as draw,
sum(case when winner = 1 then 1 else 0 end) * 100.0 / count(games.id) as side1_won_p,
sum(case when winner = 2 then 1 else 0 end) * 100.0 / count(games.id) as side2_won_p,
sum(case when winner = 0 then 1 else 0 end) * 100.0 / count(games.id) as draw_p,
avg(games.end_turn) as avg_end_turn
FROM games
WHERE test_id = " . $id . "
GROUP BY test_id";
$result = $db->query($query) or die('Query failed');
$row = $result->fetchArray()
?>

<table>
<tr>
<td><b>ai_ident1</b></td>
<td><?echo $ai_ident1;?></td>
</tr>
<tr>
<td><b>ai_ident2</b></td>
<td><?echo $ai_ident2;?></td>
</tr>
<tr>
<td><b>ai_config1</b></td>
<td><?echo $ai_config1;?></td>
</tr>
<tr>
<td><b>ai_config2</b></td>
<td><?echo $ai_config2;?></td>
</tr>
<tr>
<td><b>map</b></td>
<td><?echo $map;?></td>
</tr>
<tr>
<td><b>time</b></td>
<td><?echo $time;?></td>
</tr>
<tr>
<td><b>version</b></td>
<td><?echo $version;?></td>
</tr>
<tr>
<td><b>total games</b></td>
<td><?echo $row['total_games'];?></td>
</tr>
<tr>
<td><b>side1 won</b></td>
<td><?echo $row['side1_won'] . " / " . round($row['side1_won_p'], 2) . "%";?></td>
</tr>
<tr>
<td><b>side2 won</b></td>
<td><?echo $row['side2_won'] . " / " . round($row['side2_won_p'], 2) . "%";?></td>
</tr>
<tr>
<td><b>draws</b></td>
<td><?echo $row['draw'] . " / " . round($row['draw_p'], 2) . "%";?></td>
</tr>
<tr>
<td><b>average end turn</b></td>
<td><?echo round($row['avg_end_turn'], 1);?></td>
</tr>
</table>
</body>
</html>
