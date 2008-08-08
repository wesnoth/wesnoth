<?php
if(empty($_POST['databasetype']))
{
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<title>ADOdb Lite Session Test Program</title>
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
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Session Test Program</u></span><br>
      </p>
    </div></td>
  </tr>
  <tr>
    <td valign="top">
<form action="test_adodb_lite_sessions.php" method="POST" enctype="multipart/form-data">
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
        <td valign="middle"><div align="right"><span class="style2">Select Encryption</span></div></td>
        <td valign="middle"><div align="left"><span class="style2">
            <select name="encrypt">
                <option value="0" selected>None</option>
                <option value="1">Mcrypt</option>
                <option value="2">MD5</option>
                <option value="3">OrdCrypt</option>
                <option value="4">Secret</option>
                <option value="5">SHA1</option>
            </select>
        </span></div></td>
      </tr>
      <tr>
        <td valign="middle"><div align="right"><span class="style2">Select Compression</span></div></td>
        <td valign="middle"><div align="left"><span class="style2">
            <select name="compress">
                <option value="0" selected>None</option>
                <option value="1">Bzip2</option>
                <option value="2">Gzip</option>
            </select>
        </span></div></td>
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
		<input type="hidden" name="USERID" value="<?=mt_rand(1,10000);?>">
          <input type="submit" name="Submit Form" value="Submit">
        </div></td>
        </tr>    </table>
</form>
</td>
  </tr>
  <tr>
    <td><div align="center">
      <p align="left"><span class="style11">This program will test the ADOdb Lite Session Handling Code.
		The program will attempt to create the database if it does not exist.  If you see 
		<u>Could not connect to the database.</u> then you will need to create a database 
		or use an existing database to perform this test. <br>
        <br>
        If you are testing a database other than MySql, MySqli or MySqlt then 
        you will need to create a database table called 'sessions' 
        with three fields.<br>
        <br>
        Field 1 = id - int(11) auto_increment<br>
        Field 2 = SessionID - VARCHAR(64)<br>
        Field 3 = session_data - longtext or text<br>
        Field 4 = expiry - int(11)<br>
        Field 5 = expireref - VARCHAR(64)<br>
        <br>
        Here is what the MySql create table query looks like.<br>
        <br>
        CREATE TABLE IF NOT EXISTS `sessions` (<br>
	ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY, <br>
	SessionID VARCHAR(64), <br>
	Data TEXT DEFAULT '', <br>
	expiry INT(11),<br>
	expireref	VARCHAR(64)	DEFAULT '',<br>
	INDEX (SessionID),<br>
	INDEX expiry (expiry)<br>
)<br>
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
	header("location: test_adodb_lite_sessions.php?databasetype= \n");
	die();
}

require_once '../adodb.inc.php'; 
require_once '../session/adodb-session.php'; 

$encryption = array('', 'adodb-encrypt-mcrypt.php', 'adodb-encrypt-md5.php', 'adodb-encrypt-ordcrypt.php', 'adodb-encrypt-secret.php', 'adodb-encrypt-sha1.php');
$encryption_object = array('', 'ADODB_Encrypt_MCrypt', 'ADODB_Encrypt_MD5', 'ADODB_Encrypt_OrdCrypt', 'ADODB_Encrypt_Secret', 'ADODB_Encrypt_SHA1');
$encryption_name = array('None', 'MCrypt', 'MD5', 'OrdCrypt', 'Secret', 'SHA1');
$compression = array('', 'adodb-compress-bzip2.php', 'adodb-compress-gzip.php');
$compression_object = array('', 'ADODB_Compress_Bzip2', 'ADODB_Compress_Gzip');
$compression_name = array('None', 'Bzip2', 'Gzip');

if($_POST['encrypt'] > 0)
{
	require_once ADODB_SESSION . '/' . $encryption[$_POST['encrypt']];
	$object = $encryption_object[$_POST['encrypt']];
	ADODB_Session::filter(new $object());
}

if($_POST['compress'] > 0)
{
	require_once ADODB_SESSION . '/' . $compression[$_POST['compress']];
	$object = $compression_object[$_POST['compress']];
	ADODB_Session::filter(new $object());
}

$db = ADONewConnection($_POST['databasetype']);

$db->createdatabase = true;

$result = $db->Connect( $_POST['dbhost'], $_POST['dbusername'], $_POST['dbpassword'], $_POST['databasename'] );
if(!$result)
{
	die("Could not connect to the database.");
}
$ADODB_FETCH_MODE = ADODB_FETCH_ASSOC;

function ExpiredSession($expireref, $sesskey)
{
	echo "Session USERID: $expireref - expired<br>";
	echo "Session Key: $sesskey<br><br>";
}

if($_POST['databasetype'] == 'mysql' || $_POST['databasetype'] == 'mysqli' || $_POST['databasetype'] == 'mysqlt')
{
	$res = $db->Execute("DROP TABLE IF EXISTS `sessions`");
	$res = $db->Execute("CREATE TABLE IF NOT EXISTS `sessions` (
		ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY, 
		SessionID VARCHAR(64), 
		session_data TEXT DEFAULT '', 
		expiry INT(11),
		expireref	VARCHAR(64)	DEFAULT '',
		INDEX (SessionID),
		INDEX expiry (expiry)
	)");
}

$USERID = $_POST['USERID'];

$ADODB_SESSION_DRIVER = $_POST['databasetype'];
$ADODB_SESSION_CONNECT = $_POST['dbhost'];
$ADODB_SESSION_USER = $_POST['dbusername'];
$ADODB_SESSION_PWD = $_POST['dbpassword'];
$ADODB_SESSION_DB = $_POST['databasename'];
$ADODB_SESSION_EXPIRE_NOTIFY = array('USERID', 'ExpiredSession');
session_start();

$_SESSION['foo'] = "bar";
$_SESSION['baz'] = "wombat";
$_SESSION['hiya'] = "berp";
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
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Test Program</u></span><br>
		Encryption Method: <?=$encryption_name[$_POST['encrypt']];?><br>
		Compression Method: <?=$compression_name[$_POST['compress']];?><br>
      </p>
    </div></td>
  </tr>
  <tr>
    <td valign="top">
<?php
echo "<h3>\$USERID = " . $USERID . "</h3>";
echo "<p><b>\$_SESSION['foo'] = " . $_SESSION['foo'] . "</b></p>";
echo "<p><b>\$_SESSION['baz'] = " . $_SESSION['baz'] . "</b></p>";
echo "<p><b>\$_SESSION['hiya'] = " . $_SESSION['hiya'] . "</b></p>";
echo "<hr> <b>Cookies</b>: ";
print_r($_COOKIE);
echo "<br><hr> <b>Sessions</b>: ";
print_r($_SESSION);

if (mt_rand() % 2 == 0) {
	echo "<hr><p><b>Garbage Collection</b></p>";
	$time = time() + 6400;
	adodb_sess_gc($time);
}
else
if (mt_rand() % 2 == 0) {
	echo "<hr><p><b>Session destroy</b></p>";
	session_destroy();
}
else
echo "<hr><p><b>Normal Execution</b></p>";
?>
<br>
	</td>
  </tr>
	<tr>
  <td>
    <div align="center">Reload/Refresh the page multiple times.  You should randomly see "Garbage Collection", "Session Destroyed" or "Normal Execution".
	<br><br>
	"Garbage Collection" and "Session Destroyed" should display the USERID of the expired/destroyed session and the Session ID.  The Session ID should match the $_COOKIE[PHPSESSID] variable displayed above.</div>
	</td>
  </tr>
   <tr>
    <td colspan="2" valign="top"><p align="center"><br>
        <span class="style10">Testing Complete</span><br>
        <br>
    </p>
    </td>
  </tr></table>
<?php
}
?>
</body>
</html>
