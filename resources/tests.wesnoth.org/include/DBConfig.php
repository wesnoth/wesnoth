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


class DBConfig extends Config {
	private static $db_config_loaded = false;

	public function set($value)
	{
		global $db;
		if (isset(self::$configs[$this->name]))
		{
			$db->Execute('UPDATE configs SET value=? WHERE name=?', array($value, $this->name));
		} else {
			$db->Execute('INSERT INTO configs (name, value)
				VALUES (?,?)', array($this->name,$value));
		}
		self::$configs[$this->name] = $value;
	}

	private static function loadConfig()
	{
		if (self::$db_config_loaded)
			return;
		global $db;

		$results = $db->Execute('SELECT * FROM configs');

		self::$db_config_loaded = true;
		if ($results === false)
			return;

		while(!$results->EOF())
		{
			self::$configs[$results->fields['name']] = $results->fields['value'];
			$results->moveNext();
		}
	}

	public function get()
	{
		self::loadConfig();

		return self::getValue($this->name);
	}
}

