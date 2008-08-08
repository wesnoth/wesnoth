<?php
/**
 * ADOdb Lite Transaction Module for Sybase
 * 
 */

eval('class sybase_ase_transaction_EXTENDER extends '. $last_module . '_ADOConnection { }');

class sybase_ase_transaction_ADOConnection extends sybase_ase_transaction_EXTENDER
{
	var $autoCommit = true;
	var $transOff = 0;
	var $transCnt = 0;
	var $transaction_status = true;

	function StartTrans($errfn = 'ADODB_TransMonitor')
	{
		if ($this->transOff > 0) {
			$this->transOff += 1;
			return;
		}
		$this->transaction_status = true;

		if ($this->debug && $this->transCnt > 0)
			ADOConnection::outp("Bad Transaction: StartTrans called within BeginTrans");

		$this->BeginTrans();
		$this->transOff = 1;
	}

	function BeginTrans()
	{
		if ($this->transOff)
			return true;

		$this->transCnt += 1;
		$this->Execute('BEGIN TRAN');
		return true;
	}

	function CompleteTrans($autoComplete = true)
	{
		if ($this->transOff > 1) {
			$this->transOff -= 1;
			return true;
		}
		$this->transOff = 0;
		if ($this->transaction_status && $autoComplete) {
			if (!$this->CommitTrans()) {
				$this->transaction_status = false;
				if ($this->debug)
					ADOConnection::outp("Smart Commit failed");
			} else
				if ($this->debug)
					ADOConnection::outp("Smart Commit occurred");
		} else {
			$this->RollbackTrans();
			if ($this->debug)
				ADOCOnnection::outp("Smart Rollback occurred");
		}
		return $this->transaction_status;
	}

	function CommitTrans($ok=true) 
	{
		if ($this->transOff)
			return true;

		if (!$ok)
			return $this->RollbackTrans();

		$this->transCnt -= 1;
		$this->Execute('COMMIT TRAN');
		return true;
	}

	function RollbackTrans()
	{
		if ($this->transOff)
			return true;

		$this->transCnt -= 1;
		$this->Execute('ROLLBACK TRAN');
		return true;
	}

	function FailTrans()
	{
		if ($this->debug) 
			if ($this->transOff == 0) {
				ADOConnection::outp("FailTrans outside StartTrans/CompleteTrans");
			} else {
				ADOConnection::outp("FailTrans was called");
			}
		$this->transaction_status = false;
	}

	function HasFailedTrans()
	{
		if ($this->transOff > 0)
			return $this->transaction_status == false;
		return false;
	}

	function RowLock($tables,$where,$flds='top 1 null as ignore') 
	{
		if (!$this->_hastrans)
			$this->BeginTrans();

		$tables = str_replace(',',' HOLDLOCK,',$tables);
		return $this->GetOne("select $flds from $tables HOLDLOCK where $where");
	}

	function CommitLock($table)
	{
		return $this->CommitTrans();
	}

	function RollbackLock($table)
	{
		return $this->RollbackTrans();
	}

}

eval('class sybase_ase_transaction_resultset_EXTENDER extends '. $last_module . '_ResultSet { }');

class sybase_ase_transaction_ResultSet extends sybase_ase_transaction_resultset_EXTENDER
{
}
?>