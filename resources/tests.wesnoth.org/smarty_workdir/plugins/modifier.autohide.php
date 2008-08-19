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


function smarty_modifier_autohide($text, $min_length_to_hide, $split_from_space, $take_end = false)
{
	if (mb_strlen($text) > $min_length_to_hide)
	{
		$split_from_space = $split_from_space?"true":"false";
		$take_end = $take_end?"true":"false";
		return "<script type='text/javascript'>
			/*<![CDATA[*/
			autohide_store.push(new Autohide('$text', $min_length_to_hide, $split_from_space, $take_end));
			/*]]>*/
			</script>";
	} else {
		return str_replace("\\'","'",str_replace('\n',"\n",$text));
	}
}

?>
