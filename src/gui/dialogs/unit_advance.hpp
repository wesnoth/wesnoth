/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "units/ptr.hpp"

namespace gui2 {

class tunit_advance : public tdialog
{
	typedef std::vector<unit_const_ptr> unit_ptr_vector;
public:
	tunit_advance(const unit_ptr_vector& samples, size_t real);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);
	void post_show(twindow& window);

	void list_item_clicked(twindow& window);

	void show_help(twindow& window);

	const unit_ptr_vector& previews_;

	size_t selected_index_, last_real_advancement_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_RECRUIT_HPP_INCLUDED */
