<?php

class Build {
	private $db;
	private $id;
	private $svn_version;
	private $time;
	private $status;
	private $error_msg;

	private $binary_name;
	private $previous_id;
	private $result;
	private $errors;

	const S_GOOD 	= 0;
	const S_ERROR 	= 1;

	function __construct($revision = -1)
	{
		global $db;
		$this->db = $db;
		$this->binary_name = false;
		$this->previous_id = -1;
		$this->result = null;
		$this->errors = array();
		if ($revision > 0)
			$this->fetch("WHERE svn_version=?", array($revision));
	}

	private function fetch($where, $params = array())
	{
		$result = $this->db->Execute('SELECT * FROM builds ' . $where . ' LIMIT 1',$params);
		if ($result !== false && !$result->EOF())
		{
			$this->init($result->fields);
		} else {
			$this->reset();
		}
	}

	public function fetchLast()
	{
		$this->fetch('ORDER BY id DESC');
	}

	public function init($values)
	{
		foreach($values as $key => $value)
		{
			$this->$key = $value;
		}
		$this->time = $this->db->UnixTimeStamp($this->time);
		$this->result = null;
		$this->errors = array();
	}

	public function reset()
	{
		$this->init(array('id' 			=> -1,
						  'svn_version'	=> -1,
					  	  'time'		=> $this->db->DBTimeStamp(0),
						  'status'		=> -1,
						  'error_msg'	=> ""));
		$this->previous_id = -1;
	}

	public function exists()
	{
		return $this->id > -1;
	}

	public function getId()
	{
		return $this->id;
	}

	public function getStatus()
	{
		return $this->status;
	}

	public function getPreviousId()
	{
		if ($this->previous_id == -1)
		{
			$result = $this->db->Execute('SELECT MAX(id) as previous_id 
				FROM builds 
				WHERE id<?
				AND status=?', 
				array($this->id,
					  self::S_GOOD));
			$this->previous_id = $result->fields['previous_id'];
		}
		return $this->previous_id;
	}
	public function getLastWorkingId()
	{
		if ($this->status == self::S_GOOD)
		{
			return $this->id;
		}
		return $this->getPreviousId();
	}

	public function compile($revision)
	{
		$compiler_log = shell_exec('scons test 2>&1');
		$m =array();
		$this->error_msg = '';
		$this->status = self::S_GOOD;
		if (strpos($compiler_log, "`test' is up to date.") !== false)
		{
			return false;
		}
		if (preg_match_all('/^.*(error:|warning:|undefined reference|ld returned \d exit status).*$/mi',$compiler_log, $m,PREG_SET_ORDER))
		{

			foreach($m as $match)
			{
				$this->error_msg .= $match[0] . "\n";
			}
			if (strpos($this->error_msg,'error') !== false 
				|| strpos($this->error_msg,'ld returned'))
				$this->status = self::S_ERROR;
		}

		if(preg_match('/test to \.\/([a-zA-Z0-9_-]*)/',$compiler_log, $m))
		{
			$this->binary_name = $m[1];
		}

		$this->time = time();
		$this->svn_revision = $revision;

		$this->db->StartTrans();

		$this->insert();
		return $this->status == self::S_GOOD;
	}

	public function getTestName()
	{
		return $this->binary_name;
	}

	private function insert()
	{
		$result = $this->db->Execute('INSERT INTO builds 
			(svn_version, status, error_msg) 
			VALUES (?, ?, ?)',
				array($this->svn_revision,
				  	  $this->status,
					  $this->error_msg));
	
		$this->id = $this->db->Insert_ID();
	}

	public function insertNull()
	{
		if ($result = $this->db->Execute('SELECT id FROM builds WHERE id=?', array(0)))
		{
			if ($result->EOF())
			{
				$result = $this->db->Execute('INSERT INTO builds 
					(svn_version, status, error_msg) 
					VALUES (?, ?, ?)',
						array(0,
						  	  self::S_GOOD,
							  ''));
				$this->id = $this->db->Insert_ID();

				$this->db->Execute('UPDATE builds SET id=? WHERE id=?', array(0, $this->id));

			}
		}
	}

	private function checkChilds()
	{
		if (is_null($this->result))
		{
			$this->result = new TestResult($this->getLastWorkingId());
			$this->errors = TestError::getErrorsForBuild($this->getLastWorkingId());
		}
	}

	private function getErrorStatistics()
	{
		$ret = array();
		foreach($this->errors as $err)
		{
			$ret[] = $err->getStatistics();
		}
		return $ret;
	}

	public function getStatistics()
	{
		$this->checkChilds();

		$build_result = '';
		if ($this->status == self::S_GOOD)
		{
			$build_result = $this->result->getResult();
		} else {
			$build_result = nl2br("Compilation failed:\n" . $this->error_msg);
		}

		return 	array('build_result' 	=> $build_result,
					  'build_time' 		=> $this->time,
					  'build_error_msg'	=> $this->error_msg,
					  'build_svn_rev'	=> $this->svn_version,
					  'result_passed'	=> $this->result->getAssertionsPassed(),
					  'result_failed'	=> $this->result->getAssertionsFailed(),
				  	  'errors'			=> $this->getErrorStatistics());
	}
}
?>
