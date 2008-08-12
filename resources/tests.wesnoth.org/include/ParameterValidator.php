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

// posible class to set default values
//class ParameterLimit {
//}

class ParameterValidator {
	private $user_params;
	function __construct(array $user_params)
	{
		$this->user_params = $user_params;
	}

	function getInt($name, $default)
	{
		if (isset($this->user_params[$name])
			&& is_numeric($this->user_params[$name]))
		{
			return (int)$this->user_params[$name];
		} else {
			return $default;
		}
	}
}
?>
