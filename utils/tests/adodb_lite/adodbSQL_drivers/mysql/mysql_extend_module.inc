<?php
/**
 * ADOdb Lite Extend Module for Mysqlt
 * 
 */

eval('class mysql_extend_EXTENDER extends '. $last_module . '_ADOConnection { }');

class mysql_extend_ADOConnection extends mysql_extend_EXTENDER
{
	function &GetAssoc($sql, $inputarr=false, $force_array = false, $first2cols = false)
	{
		$data = false;
		$result =& $this->Execute($sql, $inputarr);
		if ($result) {
			$data =& $result->GetAssoc($force_array, $first2cols);
			$result->Close();
		}
		return $data;
	}

	/**
	 * Generates a sequence id and stores it in $this->genID;
	 * GenID is only available if $this->hasGenID = true;
	 *
	 * @param seqname		name of sequence to use
	 * @param startID		if sequence does not exist, start at this ID
	 * @return		0 if not supported, otherwise a sequence id
	 */

	var $genID = 0;
 	var $_genIDSQL = "update %s set id=LAST_INSERT_ID(id+1);";
	var $_genSeqSQL = "create table %s (id int not null)";
	var $_genSeq2SQL = "insert into %s values (%s)";
	var $_dropSeqSQL = "drop table %s";

	function GenID($seqname='adodbseq', $startID=1)
	{
		$getnext = sprintf($this->_genIDSQL, $seqname);
		$holdtransOK = $this->transaction_status;
		$result = @$this->Execute($getnext);
		if (!$result) {
			if ($holdtransOK)
				$this->transaction_status = true;
//			$u = strtoupper($seqname);
			$this->Execute(sprintf($this->_genSeqSQL, $seqname));
			$this->Execute(sprintf($this->_genSeq2SQL, $seqname, $startID-1));
			$result = $this->Execute($getnext);
		}
		$this->genID = mysql_insert_id($this->connectionId);

		if ($result)
			$result->Close();

		return $this->genID;
	}

	function CreateSequence($seqname='adodbseq',$startID=1)
	{
		$u = strtoupper($seqname);

		$ok = $this->Execute(sprintf($this->_genSeqSQL, $seqname));
		if (!$ok)
			return false;
		return $this->Execute(sprintf($this->_genSeq2SQL, $seqname, $startID-1));
	}

	function DropSequence($seqname='adodbseq')
	{
		return $this->Execute(sprintf($this->_dropSeqSQL, $seqname));
	}

}

eval('class mysql_extend_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class mysql_extend_ResultSet extends mysql_extend_resultset_EXTENDER
{
	function &GetAssoc($force_array = false, $first2cols = false) 
	{
		$results = false;
		
		if ($this->_numOfFields > 1) {
			$numIndex = isset($this->fields[0]);
			$results = array();
			if (!$first2cols && ($this->_numOfFields > 2 || $force_array)) {
				if ($numIndex) {
					while (!$this->EOF) {
						$results[trim($this->fields[0])] = array_slice($this->fields, 1);
						$this->MoveNext();
					}
				} else {
					while (!$this->EOF) {
						$results[trim(reset($this->fields))] = array_slice($this->fields, 1);
						$this->MoveNext();
					}
				}
			} else {
				if ($numIndex) {
					while (!$this->EOF) {
						$results[trim(($this->fields[0]))] = $this->fields[1];
						$this->MoveNext();
					}
				} else {
					while (!$this->EOF) {
						$v1 = trim(reset($this->fields));
						$v2 = ''.next($this->fields); 
						$results[$v1] = $v2;
						$this->MoveNext();
					}
				}
			}
		}
		return $results; 
	}

	function PO_RecordCount($table="", $condition="")
	{
		$lnumrows = $this->_numOfRows;
		if($lnumrows == -1 && $this->connectionId)
		{
			if($table)
			{
				if ($condition)
					$condition = " WHERE " . $condition; 
				$resultrows = &$this->connectionId->Execute("SELECT COUNT(*) FROM $table $condition");
				if ($resultrows)
					$lnumrows = reset($resultrows->fields);
			}
		}
		return $lnumrows;
	}

	function CurrentRow()
	{
		return $this->_currentRow;
	}

	function AbsolutePosition()
	{
		return $this->_currentRow;
	}

	function NextRecordSet()
	{
		return false;
	}

}
?>