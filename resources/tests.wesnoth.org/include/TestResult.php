<?php
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

class TestResult {
	private $db;
	private $id;
	private $build_id;
	private $name;
	private $result;
	private $assertions_passed;
	private $assertions_failed;
	private $test_cases_passed;
	private $test_cases_failed;
	private $test_cases_skipped;
	private $test_cases_aborted;

	function __construct($data = null, Build $build = null)
	{
		global $db;
		$this->db = $db;
		if (!is_null($data))
		{
			if ($data instanceof SimpleXMLElement)
			{
				$this->build_id = $build->getId();
				$attr = $data->attributes();
				foreach($attr as $name => $value)
				{
					$this->$name = (string)$value;
					if (is_numeric($this->name))
						$this->$name = (int)$this->$name;
				}
			} else {
				$this->fetch('WHERE build_id = ?', array($data));
			}
		} else {
			$this->reset();
		}
	}

	private function fetch($where, $params = array())
	{
		$result = $this->db->Execute('SELECT * FROM test_results '
			. $where . ' LIMIT 1', $params);
		if (!$result->EOF())
		{
			$this->init($result->fields);
		} else {
			$this->reset();
		}
	}

	private static function multiFetch($where, $params = array())
	{
		global $db;
		$result = $db->Execute('SELECT * FROM test_results '
			. $where, $params);
		$ret = array();
		while (!$result->EOF())
		{
			$obj = new TestResult();
			$obj->init($result->fields);
			$ret[] = $obj;
			$result->moveNext();
		} 
		return $ret;
	}


	public static function fetchResultsForbuilds(&$builds)
	{
		$id_finder = array();
		$id_list = array();
		foreach($builds as $index => $build)
		{
			$id_list[] = $build->getId();
			$id_finder[$build->getId()] = $index;
		}
		if (empty($id_list))
			return;

		$results = self::multiFetch('WHERE build_id IN (?'.str_repeat(',?',count($id_list) - 1).')', $id_list);

		foreach($results as $result)
		{
			$builds[$id_finder[$result->getBuildId()]]->setResult($result);
			unset($id_finder[$result->getBuildId()]);
		}
		
		if (empty($id_finder))
			return;

		$empty_result = new TestResult();
		foreach($id_finder as $index)
		{
			$builds[$index]->setResult($empty_result);
		}
	}

	public function getBuildId()
	{
		return $this->build_id;
	}

	private function init($values)
	{
		foreach($values as $key => $value)
		{
			$this->$key = $value;
		}
	}

	private function reset()
	{
		$this->id = -1;
		$this->build_id = -1;
		$this->name = '';
		$this->result = 'notstarted';
		$this->assertions_passed = 0;
		$this->assertions_failed = 0;
		$this->test_cases_passed = 0;
		$this->test_cases_failed = 0;
		$this->test_cases_skipped = 0;
		$this->test_cases_aborted = 0;

	}

	public function updateDB()
	{
		if (!empty($this->result))
			$this->insert();
	}

	private function insert()
	{
		$this->db->Execute('INSERT INTO test_results 
			(build_id, result, assertions_passed,
			 assertions_failed, test_cases_passed,
		 	 test_cases_failed, test_cases_skipped,
		 	 test_cases_aborted)
			 VALUES(?,?,?,
				 ?,?,
			 	 ?,?,
			 	 ?)',
				 array($this->build_id, $this->result, $this->assertions_passed,
				 $this->assertions_failed, $this->test_cases_passed,
				 $this->test_cases_failed, $this->test_cases_skipped,
				 $this->test_cases_aborted));
	}

	public function getResult()
	{
		return $this->result;
	}

	public function getAssertionsPassed()
	{
		return $this->assertions_passed;
	}

	public function getAssertionsFailed()
	{
		return $this->assertions_failed;
	}
}
?>
