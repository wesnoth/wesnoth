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
$root_dir = './';

require_once($root_dir . '../include/settup.php');

$header = new Header('build_history');

$footer = new Footer('build_history');

$header->show();

$user_params = new ParameterValidator($_GET);

$smarty->assign(Build::GetVisibleBuilds($user_params));

$smarty->display('build_history.tpl');

$footer->show();
?>
