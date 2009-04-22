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


class FilenameConverter {
	public static function stripBuildDirs($text)
	{
		return preg_replace('/^(build\/[^\/]*\/)?(.*[^\/]+:[0-9]+:.*|.*[^\/]+)$/m','$2',$text);
	}
}
?>
