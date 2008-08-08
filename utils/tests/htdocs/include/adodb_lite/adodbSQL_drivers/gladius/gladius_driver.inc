<?php
## Gladius Database Engine
# @author legolas558
# @version 0.3.94
# Licensed under GNU General Public License (GPL)
#
#
# ADODB lite main driver

class gladius_driver_ADOConnection extends ADOConnection
{
	var $nameQuote = '';		// identifier delimiters not supported!
	var $sysDate = 'DATE()';	// function not yet available! (v0.3)
	var $sysTimeStamp = 'NOW()';// function not yet available! (v0.3)

	var $g;	// instance of Gladius engine

	function gladius_driver_ADOConnection()
	{
		$this->g = new Gladius();
		$this->g->fetch_mode =& $GLOBALS['ADODB_FETCH_MODE'];
		$this->dbtype = 'gladius';
		$this->dataProvider = 'gladius';
	}

	/**
	 * Connection to database server and selected database
	 *
	 * @access private
	 */

	function _connect($host_ignored, $username_ignored, $password_ignored, $database = '', $persistent_ignored, $forcenew_ignored)
	{
		if ($database!=='')
			return $this->SelectDB( $database );
		else
			$this->database = $database;
		return true;
	}

	/**
	 * Choose a database to connect.
	 *
	 * @param dbname 	is the name of the database to select
	 * @return 			true or false
	 * @access public
	 */

	function SelectDB($dbname)
	{
		$this->database = $dbname;

		return $this->g->SelectDB($dbname);
	}

	/**
	 * Return database error message
	 * Usage: $errormessage =& $db->ErrorMsg();
	 *
	 * @access public
	 */

	function ErrorMsg()
	{
		return $this->g->errstr;
	}

	/**
	 * Return database error number
	 * Usage: $errorbo =& $db->ErrorNo();
	 *
	 * @access public
	 */

	function ErrorNo()
	{
		return $this->g->errno;
	}

	/**
	 * Returns # of affected rows from insert/delete/update query
	 *
	 * @access public
	 * @return integer Affected rows
	 */

	function Affected_Rows()
	{
		return $this->g->affected_rows;
	}

	/**
	 * Returns the last record id of an inserted item
	 * Usage: $db->Insert_ID();
	 *
	 * @access public
	 */

	function Insert_ID()
	{
		return $this->g->insert_id;
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
		return "'".$this->g->escape($string, $magic_quotes)."'";
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
	 * NOT IMPLEMENTED
	 * software emulation?
	 */
	function Concat()
	{
		$arr = func_get_args();
		$list = implode(', ', $arr);

		if (strlen($list) > 0) return "CONCAT($list)";
		else return '';
	}

	function IfNull( $field, $ifNull )
	{
		return false;
	}

	/**
	 * Closes database connection
	 * Usage: $db->close();
	 *
	 * @access public
	 */

	function Close()
	{
		$this->g->Close();
	}

	 /**
	 * Returns All Records in an array
	 *
	 * Usage: $db->GetAll($sql);
	 * @access public
	 */

	function &GetAll($sql, $inputarr = false)
	{
		return $this->GetArray($sql, $inputarr);
	}

	/**
	 * Returns All Records in an array
	 *
	 * Usage: $db->GetArray($sql);
	 * @access public
	 */

	function &GetArray($sql, $inputarr = false)
	{
		$data =& $this->Execute($sql, $inputarr);
		if (!is_bool($data))
			$data = $data->GetArray();
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
		$false = false;

		$limit = '';
		if ($offset != -1 || $nrows != -1)
		{
			$offset = ($offset>=0) ? $offset. ", " : '';
 			$limit = ' LIMIT '.$offset.$nrows;
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
				$recordset = $this->g->Query( $this->sql );
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
				if ($recordset === false) { // error handling if query fails
					if ($fn = $this->raiseErrorFn)
						$fn($this->dbtype, 'EXECUTE', $this->ErrorNo(), $this->ErrorMsg(), $this->sql, $inputarr, $this);
					return $false;
				}
			}
		} else {
				$this->sql = $sql . $limit;
				$time_start = array_sum(explode(' ', microtime()));
				$this->query_count++;
				$recordset = $this->g->query( $this->sql );
				$time_total = (array_sum(explode(' ', microtime())) - $time_start);
				$this->query_time_total += $time_total;
				if($this->debug_console)
				{
					$this->query_list[] = $this->sql;
					$this->query_list_time[] = $time_total;
					$this->query_list_errors[] = $this->ErrorMsg();
				}
				if($this->debug)
					$this->outp($sql . $limit);
		}

		if ($recordset === false) { // error handling if query fails
			if ($fn = $this->raiseErrorFn)
				$fn($this->dbtype, 'EXECUTE', $this->ErrorNo(), $this->ErrorMsg(), $this->sql, $inputarr, $this);
			return $false;
		}

		if ($recordset === true) { // return simplified recordset for inserts/updates/deletes with lower overhead
			$recordset = new ADORecordSet_empty();
			return $recordset;
		}

//		$recordset->_fetch();	// unnecessary

		return $recordset;
	}

}

class gladius_driver_ResultSet extends Gladius_Resultset { }

?>