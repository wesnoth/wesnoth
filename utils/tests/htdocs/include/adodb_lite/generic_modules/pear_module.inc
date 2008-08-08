<?php
// Generic Pear Module
// This module contains alternative Pear commands.

eval('class pear_EXTENDER extends ' . $last_module . '_ADOConnection { }');

class pear_ADOConnection extends pear_EXTENDER
{
	/**
	* Set how select queries will retrieve data.
	* Usage: $oldmode = $db->SetFetchMode($mode)
	*
	* @param mode	The fetchmode ADODB_FETCH_ASSOC or ADODB_FETCH_NUM
	* @returns		The previous fetch mode
	*/
	function SetFetchMode($mode)
	{	
		GLOBAL $ADODB_FETCH_MODE;
		$old = $ADODB_FETCH_MODE;
		$ADODB_FETCH_MODE = $mode;
		return $old;
	}

	/**
	 * Returns the last record id of an inserted item
	 * Usage: $db->GetCol($sql);
	 * 
	 * @access public 
	 */

	function GetCol($sql, $inputarr = false, $trim = false)
	{
		$data = false;
		$result =& $this->do_query($sql, -1, -1, $inputarr);
		if ($result) {
			$data = array();
			while (!$result->EOF) {
				$data[] = ($trim) ? trim(reset($result->fields)) : reset($result->fields);
				$result->MoveNext();
			}
			$result->Close();
		}
		return $data;
	}

	 /**
	 * Return first element of first row of sql statement. Recordset is disposed
	 * for you.
	 *
	 * Usage: $db->GetOne($sql);
	 * @access public 
	 */

	function &GetOne($sql, $inputarr = false)
	{
		$data =& $this->GetRow($sql, $inputarr, true);
		return $data;
	}

	 /**
	 * Return one row of sql statement. Recordset is disposed for you.
	 *
	 * Usage: $db->GetRow($sql);
	 * @access public 
	 */

	function &GetRow($sql, $inputarr = false, $getone = false)
	{
		$data = false;
		$result =& $this->do_query($sql, -1, -1, $inputarr);
		if ($result) {
			if ($getone)
			{
				if (!$result->EOF) $data = reset($result->fields);
			}
			else
			{
				if (!$result->EOF) $data = $result->fields;
				else $data = array();
			}
			$result->Close();
		}
		return $data;
	}

	/**
	* PEAR DB Compat - do not use internally
	*/
	function &Query($sql, $inputarr = false)
	{
		$rs =& $this->do_query($sql, -1, -1, $inputarr);
		return $rs;
	}

	/**
	* PEAR DB Compat - do not use internally
	*/
	function &LimitQuery($sql, $offset, $nrows, $inputarr = false)
	{
		$rs =& $this->do_query($sql, $nrows, $offset, $inputarr); 
		return $rs;
	}

	/**
	* PEAR DB Compat - do not use internally
	*/
	function Disconnect()
	{
		return $this->Close();
	}

	/**
	* PEAR DB Compat - do not use internally
	*/
	function ErrorNative()
	{
		return $this->ErrorNo();
	}

	/**
	* PEAR DB Compat - do not use internally
	*/
	function Quote($string)
	{
		return $this->qstr($string, false);
	}

}

eval('class pear_resultset_EXTENDER extends ' . $last_module . '_ResultSet { }');

class pear_ResultSet extends pear_resultset_EXTENDER
{
	/**
	* PEAR DB Compatable Command
	*/
	function Free()
	{
		return $this->Close();
	}

	/**
	* PEAR DB Compatable Command
	*/
	function NumRows()
	{
		return $this->_numOfRows;
	}

	/**
	* PEAR DB Compatable Command
	*/
	function NumCols()
	{
		return $this->_numOfFields;
	}

	/**
	* Fetch a row, returning false if no more rows. 
	* PEAR DB Compatable Command
	*
	* @return false or array containing the current record
	*/
	function FetchRow()
	{
		if ($this->EOF) {
			$false = false;
			return $false;
		}
		$arr = $this->fields;
		$this->_currentRow++;
		if (!$this->_fetch()) $this->EOF = true;
		return $arr;
	}

	/**
	* Fetch a row, returning PEAR_Error if no more rows. 
	* PEAR DB Compatable Command
	*
	*/
	function FetchInto(&$arr)
	{
		$false = false;
		$true = 1;
		if ($this->EOF) return $false;
		$arr = $this->fields;
		$this->MoveNext();
		return $true;
	}

}
?>