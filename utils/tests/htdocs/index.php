<?php
$root_dir = './';

require_once($root_dir . '../include/settup.php');

$header = new Header('index');

$footer = new Footer('index');

$header->show();

$build = new Build();
$build->fetchLast();

$smarty->assign($build->getStatistics());

$smarty->display('index.tpl');


$footer->show();

?>
