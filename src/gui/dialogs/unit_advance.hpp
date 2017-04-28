/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_UNIT_ADVANCE_HPP_INCLUDED
#define GUI_DIALOGS_UNIT_ADVANCE_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "units/ptr.hpp"

namespace gui2
{
namespace dialogs
{

class unit_advance : public modal_dialog
{
	typedef std::vector<unit_const_ptr> unit_ptr_vector;
public:
	unit_advance(const unit_ptr_vector& samples, size_t real);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

	void list_item_clicked(window& window);

	void show_help(window& window);

	const unit_ptr_vector& previews_;

	size_t selected_index_, last_real_advancement_;
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_UNIT_ADVANCE_HPP_INCLUDED */
