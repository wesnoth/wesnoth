<?php
/**
 * ADOdb Lite Extend Module for Mysqlt
 * 
 */

eval('class sqlitepo_extend_EXTENDER extends '. $last_module . '_ADOConnection { }');

class sqlitepo_extend_ADOConnection extends sqlitepo_extend_EXTENDER
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

	var $_genSeqSQL = "create table %s (id integer)";
	var $_dropSeqSQL = 'drop table %s';
	var $genID = 0;

	function GenID($seqname='adodbseq', $startID=1)
	{
		$MAXLOOPS = 100;
		while (--$MAXLOOPS>=0) {
			@($num = $this->GetOne("select id from $seq"));
			if ($num === false) {
				$this->Execute(sprintf($this->_genSeqSQL, $seq));	
				$start -= 1;
				$num = '0';
				$result = $this->Execute("insert into $seq values($start)");
				if (!$result)
					return false;
			}
			$this->Execute("update $seq set id=id+1 where id=$num");

			if ($this->affected_rows() > 0) {
				$num += 1;
				$this->genID = $num;
				return $num;
			}
		}
		if ($fn = $this->raiseErrorFn) {
			$fn($this->databaseType, 'GENID',-32000,"Unable to generate unique id after $MAXLOOPS attempts", $seq, $num);
		}
		return false;
	}

	function CreateSequence($seqname='adodbseq', $start=1)
	{
		$ok = $this->Execute(sprintf($this->_genSeqSQL, $seqname));
		if (!$ok)
			return false;
		$start -= 1;
		return $this->Execute("insert into $seqname values($start)");
	}

	function DropSequence($seqname)
	{
		return $this->Execute(sprintf($this->_dropSeqSQL, $seqname));
	}

}

eval('class sqlitepo_extend_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class sqlitepo_extend_ResultSet extends sqlitepo_extend_resultset_EXTENDER
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