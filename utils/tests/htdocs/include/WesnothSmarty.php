<?php

global $smarty_dir;
require(/*$smarty_dir .*/ 'smarty/libs/Smarty.class.php');

class WesnothSmarty extends Smarty {
	function __construct($work_dir)
	{
		parent::__construct();
		$this->template_dir = $work_dir . '/templates';
		$this->compile_dir = $work_dir . '/templates_c';
		$this->cache_dir = $work_dir . '/cache';
		$this->config_dir = $work_dir . '/configs';
	}
}
?>
