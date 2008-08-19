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
		$this->errors = null;
		if ($revision >= 0)
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

	private static function multiFetch($where, $params = array())
	{
		global $db;
		$res = array();
		$id_list = array();
		$result = $db->Execute('SELECT * FROM builds ' . $where ,$params);
		if ($result === false)
			return $res;
		while (!$result->EOF())
		{			
			$build = new Build();
			$build->init($result->fields);
			$id_list[] = $build->getId();
			$res[] = $build;
			$result->moveNext();
		} 
		return $res;
	}

	public function fetchLast()
	{
		$this->fetch('WHERE id=(SELECT MAX(id) FROM builds)');
	}

	public function fetchBuildById($id)
	{
		$this->fetch('WHERE id=?', array($id));
	}

	private static function fetchVisibleBuilds($page, $builds_per_page)
	{
		$ret = self::multiFetch('ORDER BY id DESC LIMIT ?,?', 
			array(($page-1)*$builds_per_page, $builds_per_page));

		TestResult::fetchResultsForBuilds($ret);
		return $ret;
	}

	private static function getNumberOfPages($builds_per_page)
	{
		global $db;
		$result = $db->Execute('SELECT COUNT(*) as number FROM builds');
		return ceil($result->fields['number']/$builds_per_page);
	}

	public function init($values)
	{
		foreach($values as $key => $value)
		{
			$this->$key = $value;
		}
		$this->time = $this->db->UnixTimeStamp($this->time);
		$this->result = null;
		$this->errors = null;
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
			if (!$result->EOF())
				$this->previous_id = $result->fields['previous_id'];
		}
		return $this->previous_id;
	}

	private function getNextAndPrevious()
	{
		$ret = array();
		$result = $this->db->Execute('SELECT MAX(id) as previous_id 
				FROM builds 
				WHERE id<?', 
				array($this->id));
		if (!$result->EOF())
			$ret['previous_id'] = $result->fields['previous_id'];
		else
			$ret['previous_id'] = 0;

		$result = $this->db->Execute('SELECT MIN(id) as next_id 
				FROM builds 
				WHERE id>?', 
				array($this->id));
		if (!$result->EOF())
			$ret['next_id'] = $result->fields['next_id'];
		else
			$ret['next_id'] = 0;


		return $ret;
	}
	public function getPreviousWorkingId()
	{
		if ($this->status == self::S_GOOD)
		{
			return $this->id;
		}
		return $this->getPreviousId();
	}


	public function compile($revision)
	{

		$cc = new Config('compile_command');
		$compile_command = $cc->get();

		$bn = new Config('binary_name');
		$this->binary_name = $bn->get();

		$compiler_log = shell_exec($compile_command);
		$m =array();
		$this->error_msg = '';
		$this->status = self::S_GOOD;

		// build/debug/editor2/
		$compiler_log = FilenameConverter::stripBuildDirs($compiler_log);
		if (preg_match_all('/^.*(error:|warning:|note:|undefined reference|ld returned \d exit status).*$/mi',$compiler_log, $m,PREG_SET_ORDER))
		{

			foreach($m as $match)
			{
				$this->error_msg .= $match[0] . "\n";
			}
			if (strpos($this->error_msg,'error') !== false 
				|| strpos($this->error_msg,'ld returned'))
					$this->status = self::S_ERROR;
			$this->error_msg = str_replace("'","\\'",$this->error_msg);
		}

		$this->time = time();
		$this->svn_revision = $revision;

		if ($this->status == self::S_GOOD)
			return true;
		$this->insert();
		return false;
	}

	public function getTestName()
	{
		return $this->binary_name;
	}

	public function insert()
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

	private function checkResult()
	{
		if (is_null($this->result))
		{
			$this->result = new TestResult($this->id);
		}
	}

	public function setResult(TestResult &$result)
	{
		$this->result = $result;
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

	public static function getVisibleBuilds(ParameterValidator $user_params)
	{
		$ret = array();
		$bpreviousp = new Config('history_builds_per_page');
		$builds_per_page = $bpreviousp->get(); 
		$nopip = new Config('history_number_of_pages_in_pagenation');
		$number_of_vissible_pages = $nopip->get();
		$page = $user_params->getInt('page', 1);
		$number_of_pages	= self::getNumberOfPages($builds_per_page);

		$pager = new Paginate('build_history.php?page=', $page, $number_of_pages, $number_of_vissible_pages);

		$ret['paginate'] = $pager->make();

		$ret['builds'] = array();
		$builds = self::fetchVisibleBuilds($ret['paginate']['page'], $builds_per_page);
		foreach($builds as $build)
		{
			$ret['builds'][] = $build->getBuildStats();
		}
		return $ret;
	}

	private function getBuildStats()
	{
		$this->checkResult();

		$build_result = '';
		if ($this->status == self::S_GOOD)
		{
			$build_result = "Build successed";
			if (!empty($this->error_msg))
				$build_result .= ":\n" . $this->error_msg;
			else
				$build_result .= ".";
				
		} else {
			$build_result = "Build failed:\n" . $this->error_msg;
		}
		$build_result = str_replace("\n"," \\n",$build_result);

	
		return array('result' 			=> $build_result,
					 'time' 			=> $this->time,
					 'style' 			=> ($this->status == self::S_GOOD?"passed":"failed"),
					 'id'				=> $this->id,
					 'result_style'		=> $this->result->getResult(),
					 'error_msg'		=> $this->error_msg,
					 'svn_rev'			=> $this->svn_version,
					 'result_passed'	=> $this->result->getAssertionsPassed(),
					 'result_failed'	=> $this->result->getAssertionsFailed());
	}

	public function getStatistics()
	{
		if (is_null($this->errors))
			$this->errors = TestError::getErrorsForBuild($this->getPreviousWorkingId());

		return 	array_merge(array('builds'	=> array($this->getBuildStats()),
			'errors'	=> $this->getErrorStatistics()),
			$this->getNextAndPrevious());
	}

	public function pruneOldBuilds()
	{
		$query = 'SELECT bcurrent.id as id
				  FROM builds bcurrent LEFT JOIN test_results as rcurrent ON bcurrent.id=rcurrent.build_id
								 LEFT JOIN test_errors as e1 ON bcurrent.id=e1.before_id
		  						 LEFT JOIN test_errors as e2 ON bcurrent.id=e2.last_id,
					   builds bnext LEFT JOIN test_results as rnext ON bnext.id=rnext.build_id,
					   builds bprevious LEFT JOIN test_results as rprevious ON bprevious.id=rprevious.build_id
				  WHERE bcurrent.time >= ? 
					   AND bcurrent.time <= ?
					   AND bcurrent.status = bnext.status 
					   AND bcurrent.status = bprevious.status 
				  	   AND bcurrent.error_msg = bnext.error_msg 
				  	   AND bcurrent.error_msg = bprevious.error_msg 
					   AND (bnext.id=(SELECT MIN(id) FROM builds sb WHERE sb.id>bcurrent.id)
				   			AND bprevious.id=(SELECT MAX(id) FROM builds sb WHERE sb.id<bcurrent.id))
					   AND (e1.id IS NULL AND e2.id IS NULL AND bcurrent.id IS NOT NULL)
					   AND ((rcurrent.id IS NULL AND rnext.id IS NULL AND rprevious.id IS NULL) 
					   		OR (rcurrent.result = rnext.result 
					   			AND rcurrent.result = rprevious.result 
								AND rcurrent.assertions_passed=rnext.assertions_passed 
								AND rcurrent.assertions_failed=rnext.assertions_failed
								AND rcurrent.assertions_passed=rprevious.assertions_passed 
								AND rcurrent.assertions_failed=rprevious.assertions_failed))';

		$bpa = new Config('build_prune_age');
		$max_age = $bpa->get(); 
		$bppt = new DBConfig('build_previous_prune_time');
		$start_time = $this->db->UnixTimeStamp($bppt->get());
		if (empty($tart_time))
			$start_time = 0;
		$end_time = time()-$max_age;
		$result = $this->db->Execute($query, array($this->db->DBTimeStamp($start_time),
												   $this->db->DBTimeStamp($end_time)));

		if (!$result || $result->EOF())
			return;

		while(!$result->EOF())
		{
			$ids[] = $result->fields['id'];
			$result->moveNext();
		}

		$this->db->StartTrans();
		$query = 'DELETE FROM test_results WHERE build_id IN (?' . str_repeat(',?',count($ids)-1) . ')';
		$result = $this->db->Execute($query, $ids);
		if ($result === false)
			$this->db->FailTrans();

		if(!$this->db->hasFailedTrans())
		{

			$query = 'DELETE FROM builds WHERE id IN (?' . str_repeat(',?',count($ids)-1) . ')';
			$result = $this->db->Execute($query, $ids);
			if ($result === false)
				$this->db->FailTrans();
		}

		if(!$this->db->hasFailedTrans())
			$bppt->set($this->db->DBTimeStamp($end_time));

		$success = $this->db->hasFailedTrans();
		$this->db->CompleteTrans();

		$optimize = new DBConfig('build_optimize_table_time');
		// Do DB optimizaion and analyze once in a day
		if ($success
			&& time() - 24*3600 > $this->db->UnixTimeStamp($optimize->get()))
		{
			// optimize tables
			$this->db->Execute('OPTIMIZE TABLE builds, test_results');
			$this->db->Execute('ANALYZE TABLE builds, test_results');
			$optimize->set($this->db->DBTimeStamp(time()));
		}
	}
}
?>
