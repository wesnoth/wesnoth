/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_SIMPLE_ITEM_SELECTOR_HPP_INCLUDED
#define GUI_DIALOGS_SIMPLE_ITEM_SELECTOR_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

#include <vector>

namespace gui2 {

class tsimple_item_selector : public tdialog
{
public:
	typedef std::vector< std::string > list_type;
	typedef std::string                item_type;

	tsimple_item_selector(
		const std::string& title,
		const std::string& message,
		list_type const& items,
		bool title_uses_markup = false,
		bool message_uses_markup = false);

	int selected_index() const         { return index_;  }
	void set_selected_index(int index) { index_ = index; }

private:
	int index_;

	std::string title_, msg_;
	bool markup_title_, markup_msg_;
	list_type items_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

}

#endif /* ! GUI_DIALOGS_SIMPLE_ITEM_SELECTOR_HPP_INCLUDED */
