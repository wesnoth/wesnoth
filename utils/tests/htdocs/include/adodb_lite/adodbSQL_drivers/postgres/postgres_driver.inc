<?php

/**
 * ADOdb Lite is a PHP class to encapsulate multiple database APIs and is compatible with 
 * a subset of the ADODB Command Syntax. 
 * Currently supports Frontbase, MaxDB, miniSQL, MSSQL, MSSQL Pro, MySQLi, MySQLt, MySQL, PostgresSQL,
 * PostgresSQL64, PostgresSQL7, SqLite and Sybase.
 * 
 */

class postgres_driver_ADOConnection extends ADOConnection
{
	var $connect_string;
	var $sysDate = "CURRENT_DATE";
	var $sysTimeStamp = "CURRENT_TIMESTAMP";

	function postgres_driver_ADOConnection()
	{
		$this->dbtype = 'postgres';
		$this->dataProvider = 'postgres';
	}

	/**
	 * Connection to database server and selected database
	 * 
	 * @access private 
	 */

	function _connect($host = "", $username = "", $password = "", $database = "", $persistent, $forcenew)
	{
		if (!function_exists('pg_connect')) return false;

		$this->host = $host;
		$this->username = $this->query_addslashes($username);
		$this->password = $this->query_addslashes($password);
		if (strlen($database) == 0) $database = 'template1';
		$this->database = $this->query_addslashes($database);		

		if ($this->username || $this->password || $this->database) {
			$this->connect_string = $this->host;
		   	if ($this->connect_string)  {
			 	$host = split(":", $this->connect_string);
				if ($host[0]) $this->connect_string = "host=" . $this->query_addslashes($host[0]);
				else $this->connect_string = 'host=localhost';
				if (isset($host[1])) $this->connect_string .= " port=$host[1]";
				else if (!empty($this->port)) $this->connect_string .= " port=" . $this->port;
			}
		   	if ($this->username) $this->connect_string .= " user=" . $this->username;
		   	if ($this->password) $this->connect_string .= " password=" . $this->password;
			if ($this->database) $this->connect_string .= " dbname=" . $this->database;
		}
		else
		{
			$this->connect_string = $this->host;
		}

		$this->persistent = $persistent;
		$this->forcenewconnection = $forcenew;

		$this->_makeconnection();

		if ($this->connectionId === false)
		{
			if ($fn = $this->raiseErrorFn) 
				$fn($this->dbtype, 'CONNECT', $this->ErrorNo(), $this->ErrorMsg(), $this->host, $this->database, $this);
			return $this->SelectDB( $this->database );
		}

		return true;
	} 

	function _makeconnection()
	{
		if($this->persistent == 1)
		{
			$this->connectionId = @pg_pconnect( $this->connect_string );
		}
		else
		{
			if(!$this->forcenewconnection)
			{
				$this->connectionId = @pg_connect( $this->connect_string );
			}
			else
			{
				$this->connectionId = @pg_connect( $this->connect_string, PGSQL_CONNECT_FORCE_NEW );
			}
		}
	}

	function query_addslashes($query)
	{
		$len = strlen($query);
		if ($len == 0)
			return "''";
		if (strncmp($query,"'",1) === 0 && substr($query,$len-1) == "'")
			return $s;
		return "'".addslashes($query)."'";
	}

	/**
	 * Choose a database to connect.
	 *
	 * @param dbname 	is the name of the database to select
	 * @return 		true or false
	 * @access public
	 */

	function SelectDB($dbname)
	{
		$this->database = $dbname;

		if ($this->connectionId === false)
		{
			if($this->createdatabase == true)
			{
				$this->connectionId = @pg_pconnect( $this->connect_string );
				$result = @pg_query($this->connectionId, "CREATE DATABASE " . $this->database );
				if ($result === false) { // error handling if query fails
					$this->connectionId = false;
					return false;
				} 

				$this->_makeconnection();
				if($this->connectionId === false)
				{
					$this->connectionId = false;
					return false;
				}
				else
				{
					return true;
				}
			}
			$this->connectionId = false;
			return false;
		}
		else
		{
			return true;
		}
	} 

	/**
	 * Return database error message
	 * Usage: $errormessage =& $db->ErrorMsg();
	 * 
	 * @access public
	 */

	function ErrorMsg()
	{
		return @pg_last_error($this->connectionId);
	}

	/**
	 * Return database error number
	 * Usage: $errorbo =& $db->ErrorNo();
	 * 
	 * @access public
	 */

	function ErrorNo()
	{
		$error = @pg_last_error( $this->connectionId );
		return strlen($error) ? $error : 0;
	}

	/**
	 * Returns # of affected rows from insert/delete/update query
	 * 
	 * @access public 
	 * @return integer Affected rows
	 */

	function Affected_Rows()
	{
		return @pg_affected_rows($this->record_set);
	} 

	/**
	 * Returns the last record id of an inserted item
	 * Usage: $db->Insert_ID();
	 * 
	 * @access public 
	 */

	function Insert_ID()
	{
		return @pg_getlastoid($this->record_set);
	}

	/**
	 * Correctly quotes a string so that all strings are escape coded.
	 * An example is  $db->qstr("Haven't a clue.");
	 * 
	 * @param string			the string to quote
	 * @param [magic_quotes]	if $s is GET/POST var, set to get_magic_quotes_gpc().
	 *
	 * @return  single-quoted string IE: 'Haven\'t a clue.'
	 */

	function qstr($string, $magic_quotes=false)
	{	
		if (!$magic_quotes) {
			if (strnatcmp(PHP_VERSION, '4.2.0') >= 0) {
				return "'" . pg_escape_string($string) . "'";
			}
			$string = str_replace("'", "\\'", str_replace('\\', '\\\\', str_replace("\0", "\\\0", $string)));
			return  "'" . $string . "'";
		}
		return "'" . str_replace('\\"', '"', $string) . "'";
	}

	function QMagic($string)
	{
		return $this->qstr($string, get_magic_quotes_gpc());
	}

	/**
	 * Returns concatenated string
	 * Usage: $db->Concat($str1,$str2);
	 * 
	 * @return concatenated string
	 */
	function Concat()
	{
		$arr = func_get_args();
		return implode("||", $arr);
	}

	function IfNull( $field, $ifNull ) 
	{
		return " coalesce($field, $ifNull) "; 
	}

	/**
	 * Closes database connection
	 * Usage: $db->close();
	 * 
	 * @access public 
	 */

	function Close()
	{
		@pg_close( $this->connectionId );
		$this->connectionId = false;
	}

	 /**
	 * Returns All Records in an array
	 *
	 * Usage: $db->GetAll($sql);
	 * @access public 
	 */

	function &GetAll($sql, $inputarr = false)
	{
		$data =& $this->GetArray($sql, $inputarr);
		return $data;
	}

	 /**
	 * Returns All Records in an array
	 *
	 * Usage: $db->GetArray($sql);
	 * @access public 
	 */

	function &GetArray($sql, $inputarr = false)
	{
		$data = false;
		$result =& $this->Execute($sql, $inputarr);
		if ($result)
		{
			$data =& $result->GetArray();
			$result->Close();
		}
		return $data;
	}

	/**
	 * Executes SQL query and instantiates resultset methods
	 * 
	 * @access private 
	 * @return mixed Resultset methods
	 */

	function &do_query( $sql, $offset, $nrows, $inputarr=false )
	{
		global $ADODB_FETCH_MODE;

		$false = false;

//		$limit = '';
//		if ($offset != -1 || $nrows != -1)
//		{
//			$offset = ($offset>=0) ? $offset . "," : '';
//			$limit = ' LIMIT ' . $offset . ' ' . $nrows;
//		}

		if ($inputarr && is_array($inputarr)) {
			$sqlarr = explode('?', $sql);
			if (!is_array(reset($inputarr))) $inputarr = array($inputarr);
			foreach($inputarr as $arr) {
				$sql = ''; $i = 0;
				foreach($arr as $v) {
					$sql .= $sqlarr[$i];
					switch(gettype($v)){
						case 'string':
							$sql .= $this->qstr($v);
							break;
						case 'double':
							$sql .= str_replace(',', '.', $v);
							break;
						case 'boolean':
							$sql .= $v ? "'t'" : "'f'";
							break;
						default:
							if ($v === null)
								$sql .= 'NULL';
							else $sql .= $v;
					}
					$i += 1;
				}
				$sql .= $sqlarr[$i];
				if ($i+1 != sizeof($sqlarr))	
					return $false;
				$this->sql = $sql;
				$time_start = array_sum(explode(' ', microtime()));
				$this->query_count++;
				$resultId = @pg_query($this->connectionId, $this->sql );
				$time_total = (array_sum(explode(' ', microtime())) - $time_start);
				$this->query_time_total += $time_total;
				if($this->debug_console)
				{
					$this->query_list[] = $this->sql;
					$this->query_list_time[] = $time_total;
					$this->query_list_errors[] = $this->ErrorMsg();
				}
				if($this->debug)
				{
					$this->outp($sql);
				}
				if ($resultId === false) { // error handling if query fails
					if ($fn = $this->raiseErrorFn)
						$fn($this->dbtype, 'EXECUTE', $this->ErrorNo(), $this->ErrorMsg(), $this->sql, $inputarr, $this);
					return $false;
				} 
			}
		}
		else
		{
				$this->sql = $sql;
				$time_start = array_sum(explode(' ', microtime()));
				$this->query_count++;
				$resultId = @pg_query($this->connectionId, $this->sql );
				$time_total = (array_sum(explode(' ', microtime())) - $time_start);
				$this->query_time_total += $time_total;
				if($this->debug_console)
				{
					$this->query_list[] = $this->sql;
					$this->query_list_time[] = $time_total;
					$this->query_list_errors[] = $this->ErrorMsg();
				}
				if($this->debug)
				{
					$this->outp($sql);
				}
		}

		if ($resultId === false) { // error handling if query fails
			if ($fn = $this->raiseErrorFn)
				$fn($this->dbtype, 'EXECUTE', $this->ErrorNo(), $this->ErrorMsg(), $this->sql, $inputarr, $this);
			return $false;
		} 

		if (@pg_numfields( $resultId ) <= 0) { // return simplified recordset for inserts/updates/deletes with lower overhead
			$recordset = new ADORecordSet_empty();
			$this->record_set = $recordset;
			return $recordset;
		}

		$resultset_name = $this->last_module_name . "_ResultSet";
		$recordset = new $resultset_name( $resultId, $this->connectionId );
		$this->record_set = $recordset;

		$recordset->_currentRow = 0;

		switch ($ADODB_FETCH_MODE)
		{
			case ADODB_FETCH_NUM: $recordset->fetchMode = PGSQL_NUM; break;
			case ADODB_FETCH_ASSOC:$recordset->fetchMode = PGSQL_ASSOC; break;
			default:
			case ADODB_FETCH_DEFAULT:
			case ADODB_FETCH_BOTH:$recordset->fetchMode = PGSQL_BOTH; break;
		}

		$recordset->_numOfRows = @pg_numrows( $resultId );
		if( $recordset->_numOfRows == 0)
		{
			$recordset->EOF = true;
		}
		$recordset->_numOfFields = @pg_numfields( $resultId );
		$recordset->_fetch();

		return $recordset;
	} 
} 

class postgres_driver_ResultSet
{
	var $connectionId;
	var $fields;
	var $resultId;
	var $_currentRow = 0;
	var $_numOfRows = -1;
	var $_numOfFields = -1;
	var $fetchMode;
	var $EOF;

	/**
	 * pgsqlResultSet Constructor
	 * 
	 * @access private 
	 * @param string $record 
	 * @param string $resultId 
	 */

	function postgres_driver_ResultSet( $resultId, $connectionId )
	{
		$this->fields = array();
		$this->connectionId = $connectionId;
		$this->record = array();
		$this->resultId = $resultId;
		$this->EOF = false;
	} 

	/**
	 * Frees resultset
	 * 
	 * @access public 
	 */

	function Close()
	{
		pg_free_result( $this->resultId );
		$this->fields = array();
		$this->resultId = false;
	} 

	/**
	 * Returns field name from select query
	 * 
	 * @access public 
	 * @param string $field
	 * @return string Field name
	 */

	function fields( $field )
	{
		if(empty($field))
		{
			return $this->fields;
		}
		else
		{
			return $this->fields[$field];
		}
	} 

	/**
	 * Returns numrows from select query
	 * 
	 * @access public 
	 * @return integer Numrows
	 */

	function RecordCount()
	{
		return $this->_numOfRows;
	} 

	/**
	 * Returns num of fields from select query
	 * 
	 * @access public 
	 * @return integer numfields
	 */

	function FieldCount()
	{
		return $this->_numOfFields;
	} 

	/**
	 * Returns next record
	 * 
	 * @access public 
	 */

	function MoveNext()
	{
		if (@$this->fields = pg_fetch_array($this->resultId, NULL, $this->fetchMode)) {
			$this->_currentRow += 1;
			return true;
		}
		if (!$this->EOF) {
			$this->_currentRow += 1;
			$this->EOF = true;
		}
		return false;
	} 

	/**
	 * Move to the first row in the recordset. Many databases do NOT support this.
	 *
	 * @return true or false
	 */

	function MoveFirst() 
	{
		if ($this->_currentRow == 0) return true;
		return $this->Move(0);			
	}			

	/**
	 * Returns the Last Record
	 * 
	 * @access public 
	 */

	function MoveLast()
	{
		if ($this->EOF) return false;
		return $this->Move($this->_numOfRows - 1);
	} 

	/**
	 * Random access to a specific row in the recordset. Some databases do not support
	 * access to previous rows in the databases (no scrolling backwards).
	 *
	 * @param rowNumber is the row to move to (0-based)
	 *
	 * @return true if there still rows available, or false if there are no more rows (EOF).
	 */

	function Move($rowNumber = 0) 
	{
		if ($rowNumber == $this->_currentRow) return true;
		$this->EOF = false;
   		if ($this->_numOfRows > 0){
			if ($rowNumber >= $this->_numOfRows - 1){
				$rowNumber = $this->_numOfRows - 1;
			}
  		}

		if ($this->_seek($rowNumber)) {
			$this->_currentRow = $rowNumber;
			if ($this->_fetch()) {
				return true;
			}
			$this->fields = false;	
		}
		$this->EOF = true;
		return false;
	}

	/**
	 * Perform Seek to specific row
	 * 
	 * @access private 
	 */

	function _seek($row)
	{
		if ($this->_numOfRows == 0) return false;
		return @pg_result_seek($this->resultId,$row);
	}

	/**
	 * Fills field array with first database element when query initially executed
	 * 
	 * @access private 
	 */

	function _fetch()
	{
		$this->fields = @pg_fetch_array($this->resultId, NULL, $this->fetchMode);
		return is_array($this->fields);
	}

	/**
	 * Check to see if last record reached
	 * 
	 * @access public 
	 */

	function EOF()
	{
		if( $this->_currentRow < $this->_numOfRows)
		{
			return false;
		}
		else
		{
			$this->EOF = true;
			return true;
		}
	} 

	/**
	 * Returns All Records in an array
	 * 
	 * @access public 
	 * @param [nRows]  is the number of rows to return. -1 means every row.
	 */

	function &GetArray($nRows = -1)
	{
		$results = array();
		$cnt = 0;
		while (!$this->EOF && $nRows != $cnt) {
			$results[] = $this->fields;
			$this->MoveNext();
			$cnt++;
		}
		return $results;
	} 

	function &GetRows($nRows = -1) 
	{
		$arr =& $this->GetArray($nRows);
		return $arr;
	}

	function &GetAll($nRows = -1)
	{
		$arr =& $this->GetArray($nRows);
		return $arr;
	}

	/**
	* Fetch field information for a table. 
	*
	* @return object containing the name, type and max_length
	*/
	function FetchField($fieldOffset = -1) 
	{
		$fieldObject= new ADOFieldObject();
		$fieldObject->name = @pg_fieldname($this->resultId, $fieldOffset);
		$fieldObject->type = @pg_fieldtype($this->resultId, $fieldOffset);
		$fieldObject->max_length = @pg_fieldsize($this->resultId, $fieldOffset);
		return $fieldObject;
	}
}
?>