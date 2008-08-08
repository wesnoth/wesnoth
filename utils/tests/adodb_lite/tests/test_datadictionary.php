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
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Data Dictionary/XML Test Program</u></span><br>
      </p>
    </div></td>
  </tr>
  <tr>
    <td valign="top">
<form action="test_datadictionary.php" method="POST" enctype="multipart/form-data">
	<table width="80%"  border="0" align="center" cellpadding="10" cellspacing="1">
      <tr>
        <td><div align="right"><span class="style2">Create Test Databases</span></div></td>
        <td width="50%"><input type="radio" name="create_test" value="0" checked>No&nbsp;&nbsp;-&nbsp;&nbsp;<input type="radio" name="create_test" value="1">Yes</td>
      </tr>
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
                <option value="sqlite">SQLite</option>
                <option value="sybase">SyBase</option>
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
          <input type="submit" name="Submit Form" value="Submit">
        </div></td>
        </tr>    </table>
</form>
</td>
  </tr>
  <tr>
    <td><div align="center">
      <p align="left"><span class="style11">This program will test the ADOdb Lite Data Dictionary for all supported databases as well as the XML Schema support. <br>
	  <br>
	  You do not need to enter database information if Create Test Databases is not enabled.<br> 
      </p>
      </div></td>
  </tr></table>
<?php
}
else
{

include_once('../adodb.inc.php');
error_reporting(E_ALL);

function print_sql($dbType, $sqla, $action)
{
	print "<pre>";
	echo "Function: $action<br><br>";
	foreach($sqla as $s) {
		$s = htmlspecialchars($s);
		print "$s;\n";
		if ($dbType == 'oci8') print "/\n";
	}
	print "</pre><hr>";
}

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
        <img src="adodblite_thumb.jpg" border="0" title="ADOdb Lite Thumbnail Logo" alt="ADOdb Lite Thumbnail Logo"><br><br><span class="style10"><u>ADOdb Lite Data Dictionary/XML Test Program</u></span><br>
      </p>
    </div></td>
  </tr>
<?php

function test_dictionary($dbType){
	echo "<h3>$dbType</h3><p>";
	$db = NewADOConnection($dbType, "pear");
	$dict = NewDataDictionary($db);

	if (!$dict) continue;
	$dict->debug = 1;
	
	$opts = array('REPLACE','mysql' => 'TYPE=INNODB', 'oci8' => 'TABLESPACE USERS');
	
	if(mt_rand(1, 10000) < 5000)
	{
		$flds = array(
		array('id',	'I',								
							'AUTO','KEY'),
							
		array('name' => 'firstname', 'type' => 'varchar','size' => 30,
							'DEFAULT'=>'Joan'),
							
		array('lastname','varchar',28,
							'DEFAULT'=>'Chen','key'),
							
		array('averylonglongfieldname','X',1024,
							'NOTNULL','default' => 'test'),
							
		array('price','N','7.2',
							'NOTNULL','default' => '0.00'),
							
		array('MYDATE', 'D', 
							'DEFDATE'),
		array('TS','T',
							'DEFTIMESTAMP')
		);
	}
	else
	{
		$flds = "
ID            I           AUTO KEY,
FIRSTNAME     VARCHAR(30) DEFAULT 'Joan',
LASTNAME      VARCHAR(28) DEFAULT 'Chen' key,
averylonglongfieldname X(1024) DEFAULT 'test',
price         N(7.2)  DEFAULT '0.00',
MYDATE        D      DEFDATE,
BIGFELLOW     X      NOTNULL,
TS            T      DEFTIMESTAMP";
	}

	$sqla = $dict->CreateDatabase('ADOdbLite',array('postgres'=>"LOCATION='/u01/postdata'"));
	print_sql($dbType,$sqla, "CreateDatabase");
	$dict->SetSchema('ADOdbLite');
	
	if (substr($dbType,0,8) != 'postgres'){
		$sqli = ($dict->CreateTableSQL('testtable',$flds, $opts));
		print_sql($dbType,$sqli, "CreateTableSQL");
	}
	$sqli = $dict->CreateIndexSQL('idx','testtable','firstname,lastname',array('BITMAP','FULLTEXT','CLUSTERED','HASH'));
	print_sql($dbType,$sqli, "CreateIndexSQL");
	$sqli = $dict->CreateIndexSQL('idx2','testtable','price,lastname');//,array('BITMAP','FULLTEXT','CLUSTERED'));
	print_sql($dbType,$sqli, "CreateIndexSQL");
	
	$addflds = array(array('height', 'F'),array('weight','F'));
	$sqli = $dict->AddColumnSQL('testtable',$addflds);
	print_sql($dbType,$sqli, "AddColumnSQL");
	$addflds = array(array('height', 'F','NOTNULL'),array('weight','F','NOTNULL'));
	$sqli = $dict->AlterColumnSQL('testtable',$addflds);
	print_sql($dbType,$sqli, "AlterColumnSQL");


	if ($dbType == 'mysql') {
		$db->Connect( $_POST['dbhost'], $_POST['dbusername'], $_POST['dbpassword'], $_POST['databasename'] );
		$dict->SetSchema('');
		$sqla2 = $dict->CreateTableSQL('adoxyz',$flds, $opts);
	    if($_POST['create_test']){
			$debug_query = $dict->ExecuteSQLArray($sqla2);
		}
		if ($sqla2) print_sql($dbType,$sqla2, "CreateTableSQL");
		print_sql($dbType, $dict->ChangeTableSQL('adoxyz','LASTNAME varchar(32)'), "ChangeTableSQL");
	}


	print_sql($dbType, $dict->DropColumnSQL('table',array('my col','`col2_with_Quotes`','A_col3','col3(10)')), "DropColumnSQL");

	unset($db, $dict);
}

$bgcolor[0] = "#ffffff";
$bgcolor[1] = "#eeeeee";
$i = 2;
foreach(array('mysql', 'mysqli', 'mysqlt', 'fbsql', 'maxdb', 'msql', 'mssql', 'mssqlpo', 'odbc', 'postgres', 'postgres64', 'postgres7', 'postgres8', 'sqlite', 'sqlitepo', 'sybase', 'sybase_ase') as $dbType) {
	$color = $i % 2;
	echo "  <tr>
    <td valign=\"top\" bgcolor=" . $bgcolor[$color] . ">";
	test_dictionary($dbType);
	echo "	</td>
  </tr>";
	$i++;
}

?>
   <tr>
    <td colspan="2" valign="top">
<?php
include_once( "../adodb-xmlschema.inc.php" );

echo "<h1>Test XML Schema</h1>";
$ff = file('xmlschema.xml');
echo "<pre>";
foreach($ff as $xml) echo htmlspecialchars($xml);
echo "</pre>";
$db = NewADOConnection( 'mysql', "pear" );
if($_POST['create_test'])
	$db->Connect( $_POST['dbhost'], $_POST['dbusername'], $_POST['dbpassword'], $_POST['databasename'] );

// To create a schema object and build the query array.
$schema = new adoSchema( $db );

// To upgrade an existing schema object, use the following 
// To upgrade an existing database to the provided schema,
// uncomment the following line:
#$schema->upgradeSchema();

print "<b>SQL to build xmlschema.xml</b>:\n<pre>";
// Build the SQL array
$sql = $schema->ParseSchema( "xmlschema.xml" );

print_r( $sql );
print "</pre>\n";

// Execute the SQL on the database
$db->debug = true;
if($_POST['create_test']){
	print "<pre><hr>\n";
	$result = $schema->ExecuteSchema( $sql );
	print "</pre>\n";
}
// Finally, clean up after the XML parser
// (PHP won't do this for you!)
//$schema->Destroy();

print "<pre><hr>\n";
foreach ($sql as $s)
echo "\$db2->Execute(\"$s\");<br><br>";
print "</pre>\n";
?>
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
