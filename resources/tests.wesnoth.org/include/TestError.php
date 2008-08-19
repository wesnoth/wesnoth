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



class TestError {
	private $db;
	private $id;
	private $before_id;
	private $last_id;
	private $error_type;
	private $file;
	private $line;
	private $error_msg;

	private $start_version;
	private $end_version;

	function __construct($name = null, $data = null, Build $build = null)
	{
		global $db;
		$this->db = $db;
		$this->start_version = -1;
		$this->end_version = -1;
		if (!is_null($name))
		{
			$this->error_type = $name;
			$this->file = FilenameConverter::stripBuildDirs((string)$data->attributes()->file);
			$this->line = (string)$data->attributes()->line;
			$this->error_msg = str_replace("'","\\'",str_replace("\n","\\n",(string)$data[0]));
			$result = $this->db->Execute('SELECT t.id as id, before_id, last_id FROM test_errors t, builds b
					WHERE t.error_type=? 
					AND t.file=?
					AND t.error_msg=?
					AND t.last_id=b.id
					AND b.time > ?
					LIMIT 1',
					array($this->error_type,
						  $this->file,
						  $this->error_msg,
						  $this->db->DBTimeStamp(time() - 24*60*60)
						));
			if (!$result->EOF())
			{
				$this->id = $result->fields['id'];
				$this->before_id = $result->fields['before_id'];
				$this->last_id = $result->fields['last_id'];
			}
		} else {
			$this->reset();
		}
	}

	private function fetch($where, $params =array(), $fields = '*')
	{
		$result = $this->db->Execute("SELECT $fields FROM test_errors $where LIMIT 1",$params);
		if (!$result->EOF())
		{
			$this->init($result->fields);
		}
	}

	private function init($values)
	{
		$this->start_version = -1;
		$this->end_version = -1;
		foreach($values as $key => $value)
		{
			$this->$key = $value;
		}
	}

	private function reset()
	{
		$this->id = -1;
		$this->before_id = -1;
		$this->last_id = -1;
		$this->error_type = "";
		$this->file = "";
		$this->line = 0;
		$this->error_msg = "";

		$this->start_version -1;
		$this->end_version -1;

	}

	public static function getErrorsForBuild($id)
	{
		global $db;
		$ret = array();
		$result = $db->Execute('SELECT * FROM test_errors
			WHERE before_id<? AND last_id >=?',
			array($id,$id));
		while(!$result->EOF())
		{
			$error = new TestError();
			$error->init($result->fields);
			$ret[] = $error;
			$result->moveNext();
		}
		return $ret;
	}

	public function updateDB(Build $build)
	{
		if (is_null($this->id))
		{
			$this->before_id = $build->getPreviousId();
			$this->last_id = $build->getId();
			$this->insert();
		} else {
			$this->db->Execute('UPDATE test_errors SET last_id=?, line=? WHERE id=?', array($build->getid(), $this->line, $this->id));
		}
	}

	private function insert()
	{
		$this->db->Execute('INSERT INTO test_errors (before_id, last_id, error_type, file, line, error_msg)
			VALUES (?, ?, ?, ?, ?, ?)',
				array($this->before_id,$this->last_id,$this->error_type,$this->file,$this->line,$this->error_msg));
		$this->id = $this->db->Insert_ID();
	}

	public function getStatistics()
	{
		return array('error_type' 		=> $this->error_type,
					 'start_version'	=> $this->getStartVersion(),
					 'end_version'		=> $this->getEndVersion(),
					 'file'				=> $this->file,
					 'line'				=> $this->line,
					 'error_msg'		=> $this->error_msg);
	}

	public function getStartVersion()
	{
		if ($this->start_version == -1)
		{
			$result = $this->db->Execute('SELECT svn_version as start_version FROM builds WHERE id=?',array($this->before_id));
			$this->start_version = $result->fields['start_version'];
		}
		return $this->start_version;
	}

	public function getEndVersion()
	{
		if ($this->end_version == -1)
		{
			// might need optimization
			$result = $this->db->Execute('SELECT MIN(svn_version) as end_version FROM builds WHERE id>? AND status=?',array($this->before_id, Build::S_GOOD));
			$this->end_version = $result->fields['end_version'];
		}
		return $this->end_version;
	}

}
?>
