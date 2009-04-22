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


class Config {
	protected $name;
	protected static $configs = array();
	private static $php_config_loaded = false;
	function __construct($name = null)
	{
		$this->name = $name;
	}

	// DBConfig will implement this but not yet used
	public function insertDefaults()
	{
	}
// set isn't supported as configs are stored in php file
	public function set($value)
	{
		trigger_error('Not implemented. Use DBConfig::set()',E_USER_WARNING);
	}

	public function get()
	{
		self::loadConfig();

		return self::getValue($this->name);
	}

	private static function loadConfig()
	{
		if (self::$php_config_loaded)
			return;
		global $root_dir;
		require_once($root_dir . '/../include/configuration.php');
		self::$configs =  array_merge(self::$configs, $config);
		self::$php_config_loaded = true;
	}

	protected static function getValue($name)
	{
		if (isset(self::$configs[$name]))
		{
			return self::$configs[$name];
		} else {
			trigger_error("No $name config option found. Did you forgot to update configuration.php?", E_USER_NOTICE);
			return "";
		}
	}

}
?>
