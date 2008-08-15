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


class SVNUpdater {
	private $revision;
	function __construct()
	{
		$this->updateSVN();
	}

	public function getRevision()
	{
		return $this->revision;
	}

	private function updateSVN()
	{
		$success = false;
		$m = array();
		$tries = 3;
		while(!$success && $tries--)
		{
			$svnlog = shell_exec('svn up 2>&1');
			global $db;
			if ($db->debug)
				echo $svnlog;
			$success = preg_match('/At revision ([0-9]*)\./m', $svnlog, $m);
			if (!$success && $tries)
				sleep(60);
		}
		if ($success)
			$this->revision = (int)$m[1];
		else
			$this->revision = false;
	}
}

?>
