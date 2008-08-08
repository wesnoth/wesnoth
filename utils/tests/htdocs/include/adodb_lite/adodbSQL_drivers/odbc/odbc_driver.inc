<?php

/**
 * ADOdb Lite is a PHP class to encapsulate multiple database APIs and is compatible with 
 * a subset of the ADODB Command Syntax. 
 * Currently supports Frontbase, MaxDB, miniSQL, MSSQL, MSSQL Pro, MySQLi, MySQLt, MySQL, PostgresSQL,
 * PostgresSQL64, PostgresSQL7, SqLite and Sybase.
 * 
 */

class odbc_driver_ADOConnection extends ADOConnection
{
	function odbc_driver_ADOConnection()
	{
		$this->dbtype = 'odbc';
		$this->dataProvider = 'odbc';
	}

	/**
	 * Connection to database server and selected database
	 * 
	 * @access private 
	 */

	function _connect($host = "", $username = "", $password = "", $database = "", $persistent, $forcenew)
	{
		if (!function_exists('odbc_connect')) return false;

		$this->host = $host;
		$this->username = $username;
		$this->password = $password;
		$this->database = $database;		
		$this->persistent = $persistent;
		$this->forcenewconnection = $forcenew;

		if($this->persistent == 1)
		{
			$this->connectionId = @odbc_pconnect( $this->host, $this->username, $this->password );
		}
		else
		{
			$this->connectionId = @odbc_connect( $this->host, $this->username, $this->password, $this->forcenewconnection );
		}

		if ($this->connectionId === false)
		{
			if ($fn = $this->raiseErrorFn) 
				$fn($this->dbtype, 'CONNECT', $this->ErrorNo(), $this->ErrorMsg(), $this->host, $this->database, $this);
			return false;
		}

		return true;
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
		return false;
	} 

	/**
	 * Return database error message
	 * Usage: $errormessage =& $db->ErrorMsg();
	 * 
	 * @access public
	 */

	function ErrorMsg()
	{
		if (strnatcmp(PHP_VERSION, '4.0.5') >= 0)
		{
			if ($this->connectionId)
			{
				return @odbc_errormsg();
			}
			else
			{
				return @odbc_errormsg($this->connectionId);
			}
		}
	}

	/**
	 * Return database error number
	 * Usage: $errorbo =& $db->ErrorNo();
	 * 
	 * @access public
	 */

	function ErrorNo()
	{
		if (strnatcmp(PHP_VERSION, '4.0.5') >= 0)
		{
			if ($this->connectionId)
			{
				return @odbc_error();
			}
			else
			{
				return @odbc_error($this->connectionId);
			}
		}
	}

	/**
	 * Returns # of affected rows from insert/delete/update query
	 * 
	 * @access public 
	 * @return integer Affected rows
	 */

	function Affected_Rows()
	{
		return false;
	} 

	/**
	 * Returns the last record id of an inserted item
	 * Usage: $db->Insert_ID();
	 * 
	 * @access public 
	 */

	function Insert_ID()
	{
		return false;
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
		return implode("+", $arr);
	}

	function IfNull( $field, $ifNull ) 
	{
		return " CASE WHEN $field is null THEN $ifNull ELSE $field END ";
	}

	/**
	 * Closes database connection
	 * Usage: $db->close();
	 * 
	 * @access public 
	 */

	function Close()
	{
		@odbc_close( $this->connectionId );
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

		$limit = '';
		if ($offset != -1 || $nrows != -1)
		{
			$offset = ($offset>=0) ? $offset . "," : '';
 			$limit = ' LIMIT ' . $offset . ' ' . $nrows;
		}

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
							$sql .= $v ? 1 : 0;
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
				$this->sql = $sql . $limit;
				$time_start = array_sum(explode(' ', microtime()));
				$this->query_count++;
				$resultId = @odbc_exec( $this->connectionId, $this->sql );
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
					$this->outp($sql . $limit);
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
				$this->sql = $sql . $limit;
				$time_start = array_sum(explode(' ', microtime()));
				$this->query_count++;
				$resultId = @odbc_exec( $this->connectionId, $this->sql );
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
					$this->outp($sql . $limit);
				}
		}

		if ($resultId === false) { // error handling if query fails
			if ($fn = $this->raiseErrorFn)
				$fn($this->dbtype, 'EXECUTE', $this->ErrorNo(), $this->ErrorMsg(), $this->sql, $inputarr, $this);
			return $false;
		} 

		if ($resultId === true) { // return simplified recordset for inserts/updates/deletes with lower overhead
			$recordset = new ADORecordSet_empty();
			return $recordset;
		}

		$resultset_name = $this->last_module_name . "_ResultSet";
		$recordset = new $resultset_name( $resultId, $this->connectionId );

		$recordset->_currentRow = 0;

		switch ($ADODB_FETCH_MODE)
		{
			case ADODB_FETCH_NUM: $recordset->fetchMode = ADODB_FETCH_NUM; break;
			case ADODB_FETCH_ASSOC: $recordset->fetchMode = ADODB_FETCH_ASSOC; break;
			default:
			case ADODB_FETCH_DEFAULT:
			case ADODB_FETCH_BOTH: $recordset->fetchMode = ADODB_FETCH_BOTH; break;
		}

		$recordset->_numOfRows = @odbc_num_rows( $resultId );
		if( $recordset->_numOfRows == 0)
		{
			$recordset->EOF = true;
		}
		$recordset->_numOfFields = @odbc_num_fields( $resultId );
		$recordset->_fetch();

		return $recordset;
	} 
} 

class odbc_driver_ResultSet
{
	var $connectionId;
	var $fields;
	var $resultId;
	var $_currentRow = 0;
	var $_numOfRows = -1;
	var $_numOfFields = -1;
	var $fetchMode;
	var $EOF;
	var $odbcchange;
	var $temp;

	/**
	 * odbcResultSet Constructor
	 * 
	 * @access private 
	 * @param string $record 
	 * @param string $resultId 
	 */

	function odbc_driver_ResultSet( $resultId, $connectionId )
	{
		$this->fields = array();
		$this->connectionId = $connectionId;
		$this->record = array();
		$this->resultId = $resultId;
		$this->EOF = false;
		$this->odbcchange = strnatcmp(PHP_VERSION, '4.2.0');
	} 

	/**
	 * Frees resultset
	 * 
	 * @access public 
	 */

	function Close()
	{
		@odbc_close( $this->resultId );
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
		if ($this->odbcchange >= 0){
			$result = @odbc_fetch_into($this->resultId, $this->fields);
		}
		else
		{
			$row = 0;
			$result = @odbc_fetch_into($this->resultId, $row, $this->fields);
		}
		if ($result) {
			if ($this->fetchMode & ADODB_FETCH_ASSOC) {
				$this->fields =& $this->GetRowAssoc(ADODB_ASSOC_CASE);
			}
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
		return false;
	}

	/**
	 * Fills field array with first database element when query initially executed
	 * 
	 * @access private 
	 */

	function _fetch()
	{
		if ($this->odbcchange >= 0){
			$result = @odbc_fetch_into($this->resultId, $this->fields);
		}
		else
		{
			$row = 0;
			$result = @odbc_fetch_into($this->resultId, $row, $this->fields);
		}
		if ($result) {
			if ($this->fetchMode & ADODB_FETCH_ASSOC) {
				$this->fields =& $this->GetRowAssoc(ADODB_ASSOC_CASE);
			}
			return true;
		}

		$this->fields = false;
		return false;
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
		$fieldOffset += 1; // offsets begin at 1
		
		$fieldObject= new ADOFieldObject();
		$fieldObject->name = @odbc_field_name($this->resultId, $fieldOffset);
		$fieldObject->type = @odbc_field_type($this->resultId, $fieldOffset);
		$fieldObject->max_length = @odbc_field_len($this->resultId, $fieldOffset);
		return $fieldObject;
	}

	function GetRowAssoc($upper=1)
	{
		$record = array();
	   	if (!$this->temp) {
			$this->temp = array();
			for ($i=0; $i < $this->_numOfFields; $i++) {
				$object = $this->FetchField($i);
				if ($upper === 2) $this->temp[$object->name] = $i;
				else $this->temp[($upper) ? strtoupper($object->name) : strtolower($object->name)] = $i;
			}
		}
		foreach($this->temp as $k => $v) {
			$record[$k] = $this->fields[$v];
		}
		return $record;
	}

}

?>