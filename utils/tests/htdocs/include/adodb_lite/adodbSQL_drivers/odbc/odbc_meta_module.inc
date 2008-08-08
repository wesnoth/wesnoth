<?php
/**
 * ADOdb Lite Meta Module for ODBC
 * 
 * Portions of the Meta Coding came from ADOdb
 */

/* 
  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt. 
*/

eval('class odbc_meta_EXTENDER extends '. $last_module . '_ADOConnection { }');

class odbc_meta_ADOConnection extends odbc_meta_EXTENDER
{
	var $metaTablesSQL = "SHOW TABLES";	
	var $metaColumnsSQL = "SHOW COLUMNS FROM %s";
	var $uCaseTables = true; // for meta* functions, uppercase table names

	function MetaError($err=false)
	{
		include_once(ADODB_DIR."/adodb-error.inc.php");
		if ($err === false)
			$err = $this->ErrorNo();

		return adodb_error($this->dataProvider,$this->databaseType,$err);
	}

	function MetaErrorMsg($errno)
	{
		include_once(ADODB_DIR."/adodb-error.inc.php");
		return adodb_errormsg($errno);
	}

	/**
	 * @returns an array with the primary key columns in it.
	 */
	function MetaPrimaryKeys($table)
	{
	global $ADODB_FETCH_MODE;
	
		if ($this->uCaseTables) $table = strtoupper($table);
		$schema = '';
		$this->_findschema($table,$schema);

		$savem = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		$qid = @odbc_primarykeys($this->_connectionID,'',$schema,$table);
		
		if (!$qid) {
			$ADODB_FETCH_MODE = $savem;
			return false;
		}
		$rs = new ADORecordSet_odbc($qid);
		$ADODB_FETCH_MODE = $savem;
		
		if (!$rs) return false;
		$rs->_has_stupid_odbc_fetch_api_change = $this->_has_stupid_odbc_fetch_api_change;
		
		$arr =& $rs->GetArray();
		$rs->Close();
		//print_r($arr);
		$arr2 = array();
		for ($i=0; $i < sizeof($arr); $i++) {
			if ($arr[$i][3]) $arr2[] = $arr[$i][3];
		}
		return $arr2;
	}

	/**
	 * @returns assoc array where keys are tables, and values are foreign keys
	 */
	function MetaForeignKeys($table, $owner=false, $upper=false)
	{
		return false;
	}

	// not the fastest implementation - quick and dirty - jlim
	// for best performance, use the actual $rs->MetaType().
	function MetaType($t,$len=-1,$fieldobj=false)
	{
		if (empty($this->_metars)) {
			$rsclass = $this->last_module_name . "_ResultSet";
			$this->_metars =& new $rsclass(false,$this->fetchMode); 
		}

		return $this->_metars->MetaType($t,$len,$fieldobj);
	}

	/**
	 * return the databases that the driver can connect to. 
	 * Some databases will return an empty array.
	 *
	 * @return an array of database names.
	 */
		function MetaDatabases() 
		{
		global $ADODB_FETCH_MODE;
		
			if ($this->metaDatabasesSQL) {
				$save = $ADODB_FETCH_MODE; 
				$ADODB_FETCH_MODE = ADODB_FETCH_NUM; 
				
				if ($this->fetchMode !== false) $savem = $this->SetFetchMode(false);
				
				$arr = $this->GetCol($this->metaDatabasesSQL);
				if (isset($savem)) $this->SetFetchMode($savem);
				$ADODB_FETCH_MODE = $save; 
			
				return $arr;
			}
			
			return false;
		}

	/**
	 * @param ttype can either be 'VIEW' or 'TABLE' or false. 
	 * 		If false, both views and tables are returned.
	 *		"VIEW" returns only views
	 *		"TABLE" returns only tables
	 * @param showSchema returns the schema/user with the table name, eg. USER.TABLE
	 * @param mask  is the input mask - only supported by oci8 and postgresql
	 *
	 * @return  array of tables for current database.
	 */ 
	function &MetaTables($ttype=false)
	{
	global $ADODB_FETCH_MODE;
	
		$savem = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		$qid = odbc_tables($this->_connectionID);
		
		$rs = new ADORecordSet_odbc($qid);
		
		$ADODB_FETCH_MODE = $savem;
		if (!$rs) {
			$false = false;
			return $false;
		}
		$rs->_has_stupid_odbc_fetch_api_change = $this->_has_stupid_odbc_fetch_api_change;
		
		$arr =& $rs->GetArray();
		//print_r($arr);
		
		$rs->Close();
		$arr2 = array();
		
		if ($ttype) {
			$isview = strncmp($ttype,'V',1) === 0;
		}
		for ($i=0; $i < sizeof($arr); $i++) {
			if (!$arr[$i][2]) continue;
			$type = $arr[$i][3];
			if ($ttype) { 
				if ($isview) {
					if (strncmp($type,'V',1) === 0) $arr2[] = $arr[$i][2];
				} else if (strncmp($type,'SYS',3) !== 0) $arr2[] = $arr[$i][2];
			} else if (strncmp($type,'SYS',3) !== 0) $arr2[] = $arr[$i][2];
		}
		return $arr2;
	}

	function _findschema(&$table,&$schema)
	{
		if (!$schema && ($at = strpos($table,'.')) !== false) {
			$schema = substr($table,0,$at);
			$table = substr($table,$at+1);
		}
	}

	/**
	 * List columns in a database as an array of ADOFieldObjects. 
	 * See top of file for definition of object.
	 *
	 * @param $table	table name to query
	 * @param $normalize	makes table name case-insensitive (required by some databases)
	 * @schema is optional database schema to use - not supported by all databases.
	 *
	 * @return  array of ADOFieldObjects for current table.
	 */
	function &MetaColumns($table)
	{
	global $ADODB_FETCH_MODE;
	
		$false = false;
		if ($this->uCaseTables) $table = strtoupper($table);
		$schema = '';
		$this->_findschema($table,$schema);
		
		$savem = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
	
		/*if (false) { // after testing, confirmed that the following does not work becoz of a bug
			$qid2 = odbc_tables($this->_connectionID);
			$rs = new ADORecordSet_odbc($qid2);		
			$ADODB_FETCH_MODE = $savem;
			if (!$rs) return false;
			$rs->_has_stupid_odbc_fetch_api_change = $this->_has_stupid_odbc_fetch_api_change;
			$rs->_fetch();
			
			while (!$rs->EOF) {
				if ($table == strtoupper($rs->fields[2])) {
					$q = $rs->fields[0];
					$o = $rs->fields[1];
					break;
				}
				$rs->MoveNext();
			}
			$rs->Close();
			
			$qid = odbc_columns($this->_connectionID,$q,$o,strtoupper($table),'%');
		} */
		
		switch ($this->databaseType) {
		case 'access':
		case 'vfp':
			$qid = odbc_columns($this->_connectionID);#,'%','',strtoupper($table),'%');
			break;
		
		
		case 'db2':
            $colname = "%";
            $qid = odbc_columns($this->_connectionID, "", $schema, $table, $colname);
            break;
			
		default:
			$qid = @odbc_columns($this->_connectionID,'%','%',strtoupper($table),'%');
			if (empty($qid)) $qid = odbc_columns($this->_connectionID);
			break;
		}
		if (empty($qid)) return $false;
		
		$rs =& new ADORecordSet_odbc($qid);
		$ADODB_FETCH_MODE = $savem;
		
		if (!$rs) return $false;
		$rs->_has_stupid_odbc_fetch_api_change = $this->_has_stupid_odbc_fetch_api_change;
		$rs->_fetch();
		
		$retarr = array();
		
		/*
		$rs->fields indices
		0 TABLE_QUALIFIER
		1 TABLE_SCHEM
		2 TABLE_NAME
		3 COLUMN_NAME
		4 DATA_TYPE
		5 TYPE_NAME
		6 PRECISION
		7 LENGTH
		8 SCALE
		9 RADIX
		10 NULLABLE
		11 REMARKS
		*/
		while (!$rs->EOF) {
		//	adodb_pr($rs->fields);
			if (strtoupper(trim($rs->fields[2])) == $table && (!$schema || strtoupper($rs->fields[1]) == $schema)) {
				$fld = new ADOFieldObject();
				$fld->name = $rs->fields[3];
				$fld->type = $this->ODBCTypes($rs->fields[4]);
				
				// ref: http://msdn.microsoft.com/library/default.asp?url=/archive/en-us/dnaraccgen/html/msdn_odk.asp
				// access uses precision to store length for char/varchar
				if ($fld->type == 'C' or $fld->type == 'X') {
					if ($this->databaseType == 'access') 
						$fld->max_length = $rs->fields[6];
					else if ($rs->fields[4] <= -95) // UNICODE
						$fld->max_length = $rs->fields[7]/2;
					else
						$fld->max_length = $rs->fields[7];
				} else 
					$fld->max_length = $rs->fields[7];
				$fld->not_null = !empty($rs->fields[10]);
				$fld->scale = $rs->fields[8];
				$retarr[strtoupper($fld->name)] = $fld;	
			} else if (sizeof($retarr)>0)
				break;
			$rs->MoveNext();
		}
		$rs->Close(); //-- crashes 4.03pl1 -- why?
		
		if (empty($retarr)) $retarr = false;
		return $retarr;
	}

	/**
	  * List indexes on a table as an array.
	  * @param table  table name to query
	  * @param primary true to only show primary keys. Not actually used for most databases
	  *
	  * @return array of indexes on current table. Each element represents an index, and is itself an associative array.
	  
		 Array (
			[name_of_index] => Array
			  (
			  [unique] => true or false
			  [columns] => Array
			  (
			  	[0] => firstname
			  	[1] => lastname
			  )
		)		
	  */
     function &MetaIndexes($table, $primary = false, $owner = false)
     {
	 		$false = false;
            return $false;
     }

	/**
	 * List columns names in a table as an array. 
	 * @param table	table name to query
	 *
	 * @return  array of column names for current table.
	 */ 
	function &MetaColumnNames($table, $numIndexes=false,$useattnum=false /* only for postgres */) 
	{
		$objarr =& $this->MetaColumns($table);
		if (!is_array($objarr)) {
			$false = false;
			return $false;
		}
		$arr = array();
		if ($numIndexes) {
			$i = 0;
			if ($useattnum) {
				foreach($objarr as $v) 
					$arr[$v->attnum] = $v->name;
				
			} else
				foreach($objarr as $v) $arr[$i++] = $v->name;
		} else
			foreach($objarr as $v) $arr[strtoupper($v->name)] = $v->name;
		
		return $arr;
	}

	function MetaTransaction($mode,$db)
	{
		$mode = strtoupper($mode);
		$mode = str_replace('ISOLATION LEVEL ','',$mode);
		
		switch($mode) {

		case 'READ UNCOMMITTED':
			switch($db) { 
			case 'oci8':
			case 'oracle':
				return 'ISOLATION LEVEL READ COMMITTED';
			default:
				return 'ISOLATION LEVEL READ UNCOMMITTED';
			}
			break;
					
		case 'READ COMMITTED':
				return 'ISOLATION LEVEL READ COMMITTED';
			break;
			
		case 'REPEATABLE READ':
			switch($db) {
			case 'oci8':
			case 'oracle':
				return 'ISOLATION LEVEL SERIALIZABLE';
			default:
				return 'ISOLATION LEVEL REPEATABLE READ';
			}
			break;
			
		case 'SERIALIZABLE':
				return 'ISOLATION LEVEL SERIALIZABLE';
			break;
			
		default:
			return $mode;
		}
	}

	function ODBCTypes($t)
	{
		switch ((integer)$t) {
		case 1:	
		case 12:
		case 0:
		case -95:
		case -96:
			return 'C';
		case -97:
		case -1: //text
			return 'X';
		case -4: //image
			return 'B';
				
		case 9:	
		case 91:
			return 'D';
		
		case 10:
		case 11:
		case 92:
		case 93:
			return 'T';
			
		case 4:
		case 5:
		case -6:
			return 'I';
			
		case -11: // uniqidentifier
			return 'R';
		case -7: //bit
			return 'L';
		
		default:
			return 'N';
		}
	}

}

eval('class odbc_meta_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class odbc_meta_ResultSet extends odbc_meta_resultset_EXTENDER
{
	/**
	 * Get the metatype of the column. This is used for formatting. This is because
	 * many databases use different names for the same type, so we transform the original
	 * type to our standardised version which uses 1 character codes:
	 *
	 * @param t  is the type passed in. Normally is ADOFieldObject->type.
	 * @param len is the maximum length of that field. This is because we treat character
	 * 	fields bigger than a certain size as a 'B' (blob).
	 * @param fieldobj is the field object returned by the database driver. Can hold
	 *	additional info (eg. primary_key for mysql).
	 * 
	 * @return the general type of the data: 
	 *	C for character < 250 chars
	 *	X for teXt (>= 250 chars)
	 *	B for Binary
	 * 	N for numeric or floating point
	 *	D for date
	 *	T for timestamp
	 * 	L for logical/Boolean
	 *	I for integer
	 *	R for autoincrement counter/integer
	 * 
	 *
	*/
	function MetaType($t,$len=-1,$fieldobj=false)
	{
		if (is_object($t)) {
			$fieldobj = $t;
			$t = $fieldobj->type;
			$len = $fieldobj->max_length;
		}
		
		$len = -1; // mysql max_length is not accurate
		switch (strtoupper($t)) {
		case 'STRING': 
		case 'CHAR':
		case 'VARCHAR': 
		case 'TINYBLOB': 
		case 'TINYTEXT': 
		case 'ENUM': 
		case 'SET': 
			if ($len <= $this->blobSize) return 'C';
			
		case 'TEXT':
		case 'LONGTEXT': 
		case 'MEDIUMTEXT':
			return 'X';
			
		// php_mysql extension always returns 'blob' even if 'text'
		// so we have to check whether binary...
		case 'IMAGE':
		case 'LONGBLOB': 
		case 'BLOB':
		case 'MEDIUMBLOB':
			return !empty($fieldobj->binary) ? 'B' : 'X';
			
		case 'YEAR':
		case 'DATE': return 'D';
		
		case 'TIME':
		case 'DATETIME':
		case 'TIMESTAMP': return 'T';
		
		case 'INT': 
		case 'INTEGER':
		case 'BIGINT':
		case 'TINYINT':
		case 'MEDIUMINT':
		case 'SMALLINT': 
			
			if (!empty($fieldobj->primary_key)) return 'R';
			else return 'I';
		
		default:
			static $typeMap = array(
		'VARCHAR' => 'C',
		'VARCHAR2' => 'C',
		'CHAR' => 'C',
		'C' => 'C',
		'STRING' => 'C',
		'NCHAR' => 'C',
		'NVARCHAR' => 'C',
		'VARYING' => 'C',
		'BPCHAR' => 'C',
		'CHARACTER' => 'C',
		'INTERVAL' => 'C',  # Postgres
		'MACADDR' => 'C', # postgres
		##
		'LONGCHAR' => 'X',
		'TEXT' => 'X',
		'NTEXT' => 'X',
		'M' => 'X',
		'X' => 'X',
		'CLOB' => 'X',
		'NCLOB' => 'X',
		'LVARCHAR' => 'X',
		##
		'BLOB' => 'B',
		'IMAGE' => 'B',
		'BINARY' => 'B',
		'VARBINARY' => 'B',
		'LONGBINARY' => 'B',
		'B' => 'B',
		##
		'YEAR' => 'D', // mysql
		'DATE' => 'D',
		'D' => 'D',
		##
		'UNIQUEIDENTIFIER' => 'C', # MS SQL Server
		##
		'TIME' => 'T',
		'TIMESTAMP' => 'T',
		'DATETIME' => 'T',
		'TIMESTAMPTZ' => 'T',
		'T' => 'T',
		'TIMESTAMP WITHOUT TIME ZONE' => 'T', // postgresql
		##
		'BOOL' => 'L',
		'BOOLEAN' => 'L', 
		'BIT' => 'L',
		'L' => 'L',
		##
		'COUNTER' => 'R',
		'R' => 'R',
		'SERIAL' => 'R', // ifx
		'INT IDENTITY' => 'R',
		##
		'INT' => 'I',
		'INT2' => 'I',
		'INT4' => 'I',
		'INT8' => 'I',
		'INTEGER' => 'I',
		'INTEGER UNSIGNED' => 'I',
		'SHORT' => 'I',
		'TINYINT' => 'I',
		'SMALLINT' => 'I',
		'I' => 'I',
		##
		'LONG' => 'N', // interbase is numeric, oci8 is blob
		'BIGINT' => 'N', // this is bigger than PHP 32-bit integers
		'DECIMAL' => 'N',
		'DEC' => 'N',
		'REAL' => 'N',
		'DOUBLE' => 'N',
		'DOUBLE PRECISION' => 'N',
		'SMALLFLOAT' => 'N',
		'FLOAT' => 'N',
		'NUMBER' => 'N',
		'NUM' => 'N',
		'NUMERIC' => 'N',
		'MONEY' => 'N',
		
		## informix 9.2
		'SQLINT' => 'I', 
		'SQLSERIAL' => 'I', 
		'SQLSMINT' => 'I', 
		'SQLSMFLOAT' => 'N', 
		'SQLFLOAT' => 'N', 
		'SQLMONEY' => 'N', 
		'SQLDECIMAL' => 'N', 
		'SQLDATE' => 'D', 
		'SQLVCHAR' => 'C', 
		'SQLCHAR' => 'C', 
		'SQLDTIME' => 'T', 
		'SQLINTERVAL' => 'N', 
		'SQLBYTES' => 'B', 
		'SQLTEXT' => 'X',
		 ## informix 10
		"SQLINT8" => 'I8',
		"SQLSERIAL8" => 'I8',
		"SQLNCHAR" => 'C',
		"SQLNVCHAR" => 'C',
		"SQLLVARCHAR" => 'X',
		"SQLBOOL" => 'L'
		);
		
		$tmap = false;
		$t = strtoupper($t);
		$tmap = (isset($typeMap[$t])) ? $typeMap[$t] : 'N';
			if ($t == 'LONG' && $this->dataProvider == 'oci8') return 'B';
			return $tmap;
		}
	}

}

?>