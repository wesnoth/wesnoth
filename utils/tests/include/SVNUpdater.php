<?php

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
			$success = preg_match('/At revision ([0-9]*)\./', $svnlog, $m);
			if (!$success)
				sleep(15);
		}
		if ($success)
			$this->revision = (int)$m[1];
		else
			$this->revision = false;
	}
}

?>
