<?php
/**
 * ADOdb Lite Meta Module for Sybase ASE
 * 
 * Portions of the Meta Coding came from ADOdb
 */

/* 
  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt. 
*/

eval('class sybase_ase_meta_EXTENDER extends '. $last_module . '_ADOConnection { }');

class sybase_ase_meta_ADOConnection extends sybase_ase_meta_EXTENDER
{
	 var $metaTablesSQL="SELECT sysobjects.name FROM sysobjects, sysusers WHERE sysobjects.type='U' AND sysobjects.uid = sysusers.uid";
	 var $metaColumnsSQL = "SELECT syscolumns.name AS field_name, systypes.name AS type, systypes.length AS width FROM sysobjects, syscolumns, systypes WHERE sysobjects.name='%s' AND syscolumns.id = sysobjects.id AND systypes.type=syscolumns.type";
	 var $metaDatabasesSQL ="SELECT a.name FROM master.dbo.sysdatabases a, master.dbo.syslogins b WHERE a.suid = b.suid and a.name like '%' and a.name != 'tempdb' and a.status3 != 256  order by 1";

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
        $sql = "SELECT c.column_name " .
               "FROM syscolumn c, systable t " .
               "WHERE t.table_name='$table' AND c.table_id=t.table_id " .
               "AND t.table_type='BASE' " .
               "AND c.pkey = 'Y' " .
               "ORDER BY c.column_id";

        $a = $this->GetCol($sql);
        if ($a && sizeof($a)>0) return $a;
        return false;
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
			$arr = array();
			if ($this->metaDatabasesSQL!='') {
				$rs = $this->Execute($this->metaDatabasesSQL);
				if ($rs && !$rs->EOF){
					while (!$rs->EOF){
						$arr[] = $rs->Fields('name');
						$rs->MoveNext();
					}
					return $arr;
				}
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
	function &MetaTables($ttype=false,$showSchema=false,$mask=false)
	{
		$false = false;
		if ($this->metaTablesSQL) {
			// complicated state saving by the need for backward compat
			
			if ($ttype == 'VIEWS'){
						$sql = str_replace('U', 'V', $this->metaTablesSQL);
			}elseif (false === $ttype){
						$sql = str_replace('U',"U' OR type='V", $this->metaTablesSQL);
			}else{ // TABLES OR ANY OTHER 
						$sql = $this->metaTablesSQL;
			}
			$rs = $this->Execute($sql);
			
			if ($rs === false || !method_exists($rs, 'GetArray')){
					return $false;
			}
			$arr =& $rs->GetArray();

			$arr2 = array();
			foreach($arr as $key=>$value){
					$arr2[] = trim($value['name']);
			}
			return $arr2;
		}
		return $false;
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
	function &MetaColumns($table,$upper=false) 
	{
		$false = false;
		if (!empty($this->metaColumnsSQL)) {
		
			$rs = $this->Execute(sprintf($this->metaColumnsSQL,$table));
			if ($rs === false) return $false;

			$retarr = array();
			while (!$rs->EOF) {
				$fld =& new ADOFieldObject();
				$fld->name = $rs->Fields('field_name');
				$fld->type = $rs->Fields('type');
				$fld->max_length = $rs->Fields('width');
				$retarr[strtoupper($fld->name)] = $fld;
				$rs->MoveNext();
			}
			$rs->Close();
			return $retarr;	
		}
		return $false;
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

}

eval('class sybase_ase_meta_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class sybase_ase_meta_ResultSet extends sybase_ase_meta_resultset_EXTENDER
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
		case 'INTEGER': return  'I';

		case 'BIT':
		case 'TINYINT': return  'I1';

		case 'SMALLINT': return 'I2';

		case 'BIGINT':  return  'I8';

		case 'REAL':
		case 'FLOAT': return 'F';

		case 'MEDIUMINT':
			
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