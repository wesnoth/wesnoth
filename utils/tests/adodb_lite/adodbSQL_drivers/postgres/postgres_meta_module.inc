<?php
/**
 * ADOdb Lite Meta Module for Postgres
 * 
 * Portions of the Meta Coding came from ADOdb
 */

/* 
  (c) 2000-2005 John Lim (jlim@natsoft.com.my). All rights reserved.
  Released under both BSD license and Lesser GPL library license. 
  Whenever there is any discrepancy between the two licenses, 
  the BSD license will take precedence. See License.txt. 
*/

eval('class postgres_meta_EXTENDER extends '. $last_module . '_ADOConnection { }');

class postgres_meta_ADOConnection extends postgres_meta_EXTENDER
{
	var $metaDatabasesSQL = "select datname from pg_database where datname not in ('template0','template1') order by 1";
    var $metaTablesSQL = "select tablename,'T' from pg_tables where tablename not like 'pg\_%'
	and tablename not in ('sql_features', 'sql_implementation_info', 'sql_languages',
	 'sql_packages', 'sql_sizing', 'sql_sizing_profiles') 
	union 
        select viewname,'V' from pg_views where viewname not like 'pg\_%'";
	var $metaColumnsSQL = "SELECT a.attname,t.typname,a.attlen,a.atttypmod,a.attnotnull,a.atthasdef,a.attnum 
		FROM pg_class c, pg_attribute a,pg_type t 
		WHERE relkind in ('r','v') AND (c.relname='%s' or c.relname = lower('%s')) and a.attname not like '....%%'
AND a.attnum > 0 AND a.atttypid = t.oid AND a.attrelid = c.oid ORDER BY a.attnum";

	// used when schema defined
	var $metaColumnsSQL1 = "SELECT a.attname, t.typname, a.attlen, a.atttypmod, a.attnotnull, a.atthasdef, a.attnum 
FROM pg_class c, pg_attribute a, pg_type t, pg_namespace n 
WHERE relkind in ('r','v') AND (c.relname='%s' or c.relname = lower('%s'))
 and c.relnamespace=n.oid and n.nspname='%s' 
	and a.attname not like '....%%' AND a.attnum > 0 
	AND a.atttypid = t.oid AND a.attrelid = c.oid ORDER BY a.attnum";
	
	// get primary key etc -- from Freek Dijkstra
	var $metaKeySQL = "SELECT ic.relname AS index_name, a.attname AS column_name,i.indisunique AS unique_key, i.indisprimary AS primary_key 
	FROM pg_class bc, pg_class ic, pg_index i, pg_attribute a WHERE bc.oid = i.indrelid AND ic.oid = i.indexrelid AND (i.indkey[0] = a.attnum OR i.indkey[1] = a.attnum OR i.indkey[2] = a.attnum OR i.indkey[3] = a.attnum OR i.indkey[4] = a.attnum OR i.indkey[5] = a.attnum OR i.indkey[6] = a.attnum OR i.indkey[7] = a.attnum) AND a.attrelid = bc.oid AND bc.relname = '%s'";
	var $metaDefaultsSQL = "SELECT d.adnum as num, d.adsrc as def from pg_attrdef d, pg_class c where d.adrelid=c.oid and c.relname='%s' order by d.adnum";

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
	function MetaPrimaryKeys($table, $owner=false)
	{
	// owner not used in base class - see oci8
		$p = array();
		$objs =& $this->MetaColumns($table);
		if ($objs) {
			foreach($objs as $v) {
				if (!empty($v->primary_key))
					$p[] = $v->name;
			}
		}
		if (sizeof($p)) return $p;
		if (function_exists('ADODB_VIEW_PRIMARYKEYS'))
			return ADODB_VIEW_PRIMARYKEYS($this->databaseType, $this->database, $table, $owner);
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
	function &MetaTables($ttype=false,$showSchema=false,$mask=false) 
	{
		$info = $this->ServerInfo();
		if ($info['version'] >= 7.3) {
	    	$this->metaTablesSQL = "select tablename,'T' from pg_tables where tablename not like 'pg\_%'
			  and schemaname  not in ( 'pg_catalog','information_schema')
	union 
        select viewname,'V' from pg_views where viewname not like 'pg\_%'  and schemaname  not in ( 'pg_catalog','information_schema') ";
		}
		if ($mask) {
			$save = $this->metaTablesSQL;
			$mask = $this->qstr(strtolower($mask));
			if ($info['version']>=7.3)
				$this->metaTablesSQL = "
select tablename,'T' from pg_tables where tablename like $mask and schemaname not in ( 'pg_catalog','information_schema')  
 union 
select viewname,'V' from pg_views where viewname like $mask and schemaname  not in ( 'pg_catalog','information_schema')  ";
			else
				$this->metaTablesSQL = "
select tablename,'T' from pg_tables where tablename like $mask 
 union 
select viewname,'V' from pg_views where viewname like $mask";
		}
		$ret =& $this->_MetaTables($ttype,$showSchema);
		
		if ($mask) {
			$this->metaTablesSQL = $save;
		}
		return $ret;
	}

	function &_MetaTables($ttype=false,$showSchema=false,$mask=false) 
	{
		global $ADODB_FETCH_MODE;

		$false = false;
		if ($mask) {
			return $false;
		}
		if ($this->metaTablesSQL) {
			$save = $ADODB_FETCH_MODE; 
			$ADODB_FETCH_MODE = ADODB_FETCH_NUM; 
			
			if ($this->fetchMode !== false) $savem = $this->SetFetchMode(false);
			
			$rs = $this->Execute($this->metaTablesSQL);
			if (isset($savem)) $this->SetFetchMode($savem);
			$ADODB_FETCH_MODE = $save; 
			
			if ($rs === false) return $false;
			$arr =& $rs->GetArray();
			$arr2 = array();
			
			if ($hast = ($ttype && isset($arr[0][1]))) { 
				$showt = strncmp($ttype,'T',1);
			}
			
			for ($i=0; $i < sizeof($arr); $i++) {
				if ($hast) {
					if ($showt == 0) {
						if (strncmp($arr[$i][1],'T',1) == 0) $arr2[] = trim($arr[$i][0]);
					} else {
						if (strncmp($arr[$i][1],'V',1) == 0) $arr2[] = trim($arr[$i][0]);
					}
				} else
					$arr2[] = trim($arr[$i][0]);
			}
			$rs->Close();
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
	function &MetaColumns($table,$normalize=true) 
	{
	global $ADODB_FETCH_MODE;
	
		$schema = false;
		$false = false;
		$this->_findschema($table,$schema);
		
		if ($normalize) $table = strtolower($table);

		$save = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = ADODB_FETCH_NUM;
		if ($this->fetchMode !== false) $savem = $this->SetFetchMode(false);
		
		if ($schema) $rs =& $this->Execute(sprintf($this->metaColumnsSQL1,$table,$table,$schema));
		else $rs =& $this->Execute(sprintf($this->metaColumnsSQL,$table,$table));
		if (isset($savem)) $this->SetFetchMode($savem);
		$ADODB_FETCH_MODE = $save;
		
		if ($rs === false) {
			return $false;
		}
		if (!empty($this->metaKeySQL)) {
			// If we want the primary keys, we have to issue a separate query
			// Of course, a modified version of the metaColumnsSQL query using a 
			// LEFT JOIN would have been much more elegant, but postgres does 
			// not support OUTER JOINS. So here is the clumsy way.
			
			$ADODB_FETCH_MODE = ADODB_FETCH_ASSOC;
			
			$rskey = $this->Execute(sprintf($this->metaKeySQL,($table)));
			// fetch all result in once for performance.
			$keys =& $rskey->GetArray();
			if (isset($savem)) $this->SetFetchMode($savem);
			$ADODB_FETCH_MODE = $save;
			
			$rskey->Close();
			unset($rskey);
		}

		$rsdefa = array();
		if (!empty($this->metaDefaultsSQL)) {
			$ADODB_FETCH_MODE = ADODB_FETCH_ASSOC;
			$sql = sprintf($this->metaDefaultsSQL, ($table));
			$rsdef = $this->Execute($sql);
			if (isset($savem)) $this->SetFetchMode($savem);
			$ADODB_FETCH_MODE = $save;
			
			if ($rsdef) {
				while (!$rsdef->EOF) {
					$num = $rsdef->fields['num'];
					$s = $rsdef->fields['def'];
					if (strpos($s,'::')===false && substr($s, 0, 1) == "'") { /* quoted strings hack... for now... fixme */
						$s = substr($s, 1);
						$s = substr($s, 0, strlen($s) - 1);
					}

					$rsdefa[$num] = $s;
					$rsdef->MoveNext();
				}
			} else {
				ADOConnection::outp( "==> SQL => " . $sql);
			}
			unset($rsdef);
		}
	
		$retarr = array();
		while (!$rs->EOF) { 	
			$fld = new ADOFieldObject();
			$fld->name = $rs->fields[0];
			$fld->type = $rs->fields[1];
			$fld->max_length = $rs->fields[2];
			$fld->attnum = $rs->fields[6];
			
			if ($fld->max_length <= 0) $fld->max_length = $rs->fields[3]-4;
			if ($fld->max_length <= 0) $fld->max_length = -1;
			if ($fld->type == 'numeric') {
				$fld->scale = $fld->max_length & 0xFFFF;
				$fld->max_length >>= 16;
			}
			// dannym
			// 5 hasdefault; 6 num-of-column
			$fld->has_default = ($rs->fields[5] == 't');
			if ($fld->has_default) {
				$fld->default_value = $rsdefa[$rs->fields[6]];
			}

			//Freek
			$fld->not_null = $rs->fields[4] == 't';
			
			
			// Freek
			if (is_array($keys)) {
				foreach($keys as $key) {
					if ($fld->name == $key['column_name'] AND $key['primary_key'] == 't') 
						$fld->primary_key = true;
					if ($fld->name == $key['column_name'] AND $key['unique_key'] == 't') 
						$fld->unique = true; // What name is more compatible?
				}
			}
			
			if ($ADODB_FETCH_MODE == ADODB_FETCH_NUM) $retarr[] = $fld;	
			else $retarr[($normalize) ? strtoupper($fld->name) : $fld->name] = $fld;
			
			$rs->MoveNext();
		}
		$rs->Close();
		if (empty($retarr))
			return  $false;
		else
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
	  function &MetaIndexes ($table, $primary = FALSE)
      {
         global $ADODB_FETCH_MODE;
                
				$schema = false;
				$this->_findschema($table,$schema);

				if ($schema) { // requires pgsql 7.3+ - pg_namespace used.
					$sql = '
SELECT c.relname as "Name", i.indisunique as "Unique", i.indkey as "Columns" 
FROM pg_catalog.pg_class c 
JOIN pg_catalog.pg_index i ON i.indexrelid=c.oid 
JOIN pg_catalog.pg_class c2 ON c2.oid=i.indrelid
	,pg_namespace n 
WHERE (c2.relname=\'%s\' or c2.relname=lower(\'%s\')) and c.relnamespace=c2.relnamespace and c.relnamespace=n.oid and n.nspname=\'%s\'';
				} else {
	                $sql = '
SELECT c.relname as "Name", i.indisunique as "Unique", i.indkey as "Columns"
FROM pg_catalog.pg_class c
JOIN pg_catalog.pg_index i ON i.indexrelid=c.oid
JOIN pg_catalog.pg_class c2 ON c2.oid=i.indrelid
WHERE (c2.relname=\'%s\' or c2.relname=lower(\'%s\'))';
    			}
				            
                if ($primary == FALSE) {
                	$sql .= ' AND i.indisprimary=false;';
                }
                
                $save = $ADODB_FETCH_MODE;
                $ADODB_FETCH_MODE = ADODB_FETCH_NUM;
                if ($this->fetchMode !== FALSE) {
                        $savem = $this->SetFetchMode(FALSE);
                }
                
                $rs = $this->Execute(sprintf($sql,$table,$table,$schema));
                if (isset($savem)) {
                        $this->SetFetchMode($savem);
                }
                $ADODB_FETCH_MODE = $save;

                if (!is_object($rs)) {
                	$false = false;
					return $false;
                }
				
                $col_names = $this->MetaColumnNames($table,true,true); 
				//3rd param is use attnum, 
				// see http://sourceforge.net/tracker/index.php?func=detail&aid=1451245&group_id=42718&atid=433976
                $indexes = array();
                while ($row = $rs->FetchRow()) {
                        $columns = array();
                        foreach (explode(' ', $row[2]) as $col) {
                                $columns[] = $col_names[$col];
                        }
                        
                        $indexes[$row[0]] = array(
                                'unique' => ($row[1] == 't'),
                                'columns' => $columns
                        );
                }
                return $indexes;
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

eval('class postgres_meta_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class postgres_meta_ResultSet extends postgres_meta_resultset_EXTENDER
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
		$is_serial = is_object($fieldobj) && $fieldobj->primary_key && $fieldobj->unique && $fieldobj->has_default && substr($fieldobj->default_value,0,8) == 'nextval(';
		switch (strtoupper($t)) {
			case 'MONEY': // stupid, postgres expects money to be a string
			case 'INTERVAL':
			case 'CHAR':
			case 'CHARACTER':
			case 'VARCHAR':
			case 'NAME':
	   		case 'BPCHAR':
			case '_VARCHAR':
			case 'INET':
			case 'MACADDR':
				if ($len <= $this->blobSize) return 'C';
			case 'TEXT':
				return 'X';
			case 'IMAGE': // user defined type
			case 'BLOB': // user defined type
			case 'BIT':	// This is a bit string, not a single bit, so don't return 'L'
			case 'VARBIT':
			case 'BYTEA':
				return 'B';
			case 'BOOL':
			case 'BOOLEAN':
				return 'L';
			case 'DATE':
				return 'D';
			case 'TIMESTAMP WITHOUT TIME ZONE':
			case 'TIME':
			case 'DATETIME':
			case 'TIMESTAMP':
			case 'TIMESTAMPTZ':
				return 'T';
			case 'INTEGER': return !$is_serial ? 'I' : 'R';
			case 'SMALLINT': 
			case 'INT2': return !$is_serial ? 'I2' : 'R';
			case 'INT4': return !$is_serial ? 'I4' : 'R';
			case 'BIGINT': 
			case 'INT8': return !$is_serial ? 'I8' : 'R';
			case 'OID':
			case 'SERIAL':
				return 'R';
			case 'FLOAT4':
			case 'FLOAT8':
			case 'DOUBLE PRECISION':
			case 'REAL':
				return 'F';
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