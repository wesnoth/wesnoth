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


global $smarty_dir;
require($smarty_dir . 'libs/Smarty.class.php');

class WesnothSmarty extends Smarty {
	function __construct($work_dir)
	{
		parent::__construct();
		$this->template_dir = $work_dir . '/templates';
		$this->compile_dir = $work_dir . '/templates_c';
		$this->cache_dir = $work_dir . '/cache';
		$this->config_dir = $work_dir . '/configs';
		$this->plugins_dir[] = $work_dir . '/plugins';
	}
}
?>
