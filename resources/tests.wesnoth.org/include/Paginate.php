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


class Paginate {
	private $link;
	private $page;
	private $number_of_pages;
	private $number_of_visible_pages;

	public function __construct($link, $page, $number_of_pages, $number_of_visible_pages)
	{
		$this->link = $link;
		$this->page = $page;
		$this->number_of_pages = $number_of_pages;
		$this->number_of_visible_pages = $number_of_visible_pages;;
	}

	public function make()
	{
		
		if ($this->page < 0)
			$this->page = 1;
		if ($this->page > $this->number_of_pages)
			$this->page = $this->number_of_pages;
		

		$this->number_of_visible_pages = 5;
		// Add one to visible if we are in begin or end
		$visible_add = 0;
		// Selected page should be in middle if possible
		// Prefer to select page that has higher number than current
		$start = $this->page - floor(($this->number_of_visible_pages-1)/2);
		if ($start <= 1)
		{
			$start = 1;
			$visible_add = 1;
		}
		$last = $start + $this->number_of_visible_pages + $visible_add;
		// add one because smarty select loops untill the last but doesn't include it inside loop
		if ($last >= $this->number_of_pages +1)
		{
			$last = $this->number_of_pages + 1;
			$start = $last - $this->number_of_visible_pages - 1;
			if ($start < 1)
				$start = 1;
		}
		$ret = array();
		$ret['link'] = $this->link;
		$ret['page'] = $this->page;
		$ret['first_page'] = $start; 
		$ret['last_page'] = $last; 
		$ret['number_of_pages'] = $this->number_of_pages;
		$ret['visible'] = $last - $start; 

		return $ret;
	
	}
}
?>
