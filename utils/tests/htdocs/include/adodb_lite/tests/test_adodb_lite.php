<?php
if(empty($_POST['databasetype']))
{
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title>ADOdb Lite Test Program</title>
<style type="text/css">
<!--
.style2 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
	color: #FF0000;
}
.style10 {
	font-family: Verdana, Arial, Helvetica, sans-serif; 
	font-weight: bold; 
	font-size: 16px; 
	color: #000000; 
}
.style11 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 10px;
	font-weight: bold;
}
-->
</style>
</head>

<body>
<table width="800"  border="1" cellspacing="1" cellpadding="5" align="center">
  <tr>
    <td><div align="center">
      <p>
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Test Program</u></span><br>
      </p>
    </div></td>
  </tr>
  <tr>
    <td valign="top">
<form action="test_adodb_lite.php" method="POST" enctype="multipart/form-data">
	<table width="80%"  border="0" align="center" cellpadding="10" cellspacing="1">
      <tr>
        <td valign="middle"><div align="right"><span class="style2">Select your Database Type</span></div></td>
        <td valign="middle"><div align="left"><span class="style2">
            <select name="databasetype">
                <option value="mysql" selected>MySql</option>
                <option value="mysqli">MySqli</option>
                <option value="MySqlt">MySqlt</option>
                <option value="fbsql">Front Base</option>
                <option value="maxdb">Max DB</option>
                <option value="msql">Mini Sql</option>
                <option value="mssql">Microsoft SQL</option>
                <option value="mssqlpo">Microsoft SQL Pro</option>
                <option value="postgres">Postgres</option>
                <option value="postgres64">Postgres 6.4</option>
                <option value="postgres7">Postgres 7</option>
                <option value="postgres8">Postgres 8</option>
                <option value="sqlite">SQLite</option>
                <option value="sqlitepo">SQLite Pro</option>
                <option value="sybase">SyBase</option>
                <option value="sybase_ase">SyBase ASE</option>
            </select>
        </span></div></td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Load the Transaction Module</span></div></td>
        <td width="50%"><input type="radio" name="transactions" value="" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="transactions" value="transaction:">Yes</td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Load the ADOdb Lite Module</span></div></td>
        <td width="50%"><input type="radio" name="adodblite" value="" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="adodblite" value="adodblite:">Yes</td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Load the Extend Module</span></div></td>
        <td width="50%"><input type="radio" name="extend" value="" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="extend" value="extend:">Yes</td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Load the Date Module</span></div></td>
        <td width="50%"><input type="radio" name="date" value="" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="date" value="date:">Yes</td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Test DSN Connection</span></div></td>
        <td width="50%"><input type="radio" name="dsn_connection" value="0" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="dsn_connection" value="1">Yes</td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Database Name</span></div></td>
        <td width="50%"><input type="text" name="databasename"></td>
      </tr>
      <tr>
        <td width="50%"><div align="right"><span class="style2">Database User Name</span></div></td>
        <td>
          <input type="text" name="dbusername">
        </td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2"> Database User Password </span></div></td>
        <td><input type="text" name="dbpassword"></td>
      </tr>
      <tr>
        <td><div align="right"><span class="style2">Database Host </span></div></td>
        <td><input type="text" name="dbhost" value="localhost"></td>
      </tr>
      <tr>
        <td colspan="2"><div align="center">
          <input type="submit" name="Submit Form" value="Submit">
        </div></td>
        </tr>    </table>
</form>
</td>
  </tr>
  <tr>
    <td><div align="center">
      <p align="left"><span class="style11">This program will test your install of ADOdb Lite by testing all of the functions/commands 
        that are provided by this package. The test will perform over 20
        different tests of all functions.<br>
        <br>
		The Pear Module will automatically be loaded and tested.<br>
        <br>
		The program will attempt to create the database if it does not exist.  If you see 
		<u>Could not connect to the database.</u> then you will need to create a database 
		or use an existing database to perform this test. <br>
        <br>
        If you are testing a database other than MySql, MySqli or MySqlt then 
        you will need to create a database table called 'adodb_lite_test' 
        with three fields.<br>
        <br>
        Field 1 = id - int(11) auto_increment<br>
        Field 2 = text - longtext or text<br>
        Field 3 = dummy - longtext or text<br>
        <br>
        Here is what the MySql create table query looks like.<br>
        <br>
        CREATE TABLE IF NOT EXISTS `adodb_lite_test` (<br>
        `id` int(11) NOT NULL auto_increment,<br>
        `text` longtext,<br>
        `dummy` longtext,<br>
        PRIMARY KEY (`id`)<br>
        ) TYPE=MyISAM<br>
        <br>
        If you are testing on MySql, MySqli or MySqlt the table will be created automatically.</span><br>
      </p>
      </div></td>
  </tr></table>
<?php
}
else
{

if(empty($_POST['dbhost']) || empty($_POST['dbusername']) || empty($_POST['databasename']))
{
	header("location: test_adodb_lite.php?databasetype= \n");
	die();
}

require_once '../adodb.inc.php'; 

if($_POST['dsn_connection'])
{
	$dsn = $_POST['databasetype'] . "://" . $_POST['dbusername'] . ":" . $_POST['dbpassword'] . "@" . $_POST['dbhost'] . "/" . $_POST['databasename'] . "#" . $_POST['transactions'] . $_POST['extend'] . $_POST['date'] . $_POST['adodblite'] . "pear";
	$db = ADONewConnection($dsn);
	if(!$db)
	{
		die("Could not connect to the database.");
	}
}
else
{
	$db = ADONewConnection($_POST['databasetype'], $_POST['transactions'] . $_POST['extend'] . $_POST['date'] . $_POST['adodblite'] . "pear");
	$db->createdatabase = true;
	$result = $db->Connect( $_POST['dbhost'], $_POST['dbusername'], $_POST['dbpassword'], $_POST['databasename'] );
	if(!$result)
	{
		die("Could not connect to the database.");
	}
}

//$db->debug = true;

$ADODB_FETCH_MODE = ADODB_FETCH_ASSOC;
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title>ADOdb Lite Test Program</title>
<style type="text/css">
<!--
.style0 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
}
.style1 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-weight: bold;
	font-size: 14px;
	color: #000000;
}
.style2 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
	color: #FF0000;
}
.style3 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
	background-color: #E9E9E9;
}
.style4 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
	background-color: #E9E9E9;
	color: #FF0000
}
.style5 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: normal;
	background-color: #E9E9E9;
	color: #008080;
}
.style6 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: bold;
	background-color: #E9E9E9;
	color: #0000FF;
}
.style7 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: normal;
	background-color: #E9E9E9;
	color: #000000;
}
.style8 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: normal;
	background-color: #E9E9E9;
	color: #008000;
}
.style10 {
	font-family: Verdana, Arial, Helvetica, sans-serif; 
	font-weight: bold; 
	font-size: 16px; 
	color: #000000; 
}
.style11 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 9px;
	font-weight: bold;
}
.style12 {
	font-family: Verdana, Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight: normal;
	background-color: #E9E9E9;
	color: #0000FF;
}
-->
</style>
</head>

<body>
<table width="800"  border="1" cellspacing="1" cellpadding="5" align="center">
  <tr>
    <td colspan="2" valign="middle"><div align="center">
      <p>
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Test Program</u><br>(<?=$_POST['databasetype'];?>)<br>
<?php
if($_POST['dsn_connection'])
{
	echo "<br>DSN: $dsn<br>";
}
?>
</span>      </p>
    </div></td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Drop and Create Table. Tests Execute() and SelectDB(). </p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo</span> <span class="style5">'Selecting Database: ' . $_POST['databasename'] . ' &lt;br&gt;'</span>;<br>
$res = $db</span><span class="style7">-&gt;SelectDB</span><span class="style3">(</span>$_POST['databasename']<span class="style3">);<br>
<?php
if($_POST['databasetype'] == 'mysql' || $_POST['databasetype'] == 'mysqli' || $_POST['databasetype'] == 'mysqlt' || $_POST['databasetype'] == 'sqlite')
{
?>
<span class="style6">echo</span> <span class="style5">'Dropping: adodb_lite_test &lt;br&gt;'</span>;<br>
$res = $db</span><span class="style7">-&gt;Execute</span><span class="style3">(</span><span class="style5">&quot;DROP TABLE IF EXISTS `adodb_lite_test`&quot;</span><span class="style3">);<br>
<span class="style6">echo</span> <span class="style5">'Creating table: adodb_lite_test '&lt;br&gt;'</span>;<br>
$res = $db<span class="style7">-&gt;Execute</span>(</span><span class="style5">&quot;CREATE TABLE IF NOT EXISTS `adodb_lite_test` (<br>
`id` int(11) NOT NULL auto_increment,<br>
`text` longtext,<br>
`dummy` longtext,<br>
PRIMARY KEY (`id`)<br>
) TYPE=MyISAM&quot;</span><span class="style3">);<br>
<?php
}
?>
      </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0">    <?php
echo 'Selecting Database: ' . $_POST['databasename'] . ' <br>';
$res = $db->SelectDB($_POST['databasename']);

if($_POST['databasetype'] == 'mysql' || $_POST['databasetype'] == 'mysqli' || $_POST['databasetype'] == 'mysqlt')
{
	echo 'Dropping: adodb_lite_test <br>';
	$res = $db->Execute("DROP TABLE IF EXISTS `adodb_lite_test`");
	echo 'Creating table: adodb_lite_test <br>';
	$res = $db->Execute("CREATE TABLE IF NOT EXISTS `adodb_lite_test` (
	`id` int(11) NOT NULL auto_increment,
	`text` longtext,
	`dummy` longtext,
	PRIMARY KEY (`id`)
	) TYPE=MyISAM");
}

if($_POST['databasetype'] == 'sqlite')
{
	echo 'Dropping: adodb_lite_test <br>';
	$res = $db->Execute("DROP TABLE adodb_lite_test");
	echo 'Creating table: adodb_lite_test <br>';
	$res = $db->Execute("CREATE TABLE adodb_lite_test (
	id DECIMAL(5) NOT NULL,
	text VARCHAR(250),
	dummy VARCHAR(250),
	PRIMARY KEY ( id )
	)");
}
?>
</p>
</td>
    <td width="50%" valign="top"><p class="style2">Insert into  Table. Tests Execute(), InsertID() and AffectedRows(); </p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
</span><span class="style6">for</span><span class="style3">( $i = </span><span class="style8">1</span><span class="style3">; $i &lt;= </span><span class="style8">12</span><span class="style3">; $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$variable = <span class="style6">mt_rand</span>( <span class="style8">100</span>, <span class="style8">999</span> ) . <span class="style5">'_count_'</span> . $i; <br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">&quot;insert into adodb_lite_test (text) values ('$variable')&quot;</span><span class="style3"> ); <br>
&nbsp;&nbsp;&nbsp;&nbsp;</span><span class="style6">echo</span> <span class="style5">'Insert: '</span><span class="style3"> . $i . </span><span class="style5">' - '</span><span class="style3">;<br>
&nbsp;&nbsp;&nbsp;&nbsp;</span><span class="style6">echo</span> <span class="style5">'Insert ID: '</span><span class="style3"> . $db</span><span class="style7">-&gt;Insert_ID</span><span class="style3">() . <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;</span><span class="style6">echo</span> <span class="style5">'Affected Rows: '</span><span class="style3"> . $db</span><span class="style7">-&gt;Affected_Rows</span><span class="style3">() . <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
</span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
for( $i = 1; $i <= 12; $i++ )
{
	$variable = mt_rand( 100, 999) . '_count_' . $i;
	$rs = $db->execute( "insert into adodb_lite_test (text) values ('$variable')" );
	echo 'Insert: ' . $variable . ' - ';
	echo 'Insert ID: ' . $db->Insert_ID() . ' - ';
	echo 'Affected Rows: ' . $db->Affected_Rows() . '<br>';
}

/*
$data = $db->GetAll( "SHOW TABLE STATUS FROM " . $_POST['databasename']. " LIKE 'adodb_lite_test'" );
echo "Update Time: " . $data[0]['Update_time'] ."<br>";
*/
?>
</p>
</td>
  </tr>
<?php
// Extend Module Test Start
if($_POST['extend'])
{
?>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">Extend Module Test</p></div><p class="style2">Retrieve Associative Array. Tests $db->GetAssoc().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetAssoc</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">foreach</span> ( $data as $dataarray )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">print_r</span>($dataarray);<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">"&lt;br&gt;"</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$data = $db->GetAssoc('select * from adodb_lite_test');
foreach($data as $dataarray) {
	print_r($dataarray);
	echo "<br>";
}
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">Extend Module Test</p></div>
<p class="style2">Retrieve Associative Array. Tests $rs->GetAssoc().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
$data = $rs<span class="style7">-&gt;GetAssoc</span>();<br>
  <span class="style6">foreach</span> ( $data as $dataarray )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">print_r</span>($dataarray);<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">"&lt;br&gt;"</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$data = $rs->GetAssoc();
foreach($data as $dataarray) {
	print_r($dataarray);
	echo "<br>";
}
?>
</p>
	</td>
  </tr>
<?php
}
// Extend Module Test End
?>
<?php
// ADOdb Lite Module Test Start
if($_POST['adodblite'])
{
?>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">ADOdb Lite Module Test</p></div><p class="style2">Retrieve data to array based upon field data. Tests $db->GetToFieldArray().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetToFieldArray</span>( <span class="style5">'select * from adodb_lite_test'</span>, <span class="style5">''</span>, <span class="style5">'text'</span> );<br>
  <span class="style6">foreach</span> ( $data as $key => $dataarray )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> $key . <span class="style5">": "</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">print_r</span>($dataarray);<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">"&lt;br&gt;"</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$data = $db->GetToFieldArray('select * from adodb_lite_test','','text');
foreach($data as $key => $dataarray) {
	echo $key . ": ";
	print_r($dataarray);
	echo "<br>";
}
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">ADOdb Lite Module Test</p></div>
<p class="style2">Retrieve data to array based upon field data. Tests $rs->GetToFieldArray().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
$data = $rs<span class="style7">-&gt;GetToFieldArray</span>(<span class="style5">'text'</span>);<br>
  <span class="style6">foreach</span> ( $data as $key => $dataarray )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> $key . <span class="style5">": "</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">print_r</span>($dataarray);<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">"&lt;br&gt;"</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$data = $rs->GetToFieldArray("text");

foreach($data as $key => $dataarray) {
	echo $key . ": ";
	print_r($dataarray);
	echo "<br>";
}
?>
</p>
	</td>
  </tr>
<?php
}
// ADOdb Lite Module Test End
?>
<?php
// Date Module Test Start
if($_POST['date'])
{
?>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
	<p class="style2">Format the $date in the format the database accepts. Tests $db->DBDate().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$unixtime = <span class="style6">time</span>();<br>
<span class="style6">echo </span><span class="style5">'Unixtime: '</span> . $unixtime . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
$date = $db<span class="style7">-&gt;DBDate</span>($unixtime);<br>
<span class="style6">echo </span><span class="style5">'$db->DBDate: '</span> . $date;<br>
$date = <span class="style6">str_replace</span>(<span class="style5">"'"</span>, <span class="style5">""</span>, $date);<br>
</span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$unixtime = time();
echo 'Unixtime: ' . $unixtime . '<br><br>';
$date = $db->DBDate($unixtime);
echo '$db->DBDate: ' . $date;
$date = str_replace("'", "", $date);
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
<p class="style2">Format the timestamp in the format the database accepts. This can be a Unix integer timestamp or an ISO format Y-m-d H:i:s. Tests $db->DBTimeStamp().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'Unixtime: '</span> . $unixtime . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
$datetime = $db<span class="style7">-&gt;DBTimeStamp</span>($unixtime);<br>
<span class="style6">echo </span><span class="style5">'$db->DBTimeStamp: '</span> . $datetime;<br>
$datetime = <span class="style6">str_replace</span>(<span class="style5">"'"</span>, <span class="style5">""</span>, $datetime);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo 'Unixtime: ' . $unixtime . '<br><br>';
$datetime = $db->DBTimeStamp($unixtime);
echo '$db->DBTimeStamp: ' . $datetime;
$datetime = str_replace("'", "", $datetime);
?>
</p>
	</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p>
	</div><p class="style2">Parses the date string $str and returns it in unix mktime format. Tests $db->UnixDate().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'Date (year-month-day): '</span> . $date . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
<span class="style6">echo </span><span class="style5">'$db->UnixDate: '</span> . $db<span class="style7">->UnixDate</span>($date);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo 'Date (year-month-day): ' . $date . '<br><br>';
echo '$db->UnixDate: ' . $db->UnixDate($date);
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
<p class="style2">Parses the timestamp string and returns it in unix mktime format. Tests $db->UnixTimeStamp().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'Datetime (year-month-day hours:minutes:seconds): '</span> . $datetime . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
<span class="style6">echo </span><span class="style5">'$db->UnixTimeStamp: '</span> . $db<span class="style7">->UnixTimeStamp</span>($datetime);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo 'Datetime (year-month-day hours:minutes:seconds): ' . $datetime . '<br><br>';
echo '$db->UnixTimeStamp: ' . $db->UnixTimeStamp($datetime);
?>
</p>
	</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
	<p class="style2">Returns a string with the native SQL functions to calculate future and past dates. Tests $db->OffsetDate().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'$db->OffsetDate(2) = '</span> . $db<span class="style7">->OffsetDate</span>(<span class="style5">2</span>);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
if(substr($_POST['databasetype'], 0, 5) == "mysql" || substr($_POST['databasetype'], 0, 8) == "postgres" || substr($_POST['databasetype'], 0, 5) == "mssql")
	echo '$db->OffsetDate(2) = ' . $db->OffsetDate(2);
else echo "Database does not support this feature.<br>";
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
<p class="style2">Returns a string which contains the native SQL functions to format a date or date column. Tests $db->SQLDate().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'$db->SQLDate(\'Y-m-d\') = '</span> . $db<span class="style7">->SQLDate</span>(<span class="style5">'Y-m-d'</span>);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
if(substr($_POST['databasetype'], 0, 5) == "mysql" || substr($_POST['databasetype'], 0, 8) == "postgres" || substr($_POST['databasetype'], 0, 5) == "mssql")
	echo '$db->SQLDate(\'Y-m-d\') = ' . $db->SQLDate('Y-m-d');
else echo "Database does not support this feature.<br>";
?>
</p>
	</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p>
	</div><p class="style2">Converts the date string to another format. Tests $db->UserDate().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'Date (year-month-day): '</span> . $date . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
<span class="style6">echo </span><span class="style5">'$db->UserDate($date, "m-d-Y") = '</span> . $db<span class="style7">->UserDate</span>($date, <span class="style5">"m-d-Y"</span>);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo 'Date (year-month-day): ' . $date . '<br><br>';
echo '$db->UserDate($date, "m-d-Y") = ' . $db->UserDate($date, "m-d-Y");
?>
</p>
</td>
    <td width="50%" valign="top"><div align="center"><p class="style6">Date Module Test</p></div>
<p class="style2">Converts the timestamp string to another format. Tests $db->UserTimeStamp().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo </span><span class="style5">'Datetime (year-month-day hours:minutes:seconds): '</span> . $datetime . <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
<span class="style6">echo </span><span class="style5">'$db->UserTimeStamp($datetime, "i:s:h m-d-Y") = '</span> . $db<span class="style7">->UserTimeStamp</span>($datetime, <span class="style5">"i:s:h m-d-Y"</span>);<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo 'Datetime (year-month-day hours:minutes:seconds): ' . $datetime . '<br><br>';
echo '$db->UserTimeStamp($datetime, "i:s:h m-d-Y") = ' . $db->UserTimeStamp($datetime, "i:s:h m-d-Y");
?>
</p>
	</td>
  </tr>
<?php
}
// Date Module Test End
?>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve all records using &quot;for&quot; loop. Tests Query(), RecordCount(), Fields, MoveNext() and close(). </p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Query</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">for</span> ($i = <span class="style8">0</span>; $i &lt; $rs<span class="style7">-&gt;RecordCount</span>(); $i++ )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs<span class="style7">-&gt;MoveNext</span>();<br>
  }<br>
$rs<span class="style7">-&gt;close</span>();<br>
  <span class="style4">?&gt;</span><br>
</span></p>
<p class="style0"> <?php
$rs = $db->Query( 'select * from adodb_lite_test' );
for ($i = 0; $i < $rs->RecordCount(); $i++ )
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
	$rs->MoveNext();
}
$rs->close();
?>
</p>
 </td>
    <td width="50%" valign="top"><p class="style2">Retrieve all records using &quot;while&quot; loop. Tests Execute(), EOF, Fields, MoveNext() and Free(). </p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">while</span> ( !$rs-&gt;EOF )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs<span class="style7">-&gt;MoveNext</span>();<br>
  }</span><span class="style3"><br>
$rs<span class="style7">-&gt;Free</span>();<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
while ( !$rs->EOF )
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
	$rs->MoveNext();
}
$rs->Free();
?>
</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve all records. Tests Execute(), RecordCount() and GetArray().</p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>(</span><span class="style5"> 'select * from adodb_lite_test' </span><span class="style3">);<br>
$totaltrecords=$rs</span><span class="style7">-&gt;RecordCount</span><span class="style3">(); <br>
$getit=$rs</span><span class="style7">-&gt;GetArray</span><span class="style3">();<br>
</span><span class="style6">for</span><span class="style3"> ($i = </span><span class="style8">0</span><span class="style3">; $i &lt; $totaltrecords; $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $getit[$i][</span><span class="style5">'id'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $getit[$i][</span><span class="style5">'text'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
</span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$totaltrecords=$rs->RecordCount();
$getit=$rs->GetArray();
for ($i = 0; $i < $totaltrecords; $i++ )
{
	$id = $getit[$i]['id'];
	$text = $getit[$i]['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p>
</td>
    <td width="50%" valign="top"><p class="style2">Retrieve all records. Tests Execute(), NumRows() and GetRows().</p>
      <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
$totaltrecords=$rs</span><span class="style7">-&gt;NumRows</span><span class="style3">(); <br>
$getit=$rs</span><span class="style7">-&gt;GetRows</span><span class="style3">();<br>
</span><span class="style6">for</span><span class="style3"> ($i = </span><span class="style8">0</span><span class="style3">; $i &lt; $totaltrecords; $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $getit[$i][</span><span class="style5">'id'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $getit[$i][</span><span class="style5">'text'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$totaltrecords=$rs->NumRows();
$getit=$rs->GetRows();
for ($i = 0; $i < $totaltrecords; $i++ )
{
	$id = $getit[$i]['id'];
	$text = $getit[$i]['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p>
 </td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve all records. Tests Execute(), RecordCount() and GetAll().</p>
      <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test' </span><span class="style3">);<br>
$totaltrecords=$rs</span><span class="style7">-&gt;RecordCount</span><span class="style3">(); <br>
$getit=$rs</span><span class="style7">-&gt;GetAll</span><span class="style3">();<br>
</span><span class="style6">for</span><span class="style3"> ($i = </span><span class="style8">0</span><span class="style3">; $i &lt; $totaltrecords; $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $getit[$i][</span><span class="style5">'id'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $getit[$i][</span><span class="style5">'text'</span><span class="style3">];<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0">
<?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$totaltrecords=$rs->RecordCount();
$getit=$rs->GetAll();
for ($i = 0; $i < $totaltrecords; $i++ )
{
	$id = $getit[$i]['id'];
	$text = $getit[$i]['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p>
  </td>
    <td width="50%" valign="top"><p class="style2">Retrieve single assigned record. Tests Execute(), EOF, Fields and MoveNext() to make sure only a single record is retrieved when only one record is present. </p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select count(id) as total from adodb_lite_test'</span><span class="style3"> );<br>
</span><span class="style6">while</span><span class="style3"> ( !$rs</span><span class="style7">-&gt;EOF</span><span class="style3"> )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs</span><span class="style7">-&gt;fields</span><span class="style3">;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> </span><span class="style5">'Column Text: ' </span><span class="style3">. $row[</span><span class="style5">'total'</span><span class="style3">] . <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs</span><span class="style7">-&gt;MoveNext</span><span class="style3">();<br>
}<br>
</span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$rs = $db->execute( 'select count(id) as total from adodb_lite_test' );
while ( !$rs->EOF )
{
	$row = $rs->fields;
	echo 'Column Text: ' . $row['total'] . '<br>';
	$rs->MoveNext();
}
?>
</p>
</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve record number 6. Tests Execute(), Move() and Fields. </p>
    <p class="style3"> <span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
$rs</span><span class="style7">-&gt;Move</span><span class="style3">(</span><span class="style8">6</span><span class="style3">);<br>
$row = $rs</span><span class="style7">-&gt;fields</span><span class="style3">;<br>
$id = $row[</span><span class="style5">'id'</span><span class="style3">];<br>
$text = $row[</span><span class="style5">'text'</span><span class="style3">];</span><br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$rs->Move(6);
$row = $rs->fields;
$id = $row['id'];
$text = $row['text'];
echo 'Column ID: ' . $id . ' - ';
echo 'Column Text: ' . $text . '<br>';
?>
</p>
 </td>
    <td width="50%" valign="top"><p class="style2">Retrieve First and Last record. Tests Execute(), MoveLast(), MoveFirst(), Fields.</p>
    <p class="style3"> <span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
$rs</span><span class="style7">-&gt;MoveLast</span><span class="style3">();<br>
$row = $rs</span><span class="style7">-&gt;fields</span><span class="style3">;<br>
$id = $row[</span><span class="style5">'id'</span><span class="style3">];<br>
$text = $row[</span><span class="style5">'text'</span><span class="style3">];</span><br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Last Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Last Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  $rs<span class="style7">-&gt;MoveFirst</span>();<br>
  $row = $rs<span class="style7">-&gt;fields</span>;<br>
  $id = $row[<span class="style5">'id'</span>];<br>
  $text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'First Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'First Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
$rs->MoveLast();
$row = $rs->fields;
$id = $row['id'];
$text = $row['text'];
echo 'Last Column ID: ' . $id . ' - ';
echo 'Last Column Text: ' . $text . '<br>';
$rs->MoveFirst();
$row = $rs->fields;
$id = $row['id'];
$text = $row['text'];
echo 'First Column ID: ' . $id . ' - ';
echo 'First Column Text: ' . $text . '<br>';
?>
</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top"> <p class="style2">Test ErrorMsg, ErrorNo and ErrorNative. </p>
      <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test where opps=6'</span><span class="style3"> );<br>
$test = $db</span><span class="style7">-&gt;ErrorMsg</span><span class="style3">();<br>
<span class="style6">echo</span> </span><span class="style5">'Error Message: ' </span><span class="style3">. $test . <span class="style5">'&lt;br&gt;'</span>;<br>
$test = $db</span><span class="style7">-&gt;ErrorNo</span><span class="style3">();<br>
<span class="style6">echo</span> </span><span class="style5">'Error Number: '</span><span class="style3"> . $test . <span class="style5">'&lt;br&gt;'</span>;<br>
$test = $db</span><span class="style7">-&gt;ErrorNative</span><span class="style3">();<br>
<span class="style6">echo</span> </span><span class="style5">'ErrorNative: '</span><span class="style3"> . $test . <span class="style5">'&lt;br&gt;'</span>;<br>
  </span><span class="style4">?&gt;</span><br>
      </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test where opps=6' );
$test = $db->ErrorMsg();
echo 'Error Message: ' . $test . '<br>';
$test = $db->ErrorNo();
echo 'Error Number: ' . $test . '<br>';
$test = $db->ErrorNative();
echo 'ErrorNative: ' . $test . '<br>';
?>
</p>
</td>
    <td width="50%" valign="top"><p class="style2">Retrieve 3 records. Tests SelectLimit(), EOF, MoveNext() and Fields.</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;SelectLimit</span>( <span class="style5">'select * from adodb_lite_test'</span>, <span class="style8">3</span>);<br>
  <span class="style6">while</span> ( !$rs<span class="style7">-&gt;EOF</span> )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs</span><span class="style7">-&gt;MoveNext</span><span class="style3">();<br>
}<br>
</span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->SelectLimit( 'select * from adodb_lite_test', 3);
while ( !$rs->EOF )
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
	$rs->MoveNext();
}
?>
</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve 3 records offset by 2 records. Tests SelectLimit(), EOF, MoveNext() and Fields.</p>
      <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;SelectLimit</span>( <span class="style5">'select * from adodb_lite_test'</span>, <span class="style8">3</span>, <span class="style8">2</span>);<br>
  <span class="style6">while</span> ( !$rs<span class="style7">-&gt;EOF</span> )<br>
  {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs</span><span class="style7">-&gt;MoveNext</span><span class="style3">();<br>
}<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->SelectLimit( 'select * from adodb_lite_test', 3, 2);
while ( !$rs->EOF )
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
	$rs->MoveNext();
}

?>
</p>
</td>
    <td width="50%" valign="top"><p class="style2">Retrieve 2 records offeset by 3 records. Tests LimitQuery(), EOF, MoveNext() and Fields.</p>
      <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;LimitQuery</span>( <span class="style5">'select * from adodb_lite_test'</span>, <span class="style8">3</span>, <span class="style8">2</span>);<br>
  <span class="style6">while</span> ( !$rs<span class="style7">-&gt;EOF</span> )<br>
  {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$rs</span><span class="style7">-&gt;MoveNext</span><span class="style3">();<br>
}<br>
  </span><span class="style4">?&gt;</span><br>
    </p>
<p class="style0"><?php
$rs = $db->LimitQuery( 'select * from adodb_lite_test', 3, 2);
while ( !$rs->EOF )
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
	$rs->MoveNext();
}
?>
</p>
</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve ADOdb Lite Version. Tests Version().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
      <span class="style6">echo</span> <span class="style5">'Version: '</span> . $db<span class="style7">-&gt;Version</span>() .  <span class="style5">'&lt;br&gt;'</span>; <br>
<span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
echo 'Version: ' . $db->Version() . '<br>'; 
?>
</p>
</td>
    <td width="50%" valign="top">
<p class="style2">Retrieve first element of one record. Tests GetOne().</p>
      <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$data = $db</span><span class="style7">-&gt;GetOne</span><span class="style3">( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
<span class="style6">echo</span> </span><span class="style5">'Column ID: '</span><span class="style3"> . $data . </span><span class="style5">'&lt;br&gt;'</span><span class="style3">;<br>
  </span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$data = $db->GetOne( 'select * from adodb_lite_test' );
echo 'Column ID: ' . $data . '<br>';
?>
</p>
	</td>
  </tr>
  <tr>
    <td width="50%" valign="top">
<p class="style2">Retrieve all records. Tests GetArray().</p>
      <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetArray</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">for</span> ($i = <span class="style8">0</span>; $i &lt; <span class="style6">count</span>($data); $i++ )<br>
  {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $data[$i];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>
 <span class="style3">&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column ID: ' </span><span class="style3">. $id .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column Text: ' </span><span class="style3">. $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
  <span class="style4">?&gt;</span><br>
 </span></p>
<p class="style0"><?php
$data = $db->GetArray( 'select * from adodb_lite_test' );
for ($i = 0; $i < count($data); $i++ )
{
	$row = $data[$i];
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p>
</td>
    <td width="50%" valign="top">
<p class="style2">Retrieve all records. Tests GetAll().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetAll</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">for</span> ($i = <span class="style8">0</span>; $i &lt; <span class="style6">count</span>($data); $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $data[$i];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>
 <span class="style3">&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column ID: '</span><span class="style3"> . $id .  <span class="style5">'&lt;br&gt;'</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column Text: '</span><span class="style3"> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
<span class="style4">?&gt;</span><br>
 </span></p>
<p class="style0"><?php
$data = $db->GetAll( 'select * from adodb_lite_test' );
for ($i = 0; $i < count($data); $i++ )
{
	$row = $data[$i];
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p>
	</td>
  </tr>
  <tr>
    <td width="50%" valign="top">
<p class="style2">Retrieve all elements of one record. Tests GetRow().</p>
      <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetRow</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
$id = $data[<span class="style5">'id'</span>];<br>
$text = $data[<span class="style5">'text'</span>];<br>
<br>
<span class="style3"> <span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column ID: '</span><span class="style3"> . $id .  <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Column Text: '</span><span class="style3"> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  </span><span class="style4">?&gt;</span><br>
  </p>
<p class="style0"> <?php
$data = $db->GetRow( 'select * from adodb_lite_test' );
$id = $data['id'];
$text = $data['text'];
echo 'Column ID: ' . $id . ' - ';
echo 'Column Text: ' . $text . '<br>';
?>
</p>
	</td>
    <td width="50%" valign="top">
<p class="style2">Retrieve first column of all records. Tests GetCol().</p>
      <p class="style3"><span class="style4">&lt;?</span><br>
$data = $db<span class="style7">-&gt;GetCol</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">for</span> ($i = <span class="style8">0</span>; $i &lt; <span class="style6">count</span>($data); $i++ )<br>
  {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">'Column: '</span> . $data[$i] .  <span class="style5">'&lt;br&gt;'</span>;<br>
  }<br>
  <span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$data = $db->GetCol( 'select * from adodb_lite_test' );
for ($i = 0; $i < count($data); $i++ )
{
	echo 'First Column: ' . $data[$i] . '<br>';
}
?>
</p>
 	</td>
  </tr>
  <tr>
    <td width="50%" valign="top">
<p class="style2">Check escape quoting of strings. Tests qstr(), Quote(), Qmagic().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style8">// Magic Quotes True do not escape quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;qstr</span>(</span><span class="style5">"I'm here \ today"</span>, <span class="style12">true</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style8">// Magic Quotes False escape quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;qstr</span>(</span><span class="style5">"I'm here \ today"</span>, <span class="style12">false</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style8">// Magic Quotes True do not escape quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;qstr</span>(</span><span class="style5">'I\'m here \ today'</span>, <span class="style12">true</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style8">// Magic Quotes False escape quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;qstr</span>(</span><span class="style5">'I\'m here \ today'</span>, <span class="style12">false</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style8">// Quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;Quote</span>(</span><span class="style5">'I\'m here \ today'</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style8">// QMagic escape quote string</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;Qmagic</span>(</span><span class="style5">'I\'m here \ today'</span>) . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
// Magic Quotes True do not escape quote string
echo $db->qstr("I'm here \ today", true) . '<br>';
// Magic Quotes False escape quote string
echo $db->qstr("I'm here \ today", false) . '<br>';
// Magic Quotes True do not escape quote string
echo $db->qstr('I\'m here \ today', true) . '<br>';
// Magic Quotes False escape quote string
echo $db->qstr('I\'m here \ today', false) . '<br>';
// Quote string
echo $db->Quote('I\'m here \ today') . '<br>';
// QMagic escape quote string
echo $db->Qmagic('I\'m here \ today') . '<br>';
?>
</p>
	</td>
    <td width="50%" valign="top">
<p class="style2">Check array input features. Tests Execute(), SelectLimit(), RecordCount(), GetAll(), Close() and qstr().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;SelectLimit</span>( <span class="style5">'select * from adodb_lite_test where id=?'</span>, <span class="style8">1</span>, <span class="style8">-1</span>, <span class="style6">array</span>(<span class="style5">'7'</span>));<br>
<br>
  <span class="style6">if</span> ($rs<span class="style7">-&gt;RecordCount</span>() == <span class="style8">1</span> )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $rs<span class="style7">-&gt;fields</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;&lt;br&gt;'</span>;<br>
  }<br>
else<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">"Couldn't find record&lt;br&gt;&lt;br&gt;"</span>;<br>
} <br>
<br>
$inputarr = <span class="style6">array</span>(<br>
<span class="style6">array</span>(</span><span class="style5">'_"count"_13'</span>, </span><span class="style5">'Dummy 13'</span>),<br>
<span class="style6">array</span>(</span><span class="style5">'_\count\_14'</span>, </span><span class="style5">'Dummy_14'</span>),<br>
<span class="style6">array</span>(</span><span class="style5">&quot;_'count'_15&quot;</span>, </span><span class="style5">&quot;Dummy_15&quot;</span>)<br>
);
<br><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">&quot;insert into adodb_lite_test (text, dummy) values (?, ?)&quot;</span>, $inputarr<span class="style3"> ); <br>
<br>
$data = $db<span class="style7">-&gt;GetAll</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">for</span> ($i = <span class="style8">0</span>; $i &lt; <span class="style6">count</span>($data); $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$row = $data[$i];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$dummy = $row[<span class="style5">'dummy'</span>];<br>
<br>
 <span class="style3">&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'ID: '</span><span class="style3"> . $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Text: '</span><span class="style3"> . $text .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span></span><span class="style5">'Dummy: '</span><span class="style3"> . $dummy .  <span class="style5">'&lt;br&gt;'</span>;<br>
}<br>
<span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$rs = $db->SelectLimit('select * from adodb_lite_test where id=?', 1, -1, array('7'));

if($rs->RecordCount() == 1)
{
	$row = $rs->fields;
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br><br>';
}
else
{
	echo "Couldn't find record.<br><br>";
}

$inputarr = array(
array('_"count"_13', 'Dummy 13'),
array('_\count\_14', 'Dummy_14'),
array("_'count'_15", "Dummy_15")
);

$rs = $db->execute( "insert into adodb_lite_test (text, dummy) values (?, ?)", $inputarr );

$data = $db->GetAll( 'select * from adodb_lite_test' );
for ($i = 0; $i < count($data); $i++ )
{
	$row = $data[$i];
	$id = $row['id'];
	$text = $row['text'];
	$dummy = $row['dummy'];
	echo 'ID: ' . $id . ' - ';
	echo 'Text: ' . $text . ' - ';
	echo 'Dummy: ' . $dummy . '<br>';
}
?>
</p>
</td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve total number of fields. Tests Execute(), FieldCount() and NumCols()</p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
<span class="style6">echo</span> </span><span class="style5">'Field Count: '</span><span class="style3"> . $rs</span><span class="style7">-&gt;FieldCount</span><span class="style3">() . <span class="style5">'&lt;br&gt;'</span>;<br>
<span class="style6">echo</span> </span><span class="style5">'Number of Columns: '</span><span class="style3"> . $rs</span><span class="style7">-&gt;NumCols</span><span class="style3">() . <span class="style5">'&lt;br&gt;'</span>;<br>
  </span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
echo 'Field Count: ' . $rs->FieldCount() . '<br>';
echo 'Number of Columns: ' . $rs->NumCols() . '<br>';
?>
</p>
</td>
    <td width="50%" valign="top">
<p class="style2">Retrieve all records using &quot;while&quot; loop. Tests Execute() and FetchRow(). </p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">while</span> ( $row = $rs-&gt;FetchRow() )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
while ( $row = $rs->FetchRow() )
{
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p></td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Create an IfNull statement. Tests IfNull()</p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;IfNull</span>( </span><span class="style5">'text'</span><span class="style3">, </span><span class="style5">'Empty'</span><span class="style3"> );<br>
  </span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
echo $db->IfNull( 'text', 'Empty' );
?>
</p>
</td>
    <td width="50%" valign="top">
<p class="style2">Concatenate a series of strings together. Tests Concat(). </p>
    <p class="style3"><span class="style4">&lt;?</span><br>
<span class="style6">echo</span> $db<span class="style7">-&gt;Concat</span>( </span><span class="style5">'text'</span><span class="style3">, </span><span class="style5">'dummy'</span><span class="style3"> ) . <span class="style5">"&lt;br&gt;&lt;br&gt;"</span>;<br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select ' . </span> $db<span class="style7">-&gt;Concat</span>( </span><span class="style5">'text'</span><span class="style3">, </span><span class="style5">'dummy'</span><span class="style3"> ) . <span class="style5">" as total from adodb_lite_test where id=13"</span> );<br>
<span class="style6">echo</span> <span class="style5">'Concatenated Result: '</span> . $db<span class="style7">-&gt;fields</span>[<span class="style5">'total'</span>] . <span class="style5">"&lt;br&gt;"</span>;<br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
echo $db->Concat( 'text', 'dummy' ) . "<br><br>";
$rs = $db->Execute( 'select ' . $db->Concat( 'text', 'dummy' ) . " as total from adodb_lite_test where id=13" );
echo 'Concatenated Result: ' . $rs->fields['total'] . '<br>';
?>
</p></td>
  </tr>
  <tr>
    <td width="50%" valign="top"><p class="style2">Retrieve all field information. Tests SelectLimit(), FieldCount() and FetchField()</p>
    <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;SelectLimit</span>( </span><span class="style5">'select * from adodb_lite_test'</span>, <span class="style8">1</span><span class="style3"> );<br>
</span><span class="style6">for</span><span class="style3"> ($i = </span><span class="style8">0</span><span class="style3">; $i &lt; $rs<span class="style7">-&gt;FieldCount</span>(); $i++ )<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;$fieldobject = $rs->FetchField($i);<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> </span>$i . <span class="style5">". Field Name: "</span><span class="style3"> . $fieldobject</span><span class="style7">-&gt;name</span><span class="style3"> . <span class="style5">"&lt;br&gt;"</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> </span>$i . <span class="style5">". Field Length: "</span><span class="style3"> . $fieldobject</span><span class="style7">-&gt;max_length</span><span class="style3"> . <span class="style5">"&lt;br&gt;"</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> </span>$i . <span class="style5">". Field Type: "</span><span class="style3"> . $fieldobject</span><span class="style7">-&gt;type</span><span class="style3"> . <span class="style5">"&lt;br&gt;&lt;hr&gt;"</span>;<br>
}<br>
  </span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$rs = $db->SelectLimit( 'select * from adodb_lite_test', 1);
for($i = 0; $i < $rs->FieldCount(); $i++)
{
	$fieldobject = $rs->FetchField($i);
	echo $i . ". Field Name: " . $fieldobject->name . "<br>";
	echo $i . ". Field Length: " . $fieldobject->max_length . "<br>";
	echo $i . ". Field Type: " . $fieldobject->type . "<br><hr>";
}
?>
</p>
</td>
    <td width="50%" valign="top">
<p class="style2">Check the debug flag. Tests GetOne(), Execute() and $db->debug = true.</p>
      <p class="style3"><span class="style3"><span class="style4">&lt;?</span><br>
$db->debug = <span class="style12">true</span>;<br>
$data = $db</span><span class="style7">-&gt;GetOne</span><span class="style3">( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
$data = $db</span><span class="style7">-&gt;GetOne</span><span class="style3">( </span><span class="style5">'select * from adodb_lite_test'</span><span class="style3"> );<br>
<br>
$inputarr = <span class="style6">array</span>(<br>
<span class="style6">array</span>(</span><span class="style5">'_"how"_16'</span>, </span><span class="style5">'start 16'</span>),<br>
<span class="style6">array</span>(</span><span class="style5">'_\about\_17'</span>, </span><span class="style5">'debug_17'</span>),<br>
<span class="style6">array</span>(</span><span class="style5">&quot;_'this'_18&quot;</span>, </span><span class="style5">&quot;test_18&quot;</span>)<br>
);
<br><br>
$rs = $db<span class="style7">-&gt;Execute</span>( </span><span class="style5">&quot;insert into adodb_lite_test (text, dummy) values (?, ?)&quot;</span>, $inputarr<span class="style3"> ); <br>
<br>
$db->debug = <span class="style12">false</span>;<br>
  </span><span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
$db->debug = true;
$data = $db->GetOne( 'select * from adodb_lite_test' );
$data = $db->GetOne( 'select * from adodb_lite_error' );

$inputarr = array(
array('_"how"_16', 'start 16'),
array('_\about\_17', 'debug_17'),
array("_'this'_18", "test_18")
);

$rs = $db->execute( "insert into adodb_lite_test (text, dummy) values (?, ?)", $inputarr );

$db->debug = false;
?>
</p>
</td>
  </tr>
  <tr>
    <td width="50%" valign="top">
<p class="style2">Retrieve all records using &quot;while&quot; loop. Tests Execute() and FetchInto(). </p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$rs = $db<span class="style7">-&gt;Execute</span>( <span class="style5">'select * from adodb_lite_test'</span> );<br>
  <span class="style6">while</span> ( $rs-&gt;FetchInto($row) )<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;$id = $row[<span class="style5">'id'</span>];<br>
&nbsp;&nbsp;&nbsp;&nbsp;$text = $row[<span class="style5">'text'</span>];<br>
<br>&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column ID: ' </span>. $id .  <span class="style5">' - '</span>;<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5">'Column Text: '</span> . $text .  <span class="style5">'&lt;br&gt;'</span>;<br>
  }</span><span class="style3"><br>
  </span><span class="style4">?&gt;</span><br>

    </p>
<p class="style0"><?php
$rs = $db->execute( 'select * from adodb_lite_test' );
while ( $rs->FetchInto($row) )
{
	$id = $row['id'];
	$text = $row['text'];
	echo 'Column ID: ' . $id . ' - ';
	echo 'Column Text: ' . $text . '<br>';
}
?>
</p></td>
    <td width="50%" valign="top">
<p class="style2">Check if connected to database and Close Database. Tests IsConnected() and Close().</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
if($db<span class="style7">-&gt;IsConnected</span>())<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">'Connected&lt;br&gt;'</span>;<br>
}<br>
else<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">'Not Connected&lt;br&gt;'</span>;<br>
} <br>
<span class="style4">?&gt;</span><br>
</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
$db<span class="style7">-&gt;Close</span>(); <br>
<span class="style4">?&gt;</span><br>
<span class="style6">echo</span> <span class="style5">'Database Closed&lt;br&gt;';</span><br>
</p>
    <p class="style3"><span class="style4">&lt;?</span><br>
if($db<span class="style7">-&gt;IsConnected</span>())<br>
      {<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">'Connected&lt;br&gt;'</span>;<br>
}<br>
else<br>
{<br>
&nbsp;&nbsp;&nbsp;&nbsp;<span class="style6">echo</span> <span class="style5"></span><span class="style5">'Not Connected&lt;br&gt;'</span>;<br>
} <br>
<span class="style4">?&gt;</span><br>
</p>
<p class="style0"><?php
if($db->IsConnected())
{
	echo 'Connected<br>';
}
else
{
	echo 'Not Connected<br>';
}
?>
</p>
<p class="style0"><?php
$db->Close(); 
echo 'Database Closed<br>';
?>
</p>
<p class="style0"><?php
if($db->IsConnected())
{
	echo 'Connected<br>';
}
else
{
	echo 'Not Connected<br>';
}
?>
</p>
	</td>
  </tr>
   <tr>
    <td colspan="2" valign="top"><p align="center"><br>
        <span class="style10">Testing Complete</span><br>
        <br>
        <span class="style10">Total Queries Executed: <?php echo $db->query_count; ?></span><br>
        <br>
        <span class="style10">Execution Time: <?php echo $db->query_time_total; ?> seconds</span><br>
        <br>
    </p>
    </td>
  </tr></table>
<?php
}
?>
</body>
</html>
