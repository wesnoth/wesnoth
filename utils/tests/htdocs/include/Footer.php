<?php

class Footer
{
	private $file;
	private $smarty;
	function __construct($file)
	{
		global $smarty;
		$this->file = $file;
		$this->smarty = $smarty;
	}

	public function show()
	{
		$this->smarty->display('footer.tpl');
	}
}
?>
