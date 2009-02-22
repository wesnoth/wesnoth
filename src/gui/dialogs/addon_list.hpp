/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_LIST_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

class config;

namespace gui2 {

class taddon_list 
	: public tdialog
{
public:
	taddon_list(const config& cfg)
		: cfg_(cfg)
	{}

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Config which contains the list with the campaigns. */
	const config& cfg_;
};

} // namespace gui2

#endif

