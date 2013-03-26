<?php
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   Adapted for git and name changed in 2013 at git conversion time.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


class RepoUpdater {
	private $revision;
	function __construct()
	{
		$this->updateRepo();
	}

	public function getRevision()
	{
		return $this->revision;
	}

	private function updateRepo()
	{
		$success = false;
		$m = array();
		$tries = 3;
		while(!$success && $tries--)
		{
			shell_exec('git checkout 2>&1');
			$gitversion = shell_exec('git rev-parse HEAD");
			global $db;
			if ($db->debug)
				echo $gitversion;
			if (!$success && $tries)
				sleep(60);
		}
		if ($success)
			$this->revision = $gitversion;
		else
			$this->revision = false;
	}
}

?>
